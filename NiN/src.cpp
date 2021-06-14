#include "includes/header.h"

void image_copy(i_dt image[Maps1][Side1][Side1], i_dt image_in[Sources]) {
#pragma HLS INLINE off
    IMAGE_COPY: for (int map = 0; map < Maps1; map++) {
    	for (int i = 0; i < Side1; i++) {
    		for (int j = 0; j < Side1; j++) {
#pragma HLS PIPELINE
    			image[map][i][j] = image_in[map*Side1*Side1+i*Side1+j];
    		}
    	}
    }
}

void reset_sc(sc_dt spike_count[Output]) {
#pragma HLS INLINE off
	OUT: for (int i=0; i<Output; i++) {
#pragma HLS PIPELINE
		spike_count[i] = 0;
	}
}

void reset_spike_pointer(sr_dt spike_input[MapsMax]) {
#pragma HLS INLINE off
	for (int map = 0; map < MapsMax; map++) {
#pragma HLS PIPELINE
		spike_input[map] = 0;
	}
}


void load_kernel(w_dt kernel[MapsMax][MapsMax][KSideMax][KSideMax], w_dt k_off[MapsMax*MapsMax*KSideMax*KSideMax], int OutMaps, int InMaps) {
#pragma HLS INLINE off
    for (int i=0; i<InMaps; i++)
    	for(int j=0; j<OutMaps; j++)
    		for(int k=0; k<KSide1; k++)
    			for(int l=0; l<KSide1; l++)
#pragma HLS PIPELINE
    				kernel[i][j][k][l] = k_off[(i*OutMaps*KSide1*KSide1)+(j*KSide1*KSide1)+(k*KSide1)+l];
}
void load_weight(w_dt kernel[MapsMax][MapsMax], w_dt k_off[MapsMax*MapsMax*KSideMax*KSideMax], int OutMaps, int InMaps) {
#pragma HLS INLINE off
    for (int i=0; i<InMaps; i++)
    	for(int j=0; j<OutMaps; j++)
#pragma HLS PIPELINE
    			kernel[i][j] = k_off[(i*OutMaps*KSide2*KSide2)+(j*KSide2*KSide2)];
}

#ifdef __SDSCC__
void load_kernel_hw(w_dt kernel[MapsMax][KSideMax][KSideMax], IW k_off[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], int OutMaps, int InMaps, int imap) {
#pragma HLS INLINE off
   for(int j=0; j<OutMaps/WIDTH_FACTOR; j++) {
   	for(int k=0; k<KSideMax; k++) {
   		for(int l=0; l<KSideMax; l++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=kernel inter false
   				IW temp = k_off[(imap*(OutMaps/WIDTH_FACTOR)*KSide1*KSide1)+(j*KSide1*KSide1)+(k*KSide1)+l];
   		   		for(int x=0; x<WIDTH_FACTOR; x++) {
#pragma HLS UNROLL
   					unsigned int range_idx = x * WWIDTH;
   	    			WW tmp_int = temp.range(range_idx + WWIDTH - 1, range_idx);
   	    			w_dt tmp = *((w_dt*)(&tmp_int));
   	    			kernel[j*WIDTH_FACTOR+x][k][l] = tmp;
   			}
   		}
   	}
   }
}

void load_weight_hw(w_dt kernel[MapsMax][MapsMax], IW k_off[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], int OutMaps, int InMaps) {
#pragma HLS INLINE off
   for (int i=0; i<InMaps; i++) {
   	for(int j=0; j<OutMaps/WIDTH_FACTOR; j++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=kernel inter false
   		IW temp = k_off[(i*(OutMaps/WIDTH_FACTOR)*KSide2*KSide2)+(j*KSide2*KSide2)];
  		   	for(int x=0; x<WIDTH_FACTOR; x++) {
#pragma HLS UNROLL
  				unsigned int range_idx = x * WWIDTH;
  				WW tmp_int = temp.range(range_idx + WWIDTH - 1, range_idx);
  	    		w_dt tmp = *((w_dt*)(&tmp_int));
  	    		kernel[i][j*WIDTH_FACTOR+x] = tmp;
			}
   	}
   }
}
#endif



void poisson_pe(i_dt image[Side1][Side1], int intensity, sr_dt spike_reg[MaxNeurons],
		sc_dt spike_cnt[MaxNeurons], sr_dt *spike_input, w_dt sf) {
#pragma HLS INLINE off

	bool flag = false;
	int temp_reg, temp_cnt;
    static unsigned int lfsr = 123456789 & 0x7fffffff;
    unsigned int bit;
    unsigned int random_int;

	L1: for (int i = 0; i < Side1; i++) {
		L2: for (int j = 0; j < Side1; j++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=spike_reg inter false
#pragma HLS DEPENDENCE variable=spike_cnt inter false
			flag = false;
			temp_reg = 0;
			temp_cnt = 0;
			L3: for (int s = 0;  s < Sims; s++) {
#pragma HLS UNROLL
				bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
				lfsr = (lfsr >> 1) | (bit << 31);
				random_int = lfsr & 0x7fffffff;
				if ((image[i][j] * intensity) > (random_int)) {
					flag = true;
					temp_reg = i*Side1+j;
					temp_cnt += 1;
				}
			}
			if (flag) {
				spike_reg[*spike_input] = temp_reg;
				spike_cnt[*spike_input] = temp_cnt*sf;
				(*spike_input)++;
			}
		}
	}
}

void poisson(i_dt image[Maps1][Side1][Side1], int intensity, sr_dt spike_reg[MapsMax][MaxNeurons],
		sc_dt spike_cnt[MapsMax][MaxNeurons], sr_dt spike_input[MapsMax], w_dt sf) {
#pragma HLS INLINE off

    L1: for (int map = 0; map < Maps1; map++) {
#pragma HLS UNROLL factor=IMaps
    	poisson_pe(image[map], intensity, spike_reg[map],
    			spike_cnt[map], &spike_input[map], sf);
    }
}

void encoding_pe (int map, q_dt vm[SideMax][SideMax], sr_dt local_spike_reg[MaxNeurons], sc_dt local_spike_cnt[MaxNeurons],
		sr_dt *local_pointer, int OutSide, w_dt vth) {
#pragma HLS INLINE off
	N2: for (int y = 0; y < OutSide; y++) {
		N3: for (int x = 0; x < OutSide; x++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm inter false
			if (vm[y][x] >= vth){
				local_spike_reg[*local_pointer] = y*OutSide + x;
				local_spike_cnt[*local_pointer] = vm[y][x]/vth;
				(*local_pointer)++;
				vm[y][x] = 0;
			}
			else {
				vm[y][x] = 0;
			}
		}
    }
}


void neuron_conv(q_dt vm[MapsMax][SideMax][SideMax],
		sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons], sr_dt spike[MapsMax],
		int OutSide, int OutMaps, w_dt vth) {
#pragma HLS INLINE off

	N1: for (int map = 0; map < OutMaps; map++) {
#pragma HLS UNROLL factor=EMaps
		encoding_pe(map, vm[map], spike_reg[map], spike_cnt[map],
				&spike[map], OutSide, vth);
    }
}



void neuron_output(q2_dt vm_fc[MaxFc], sc_dt spike_count[Output], int *dom, w_dt vth) {
#pragma HLS INLINE off
	NEURONOUT: for (int i=0; i < Output; i++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm_fc inter false
		if (vm_fc[i] >= vth) {
			spike_count[i] = vm_fc[i]/vth;
			vm_fc[i] = 0;
		}
		else {
			vm_fc[i] = 0;
		}
		if (spike_count[i] > spike_count[*dom]) {
			*dom = i;
		}
	}
}


void conv_pe_1(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input, sr_dt spike_reg[MaxNeurons],
		sc_dt spike_cnt[MaxNeurons], w_dt kernel[MapsMax][KSideMax][KSideMax], int Side, int OutSide, int OutMaps, int TMaps, w_dt sf) {
#pragma HLS INLINE OFF
    int pre_x, pre_y;
    int oy, ox;
    sc_dt temp_cnt;
    ca1: for (int map = 0; map < OutMaps/TMaps; map++){
    	ca2: for (int spiker = 0; spiker < spike_input; spiker++){
#pragma HLS loop_tripcount min=40 max=40
    		ca3: for (int ky = 0; ky < KSide1; ky++) {
    			ca4: for (int kx = 0; kx < KSide1; kx++) {
				#pragma HLS PIPELINE
				#pragma HLS DEPENDENCE variable=vm inter false
    				pre_y = spike_reg[spiker] / Side;
    				pre_x = spike_reg[spiker] % Side;
    				oy = (pre_y + PaddingC1 - ky);
    				ox = (pre_x + PaddingC1 - kx);
    				temp_cnt = spike_cnt[spiker]*sf;
    				ca5: for (int u = 0; u < Maps; u++) {
					#pragma HLS UNROLL
    					if ((ox >= 0) & (ox < OutSide) & (oy >= 0) & (oy < OutSide) & (u<OutMaps)) {
						vm[map*Maps+u][oy][ox] += (kernel[map*Maps+u][ky][kx]*temp_cnt);
    					}
    				}
    			}
    		}
    	}
    }
}

#ifndef __SDSCC__
void conv_spike_aggregation_1(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons],
		w_dt kernel[MapsMax][MapsMax][KSideMax][KSideMax], int Side, int InMaps, int OutSide, int OutMaps, w_dt sf) {
#pragma HLS INLINE off
	int TMaps = Maps;
	if (OutMaps < Maps) {
		TMaps = OutMaps;
	}

    cs1: for (int imap = 0; imap < InMaps; imap++) {
    		conv_pe_1(vm, spike_input[imap], spike_reg[imap],
					spike_cnt[imap], kernel[imap], Side, OutSide, OutMaps, TMaps, sf);
    }
}
#else
void conv_spike_aggregation_1(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons],
		w_dt kernel[MapsMax][KSideMax][KSideMax], IW k_off[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], int Side, int InMaps, int OutSide, int OutMaps, w_dt sf) {
#pragma HLS INLINE off
	int TMaps = Maps;
	if (OutMaps < Maps) {
		TMaps = OutMaps;
	}

   cs1: for (int imap = 0; imap < InMaps; imap++) {
   	load_kernel_hw(kernel, k_off, OutMaps, InMaps, imap);
   	conv_pe_1(vm, spike_input[imap], spike_reg[imap],
				spike_cnt[imap], kernel, Side, OutSide, OutMaps, TMaps, sf);
   }
}

void conv_spike_aggregation_pf(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons],
		w_dt kernel[MapsMax][MapsMax][KSideMax][KSideMax], int Side, int InMaps, int OutSide, int OutMaps, w_dt sf) {
#pragma HLS INLINE off
	int TMaps = Maps;
	if (OutMaps < Maps) {
		TMaps = OutMaps;
	}

   cs1: for (int imap = 0; imap < InMaps; imap++) {
   	conv_pe_1(vm, spike_input[imap], spike_reg[imap],
				spike_cnt[imap], kernel[imap], Side, OutSide, OutMaps, TMaps, sf);
   }
}
void load_kernel_pf(w_dt kernel[MapsMax][MapsMax][KSideMax][KSideMax], IW k_off[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], int OutMaps, int InMaps) {
#pragma HLS INLINE off
   for (int i=0; i<InMaps; i++) {
   	for(int j=0; j<OutMaps/WIDTH_FACTOR; j++) {
   		for(int k=0; k<KSide1; k++) {
   			for(int l=0; l<KSide1; l++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=kernel inter false
   				IW temp = k_off[(i*(OutMaps/WIDTH_FACTOR)*KSide1*KSide1)+(j*KSide1*KSide1)+(k*KSide1)+l];
   		   		for(int x=0; x<WIDTH_FACTOR; x++) {
#pragma HLS UNROLL
   					unsigned int range_idx = x * WWIDTH;
   	    			WW tmp_int = temp.range(range_idx + WWIDTH - 1, range_idx);
   	    			w_dt tmp = *((w_dt*)(&tmp_int));
   	    			kernel[i][j*WIDTH_FACTOR+x][k][l] = tmp;
   				}
   			}
   		}
   	}
   }
}
#endif



void conv_pe_2(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input, sr_dt spike_reg[MaxNeurons],
		sc_dt spike_cnt[MaxNeurons], w_dt kernel[MapsMax], int Side, int OutSide, int OutMaps, int TMaps, w_dt sf) {
#pragma HLS INLINE OFF
    int pre_x, pre_y;
    int oy, ox;
    sc_dt temp_cnt;

    ca1: for (int map = 0; map < OutMaps/TMaps; map++){
    	ca2: for (int spiker = 0; spiker < spike_input; spiker++){
#pragma HLS loop_tripcount min=40 max=40
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm inter false
    		pre_y = spike_reg[spiker] / Side;
    		pre_x = spike_reg[spiker] % Side;
    		temp_cnt = spike_cnt[spiker]*sf;
    		ca3: for (int u = 0; u < CMaps; u++) {
#pragma HLS UNROLL
    			if ((pre_x >= 0) & (pre_x < OutSide) & (pre_y >= 0) & (pre_y < OutSide) & (u<OutMaps)) {
    				vm[map*Maps+u][pre_y][pre_x] += (kernel[map*Maps+u]*temp_cnt);
    			}
    		}
    	}
	}
}

void conv_spike_aggregation_2(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons],
		sc_dt spike_cnt[MapsMax][MaxNeurons], w_dt kernel[MapsMax][MapsMax], int Side, int InMaps, int OutSide, int OutMaps, w_dt sf) {
#pragma HLS INLINE off

	int TMaps = CMaps;
	if (OutMaps < CMaps) {
		TMaps = OutMaps;
	}

    cs1: for (int imap = 0; imap < InMaps; imap++) {
    	conv_pe_2(vm, spike_input[imap], spike_reg[imap],
			spike_cnt[imap], kernel[imap], Side, OutSide, OutMaps, TMaps, sf);
    }
}


// void pool_pe(q_dt vm[SideMax][SideMax], sr_dt *spiked, sr_dt spike_reg[MaxNeurons], sc_dt spike_cnt[MaxNeurons],
		// int Side, int OutSide, w_dt sf) {
// #pragma HLS INLINE off
	// int pre_x, pre_y;
   // int oy, ox, kky, kkx;
   // sc_dt temp_cnt;
	// ps_compute: for (int spiker = 0; spiker < *spiked; spiker++){
// #pragma HLS loop_tripcount min=40 max=40
		// for (int ky=0; ky<KSide3/StrideP; ky++) {
			// for (int kx=0; kx<KSide3/StrideP; kx++) {
// #pragma HLS PIPELINE
			// pre_y = spike_reg[spiker] / Side;
			// pre_x = spike_reg[spiker] % Side;
			// kky = pre_y % StrideP;
			// kkx = pre_x % StrideP;
			// temp_cnt = spike_cnt[spiker]*sf;
			// oy = ((pre_y + PaddingP - kky) / StrideP) + (ky*StrideP);
			// ox = ((pre_x + PaddingP - kkx) / StrideP) + (kx*StrideP);
			// if ((ox >= 0) & (ox < OutSide) & (oy >= 0) & (oy < OutSide))
				// vm[oy][ox] += q_dt(scale)*temp_cnt;
			// }
		// }
	// }
// }

void pool_pe(q_dt vm[SideMax][SideMax], sr_dt *spiked, sr_dt spike_reg[MaxNeurons], sc_dt spike_cnt[MaxNeurons],
		int Side, int OutSide, w_dt sf) {
#pragma HLS INLINE off
	int pre_x, pre_y;
    int oy, ox, kky, kkx;
    sc_dt temp_cnt;
	ps_compute: for (int spiker = 0; spiker < *spiked; spiker++){
#pragma HLS loop_tripcount min=40 max=40
		pre_y = spike_reg[spiker] / Side;
		pre_x = spike_reg[spiker] % Side;
		for (int ky=pre_y % StrideP; ky<KSide3; ky+=StrideP) {
			for (int kx=pre_x % StrideP; kx<KSide3; kx+=StrideP) {
#pragma HLS PIPELINE
				temp_cnt = spike_cnt[spiker]*sf;
				oy = (pre_y + PaddingP - ky) / StrideP;
				ox = (pre_x + PaddingP - kx) / StrideP;
				if ((ox >= 0) & (ox < OutSide) & (oy >= 0) & (oy < OutSide))
					vm[oy][ox] += q_dt(scale)*temp_cnt;
			}
		}
	}
}


void pool_spike_aggregation(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons],
		int Side, int InMaps, int OutSide, w_dt sf){
#pragma HLS INLINE off

	ps_1: for (int imap = 0; imap < InMaps; imap++) {
#pragma HLS UNROLL factor=PMaps
		pool_pe(vm[imap], &spike_input[imap], spike_reg[imap], spike_cnt[imap], Side, OutSide, sf);
	}
}

void global_pool_pe(q2_dt *vm, sr_dt *spiked, sr_dt spike_reg[MaxNeurons], sc_dt spike_cnt[MaxNeurons], w_dt sf) {
#pragma HLS INLINE off
	int pre_x, pre_y;
    int oy, ox, ky, kx;
    sc_dt temp_cnt;
	ps_compute: for (int spiker = 0; spiker < *spiked; spiker++){
#pragma HLS PIPELINE
		temp_cnt = spike_cnt[spiker]*sf;
		*vm += q2_dt(scale_2)*temp_cnt;
	}
}

void global_pool_spike_aggregation(q2_dt vm_fc[MaxFc], sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons],
		sr_dt spike_input[MapsMax], int InMaps, w_dt sf) {
#pragma HLS INLINE off

	for (int imap = 0; imap < InMaps; imap++) {
#pragma HLS UNROLL factor=PMaps
		global_pool_pe(&vm_fc[imap], &spike_input[imap], spike_reg[imap], spike_cnt[imap], sf);
	}
}


#ifndef __SDSCC__
void max_measurement(double *spike_average, int *spike_max, sc_dt *spike_cnt_max, sc_dt spike_cnt[MapsMax][MaxNeurons], int spike_input[MapsMax], int LMaps) {
	for (int i=0; i<LMaps; i++) {
		if (*spike_max < spike_input[i]) {
			*spike_max = spike_input[i];
		}
		*spike_average = *spike_average +spike_input[i];
		for (int j=0; j<spike_input[i]; j++) {
			if (*spike_cnt_max < spike_cnt[i][j]) {
				*spike_cnt_max = spike_cnt[i][j];
			}
		}
	}
}

void max_measurement_2(sc_dt *spike_cnt_max, sc_dt spike_count[Output]) {
	for (int i=0; i<Output; i++) {
		if (*spike_cnt_max < spike_count[i]) {
			*spike_cnt_max = spike_count[i];
		}
	}
}

void print_vm(q_dt vm_conv[MapsMax][SideMax][SideMax], int PSide, int PMaps) {
	cout << endl;
	for (int m=0; m<PMaps; m++) {
		cout << vm_conv[m][0][0] << ", " ;
	}
	cout << endl;
}

void print_vm_check(q_dt vm_conv[MapsMax][SideMax][SideMax], int PSide, int PMaps) {
	cout << endl;
	for (int x=0; x<5; x++) {
		cout << endl;
		cout << " {" ;
		for (int y=0; y<5; y++) {
			cout << vm_conv[0][x][y] << ", " ;
		}
		cout << " }, " <<endl;
	}
	cout << endl;
}

void print_encoding(q_dt spike_cnt[MapsMax][MaxNeurons], int PSide, int PMaps) {
	cout << endl;
	for (int m=0; m<PMaps; m++) {
		cout << spike_cnt[m][0] << ", " ;
	}
	cout << endl;
}

void print_encoding_check(q_dt spike_cnt[MapsMax][MaxNeurons], int PSide, int PMaps) {
	cout << endl;
	for (int x=0; x<5; x++) {
		cout << endl;
		cout << " {" ;
		for (int y=0; y<5; y++) {
			cout << spike_cnt[0][x*PSide+y] << ", " ;
		}
		cout << " }, " <<endl;
	}
	cout << endl;
}



#endif

#ifndef __SDSCC__
void network(i_dt image_in[Sources], int intensity,
		        int *dom, w_dt k_1[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_2[MapsMax*MapsMax*KSideMax*KSideMax], w_dt k_3[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_4[MapsMax*MapsMax*KSideMax*KSideMax], w_dt k_5[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_6[MapsMax*MapsMax*KSideMax*KSideMax], w_dt k_7[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_8[MapsMax*MapsMax*KSideMax*KSideMax], w_dt k_9[MapsMax*MapsMax*KSideMax*KSideMax],
				bool flag, sc_dt spike_cnt_max[NumLayers], int vm_max[NumLayers],
				double spike_average[NumLayers], int spike_max[NumLayers])
#else
#pragma SDS data zero_copy(image_in, k_1, k_2, k_3, k_4, k_5, k_6, k_7, k_8, k_9)
#pragma SDS data mem_attribute(image_in:PHYSICAL_CONTIGUOUS|NON_CACHEABLE, k_1:PHYSICAL_CONTIGUOUS|NON_CACHEABLE, k_2:PHYSICAL_CONTIGUOUS|NON_CACHEABLE, w_1:PHYSICAL_CONTIGUOUS|NON_CACHEABLE, w_2:PHYSICAL_CONTIGUOUS|NON_CACHEABLE)
void network(i_dt image_in[Sources], int intensity,
	        int *dom, IW k_1[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW k_2[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			IW k_3[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW k_4[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			IW k_5[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW k_6[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			IW k_7[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW k_8[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			w_dt k_9[MapsMax*MapsMax*KSideMax*KSideMax], bool flag)
#endif
{
#ifndef __SDSCC__
    static w_dt kernel_1[NumKernels1][MapsMax][MapsMax][KSideMax][KSideMax];
#pragma HLS ARRAY_PARTITION variable=kernel_1 cyclic factor=CIMaps dim=1
#pragma HLS ARRAY_PARTITION variable=kernel_1 cyclic factor=Maps dim=2

    static w_dt kernel_2[NumKernels2][MapsMax][MapsMax];
#pragma HLS ARRAY_PARTITION variable=kernel_2 cyclic factor=CIMaps dim=1
#pragma HLS ARRAY_PARTITION variable=kernel_2 cyclic factor=Maps dim=2

#else

    static w_dt kernel_1a[MapsMax][MapsMax][KSideMax][KSideMax];
#pragma HLS ARRAY_PARTITION variable=kernel_1a cyclic factor=Maps dim=2

    static w_dt kernel_1b[MapsMax][MapsMax][KSideMax][KSideMax];
#pragma HLS ARRAY_PARTITION variable=kernel_1b cyclic factor=Maps dim=2

    static w_dt kernel_1c[MapsMax][MapsMax][KSideMax][KSideMax];
#pragma HLS ARRAY_PARTITION variable=kernel_1c cyclic factor=Maps dim=2


//    static w_dt kernel_1[MapsMax][KSideMax][KSideMax];
//#pragma HLS ARRAY_PARTITION variable=kernel_1 cyclic factor=Maps dim=1

//    static w_dt kernel_2[MapsMax][MapsMax];
//#pragma HLS ARRAY_PARTITION variable=kernel_2 cyclic factor=CMaps dim=2

    static w_dt kernel_2a[MapsMax][MapsMax];
#pragma HLS ARRAY_PARTITION variable=kernel_2a cyclic factor=CMaps dim=2
    static w_dt kernel_2b[MapsMax][MapsMax];
#pragma HLS ARRAY_PARTITION variable=kernel_2b cyclic factor=CMaps dim=2
    static w_dt kernel_2c[MapsMax][MapsMax];
#pragma HLS ARRAY_PARTITION variable=kernel_2c cyclic factor=CMaps dim=2
    static w_dt kernel_2d[MapsMax][MapsMax];
#pragma HLS ARRAY_PARTITION variable=kernel_2d cyclic factor=CMaps dim=2
    static w_dt kernel_2e[MapsMax][MapsMax];
#pragma HLS ARRAY_PARTITION variable=kernel_2e cyclic factor=CMaps dim=2
    static w_dt kernel_2f[MapsMax][MapsMax];
#pragma HLS ARRAY_PARTITION variable=kernel_2f cyclic factor=CMaps dim=2
#endif


    i_dt image[Maps1][Side1][Side1];
//#pragma HLS RESOURCE variable=image core=XPM_MEMORY uram
#pragma HLS ARRAY_PARTITION variable=image cyclic factor=IMaps

    sc_dt spike_count[Output];

    q_dt vm_conv[MapsMax][SideMax][SideMax];
#pragma HLS ARRAY_PARTITION variable=vm_conv_1 cyclic factor=Maps dim=1
    q2_dt vm_fc[MaxFc];
#pragma HLS ARRAY_PARTITION variable=vm_fc complete


    sc_dt spike_cnt[MapsMax][MaxNeurons];
//#pragma HLS RESOURCE variable=spike_cnt core=XPM_MEMORY uram
#pragma HLS ARRAY_PARTITION variable=spike_cnt cyclic factor=Part dim=1
    sr_dt spike_reg[MapsMax][MaxNeurons];
//#pragma HLS RESOURCE variable=spike_reg core=XPM_MEMORY uram
#pragma HLS ARRAY_PARTITION variable=spike_reg cyclic factor=Part dim=1
    sr_dt spike_input[MapsMax];
#pragma HLS ARRAY_PARTITION variable=spike_input complete


    if (!(flag)){
        for (int map = 0; map < MapsMax; map++) {
               for (int x=0; x<SideMax; x++) {
                     for(int y=0; y<SideMax; y++) {
#pragma HLS PIPELINE
                       		vm_conv[map][x][y] = 0;
                     }
                }
        }

        for (int map=0; map<MapsMax; map++) {
			for (int i=0; i<MaxNeurons; i++) {
#pragma HLS PIPELINE
				spike_reg[map][i] = 0;
				spike_cnt[map][i] = 0;
			}
        }

        for (int i=0; i<MaxFc; i++) {
#pragma HLS PIPELINE
        	vm_fc[i] = 0;
        }
#ifdef __SDSCC__
        load_kernel_pf(kernel_1a, k_1, Maps2, Maps1);
        load_weight_hw(kernel_2a, k_2, Maps3, Maps2);
		load_weight_hw(kernel_2b, k_3, Maps4, Maps3);
		load_kernel_pf(kernel_1b, k_4, Maps6, Maps5);
		load_weight_hw(kernel_2c, k_5, Maps7, Maps6);
		load_weight_hw(kernel_2d, k_6, Maps8, Maps7);
		load_kernel_pf(kernel_1c, k_7, Maps10, Maps9);
		load_weight_hw(kernel_2e, k_8, Maps11, Maps10);
		load_weight(kernel_2f, k_9, Maps12, Maps11);
#else
        load_kernel(kernel_1[0], k_1, Maps2, Maps1);
        load_weight(kernel_2[0], k_2, Maps3, Maps2);
        load_weight(kernel_2[1], k_3, Maps4, Maps3);
		load_kernel(kernel_1[1], k_4, Maps6, Maps5);
		load_weight(kernel_2[2], k_5, Maps7, Maps6);
		load_weight(kernel_2[3], k_6, Maps8, Maps7);
		load_kernel(kernel_1[2], k_7, Maps10, Maps9);
		load_weight(kernel_2[4], k_8, Maps11, Maps10);
		load_weight(kernel_2[5], k_9, Maps12, Maps11);

#endif
    }
    else {
#ifdef __SDSCC__
 		*dom = 0;
		image_copy(image, image_in);
		reset_sc(spike_count);
		reset_spike_pointer(spike_input);
		poisson(image, intensity, spike_reg, spike_cnt, spike_input, sf1);
//		load_kernel_pf(kernel_1a, k_1, Maps2, Maps1);
		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel_1a, Side1, Maps1, Side2, Maps2, sf2);
//		conv_spike_aggregation_1(vm_conv, spike_input, spike_reg, spike_cnt, kernel_1, k_1, Side1, Maps1, Side2, Maps2, sf2);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side2, Maps2);
//		load_weight_hw(kernel_2, k_2, Maps3, Maps2);
		conv_spike_aggregation_2(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2a, Side2, Maps2, Side3, Maps3, sf3);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side3, Maps3);
//		load_weight_hw(kernel_2, k_3, Maps4, Maps3);
		conv_spike_aggregation_2(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2b, Side3, Maps3, Side4, Maps4, sf4);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side4, Maps4);
		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side4, Maps4, Side5, sf5);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side5, Maps5);
//		load_kernel_pf(kernel_1a, k_4, Maps6, Maps5);
		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel_1b, Side5, Maps5, Side6, Maps6, sf6);
//		conv_spike_aggregation_1(vm_conv, spike_input, spike_reg, spike_cnt, kernel_1, k_4, Side5, Maps5, Side6, Maps6, sf6);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side6, Maps6);
//		load_weight_hw(kernel_2, k_5, Maps7, Maps6);
		conv_spike_aggregation_2(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2c, Side6, Maps6, Side7, Maps7, sf7);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side7, Maps7);
//		load_weight_hw(kernel_2, k_6, Maps8, Maps7);
		conv_spike_aggregation_2(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2d, Side7, Maps7, Side8, Maps8, sf8);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side8, Maps8);
		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side8, Maps8, Side9, sf9);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side9, Maps9);
//		load_kernel_pf(kernel_1a, k_7, Maps10, Maps9);
		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel_1c, Side9, Maps9, Side10, Maps10, sf10);
//		conv_spike_aggregation_1(vm_conv, spike_input, spike_reg, spike_cnt, kernel_1, k_7, Side9, Maps9, Side10, Maps10, sf10);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side10, Maps10);
//		load_weight_hw(kernel_2, k_8, Maps11, Maps10);
		conv_spike_aggregation_2(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2e, Side10, Maps10, Side11, Maps11, sf11);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side11, Maps11);
//		load_weight(kernel_2, k_9, Maps12, Maps11);
//		load_weight_hw(kernel_2, k_9, Maps12, Maps11);
		conv_spike_aggregation_2(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2f, Side11, Maps11, Side12, Maps12, sf12);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side12, Maps12);
		global_pool_spike_aggregation(vm_fc, spike_reg, spike_cnt, spike_input, Maps12, sf13);
		neuron_output(vm_fc, spike_count, dom);
#else
 		*dom = 0;
		image_copy(image, image_in);
		reset_sc(spike_count);
		reset_spike_pointer(spike_input);
		poisson(image, intensity, spike_reg, spike_cnt, spike_input, sf1);
		max_measurement(&spike_average[0], &spike_max[0], &spike_cnt_max[0], spike_cnt, spike_input, Maps1);
		conv_spike_aggregation_1(vm_conv, spike_input, spike_reg, spike_cnt, kernel_1[0], Side1, Maps1, Side2, Maps2, sf2);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side2, Maps2, vth1);
		max_measurement(&spike_average[1], &spike_max[1], &spike_cnt_max[1], spike_cnt, spike_input, Maps2);
		conv_spike_aggregation_2(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2[0], Side2, Maps2, Side3, Maps3, sf3);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side3, Maps3, vth2);
		max_measurement(&spike_average[2], &spike_max[2], &spike_cnt_max[2], spike_cnt, spike_input, Maps3);
		conv_spike_aggregation_2(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2[1], Side3, Maps3, Side4, Maps4, sf4);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side4, Maps4, vth3);
		max_measurement(&spike_average[3], &spike_max[3], &spike_cnt_max[3], spike_cnt, spike_input, Maps4);
		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side4, Maps4, Side5, sf5);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side5, Maps5, vth4);
		max_measurement(&spike_average[4], &spike_max[4], &spike_cnt_max[4], spike_cnt, spike_input, Maps5);
		conv_spike_aggregation_1(vm_conv, spike_input, spike_reg, spike_cnt, kernel_1[1], Side5, Maps5, Side6, Maps6, sf6);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side6, Maps6, vth5);
		max_measurement(&spike_average[5], &spike_max[5], &spike_cnt_max[5], spike_cnt, spike_input, Maps6);
		conv_spike_aggregation_2(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2[2], Side6, Maps6, Side7, Maps7, sf7);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side7, Maps7, vth6);
		max_measurement(&spike_average[6], &spike_max[6], &spike_cnt_max[6], spike_cnt, spike_input, Maps7);
		conv_spike_aggregation_2(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2[3], Side7, Maps7, Side8, Maps8, sf8);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side8, Maps8, vth7);
		max_measurement(&spike_average[7], &spike_max[7], &spike_cnt_max[7], spike_cnt, spike_input, Maps8);
		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side8, Maps8, Side9, sf9);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side9, Maps9, vth8);
		max_measurement(&spike_average[8], &spike_max[8], &spike_cnt_max[8], spike_cnt, spike_input, Maps9);
		conv_spike_aggregation_1(vm_conv, spike_input, spike_reg, spike_cnt, kernel_1[2], Side9, Maps9, Side10, Maps10, sf10);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side10, Maps10, vth9);
		max_measurement(&spike_average[9], &spike_max[9], &spike_cnt_max[9], spike_cnt, spike_input, Maps10);
		conv_spike_aggregation_2(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2[4], Side10, Maps10, Side11, Maps11, sf11);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side11, Maps11, vth10);
		max_measurement(&spike_average[10], &spike_max[10], &spike_cnt_max[10], spike_cnt, spike_input, Maps11);
		conv_spike_aggregation_2(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2[5], Side11, Maps11, Side12, Maps12, sf12);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side12, Maps12, vth11);
		max_measurement(&spike_average[11], &spike_max[11], &spike_cnt_max[11], spike_cnt, spike_input, Maps12);
		global_pool_spike_aggregation(vm_fc, spike_reg, spike_cnt, spike_input, Maps12, sf13);
		neuron_output(vm_fc, spike_count, dom, vth12);
		max_measurement_2(&spike_cnt_max[12], spike_count);
#endif
    }
}
