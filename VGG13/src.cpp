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


void poisson(i_dt image[Maps1][Side1][Side1], int intensity, sr_dt spike_reg[MapsMax1][MaxNeurons1],
		sc_dt spike_cnt[MapsMax1][MaxNeurons1], sr_dt spike_input[MapsMax], w_dt sf) {
#pragma HLS INLINE off

	bool flag = false;
    L1: for (int map = 0; map < Maps1; map++) {
    	poisson_pe(image[map], intensity, spike_reg[map],
    			spike_cnt[map], &spike_input[map], sf);
    }
}

void encoding_pe (int map, q_dt vm[SideMax][SideMax], sr_dt local_spike_reg[MaxNeurons], sc_dt local_spike_cnt[MaxNeurons],
		sr_dt *local_pointer, int OutSide) {
#pragma HLS INLINE off
	N2: for (int y = 0; y < OutSide; y++) {
		N3: for (int x = 0; x < OutSide; x++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm inter false
			if (vm[y][x] >= vth){
				local_spike_reg[*local_pointer] = map*OutSide*OutSide + y*OutSide + x;
				local_spike_cnt[*local_pointer] = vm[y][x];
				(*local_pointer)++;
			}
			vm[y][x] = 0;
		}
    }
}

void neuron_conv(q_dt vm[MapsMax][SideMax][SideMax],
		sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons], sr_dt spike[MapsMax],
		int OutSide, int OutMaps) {
#pragma HLS INLINE off

	N1: for (int map = 0; map < OutMaps; map++) {
#pragma HLS UNROLL factor=EMaps
		encoding_pe(map, vm[map], spike_reg[map], spike_cnt[map],
				&spike[map], OutSide);
    }
}


void neuron_fc(q2_dt vm_fc[MaxFc], sr_dt spike_reg[MapsMax][MaxNeurons],sc_dt spike_cnt[MapsMax][MaxNeurons], sr_dt spike[MapsMax]) {
#pragma HLS INLINE off
	*spike = 0;
	NF: for (int i=0; i<MaxFc; i++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm_fc inter false
		if (vm_fc[i] >= vth) {
			spike_reg[0][spike[0]] = i;
			spike_cnt[0][spike[0]] = vm_fc[i];
			(spike[0])++;
			vm_fc[i]=0;
		}
		else {
			vm_fc[i]=0;
		}
	}
}


void neuron_output(q2_dt vm_fc[MaxFc], sc_dt spike_count[Output], int *dom) {
#pragma HLS INLINE off
	NEURONOUT: for (int i=0; i < Output; i++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm_fc inter false
		if (vm_fc[i] >= vth) {
			spike_count[i] = vm_fc[i];
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


void pool_pe(q_dt vm[SideMax][SideMax], sr_dt *spiked, sr_dt spike_reg[MaxNeurons], sc_dt spike_cnt[MaxNeurons],
		int Side, int OutSide, w_dt sf) {
#pragma HLS INLINE off
	int pre_x, pre_y;
    int oy, ox, ky, kx;
    sc_dt temp_cnt;
	ps_compute: for (int spiker = 0; spiker < *spiked; spiker++){
#pragma HLS PIPELINE
		pre_y = (spike_reg[spiker] % (Side*Side)) / Side;
		pre_x = (spike_reg[spiker] % (Side*Side)) % Side;
		ky = pre_y % StrideP;
		kx = pre_x % StrideP;
		temp_cnt = spike_cnt[spiker]*sf;
		oy = (pre_y + PaddingP - ky) / StrideP;
		ox = (pre_x + PaddingP - kx) / StrideP;
		if ((ox >= 0) & (ox < OutSide) & (oy >= 0) & (oy < OutSide))
			vm[oy][ox] += q_dt(scale)*temp_cnt;
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

void pool_pe_1(q_dt vm[SideMax][SideMax], sr_dt *spiked, sr_dt spike_reg[MaxNeurons1], sc_dt spike_cnt[MaxNeurons1],
		int Side, int OutSide, w_dt sf) {
#pragma HLS INLINE off
	int pre_x, pre_y;
    int oy, ox, ky, kx;
    sc_dt temp_cnt;
	ps_compute: for (int spiker = 0; spiker < *spiked; spiker++){
#pragma HLS PIPELINE
		pre_y = (spike_reg[spiker] % (Side*Side)) / Side;
		pre_x = (spike_reg[spiker] % (Side*Side)) % Side;
		ky = pre_y % StrideP;
		kx = pre_x % StrideP;
		temp_cnt = spike_cnt[spiker]*sf;
		oy = (pre_y + PaddingP - ky) / StrideP;
		ox = (pre_x + PaddingP - kx) / StrideP;
		if ((ox >= 0) & (ox < OutSide) & (oy >= 0) & (oy < OutSide))
			vm[oy][ox] += q_dt(scale)*temp_cnt;
	}
}


void pool_spike_aggregation_1(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input[MapsMax], sr_dt spike_reg[MapsMax1][MaxNeurons1], sc_dt spike_cnt[MapsMax1][MaxNeurons1],
		int Side, int InMaps, int OutSide, w_dt sf){
#pragma HLS INLINE off
	ps_1: for (int imap = 0; imap < InMaps; imap++) {
#pragma HLS UNROLL factor=PMaps
		pool_pe_1(vm[imap], &spike_input[imap], spike_reg[imap], spike_cnt[imap], Side, OutSide, sf);
	}
}


#ifndef __SDSCC__
void fc_spike_aggregation(q_dt vm[MaxFc], sr_dt spiked[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons],
                         w_dt weight[BufRows+1][MaxCols], int InMaps, int ARows, int ACols, w_dt sf){
#pragma HLS INLINE off
	int pre, temp;
	bool flag = false;

	int TFactor = FUnroll;
	if (ACols < FUnroll) {
		TFactor = ACols;
	}

	FL0: for (int imap = 0; imap < InMaps; imap++) {
		FL1: for (int spiker = 0; spiker < spiked[imap]; spiker++){
#pragma HLS loop_tripcount avg=200
			pre = spike_reg[imap][spiker];
			temp = pre;
			FL2: for (int post = 0; post < ACols/TFactor; post++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm inter false
				FL3: for (int u=0; u < FUnroll; u++) {
#pragma HLS UNROLL
					if (u<ACols) {
						vm[post*FUnroll+u] += (weight[temp][post*FUnroll+u] * spike_cnt[imap][spiker] * sf);
					}
				}
			}
		}
	}
}
#else

void load (int index, int boundary, IW w_off[MaxRows*MaxCols/WIDTH_FACTOR], w_dt weight_buf[MaxFc],
		sr_dt spike_reg[MaxNeurons], int ACols) {
#pragma HLS INLINE off
	if (index<boundary) {
		int pre = spike_reg[index];
		F1: for (int w = 0; w < ACols/WIDTH_FACTOR; w++) {
	#pragma HLS PIPELINE
	#pragma HLS DEPENDENCE variable=weight_buf inter false
			IW tw = w_off[pre*(ACols/WIDTH_FACTOR)+w];
			F2 : for (int x = 0; x< WIDTH_FACTOR; x++) {
	#pragma HLS UNROLL
				unsigned int range_idx = x * WWIDTH;
				WW tmp_int = tw.range(range_idx + WWIDTH - 1, range_idx);
				w_dt tmp = *((w_dt*)(&tmp_int));
				weight_buf[w*WIDTH_FACTOR+x] = tmp;
			}
		}
	}
}

void compute (int index, w_dt weight_buf[MaxFc], q2_dt vm[MaxFc], int ACols, sc_dt spike_cnt[MaxNeurons], w_dt sf) {
#pragma HLS INLINE off
	sc_dt temp_cnt;
	if (index >= 0) {
		temp_cnt = spike_cnt[index] * sf;
    FC1: for (int post = 0; post < ACols/FUnroll; post++)
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm inter false
    	FC2: for (int u=0; u < FUnroll; u++) {
    		vm[post*FUnroll+u] += weight_buf[post*FUnroll+u]*temp_cnt;
    	}
	}
}

void fc_spike_aggregation(q2_dt vm[MaxFc], sr_dt spiked[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons],
	                         IW w_off[MaxRows*MaxCols/WIDTH_FACTOR], w_dt weight[BufRows+1][MaxCols], int InMaps, int ARows, int ACols, w_dt sf){
#pragma HLS INLINE off
	int pre, temp;
	bool flag = false;

	static w_dt weight_buf_a[MaxFc];
#pragma HLS ARRAY_PARTITION variable=weight_buf_a cyclic factor=FUnroll dim=1
	static w_dt weight_buf_b[MaxFc];
#pragma HLS ARRAY_PARTITION variable=weight_buf_b cyclic factor=FUnroll dim=1

	int TFactor = FUnroll;
	if (ACols < FUnroll) {
		TFactor = ACols;
	}

	if (BufRows==0) {
		OL0: for (int imap = 0; imap < InMaps; imap++) {
			OL1: for (int spiker = 0; spiker < spiked[imap]+1; spiker++){
		    	if (spiker % 2) {
		    		load(spiker, spiked[imap], w_off, weight_buf_a, spike_reg[imap], ACols);
		    		compute(spiker-1, weight_buf_b, vm, ACols, spike_cnt[imap], sf);
		    	}
		    	else {
		    		load(spiker, spiked[imap], w_off, weight_buf_b, spike_reg[imap], ACols);
		    		compute(spiker-1, weight_buf_a, vm, ACols, spike_cnt[imap], sf);
		    	}
			}
		}
	}
	else {
		FL0: for (int imap = 0; imap < InMaps; imap++) {
			FL1: for (int spiker = 0; spiker < spiked[imap]; spiker++){
#pragma HLS loop_tripcount avg=200
				pre = spike_reg[imap][spiker];
				temp = pre;
				if (pre>=BufRows) {
					temp = BufRows;
					FL2: for (int w = 0; w < ACols/WIDTH_FACTOR; w++) {
#pragma HLS PIPELINE
						IW tw = w_off[pre*(ACols/WIDTH_FACTOR)+w];
						FLx : for (int x = 0; x< WIDTH_FACTOR; x++) {
#pragma HLS UNROLL
							unsigned int range_idx = x * WWIDTH;
							WW tmp_int = tw.range(range_idx + WWIDTH - 1, range_idx);
							w_dt tmp = *((w_dt*)(&tmp_int));
							weight[temp][w*WIDTH_FACTOR+x] = tmp;
						}
					}
				}
				FL3: for (int post = 0; post < ACols/TFactor; post++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm inter false
					FL4: for (int u=0; u < FUnroll; u++) {
#pragma HLS UNROLL
						if (u<ACols) {
							vm[post*FUnroll+u] += (weight[temp][post*FUnroll+u] * spike_cnt[imap][spiker]);
						}
					}
				}
			}
		}
	}
}
#endif

void conv_pe(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input, sr_dt spike_reg[MaxNeurons],
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
                	pre_y = (spike_reg[spiker] % (Side*Side)) / Side;
                	pre_x = (spike_reg[spiker] % (Side*Side)) % Side;
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

void conv_pe_1(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input, sr_dt spike_reg[MaxNeurons1],
		sc_dt spike_cnt[MaxNeurons1], w_dt kernel[MapsMax][KSideMax][KSideMax], int Side, int OutSide, int OutMaps, int TMaps, w_dt sf) {
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
                	pre_y = (spike_reg[spiker] % (Side*Side)) / Side;
                	pre_x = (spike_reg[spiker] % (Side*Side)) % Side;
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
void load_kernel(w_dt kernel[MapsMax][MapsMax][KSideMax][KSideMax], w_dt k_off[MapsMax*MapsMax*KSideMax*KSideMax], int OutMaps, int InMaps) {
#pragma HLS INLINE off
    for (int i=0; i<InMaps; i++)
    	for(int j=0; j<OutMaps; j++)
    		for(int k=0; k<KSideMax; k++)
    			for(int l=0; l<KSideMax; l++)
#pragma HLS PIPELINE
    				kernel[i][j][k][l] = k_off[(i*OutMaps*KSideMax*KSideMax)+(j*KSideMax*KSideMax)+(k*KSideMax)+l];
}
#else
void load_kernel_pf(w_dt kernel[BufMaps+1][MapsMax][KSideMax][KSideMax], IW k_off[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], int OutMaps, int InMaps) {
#pragma HLS INLINE off
	int AMaps = BufMaps;
	if (InMaps<BufMaps) {
		AMaps = InMaps;
	}

    for (int i=0; i<AMaps; i++) {
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

void load_kernel_hw(w_dt kernel[MapsMax][KSideMax][KSideMax], IW k_off[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], int OutMaps, int InMaps, int imap) {
#pragma HLS INLINE off
    for(int j=0; j<OutMaps/WIDTH_FACTOR; j++) {
    	for(int k=0; k<KSide1; k++) {
    		for(int l=0; l<KSide1; l++) {
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
#endif

void conv_spike_aggregation_pf(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons],
		sc_dt spike_cnt[MapsMax][MaxNeurons], w_dt kernel[MapsMax][MapsMax][KSideMax][KSideMax], int Side, int InMaps, int OutSide, int OutMaps, w_dt sf) {
#pragma HLS INLINE off
	int TMaps = Maps;
	if (OutMaps < Maps) {
		TMaps = OutMaps;
	}

    cs1: for (int imap = 0; imap < InMaps; imap++) {
    		conv_pe(vm, spike_input[imap], spike_reg[imap],
					spike_cnt[imap], kernel[imap], Side, OutSide, OutMaps, TMaps, sf);
    }

}

void conv_spike_aggregation_pf_1(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input[MapsMax], sr_dt spike_reg[MapsMax1][MaxNeurons1],
		sc_dt spike_cnt[MapsMax1][MaxNeurons1], w_dt kernel[MapsMax][MapsMax][KSideMax][KSideMax], int Side, int InMaps, int OutSide, int OutMaps, w_dt sf) {
#pragma HLS INLINE off
	int TMaps = Maps;
	if (OutMaps < Maps) {
		TMaps = OutMaps;
	}

    cs1: for (int imap = 0; imap < InMaps; imap++) {
    		conv_pe(vm, spike_input[imap], spike_reg[imap],
					spike_cnt[imap], kernel[imap], Side, OutSide, OutMaps, TMaps, sf);
    }

}

#ifdef __SDSCC__
void conv_spike_aggregation(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons],
		sc_dt spike_cnt[MapsMax][MaxNeurons], w_dt kernel[BufMaps+1][MapsMax][KSideMax][KSideMax], IW k_off[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
		int Side, int InMaps, int OutSide, int OutMaps, w_dt sf) {
#pragma HLS INLINE off
	int TMaps = Maps;
	if (OutMaps < Maps) {
		TMaps = OutMaps;
	}
	int temp;
    cs1: for (int imap = 0; imap < InMaps; imap++) {
    	temp = imap;
    	if(imap >= BufMaps) {
    		temp = BufMaps;
    		load_kernel_hw(kernel[temp], k_off, OutMaps, InMaps, imap);
    	}
    	conv_pe(vm, spike_input[imap], spike_reg[imap],
					spike_cnt[imap], kernel[temp], Side, OutSide, OutMaps, TMaps, sf);
    }
}
void conv_spike_aggregation_1(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input[MapsMax], sr_dt spike_reg[MapsMax1][MaxNeurons1],
		sc_dt spike_cnt[MapsMax1][MaxNeurons1], w_dt kernel[BufMaps+1][MapsMax][KSideMax][KSideMax], IW k_off[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
		int Side, int InMaps, int OutSide, int OutMaps, w_dt sf) {
#pragma HLS INLINE off
	int TMaps = Maps;
	if (OutMaps < Maps) {
		TMaps = OutMaps;
	}

	int temp;
    cs1: for (int imap = 0; imap < InMaps; imap++) {
    	temp = imap;
    	if(imap >= BufMaps) {
    		temp = BufMaps;
    		load_kernel_hw(kernel[temp], k_off, OutMaps, InMaps, imap);
    	}
    	conv_pe(vm, spike_input[imap], spike_reg[imap],
					spike_cnt[imap], kernel[temp], Side, OutSide, OutMaps, TMaps, sf);
    }
}
#endif

void output_spike_aggregation(q2_dt vm[MaxFc], sr_dt spiked[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons],
                                        w_dt weights_out[Layer2][Output]){
#pragma HLS INLINE off
	int pre;
    FL4: for (int spiker = 0; spiker < spiked[0]; spiker++){
#pragma HLS loop_tripcount avg=200
        FL5: for (int post = 0; post < Output; post++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm inter false
        	if (post==0) {
        		pre = spike_reg[0][spiker];
        	}
        	vm[post] += (weights_out[pre][post]*spike_cnt[0][spiker]);
        }
    }
}

#ifndef __SDSCC__
void max_measurement(double *spike_average, int *spike_max, int *spike_cnt_max, int spike_cnt[MapsMax][MaxNeurons], int spike_input[MapsMax], int LMaps) {
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

void max_measurement_2(int *spike_cnt_max, int spike_count[Output]) {
	for (int i=0; i<Output; i++) {
		if (*spike_cnt_max < spike_count[i]) {
			*spike_cnt_max = spike_count[i];
		}
	}
}
#endif

#ifdef __SDSCC__
#pragma SDS data zero_copy(image_in, k_1, k_2, k_3, k_4, k_5, k_6, k_7, k_8, k_9, k_10, w_1, w_2, w_3)
void network(i_dt image_in[Sources], int intensity,
	        int *dom, IW k_1[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW k_2[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			IW k_3[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW k_4[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			IW k_5[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW k_6[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			IW k_7[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW k_8[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			IW k_9[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			IW k_10[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW w_1[MaxRows*MaxCols/WIDTH_FACTOR],
			IW w_2[MaxRows*MaxCols/WIDTH_FACTOR], IW w_3[MaxRows/WIDTH_FACTOR*MaxCols], bool flag) {
#else
void network(i_dt image_in[Sources], int intensity,
		        int *dom, w_dt k_1[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_2[MapsMax*MapsMax*KSideMax*KSideMax], w_dt k_3[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_4[MapsMax*MapsMax*KSideMax*KSideMax], w_dt k_5[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_6[MapsMax*MapsMax*KSideMax*KSideMax], w_dt k_7[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_8[MapsMax*MapsMax*KSideMax*KSideMax], w_dt k_9[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_10[MapsMax*MapsMax*KSideMax*KSideMax], w_dt w_1[MaxRows*MaxCols],
				w_dt w_2[MaxRows*MaxCols], w_dt w_3[MaxRows*MaxCols], bool flag, int spike_cnt_max[NumLayers],
				double spike_average[NumLayers], int spike_max[NumLayers]) {
#endif

#ifdef __SDSCC__
    static w_dt kernel[BufMaps+1][MapsMax][KSideMax][KSideMax];
#pragma HLS ARRAY_PARTITION variable=kernel cyclic factor=Maps dim=2

//    static w_dt kernel[MapsMax][MapsMax][KSideMax][KSideMax];
//#pragma HLS ARRAY_PARTITION variable=kernel cyclic factor=Maps dim=2


    static w_dt weight_1[BufRows+1][MaxCols];
//#pragma HLS RESOURCE variable=weight core=XPM_MEMORY uram
#pragma HLS ARRAY_PARTITION variable=weight_1 cyclic factor=FUnroll dim=2

    static w_dt weight_2[BufRows+1][MaxCols];
//#pragma HLS RESOURCE variable=weight core=XPM_MEMORY uram
#pragma HLS ARRAY_PARTITION variable=weight_2 cyclic factor=FUnroll dim=2

    static w_dt weights_out[Layer2][Output];
//#pragma HLS RESOURCE variable=weights_out core=XPM_MEMORY uram
#else
    static w_dt kernel[NumKernels][MapsMax][MapsMax][KSideMax][KSideMax];
    static w_dt weight_1[MaxRows][MaxCols];
    static w_dt weight_2[MaxRows][MaxCols];
    static w_dt weights_out[Layer2][Output];
#endif

    i_dt image[Maps1][Side1][Side1];
//#pragma HLS RESOURCE variable=image core=XPM_MEMORY uram
#pragma HLS ARRAY_PARTITION variable=image cyclic factor=IMaps

    sc_dt spike_count[Output];

    q_dt vm_conv[MapsMax][SideMax][SideMax];
#pragma HLS ARRAY_PARTITION variable=vm_conv_1 cyclic factor=Maps dim=1
    q2_dt vm_fc[MaxFc];
//#pragma HLS RESOURCE variable=vm_fc core=XPM_MEMORY uram
#pragma HLS ARRAY_PARTITION variable=vm_fc cyclic factor=FUnroll


//    sc_dt spike_cnt_1[MapsMax1][MaxNeurons1];
////#pragma HLS RESOURCE variable=spike_cnt core=XPM_MEMORY uram
//#pragma HLS ARRAY_PARTITION variable=spike_cnt_1 cyclic factor=Part dim=1
//
//    sr_dt spike_reg_1[MapsMax1][MaxNeurons1];
////#pragma HLS RESOURCE variable=spike_reg core=XPM_MEMORY uram
//#pragma HLS ARRAY_PARTITION variable=spike_reg_1 cyclic factor=Part dim=1

    sc_dt spike_cnt[MapsMax][MaxNeurons];
//#pragma HLS RESOURCE variable=spike_cnt core=XPM_MEMORY uram
#pragma HLS ARRAY_PARTITION variable=spike_cnt cyclic factor=Part dim=1

    sr_dt spike_reg[MapsMax][MaxNeurons];
//#pragma HLS RESOURCE variable=spike_reg core=XPM_MEMORY uram
#pragma HLS ARRAY_PARTITION variable=spike_reg cyclic factor=Part dim=1

    sr_dt spike_input[MapsMax];
#pragma HLS ARRAY_PARTITION variable=spike_input cyclic factor=Part


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
    	for (int i = 0; i < BufRows; i++) {
    	   for (int j = 0; j<Layer2/WIDTH_FACTOR; j++) {
#pragma HLS PIPELINE
    		   IW temp = w_1[i*(Layer2/WIDTH_FACTOR)+j];
    		   for (int x=0; x< WIDTH_FACTOR; x++) {
#pragma HLS UNROLL
    			   unsigned int range_idx = x * WWIDTH;
    			   WW tmp_int = temp.range(range_idx + WWIDTH - 1, range_idx);
    			   w_dt tmp = *((w_dt*)(&tmp_int));
    			   weight_1[i][j*WIDTH_FACTOR+x] = tmp;
    		   }
    	   }
    	}

    	for (int i = 0; i < BufRows; i++) {
    	   for (int j = 0; j<Layer3/WIDTH_FACTOR; j++) {
#pragma HLS PIPELINE
    		   IW temp = w_2[i*(Layer3/WIDTH_FACTOR)+j];
    		   for (int x=0; x< WIDTH_FACTOR; x++) {
#pragma HLS UNROLL
    			   unsigned int range_idx = x * WWIDTH;
    			   WW tmp_int = temp.range(range_idx + WWIDTH - 1, range_idx);
    			   w_dt tmp = *((w_dt*)(&tmp_int));
    			   weight_2[i][j*WIDTH_FACTOR+x] = tmp;
    		   }
    	   }
    	}

    	for (int i=0; i < Layer3/WIDTH_FACTOR; i++) {
    		for (int j=0; j<Output; j++) {
#pragma HLS PIPELINE
    			IW temp = w_3[i*Output+j];
    			for (int x=0; x<WIDTH_FACTOR; x++) {
#pragma HLS UNROLL
    			   unsigned int range_idx = x * WWIDTH;
    			   WW tmp_int = temp.range(range_idx + WWIDTH - 1, range_idx);
    			   w_dt tmp = *((w_dt*)(&tmp_int));
    			   weights_out[i*WIDTH_FACTOR+x][j] = tmp;
    		   }
    	   }
    	}
#else
    	for (int i = 0; i < Layer1; i++) {
    	   for (int j = 0; j<Layer2; j++) {
#pragma HLS PIPELINE
    	      	weight_1[i][j] = w_1[i*Layer2+j];
    	   }
    	}
    	for (int i = 0; i < Layer2; i++) {
    	   for (int j = 0; j<Layer3; j++) {
#pragma HLS PIPELINE
    	      	weight_2[i][j] = w_2[i*Layer3+j];
    	   }
    	}

        for (int i = 0; i < Layer3; i++) {
        	for (int j = 0; j<Output; j++) {
#pragma HLS PIPELINE
        		weights_out[i][j] = w_3[i*Output+j];
        	}
        }
        load_kernel(kernel[0], k_1, Maps2, Maps1);
        load_kernel(kernel[1], k_2, Maps3, Maps2);
        load_kernel(kernel[2], k_3, Maps5, Maps4);
        load_kernel(kernel[3], k_4, Maps6, Maps5);
        load_kernel(kernel[4], k_5, Maps8, Maps7);
        load_kernel(kernel[5], k_6, Maps9, Maps8);
        load_kernel(kernel[6], k_7, Maps11, Maps10);
        load_kernel(kernel[7], k_8, Maps12, Maps11);
        load_kernel(kernel[8], k_9, Maps14, Maps13);
        load_kernel(kernel[9], k_10, Maps15, Maps14);
#endif
    }
    else {
#ifdef __SDSCC__
		*dom = 0;
		image_copy(image, image_in);
		reset_sc(spike_count);
		reset_spike_pointer(spike_input);
		poisson(image, intensity, spike_reg_1, spike_cnt_1, spike_input, sf1);

		load_kernel_pf(kernel, k_1, Maps2, Maps1);
		conv_spike_aggregation_pf_1(vm_conv, spike_input, spike_reg_1, spike_cnt_1, kernel, Side1, Maps1, Side2, Maps2, sf2);
		// conv_spike_aggregation_1(vm_conv, spike_input, spike_reg_1, spike_cnt_1, kernel, k_1, Side1, Maps1, Side2, Maps2, sf2);
		reset_spike_pointer(spike_input);
		neuron_conv_1(vm_conv, spike_reg_1, spike_cnt_1, spike_input, Side2, Maps2);

		load_kernel_pf(kernel, k_2, Maps3, Maps2);
		conv_spike_aggregation_pf_1(vm_conv, spike_input, spike_reg_1, spike_cnt_1, kernel, Side2, Maps2, Side3, Maps3, sf3);
		// conv_spike_aggregation_1(vm_conv, spike_input, spike_reg_1, spike_cnt_1, kernel, k_2, Side2, Maps2, Side3, Maps3, sf3);
		reset_spike_pointer(spike_input);
		neuron_conv_1(vm_conv, spike_reg_1, spike_cnt_1, spike_input, Side3, Maps3);

		pool_spike_aggregation_1(vm_conv, spike_input, spike_reg_1, spike_cnt_1, Side3, Maps3, Side4, sf4);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side4, Maps4);

		load_kernel_pf(kernel, k_3, Maps5, Maps4);
		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel, Side4, Maps4, Side5, Maps5, sf5);
		// conv_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, kernel, k_3, Side4, Maps4, Side5, Maps5, sf5);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side5, Maps5);

		load_kernel_pf(kernel, k_4, Maps6, Maps5);
		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel, Side5, Maps5, Side6, Maps6, sf6);
		// conv_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, kernel, k_4, Side5, Maps5, Side6, Maps6, sf6);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side6, Maps6);

		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side6, Maps6, Side7, sf7);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side7, Maps7);


		load_kernel_pf(kernel, k_5, Maps8, Maps7);
		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel, Side7, Maps7, Side8, Maps8, sf8);
		// conv_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, kernel, k_5, Side7, Maps7, Side8, Maps8, sf8);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side8, Maps8);

		load_kernel_pf(kernel, k_6, Maps9, Maps8);
		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel, Side8, Maps8, Side9, Maps9, sf9);
		// conv_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, kernel, k_6, Side8, Maps8, Side9, Maps9, sf9);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side9, Maps9);

		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side9, Maps9, Side10, sf10);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side10, Maps10);


		load_kernel_pf(kernel, k_7, Maps11, Maps10);
		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel, Side10, Maps10, Side11, Maps11, sf11);
		// conv_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, kernel, k_7, Side10, Maps10, Side11, Maps11, sf11);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side11, Maps11);

		load_kernel_pf(kernel, k_8, Maps12, Maps11);
		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel, Side11, Maps11, Side12, Maps12, sf12);
		// conv_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, kernel, k_8, Side11, Maps11, Side12, Maps12, sf12);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side12, Maps12);

		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side12, Maps12, Side13, sf13);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side13, Maps13);


		load_kernel_pf(kernel, k_9, Maps14, Maps13);
		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel, Side13, Maps13, Side14, Maps14, sf14);
		// conv_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, kernel, k_9, Side13, Maps13, Side14, Maps14, sf14);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side14, Maps14);

		load_kernel_pf(kernel, k_10, Maps15, Maps14);
		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel, Side14, Maps14, Side15, Maps15, sf15);
		// conv_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, kernel, k_10, Side14, Maps14, Side15, Maps15, sf15);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side15, Maps15);

		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side15, Maps15, Side16, sf16);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side16, Maps16);


		fc_spike_aggregation(vm_fc, spike_input, spike_reg, spike_cnt, w_1, weight_1, Maps16, Layer1, Layer2, sf17);
		reset_spike_pointer(spike_input);
		neuron_fc(vm_fc, spike_reg, spike_cnt, spike_input);

		fc_spike_aggregation(vm_fc, spike_input, spike_reg, spike_cnt, w_2, weight_2, 0, Layer2, Layer3, sf18);
		reset_spike_pointer(spike_input);
		neuron_fc(vm_fc, spike_reg, spike_cnt, spike_input);

		output_spike_aggregation(vm_fc, spike_input, spike_reg, spike_cnt, weights_out);
		neuron_output(vm_fc, spike_count, dom);
#else
 		*dom = 0;
		image_copy(image, image_in);
		reset_sc(spike_count);
		reset_spike_pointer(spike_input);
		poisson(image, intensity, spike_reg, spike_cnt, spike_input, sf1);
		max_measurement(&spike_average[0], &spike_max[0], &spike_cnt_max[0], spike_cnt, spike_input, Maps1);

		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel[0], Side1, Maps1, Side2, Maps2, sf2);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side2, Maps2);
		max_measurement(&spike_average[1], &spike_max[1], &spike_cnt_max[1], spike_cnt, spike_input, Maps2);

		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel[1], Side2, Maps2, Side3, Maps3, sf3);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side3, Maps3);
		max_measurement(&spike_average[2], &spike_max[2], &spike_cnt_max[2], spike_cnt, spike_input, Maps3);

		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side3, Maps3, Side4, sf4);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side4, Maps4);
		max_measurement(&spike_average[3], &spike_max[3], &spike_cnt_max[3], spike_cnt, spike_input, Maps4);

		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel[2], Side4, Maps4, Side5, Maps5, sf5);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side5, Maps5);
		max_measurement(&spike_average[4], &spike_max[4], &spike_cnt_max[4], spike_cnt, spike_input, Maps5);

		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel[3], Side5, Maps5, Side6, Maps6, sf6);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side6, Maps6);
		max_measurement(&spike_average[5], &spike_max[5], &spike_cnt_max[5], spike_cnt, spike_input, Maps6);

		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side6, Maps6, Side7, sf7);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side7, Maps7);
		max_measurement(&spike_average[6], &spike_max[6], &spike_cnt_max[6], spike_cnt, spike_input, Maps7);

		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel[4], Side7, Maps7, Side8, Maps8, sf8);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side8, Maps8);
		max_measurement(&spike_average[7], &spike_max[7], &spike_cnt_max[7], spike_cnt, spike_input, Maps8);

		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel[5], Side8, Maps8, Side9, Maps9, sf9);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side9, Maps9);
		max_measurement(&spike_average[8], &spike_max[8], &spike_cnt_max[8], spike_cnt, spike_input, Maps9);

		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side9, Maps9, Side10, sf10);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side10, Maps10);
		max_measurement(&spike_average[9], &spike_max[9], &spike_cnt_max[9], spike_cnt, spike_input, Maps10);

		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel[6], Side10, Maps10, Side11, Maps11, sf11);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side11, Maps11);
		max_measurement(&spike_average[10], &spike_max[10], &spike_cnt_max[10], spike_cnt, spike_input, Maps11);

		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel[7], Side11, Maps11, Side12, Maps12, sf12);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side12, Maps12);
		max_measurement(&spike_average[11], &spike_max[11], &spike_cnt_max[11], spike_cnt, spike_input, Maps12);

		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side12, Maps12, Side13, sf13);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side13, Maps13);
		max_measurement(&spike_average[12], &spike_max[12], &spike_cnt_max[12], spike_cnt, spike_input, Maps13);

		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel[8], Side13, Maps13, Side14, Maps14, sf14);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side14, Maps14);
		max_measurement(&spike_average[13], &spike_max[13], &spike_cnt_max[13], spike_cnt, spike_input, Maps14);

		conv_spike_aggregation_pf(vm_conv, spike_input, spike_reg, spike_cnt, kernel[9], Side14, Maps14, Side15, Maps15, sf15);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side15, Maps15);
		max_measurement(&spike_average[14], &spike_max[14], &spike_cnt_max[14], spike_cnt, spike_input, Maps15);

		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side15, Maps15, Side16, sf16);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side16, Maps16);
		max_measurement(&spike_average[15], &spike_max[15], &spike_cnt_max[15], spike_cnt, spike_input, Maps16);


		fc_spike_aggregation(vm_fc, spike_input, spike_reg, spike_cnt, weight_1, Maps16, Layer1, Layer2, sf17);
		reset_spike_pointer(spike_input);
		neuron_fc(vm_fc, spike_reg, spike_cnt, spike_input);
		max_measurement(&spike_average[16], &spike_max[16], &spike_cnt_max[16], spike_cnt, spike_input, 1);

		fc_spike_aggregation(vm_fc, spike_input, spike_reg, spike_cnt, weight_2, 1, Layer2, Layer3, sf18);
		reset_spike_pointer(spike_input);
		neuron_fc(vm_fc, spike_reg, spike_cnt, spike_input);
		max_measurement(&spike_average[17], &spike_max[17], &spike_cnt_max[17], spike_cnt, spike_input, 1);

		output_spike_aggregation(vm_fc, spike_input, spike_reg, spike_cnt, weights_out);
		neuron_output(vm_fc, spike_count, dom);
		max_measurement_2(&spike_cnt_max[18], spike_count);
#endif
    }



}
