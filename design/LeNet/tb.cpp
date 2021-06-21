#include "includes/header.h"
#include "includes/neuron_labels.h"

//MNIST Networks
// Lenet-S 16 Bits
// #include "includes/mnist_1/16/kernel_1.h"
// #include "includes/mnist_1/16/kernel_2.h"
// #include "includes/mnist_1/16/weights.h"
// #include "includes/mnist_1/16/weights1.h"


// Lenet-S 8 Bits
// #include "includes/mnist_1/8/kernel_1.h"
// #include "includes/mnist_1/8/kernel_2.h"
// #include "includes/mnist_1/8/weights.h"
// #include "includes/mnist_1/8/weights1.h"


// Lenet-S 4 Bits
// #include "includes/mnist_1/4/kernel_1.h"
// #include "includes/mnist_1/4/kernel_2.h"
// #include "includes/mnist_1/4/weights.h"
// #include "includes/mnist_1/4/weights1.h"


// Lenet-L 16 Bits
// #include "includes/mnist_2/16/kernel_1.h"
// #include "includes/mnist_2/16/kernel_2.h"
// #include "includes/mnist_2/16/weights.h"
// #include "includes/mnist_2/16/weights1.h"


// Lenet-L 8 Bits
// #include "includes/mnist_2/8/kernel_1.h"
// #include "includes/mnist_2/8/kernel_2.h"
// #include "includes/mnist_2/8/weights.h"
// #include "includes/mnist_2/8/weights1.h"

// Lenet-L 4 Bits
#include "includes/mnist_2/4/kernel_1.h"
#include "includes/mnist_2/4/kernel_2.h"
#include "includes/mnist_2/4/weights.h"
#include "includes/mnist_2/4/weights1.h"


// SVHN

// 16Bits
// #include "includes/svhn_1/16/kernel_1.h"
// #include "includes/svhn_1/16/kernel_2.h"
// #include "includes/svhn_1/16/weights.h"
// #include "includes/svhn_1/16/weights1.h"


// 8Bits
// #include "includes/svhn_1/8/kernel_1.h"
// #include "includes/svhn_1/8/kernel_2.h"
// #include "includes/svhn_1/8/weights.h"
// #include "includes/svhn_1/8/weights1.h"


// 4Bits
// #include "includes/svhn_1/4/kernel_1.h"
// #include "includes/svhn_1/4/kernel_2.h"
// #include "includes/svhn_1/4/weights.h"
// #include "includes/svhn_1/4/weights1.h"


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

    static w_dt* k1 = (w_dt*)sds_alloc_non_cacheable(sizeof(w_dt) * MapsMax*MapsMax*KSide*KSide);
    static w_dt* k2 = (w_dt*)sds_alloc_non_cacheable(sizeof(w_dt) * MapsMax*MapsMax*KSide*KSide);
    static w_dt* w1 = (w_dt*)sds_alloc_non_cacheable(sizeof (w_dt) * MaxRows * MaxCols);
    static w_dt* w2 = (w_dt*)sds_alloc_non_cacheable(sizeof(w_dt) * MaxRows * MaxCols);

    for (int i=0; i<Maps2; i++)
    	for(int j=0; j<Maps1; j++)
    		for(int k=0; k<KSide; k++)
    			for(int l=0; l<KSide; l++)
    				k1[(j*Maps2*KSide*KSide)+(i*KSide*KSide)+(k*KSide)+l] = kernel_1[i][j][k][l];

    for (int i=0; i<Maps4; i++)
    	for(int j=0; j<Maps3; j++)
    		for(int k=0; k<KSide; k++)
    			for(int l=0; l<KSide; l++)
    				k2[(j*Maps4*KSide*KSide)+(i*KSide*KSide)+(k*KSide)+l] = kernel_2[i][j][k][l];

    for(int i = 0; i < Layer1; i++)
    	for(int j = 0; j < Layer2; j++)
    		w1[(i*Layer2)+j] = weights[i][j];


    for(int i = 0; i < Layer2; i++)
    	for(int j = 0; j < Output; j++) {
    		w2[(i*Output)+j] = weights1[i][j];
    }

#ifdef __SDSCC__
    static IW* kk1 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MapsMax* MapsMax/WIDTH_FACTOR* KSide * KSide);
    static IW* kk2 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MapsMax* MapsMax/WIDTH_FACTOR* KSide * KSide);
    static IW* ww1 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MaxRows * MaxCols/WIDTH_FACTOR);
    static IW* ww2 = (IW*)sds_alloc_non_cacheable(sizeof(IW) * MaxRows * MaxCols/WIDTH_FACTOR);

    for (int i=0; i<Maps1; i++) {
    	for(int j=0; j<Maps2/WIDTH_FACTOR; j++) {
    		for (int w=0; w<WIDTH_FACTOR; w++) {
    			for(int k=0; k<KSide; k++) {
    				for(int l=0; l<KSide; l++) {
						unsigned int range_idx = w * WWIDTH;
						w_dt tmp = k1[(i*Maps2*KSide*KSide)+((j*WIDTH_FACTOR+w)*KSide*KSide)+(k*KSide)+l];
						kk1[(i*(Maps2/WIDTH_FACTOR)*KSide*KSide)+(j*KSide*KSide)+(k*KSide)+l].range(range_idx + WWIDTH - 1, range_idx) =  *((WW*)(&tmp));
    				}
    			}
    		}
    	}
    }

    for (int i=0; i<Maps3; i++) {
    	for(int j=0; j<Maps4/WIDTH_FACTOR; j++) {
    		for (int w=0; w<WIDTH_FACTOR; w++) {
    			for(int k=0; k<KSide; k++) {
    				for(int l=0; l<KSide; l++) {
						unsigned int range_idx = w * WWIDTH;
						w_dt tmp = k2[(i*Maps4*KSide*KSide)+((j*WIDTH_FACTOR+w)*KSide*KSide)+(k*KSide)+l];
						kk2[(i*(Maps4/WIDTH_FACTOR)*KSide*KSide)+(j*KSide*KSide)+(k*KSide)+l].range(range_idx + WWIDTH - 1, range_idx) =  *((WW*)(&tmp));
    				}
    			}
    		}
    	}
    }


    for(int i = 0; i < Layer1; i++) {
    	for(int j = 0; j < Layer2/WIDTH_FACTOR; j++) {
    		for (int w=0; w<WIDTH_FACTOR; w++) {
    			unsigned int range_idx = w * WWIDTH;
    			w_dt tmp = w1[(i*Layer2)+(j*WIDTH_FACTOR+w)];
    			ww1[(i*(Layer2/WIDTH_FACTOR))+j].range(range_idx + WWIDTH - 1, range_idx) = *((WW*)(&tmp));
    		}
    	}
    }

    for(int i = 0; i < Layer2/WIDTH_FACTOR; i++) {
    	for (int w=0; w<WIDTH_FACTOR; w++) {
    		for(int j = 0; j < Output; j++) {
    			unsigned int range_idx = w * WWIDTH;
    			w_dt tmp = w2[((i*WIDTH_FACTOR+w)*Output)+j];
    			ww2[(i*Output)+j].range(range_idx + WWIDTH - 1, range_idx) = *((WW*)(&tmp));
    		}
    	}
    }
#endif

    string line;


    ifstream images("includes/dataset_mnist/testing_10k_images.txt");
    ifstream labels("includes/dataset_mnist/testing_10k_labels.txt");


   // ifstream images("includes/dataset_svhn/images.txt");
   // ifstream labels("includes/dataset_svhn/labels.txt");


    int TEST_CASES;

    TEST_CASES = atoi(argv[1]);

    int dom = 0;

    bool flag = false;

#ifndef __SDSCC__
    static sc_dt spike_cnt_max[NumLayers];
    static int vm_max[NumLayers];
    static double spike_average[NumLayers];
    static int spike_max[NumLayers];
    network(image, intensity, &dom, k1, k2, w1, w2, flag, spike_cnt_max, vm_max, spike_average, spike_max);
    for (int i=0; i<NumLayers; i++) {
    	spike_cnt_max[i]= 0;
    	vm_max[i] = 0;
    	spike_average[i] = 0;
    	spike_max[i] = 0;
    }
#else
    pf_cycles.start();
    network(image, intensity, &dom, kk1, kk2, ww1, ww2, flag);
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
        network(image, intensity, &dom, kk1, kk2, ww1, ww2, flag);
		hw_cycles.stop();
#else
		network(image, intensity, &dom, k1, k2, w1, w2, flag, spike_cnt_max, vm_max, spike_average, spike_max);
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
    	cout << endl;
    	cout << " Layer - " << i << ", Max Count - " << spike_cnt_max[i] << ", Vm Max - " << vm_max[i] << endl;
    	cout << " Spike Max Count - " << spike_max[i] << ", Spike Average Count - " << spike_average[i]/TEST_CASES << endl;
    }
#endif
    images.close();
    labels.close();

    return 0;
}
