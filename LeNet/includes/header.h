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


// MNIST_S Network
// #include "mnist_1.h"

// // MNIST_L Network
#include "mnist_2.h"

// // SVHN Network 
// #include "svhn_1.h"


#ifdef __SDSCC__
	void network(i_dt image_in[Sources], int intensity,
	        int *dom, IW k_1[MapsMax*MapsMax/WIDTH_FACTOR*KSide*KSide],
			IW k_2[MapsMax*MapsMax/WIDTH_FACTOR*KSide*KSide], IW w_1[MaxRows*MaxCols/WIDTH_FACTOR],
			IW w_2[MaxRows*MaxCols/WIDTH_FACTOR], bool flag);
#else
	void network(i_dt image_in[Sources], int intensity,
			        int *dom, w_dt k_1[MapsMax*MapsMax*KSide*KSide],
					w_dt k_2[MapsMax*MapsMax*KSide*KSide], w_dt w_1[MaxRows*MaxCols],
					w_dt w_2[MaxRows*MaxCols], bool flag, sc_dt spike_cnt_max[NumLayers], int vm_max[NumLayers],
					double spike_average[NumLayers], int spike_max[NumLayers]);
#endif

#endif //SCNN_HEADER_H
