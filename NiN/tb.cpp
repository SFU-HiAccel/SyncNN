#include "includes/header.h"
#include "includes/neuron_labels.h"


//Cifar-10 NiN

#include "includes/cifar10_new/16/kernel_1.h"
#include "includes/cifar10_new/16/kernel_2.h"
#include "includes/cifar10_new/16/kernel_3.h"
#include "includes/cifar10_new/16/kernel_4.h"
#include "includes/cifar10_new/16/kernel_5.h"
#include "includes/cifar10_new/16/kernel_6.h"
#include "includes/cifar10_new/16/kernel_7.h"
#include "includes/cifar10_new/16/kernel_8.h"
#include "includes/cifar10_new/16/kernel_9.h"


//#include "includes/cifar10_new/8s/kernel_1.h"
//#include "includes/cifar10_new/8s/kernel_2.h"
//#include "includes/cifar10_new/8s/kernel_3.h"
//#include "includes/cifar10_new/8s/kernel_4.h"
//#include "includes/cifar10_new/8s/kernel_5.h"
//#include "includes/cifar10_new/8s/kernel_6.h"
//#include "includes/cifar10_new/8s/kernel_7.h"
//#include "includes/cifar10_new/8s/kernel_8.h"
//#include "includes/cifar10_new/8s/kernel_9.h"


#ifdef __SDSCC__
sds_utils::perf_counter hw_cycles;
sds_utils::perf_counter pf_cycles;
#else
#define sds_alloc_non_cacheable malloc
#endif


int main(int argc, char *argv[]) {
    if (argc < 2){
        cout << "Usage : <executable> <TEST_CASES>" << endl;
        exit(1);
    }

    srand(time(NULL));
    float correct = 0;
    int intensity = 2;
    int label;

    static i_dt* image = (i_dt*)sds_alloc_non_cacheable(sizeof(i_dt) * Sources);

    static w_dt* k1 = (w_dt*)sds_alloc_non_cacheable(sizeof(w_dt) * MapsMax*MapsMax*KSideMax*KSideMax);
    static w_dt* k2 = (w_dt*)sds_alloc_non_cacheable(sizeof(w_dt) * MapsMax*MapsMax*KSideMax*KSideMax);
    static w_dt* k3 = (w_dt*)sds_alloc_non_cacheable(sizeof(w_dt) * MapsMax*MapsMax*KSideMax*KSideMax);
    static w_dt* k4 = (w_dt*)sds_alloc_non_cacheable(sizeof(w_dt) * MapsMax*MapsMax*KSideMax*KSideMax);
    static w_dt* k5 = (w_dt*)sds_alloc_non_cacheable(sizeof(w_dt) * MapsMax*MapsMax*KSideMax*KSideMax);
    static w_dt* k6 = (w_dt*)sds_alloc_non_cacheable(sizeof(w_dt) * MapsMax*MapsMax*KSideMax*KSideMax);
    static w_dt* k7 = (w_dt*)sds_alloc_non_cacheable(sizeof(w_dt) * MapsMax*MapsMax*KSideMax*KSideMax);
    static w_dt* k8 = (w_dt*)sds_alloc_non_cacheable(sizeof(w_dt) * MapsMax*MapsMax*KSideMax*KSideMax);
    static w_dt* k9 = (w_dt*)sds_alloc_non_cacheable(sizeof(w_dt) * MapsMax*MapsMax*KSideMax*KSideMax);

    for (int i=0; i<Maps2; i++)
    	for(int j=0; j<Maps1; j++)
    		for(int k=0; k<KSide1; k++)
    			for(int l=0; l<KSide1; l++)
    				k1[(j*Maps2*KSide1*KSide1)+(i*KSide1*KSide1)+(k*KSide1)+l] = kernel_1[i][j][k][l];

    for (int i=0; i<Maps3; i++)
    	for(int j=0; j<Maps2; j++)
    		for(int k=0; k<KSide2; k++)
    			for(int l=0; l<KSide2; l++)
    				k2[(j*Maps3*KSide2*KSide2)+(i*KSide2*KSide2)+(k*KSide2)+l] = kernel_2[i][j][k][l];

    for (int i=0; i<Maps4; i++)
    	for(int j=0; j<Maps3; j++)
    		for(int k=0; k<KSide2; k++)
    			for(int l=0; l<KSide2; l++)
    				k3[(j*Maps4*KSide2*KSide2)+(i*KSide2*KSide2)+(k*KSide2)+l] = kernel_3[i][j][k][l];

    for (int i=0; i<Maps6; i++)
    	for(int j=0; j<Maps5; j++)
    		for(int k=0; k<KSide1; k++)
    			for(int l=0; l<KSide1; l++)
    				k4[(j*Maps6*KSide1*KSide1)+(i*KSide1*KSide1)+(k*KSide1)+l] = kernel_4[i][j][k][l];

    for (int i=0; i<Maps7; i++)
    	for(int j=0; j<Maps6; j++)
    		for(int k=0; k<KSide2; k++)
    			for(int l=0; l<KSide2; l++)
    				k5[(j*Maps7*KSide2*KSide2)+(i*KSide2*KSide2)+(k*KSide2)+l] = kernel_5[i][j][k][l];

    for (int i=0; i<Maps8; i++)
    	for(int j=0; j<Maps7; j++)
    		for(int k=0; k<KSide2; k++)
    			for(int l=0; l<KSide2; l++)
    				k6[(j*Maps8*KSide2*KSide2)+(i*KSide2*KSide2)+(k*KSide2)+l] = kernel_6[i][j][k][l];


    for (int i=0; i<Maps10; i++)
    	for(int j=0; j<Maps9; j++)
    		for(int k=0; k<KSide1; k++)
    			for(int l=0; l<KSide1; l++)
    				k7[(j*Maps10*KSide1*KSide1)+(i*KSide1*KSide1)+(k*KSide1)+l] = kernel_7[i][j][k][l];

    for (int i=0; i<Maps11; i++)
    	for(int j=0; j<Maps10; j++)
    		for(int k=0; k<KSide2; k++)
    			for(int l=0; l<KSide2; l++)
    				k8[(j*Maps11*KSide2*KSide2)+(i*KSide2*KSide2)+(k*KSide2)+l] = kernel_8[i][j][k][l];

    for (int i=0; i<Maps12; i++)
    	for(int j=0; j<Maps11; j++)
    		for(int k=0; k<KSide2; k++)
    			for(int l=0; l<KSide2; l++)
    				k9[(j*Maps12*KSide2*KSide2)+(i*KSide2*KSide2)+(k*KSide2)+l] = kernel_9[i][j][k][l];


#ifdef __SDSCC__
    static IW* kk1 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MapsMax* MapsMax/WIDTH_FACTOR* KSideMax * KSideMax);
    static IW* kk2 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MapsMax* MapsMax/WIDTH_FACTOR* KSideMax * KSideMax);
    static IW* kk3 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MapsMax* MapsMax/WIDTH_FACTOR* KSideMax * KSideMax);
    static IW* kk4 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MapsMax* MapsMax/WIDTH_FACTOR* KSideMax * KSideMax);
    static IW* kk5 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MapsMax* MapsMax/WIDTH_FACTOR* KSideMax * KSideMax);
    static IW* kk6 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MapsMax* MapsMax/WIDTH_FACTOR* KSideMax * KSideMax);
    static IW* kk7 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MapsMax* MapsMax/WIDTH_FACTOR* KSideMax * KSideMax);
    static IW* kk8 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MapsMax* MapsMax/WIDTH_FACTOR* KSideMax * KSideMax);
    static IW* kk9 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MapsMax* MapsMax/WIDTH_FACTOR* KSideMax * KSideMax);

    for (int i=0; i<Maps1; i++) {
    	for(int j=0; j<Maps2/WIDTH_FACTOR; j++) {
    		for (int w=0; w<WIDTH_FACTOR; w++) {
    			for(int k=0; k<KSide1; k++) {
    				for(int l=0; l<KSide1; l++) {
						unsigned int range_idx = w * WWIDTH;
						w_dt tmp = k1[(i*Maps2*KSide1*KSide1)+((j*WIDTH_FACTOR+w)*KSide1*KSide1)+(k*KSide1)+l];
						kk1[(i*(Maps2/WIDTH_FACTOR)*KSide1*KSide1)+(j*KSide1*KSide1)+(k*KSide1)+l].range(range_idx + WWIDTH - 1, range_idx) =  *((WW*)(&tmp));
    				}
    			}
    		}
    	}
    }

    for (int i=0; i<Maps2; i++) {
    	for(int j=0; j<Maps3/WIDTH_FACTOR; j++) {
    		for (int w=0; w<WIDTH_FACTOR; w++) {
    			for(int k=0; k<KSide2; k++) {
    				for(int l=0; l<KSide2; l++) {
						unsigned int range_idx = w * WWIDTH;
						w_dt tmp = k2[(i*Maps3*KSide2*KSide2)+((j*WIDTH_FACTOR+w)*KSide2*KSide2)+(k*KSide2)+l];
						kk2[(i*(Maps3/WIDTH_FACTOR)*KSide2*KSide2)+(j*KSide2*KSide2)+(k*KSide2)+l].range(range_idx + WWIDTH - 1, range_idx) =  *((WW*)(&tmp));
    				}
    			}
    		}
    	}
    }
    for (int i=0; i<Maps3; i++) {
    	for(int j=0; j<Maps4/WIDTH_FACTOR; j++) {
    		for (int w=0; w<WIDTH_FACTOR; w++) {
    			for(int k=0; k<KSide2; k++) {
    				for(int l=0; l<KSide2; l++) {
						unsigned int range_idx = w * WWIDTH;
						w_dt tmp = k3[(i*Maps4*KSide2*KSide2)+((j*WIDTH_FACTOR+w)*KSide2*KSide2)+(k*KSide2)+l];
						kk3[(i*(Maps4/WIDTH_FACTOR)*KSide2*KSide2)+(j*KSide2*KSide2)+(k*KSide2)+l].range(range_idx + WWIDTH - 1, range_idx) =  *((WW*)(&tmp));
    				}
    			}
    		}
    	}
    }
    for (int i=0; i<Maps5; i++) {
    	for(int j=0; j<Maps6/WIDTH_FACTOR; j++) {
    		for (int w=0; w<WIDTH_FACTOR; w++) {
    			for(int k=0; k<KSide1; k++) {
    				for(int l=0; l<KSide1; l++) {
						unsigned int range_idx = w * WWIDTH;
						w_dt tmp = k4[(i*Maps6*KSide1*KSide1)+((j*WIDTH_FACTOR+w)*KSide1*KSide1)+(k*KSide1)+l];
						kk4[(i*(Maps6/WIDTH_FACTOR)*KSide1*KSide1)+(j*KSide1*KSide1)+(k*KSide1)+l].range(range_idx + WWIDTH - 1, range_idx) =  *((WW*)(&tmp));
    				}
    			}
    		}
    	}
    }
    for (int i=0; i<Maps6; i++) {
    	for(int j=0; j<Maps7/WIDTH_FACTOR; j++) {
    		for (int w=0; w<WIDTH_FACTOR; w++) {
    			for(int k=0; k<KSide2; k++) {
    				for(int l=0; l<KSide2; l++) {
						unsigned int range_idx = w * WWIDTH;
						w_dt tmp = k5[(i*Maps7*KSide2*KSide2)+((j*WIDTH_FACTOR+w)*KSide2*KSide2)+(k*KSide2)+l];
						kk5[(i*(Maps7/WIDTH_FACTOR)*KSide2*KSide2)+(j*KSide2*KSide2)+(k*KSide2)+l].range(range_idx + WWIDTH - 1, range_idx) =  *((WW*)(&tmp));
    				}
    			}
    		}
    	}
    }
    for (int i=0; i<Maps7; i++) {
    	for(int j=0; j<Maps8/WIDTH_FACTOR; j++) {
    		for (int w=0; w<WIDTH_FACTOR; w++) {
    			for(int k=0; k<KSide2; k++) {
    				for(int l=0; l<KSide2; l++) {
						unsigned int range_idx = w * WWIDTH;
						w_dt tmp = k6[(i*Maps8*KSide2*KSide2)+((j*WIDTH_FACTOR+w)*KSide2*KSide2)+(k*KSide2)+l];
						kk6[(i*(Maps8/WIDTH_FACTOR)*KSide2*KSide2)+(j*KSide2*KSide2)+(k*KSide2)+l].range(range_idx + WWIDTH - 1, range_idx) =  *((WW*)(&tmp));
    				}
    			}
    		}
    	}
    }
    for (int i=0; i<Maps9; i++) {
    	for(int j=0; j<Maps10/WIDTH_FACTOR; j++) {
    		for (int w=0; w<WIDTH_FACTOR; w++) {
    			for(int k=0; k<KSide1; k++) {
    				for(int l=0; l<KSide1; l++) {
						unsigned int range_idx = w * WWIDTH;
						w_dt tmp = k7[(i*Maps10*KSide1*KSide1)+((j*WIDTH_FACTOR+w)*KSide1*KSide1)+(k*KSide1)+l];
						kk7[(i*(Maps10/WIDTH_FACTOR)*KSide1*KSide1)+(j*KSide1*KSide1)+(k*KSide1)+l].range(range_idx + WWIDTH - 1, range_idx) =  *((WW*)(&tmp));
    				}
    			}
    		}
    	}
    }
    for (int i=0; i<Maps10; i++) {
    	for(int j=0; j<Maps11/WIDTH_FACTOR; j++) {
    		for (int w=0; w<WIDTH_FACTOR; w++) {
    			for(int k=0; k<KSide2; k++) {
    				for(int l=0; l<KSide2; l++) {
						unsigned int range_idx = w * WWIDTH;
						w_dt tmp = k8[(i*Maps11*KSide2*KSide2)+((j*WIDTH_FACTOR+w)*KSide2*KSide2)+(k*KSide2)+l];
						kk8[(i*(Maps11/WIDTH_FACTOR)*KSide2*KSide2)+(j*KSide2*KSide2)+(k*KSide2)+l].range(range_idx + WWIDTH - 1, range_idx) =  *((WW*)(&tmp));
    				}
    			}
    		}
    	}
    }
#endif

    string line;

    ifstream images("includes/dataset_cifar10/testing_10k_images.txt");
    ifstream labels("includes/dataset_cifar10/testing_10k_labels.txt");

    int TEST_CASES;

    TEST_CASES = atoi(argv[1]);

    int dom = 0;

    bool flag = false;
    static sc_dt spike_cnt_max[NumLayers];
    static int vm_max[NumLayers];
    static double spike_average[NumLayers];
    static int spike_max[NumLayers];
    for (int i=0; i<NumLayers; i++) {
    	spike_cnt_max[i]= 0;
    	vm_max[i] = 0;
    	spike_average[i] = 0;
    	spike_max[i] = 0;
    }

#ifndef __SDSCC__
    network(image, intensity, &dom, k1, k2, k3, k4, k5, k6, k7, k8, k9, flag, spike_cnt_max, vm_max, spike_average, spike_max);
#else
    pf_cycles.start();
    network(image, intensity, &dom, kk1, kk2, kk3, kk4, kk5, kk6, kk7, kk8, k9,  flag);
    pf_cycles.stop();
#endif

    flag = true;

    for (int tcase = 0; tcase < TEST_CASES; tcase++) {
        if (images.is_open()) {
            for (int j = 0; j < Sources; j++) {
                getline(images, line);
                image[j] = stoi(line);
            }
        }
        if (labels.is_open()) {
            getline(labels, line);
            label = stoi(line);
        }

        intensity = 8421000;

#ifdef __SDSCC__
        hw_cycles.start();
        network(image, intensity, &dom, kk1, kk2, kk3, kk4, kk5, kk6, kk7, kk8, k9,  flag);
		hw_cycles.stop();
#else
		network(image, intensity, &dom, k1, k2, k3, k4, k5, k6, k7, k8, k9, flag, spike_cnt_max, vm_max, spike_average, spike_max);
#endif
        if (label == neuron_labels[dom])
            correct += 1.0;
    }
    std::cout << "Accuracy :" << correct * 100 / TEST_CASES << "%" << std::endl;
#ifdef __SDSCC__
    uint64_t hw_avg_cycles = hw_cycles.avg_cpu_cycles();
    std::cout << "Number of CPU cycles for Hardware operation: "
              << hw_avg_cycles << std::endl;
    uint64_t pf_avg_cycles = pf_cycles.avg_cpu_cycles()/TEST_CASES;
    std::cout << "Number of CPU cycles for Data Transfer operation: "
              << pf_avg_cycles << std::endl;
    std::cout << "Overall Number of CPU cycles per image: "
              << hw_avg_cycles+pf_avg_cycles << std::endl;
#else
    for (int i=0; i<NumLayers; i++) {
    	cout << " Layer - " << i << ", Max Count - " << spike_cnt_max[i] << " Spike Max Count - " << spike_max[i] << ", Spike Average Count - " << spike_average[i]/TEST_CASES << endl;
    }
#endif
    images.close();
    labels.close();

    return 0;
}
