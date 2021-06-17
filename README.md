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
    * **Training Network**
      * Google Collab or any Python compiler supporting Jupyter Notebook
    * **Software Version**
      * Any personal PC/IDE that support G++ compiler  

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
    * Change directory to `quantize/`
    * This part involves the reduction of network weight precision to lower number of  bits (16 bits, 8 bits and 4 bits).
    * The quantize folder has three different networks.
      * **lenet_quantize.ipynb** - LeNet network 
      * **nin_quantize.ipynb** - NiN network 
      * **vgg_quantize.ipynb** - VGG13 network
      The LeNet network supports 16, 8, and 4 bits. The NiN and VGG networks supports 16 and 8 bits. 
    * Load the '.h5 file' correspondingly for the choosen network. Change the name of the load_model to the choosen .h5 file.
    * Set the 'bits' variable with the number of bits needed for network weights. 
    * Set the 'p' variable with the percentile of weights to skip from both ends.
    *  The code would retrieve weights as .h files for every layer. Save them.


> Note: The following link contains pre-trained network (.h files) for MNIST network at 4 bits precision. 
  https://drive.google.com/drive/folders/1Ldh3FblFktVm4c3h7cyvigiBbBLZ9qhI?usp=sharing

4. Configure SyncNN
    * Change directory to `design/`
    * Choose the network folder that you wish to evaulate.
      * **LeNet** or **NiN** or **VGG**


	Network	      | Configuration
	------------- | -------------
	LeNet-S       | Input-32C3-P2-32C3-P2-256-Output
	LeNet-L       | Input-32C5-P2-64C5-P2-2048-Output
	NiN	      | Input-(192C5-192C1-192C1-P3)*2-192C5-192C1-10C1-GAP-Output
	VGG	      | Input-(64C3-64C3-P2)-(128C3-128C3-P2)-(256C3-256C3-P2)-(512C3-512C3-P2)*2-256-256-Output
    

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


