#ifndef SCNN_HEADER_H
#define SCNN_HEADER_H

#include<stdlib.h>
#include<iostream>
#include<fstream>
#include<string>
#include<cstring>
#include<cstdlib>
#include<cmath>
#include<time.h>
#ifdef __SDSCC__
#include "sds_lib.h"
#include "sds_utils.h"
#include "ap_fixed.h"
#include "ap_int.h"
#endif

using namespace std;

#include "cifar10_1.h"


#ifndef __SDSCC__
void network(i_dt image_in[Sources], int intensity,
		        int *dom, w_dt k_1[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_2[MapsMax*MapsMax*KSideMax*KSideMax], w_dt k_3[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_4[MapsMax*MapsMax*KSideMax*KSideMax], w_dt k_5[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_6[MapsMax*MapsMax*KSideMax*KSideMax], w_dt k_7[MapsMax*MapsMax*KSideMax*KSideMax],
				w_dt k_8[MapsMax*MapsMax*KSideMax*KSideMax], w_dt k_9[MapsMax*MapsMax*KSideMax*KSideMax],
				bool flag, sc_dt spike_cnt_max[NumLayers], int vm_max[NumLayers],
				double spike_average[NumLayers], int spike_max[NumLayers]);
#else
void network(i_dt image_in[Sources], int intensity,
	        int *dom, IW k_1[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW k_2[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			IW k_3[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW k_4[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			IW k_5[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW k_6[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			IW k_7[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], IW k_8[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax],
			w_dt k_9[MapsMax*MapsMax/WIDTH_FACTOR*KSideMax*KSideMax], bool flag);
#endif
#endif
