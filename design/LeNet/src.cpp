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

#ifndef __SDSCC__
void load_kernel(w_dt kernel[MapsMax][MapsMax][KSide][KSide], w_dt k_off[MapsMax*MapsMax*KSide*KSide], int OutMaps, int InMaps) {
#pragma HLS INLINE off
    for (int i=0; i<InMaps; i++)
    	for(int j=0; j<OutMaps; j++)
    		for(int k=0; k<KSide; k++)
    			for(int l=0; l<KSide; l++)
#pragma HLS PIPELINE
    				kernel[i][j][k][l] = k_off[(i*OutMaps*KSide*KSide)+(j*KSide*KSide)+(k*KSide)+l];
}
#else
void load_kernel_hw(w_dt kernel[MapsMax][MapsMax][KSide][KSide], IW k_off[MapsMax*MapsMax/WIDTH_FACTOR*KSide*KSide], int OutMaps, int InMaps) {
#pragma HLS INLINE off
    for (int i=0; i<InMaps; i++) {
    	for(int j=0; j<OutMaps/WIDTH_FACTOR; j++) {
    		for(int k=0; k<KSide; k++) {
    			for(int l=0; l<KSide; l++) {
#pragma HLS PIPELINE
    				IW temp = k_off[(i*(OutMaps/WIDTH_FACTOR)*KSide*KSide)+(j*KSide*KSide)+(k*KSide)+l];
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

void poisson_pe(i_dt image[Side1][Side1], int intensity, sr_dt spike_reg[MaxNeurons],
		sc_dt spike_cnt[MaxNeurons], sr_dt *spike_input, w_dt sf) {
#pragma HLS INLINE off

	bool flag;
	int temp_reg[TSims/Sims], temp_cnt[TSims/Sims];
   static unsigned int lfsr = 123456789 & 0x7fffffff;
   unsigned int bit;
   unsigned int random_int;

   int accum_cnt;

   int ASims = TSims/Sims;
   int LSims;

   if (ASims==1) {
   	LSims = 2;
   } else {
   	LSims = ASims;
   }

	L1: for (int i = 0; i < Side1; i++) {
		L2: for (int j = 0; j < Side1; j++) {
			L3: for (int s = 0;  s < LSims; s++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=spike_reg inter false
#pragma HLS DEPENDENCE variable=spike_cnt inter false
				if (s<ASims) {
					if (s==0) {
						flag = false;
						accum_cnt = 0;
					}
					L4: for (int sims = 0;  sims < Sims; sims++) {
#pragma HLS UNROLL
						if (sims==0) {
							temp_reg[s] = 0;
							temp_cnt[s] = 0;
						}
						bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
						lfsr = (lfsr >> 1) | (bit << 31);
						random_int = lfsr & 0x7fffffff;
						if ((image[i][j] * intensity) > (random_int)) {
							flag = true;
							temp_reg[s] = i*Side1+j;
							temp_cnt[s] += 1;
						}
					}
					accum_cnt +=  temp_cnt[s];
					if ((s==(ASims-1)) && flag) {
						spike_reg[*spike_input] = temp_reg[s];
						spike_cnt[*spike_input] = accum_cnt;
						(*spike_input)++;
					}
				}
			}
		}
	}
}


void poisson(i_dt image[Maps1][Side1][Side1], int intensity, sr_dt spike_reg[MapsMax][MaxNeurons],
		sc_dt spike_cnt[MapsMax][MaxNeurons], sr_dt spike_input[MapsMax], w_dt sf) {
#pragma HLS INLINE off

	bool flag = false;

    L1: for (int map = 0; map < Maps1; map++) {
#pragma HLS UNROLL factor=IMaps
    	poisson_pe(image[map], intensity, spike_reg[map],
    			spike_cnt[map], &spike_input[map], sf);
    }
}

void encoding_pe (int map, q_dt vm[SideMax][SideMax], sr_dt local_spike_reg[MaxNeurons], sc_dt local_spike_cnt[MaxNeurons],
		sr_dt *local_pointer, int OutSide, w_dt sf) {
#pragma HLS INLINE off
	N2: for (int y = 0; y < OutSide; y++) {
		N3: for (int x = 0; x < OutSide; x++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm inter false
			if (vm[y][x] >= vth){
				local_spike_reg[*local_pointer] = map*OutSide*OutSide + y*OutSide + x;
				local_spike_cnt[*local_pointer] = vm[y][x]*sf;
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
		int OutSide, int OutMaps, w_dt sf) {
#pragma HLS INLINE off

	N1: for (int map = 0; map < OutMaps; map++) {
#pragma HLS UNROLL factor=EMaps
		encoding_pe(map, vm[map], spike_reg[map], spike_cnt[map],
				&spike[map], OutSide, sf);
    }
}

void neuron_fc(q2_dt vm_fc[MaxFc], sr_dt spike_reg[MapsMax][MaxNeurons],sc_dt spike_cnt[MapsMax][MaxNeurons], sr_dt spike[MapsMax], w_dt sf) {
#pragma HLS INLINE off
	*spike = 0;
	NF: for (int i=0; i<MaxFc; i++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm_fc inter false
		if (vm_fc[i] >= vth) {
			spike_reg[0][spike[0]] = i;
			spike_cnt[0][spike[0]] = vm_fc[i]* sf;
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


void conv_pe(int map, q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input, sr_dt spike_reg[MaxNeurons],
		sc_dt spike_cnt[MaxNeurons], w_dt kernel[MapsMax][KSide][KSide], int Side, int OutSide, int OutMaps) {
#pragma HLS INLINE OFF
    int pre_x, pre_y;
    int oy, ox;
    sc_dt temp_cnt;
	ca1: for (int spiker = 0; spiker < spike_input; spiker++){
#pragma HLS loop_tripcount min=40 max=40
		ca2: for (int ky = 0; ky < KSide; ky++) {
			ca3: for (int kx = 0; kx < KSide; kx++) {
				#pragma HLS PIPELINE
				#pragma HLS DEPENDENCE variable=vm inter false
            	pre_y = (spike_reg[spiker] % (Side*Side)) / Side;
            	pre_x = (spike_reg[spiker] % (Side*Side)) % Side;
				oy = (pre_y + PaddingC - ky) / StrideC;
				ox = (pre_x + PaddingC - kx) / StrideC;
				temp_cnt = spike_cnt[spiker];
				ca4: for (int u = 0; u < Maps; u++) {
				#pragma HLS UNROLL
					if ((ox >= 0) & (ox < OutSide) & (oy >= 0) & (oy < OutSide) & (u<OutMaps)) {
						vm[map*Maps+u][oy][ox] += (kernel[map*Maps+u][ky][kx]*temp_cnt);
					}
				}
			}
		}
	}
}

void conv_spike_aggregation(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons],
		sc_dt spike_cnt[MapsMax][MaxNeurons], w_dt kernel[MapsMax][MapsMax][KSide][KSide], int Side, int InMaps, int OutSide, int OutMaps) {
#pragma HLS INLINE off
	int TMaps = Maps;
	if (OutMaps < Maps) {
		TMaps = OutMaps;
	}

	int TIMaps = CIMaps;
	if (InMaps < CIMaps) {
		TIMaps = InMaps;
	}

	q_dt temp_vm = 0;

	static q_dt local_vm[CIMaps][MapsMax][SideMax][SideMax];
#pragma HLS ARRAY_PARTITION variable=local_vm cyclic factor=CIMaps dim=1
#pragma HLS ARRAY_PARTITION variable=local_vm cyclic factor=Maps dim=2

    cs1: for (int map = 0; map < OutMaps/TMaps; map++) {
    	cs2: for (int imap = 0; imap < InMaps/TIMaps; imap++) {
    		cs3: for (int ui=0; ui<CIMaps; ui++) {
#pragma HLS UNROLL
    			int temp = imap*CIMaps+ui;
    			conv_pe(map, local_vm[ui], spike_input[temp], spike_reg[temp],
					spike_cnt[temp], kernel[temp], Side, OutSide, OutMaps);
    		}
    	}
    }

    accum1: for (int map = 0; map < OutMaps/TMaps; map++) {
		accum2: for (int y = 0; y < OutSide; y++) {
			accum3: for (int x = 0; x < OutSide; x++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm inter false
#pragma HLS DEPENDENCE variable=local_vm inter false
				accum4: for (int u = 0; u < Maps; u++) {
#pragma HLS UNROLL
					temp_vm = 0;
					accum5: for (int ui=0; ui<CIMaps; ui++) {
#pragma HLS UNROLL
						if (ui < InMaps) {
							temp_vm += local_vm[ui][map*Maps+u][y][x];
							local_vm[ui][map*Maps+u][y][x] = 0;
						}
					}
					vm[map*Maps+u][y][x] = temp_vm;
				}
			}
		}
	}
}

void pool_pe(q_dt vm[SideMax][SideMax], sr_dt *spiked, sr_dt spike_reg[MaxNeurons], sc_dt spike_cnt[MaxNeurons],
		int Side, int OutSide) {
#pragma HLS INLINE off
	int pre_x, pre_y;
    int oy, ox, ky, kx;
	ps_compute: for (int spiker = 0; spiker < *spiked; spiker++){
#pragma HLS PIPELINE
		pre_y = (spike_reg[spiker] % (Side*Side)) / Side;
		pre_x = (spike_reg[spiker] % (Side*Side)) % Side;
		ky = pre_y % StrideP;
		kx = pre_x % StrideP;
		oy = (pre_y + PaddingP - ky) / StrideP;
		ox = (pre_x + PaddingP - kx) / StrideP;
		if ((ox >= 0) & (ox < OutSide) & (oy >= 0) & (oy < OutSide))
			vm[oy][ox] += q_dt(scale)*spike_cnt[spiker];
	}
}


void pool_spike_aggregation(q_dt vm[MapsMax][SideMax][SideMax], sr_dt spike_input[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons],
		int Side, int InMaps, int OutSide){
#pragma HLS INLINE off
	ps_1: for (int imap = 0; imap < InMaps; imap++) {
#pragma HLS UNROLL factor=PMaps
		pool_pe(vm[imap], &spike_input[imap], spike_reg[imap], spike_cnt[imap], Side, OutSide);
	}
}

#ifndef __SDSCC__
void fc_spike_aggregation(q_dt vm[MaxFc], sr_dt spiked[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons],
                         w_dt w_off[MaxRows*MaxCols], w_dt weight[BufRows+1][MaxCols], int InMaps, int ARows, int ACols){
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
			if (pre>=BufRows) {
				temp = BufRows;
				FL2: for (int w = 0; w < ACols; w++) {
#pragma HLS PIPELINE
					weight[temp][w] = w_off[pre*ACols+w];
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

void compute (int index, w_dt weight_buf[MaxFc], q2_dt vm[MaxFc], int ACols, sc_dt spike_cnt[MaxNeurons]) {
#pragma HLS INLINE off
	if (index >= 0) {
    FC1: for (int post = 0; post < ACols/FUnroll; post++)
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=vm inter false
    	FC2: for (int u=0; u < FUnroll; u++) {
    		vm[post*FUnroll+u] += weight_buf[post*FUnroll+u]*spike_cnt[index];
    	}
	}
}

void fc_spike_aggregation(q2_dt vm[MaxFc], sr_dt spiked[MapsMax], sr_dt spike_reg[MapsMax][MaxNeurons], sc_dt spike_cnt[MapsMax][MaxNeurons],
	                         IW w_off[MaxRows*MaxCols/WIDTH_FACTOR], w_dt weight[BufRows+1][MaxCols], int InMaps, int ARows, int ACols){
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
		    		compute(spiker-1, weight_buf_b, vm, ACols, spike_cnt[imap]);
		    	}
		    	else {
		    		load(spiker, spiked[imap], w_off, weight_buf_b, spike_reg[imap], ACols);
		    		compute(spiker-1, weight_buf_a, vm, ACols, spike_cnt[imap]);
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

void max_vm(int *vm_max, q_dt vm_conv[MapsMax][SideMax][SideMax]) {
    for (int map = 0; map < MapsMax; map++) {
            for (int x=0; x<SideMax; x++) {
                    for(int y=0; y<SideMax; y++) {
                    	if(*vm_max < vm_conv[map][x][y]) {
                    		*vm_max = vm_conv[map][x][y];
                    	}
                    }
            }
    }
}

void max_vm2(int *vm_max, q_dt vm_fc[MaxFc]) {
    for (int map = 0; map < MaxFc; map++) {
        if(*vm_max < vm_fc[map]) {
           *vm_max = vm_fc[map];
        }
    }
}
#endif

#ifdef __SDSCC__
#pragma SDS data zero_copy(image_in, k_1, k_2, w_1, w_2)
#pragma SDS data mem_attribute(image_in:PHYSICAL_CONTIGUOUS|NON_CACHEABLE, k_1:PHYSICAL_CONTIGUOUS|NON_CACHEABLE, k_2:PHYSICAL_CONTIGUOUS|NON_CACHEABLE, w_1:PHYSICAL_CONTIGUOUS|NON_CACHEABLE, w_2:PHYSICAL_CONTIGUOUS|NON_CACHEABLE)
void network(i_dt image_in[Sources], int intensity,
	        int *dom, IW k_1[MapsMax*MapsMax/WIDTH_FACTOR*KSide*KSide],
			IW k_2[MapsMax*MapsMax/WIDTH_FACTOR*KSide*KSide], IW w_1[MaxRows*MaxCols/WIDTH_FACTOR],
			IW w_2[MaxRows*MaxCols/WIDTH_FACTOR], bool flag) {
#else
void network(i_dt image_in[Sources], int intensity,
		        int *dom, w_dt k_1[MapsMax*MapsMax*KSide*KSide],
				w_dt k_2[MapsMax*MapsMax*KSide*KSide], w_dt w_1[MaxRows*MaxCols],
				w_dt w_2[MaxRows*MaxCols], bool flag, sc_dt spike_cnt_max[NumLayers], int vm_max[NumLayers],
				double spike_average[NumLayers], int spike_max[NumLayers]) {
#endif
    static w_dt kernel_1[MapsMax][MapsMax][KSide][KSide];
#pragma HLS ARRAY_PARTITION variable=kernel_1 cyclic factor=CIMaps dim=1
#pragma HLS ARRAY_PARTITION variable=kernel_1 cyclic factor=Maps dim=2

    static w_dt kernel_2[MapsMax][MapsMax][KSide][KSide];
#pragma HLS ARRAY_PARTITION variable=kernel_2 cyclic factor=CIMaps dim=1
#pragma HLS ARRAY_PARTITION variable=kernel_2 cyclic factor=Maps dim=2

    static w_dt weight[BufRows+1][MaxCols];
#pragma HLS RESOURCE variable=weight core=XPM_MEMORY uram
#pragma HLS ARRAY_RESHAPE variable=weights cyclic factor=FPart dim=2
//#pragma HLS ARRAY_PARTITION variable=weights cyclic factor=FUnroll dim=2

    static w_dt weights_out[Layer2][Output];
#pragma HLS RESOURCE variable=weights_out core=XPM_MEMORY uram

    i_dt image[Maps1][Side1][Side1];
#pragma HLS RESOURCE variable=image core=XPM_MEMORY uram
#pragma HLS ARRAY_PARTITION variable=image cyclic factor=IMaps

    sc_dt spike_count[Output];

    q_dt vm_conv[MapsMax][SideMax][SideMax];
#pragma HLS ARRAY_PARTITION variable=vm_conv_1 cyclic factor=Maps dim=1
    q2_dt vm_fc[MaxFc];
#pragma HLS RESOURCE variable=vm_fc core=XPM_MEMORY uram
#pragma HLS ARRAY_PARTITION variable=vm_fc cyclic factor=FUnroll


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
    	for (int i = 0; i < BufRows; i++) {
    	   for (int j = 0; j<Layer2/WIDTH_FACTOR; j++) {
#pragma HLS PIPELINE
    		   IW temp = w_1[i*(Layer2/WIDTH_FACTOR)+j];
    		   for (int x=0; x< WIDTH_FACTOR; x++) {
#pragma HLS UNROLL
    			   unsigned int range_idx = x * WWIDTH;
    			   WW tmp_int = temp.range(range_idx + WWIDTH - 1, range_idx);
    			   w_dt tmp = *((w_dt*)(&tmp_int));
    			   weight[i][j*WIDTH_FACTOR+x] = tmp;
    		   }
    	   }
    	}

    	for (int i=0; i < Layer2/WIDTH_FACTOR; i++) {
    		for (int j=0; j<Output; j++) {
#pragma HLS PIPELINE
    			IW temp = w_2[i*Output+j];
    			for (int x=0; x<WIDTH_FACTOR; x++) {
#pragma HLS UNROLL
    			   unsigned int range_idx = x * WWIDTH;
    			   WW tmp_int = temp.range(range_idx + WWIDTH - 1, range_idx);
    			   w_dt tmp = *((w_dt*)(&tmp_int));
    			   weights_out[i*WIDTH_FACTOR+x][j] = tmp;
    		   }
    	   }
    	}

        load_kernel_hw(kernel_1, k_1, Maps2, Maps1);
        load_kernel_hw(kernel_2, k_2, Maps4, Maps3);
#else
    	for (int i = 0; i < BufRows; i++) {
    	   for (int j = 0; j<Layer2; j++) {
#pragma HLS PIPELINE
    	      	weight[i][j] = w_1[i*Layer2+j];
    	   }
    	}
        for (int i = 0; i < Layer2; i++) {
        	for (int j = 0; j<Output; j++) {
#pragma HLS PIPELINE
        		weights_out[i][j] = w_2[i*Output+j];
        	}
        }
        load_kernel(kernel_1, k_1, Maps2, Maps1);
        load_kernel(kernel_2, k_2, Maps4, Maps3);
#endif
    }
    else {
#ifdef __SDSCC__
 		*dom = 0;
		image_copy(image, image_in);
		reset_sc(spike_count);
		reset_spike_pointer(spike_input);
		poisson(image, intensity, spike_reg, spike_cnt, spike_input, sf1);
		conv_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, kernel_1, Side1, Maps1, Side2, Maps2);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side2, Maps2, sf2);
		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side2, Maps2, Side3);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side3, Maps3, sf3);
		conv_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2, Side3, Maps3, Side4, Maps4);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side4, Maps4, sf4);
		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side4, Maps4, Side5);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side5, Maps5, sf5);
		fc_spike_aggregation(vm_fc, spike_input, spike_reg, spike_cnt, w_1, weight, Maps5, Layer1, Layer2);
		reset_spike_pointer(spike_input);
		neuron_fc(vm_fc, spike_reg, spike_cnt, spike_input, sf6);
		output_spike_aggregation(vm_fc, spike_input, spike_reg, spike_cnt, weights_out);
		neuron_output(vm_fc, spike_count, dom);
#else
 		*dom = 0;
		image_copy(image, image_in);
		reset_sc(spike_count);
		reset_spike_pointer(spike_input);
		poisson(image, intensity, spike_reg, spike_cnt, spike_input, sf1);;
		max_measurement(&spike_average[0], &spike_max[0], &spike_cnt_max[0], spike_cnt, spike_input, Maps1);
		conv_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, kernel_1, Side1, Maps1, Side2, Maps2);
		max_vm(&vm_max[0], vm_conv);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side2, Maps2, sf2);
		max_measurement(&spike_average[1], &spike_max[1], &spike_cnt_max[1], spike_cnt, spike_input, Maps2);
		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side2, Maps2, Side3);
		max_vm(&vm_max[1], vm_conv);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side3, Maps3, sf3);
		max_measurement(&spike_average[2], &spike_max[2], &spike_cnt_max[2], spike_cnt, spike_input, Maps3);
		conv_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, kernel_2, Side3, Maps3, Side4, Maps4);
		max_vm(&vm_max[2], vm_conv);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side4, Maps4, sf4);
		max_measurement(&spike_average[3], &spike_max[3], &spike_cnt_max[3], spike_cnt, spike_input, Maps4);
		pool_spike_aggregation(vm_conv, spike_input, spike_reg, spike_cnt, Side4, Maps4, Side5);
		max_vm(&vm_max[3], vm_conv);
		reset_spike_pointer(spike_input);
		neuron_conv(vm_conv, spike_reg, spike_cnt, spike_input, Side5, Maps5, sf5);
		max_measurement(&spike_average[4], &spike_max[4], &spike_cnt_max[4], spike_cnt, spike_input, Maps5);
		fc_spike_aggregation(vm_fc, spike_input, spike_reg, spike_cnt, w_1, weight, Maps5, Layer1, Layer2);
		max_vm2(&vm_max[4], vm_fc);
		reset_spike_pointer(spike_input);
		neuron_fc(vm_fc, spike_reg, spike_cnt, spike_input, sf6);
		max_measurement(&spike_average[5], &spike_max[5], &spike_cnt_max[5], spike_cnt, spike_input, 1);
		output_spike_aggregation(vm_fc, spike_input, spike_reg, spike_cnt, weights_out);
		max_vm2(&vm_max[5], vm_fc);
		neuron_output(vm_fc, spike_count, dom);
		max_measurement_2(&spike_cnt_max[6], spike_count);
#endif
    }
}
