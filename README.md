## Introduction

SyncNN is the framework for Synchronous fashion Spiking Neural Networks acceleration on Xilinx SoC FPGA boards. The framework is scalable to run any deep network on any given FPGA board. The code base has three widely used image classification networks: LeNet, Network in Network (NiN), and VGG-13. The networks have been evaluated on MNIST, CIFAR-10 and SVHN datasets.

If you use SyncNN in your research, please cite our FPL'21 paper: 
> Sathish Panchapakesan, Zhenman Fang, Jian Li. SyncNN: Evaluating and Accelerating Spiking Neural Networks on FPGAs.  International Conference on Field-Programmable Logic and Applications (FPL). September 2021.

## Download SyncNN

	git clone https://github.com/SFU-HiAccel/SyncNN.git

## Setup Requirements

1. **Evaluated hardware platforms:**
    * **Xilinx SoC FPGA:**
      * Xilinx ZCU 102 Board
      * Xilinx ZCU 104 Board
      * Xilinx ZED Board

2. **Software tools:**
    * **HLS tool:**
      * Xilinx SDSoC 2019.1

## Accelerate SNN Algorithm using SyncNN

Assuming you are in the project home directory of the checked out SyncNN github repo.

1. Train Networks
    * Change directory to `train/`
    * Training is done using Keras.
    * While training CNNs, we have some conditions to realize them in SNN for evaluation
      * Bias Less,
      * ReLu as activation factor,
      * No Batch Normilization layers,
      * Use Average Pooling instead of Max. Pooling
    * The train folder has three different networks.
      * **mnist_lenet.ipynb** - LeNet network for MNIST dataset
      * **cifar10_nin.ipynb** - NiN network for CIFAR-10 dataset
      * **cifar10_vgg.ipynb** - VGG13 network for CIFAR-10 dataset
    * After you train the networks, make sure to save the (.h5) file generated.    	
    
2. Convert to .h files
    * Change directory to `convert_h/`
    * This part involves the conversion of trained weights to .h files.
    * The convert_h folder has three different networks.
      * **lenet_convert_h.ipynb** - LeNet network 
      * **nin_convert_h.ipynb** - NiN network 
      * **vgg_convert_h.ipynb** - VGG13 network 
    * Load the '.h5 file' correspondingly for the choosen network. Change the name of the load_model to the choosen .h5 file.
    * The code would retrieve weights as .h files for every layer. Save them.


3. Quantize Networks


4. Configure SyncNN
    * Change directory to `<$CHIP-KNN_HOME>/scripts/`
    * Update KNN parameters:
      * **N** - # of points in search space,
      * **D** - # of data dimension,
      * **Dist** - distance metric,
      * **K** - # of nearest neighbors
    * Update FPGA platform specifications:
      * **FPGA_part_name** - FPGA part name used during Vivado_HLS synthesis
      * **num_SLR** - # of Super-Logic-Regions (SLRs) on the FPGA
      * **SLR_resource** - available resources on each SLR (FPGA dies)
      * **memory** - off-chip memory type on the FPGA
      * **num_memory_banks** - # of available memory banks on the FPGA
    * Update advanced design configurations (***optional**):
      * **singlePE_template_config** - list of single PE configurations based on port_width and buffer_size to explore 
      * **resource_limit** - upper limit scale for the FPGA resource utilization
      * **kernel_frequency** - frequency constraint when building hw kernels
    

5. Build SyncNN Design
    * Change directory to `<$CHIP-KNN_HOME>/scripts/gen_test`
    * Run: `make build TARGET=hw DEVICE=<FPGA platform>`
    * Output: 
      * host code: `knn`
      * kernel code: `build_dir.hw.<FPGA platform>/knn.xclbin`

6. Run SyncNN
    * Change directory to `<$CHIP-KNN_HOME>/scripts/gen_test`
    * Run: `make check TARGET=hw DEVICE=<FPGA platform>` or `./knn knn.xclbin`

Now you have completed the flow of the framework. Hack the code and have fun!

## Contacts

Still have further questions about SyncNN? Please contact:

* **Sathish Panchapakesan**, MASc Student

* HiAccel Lab, Simon Fraser University (SFU)

* Email: sathishp@sfu.ca 

* Website: http://www.sfu.ca/~sathishp/


