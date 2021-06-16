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
      * Xilinx Zed Board

2. **Software tools:**
    * **HLS tool:**
      * Xilinx SDSoC 2019.1



