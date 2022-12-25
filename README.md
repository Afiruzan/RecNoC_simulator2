
3D Reconfigurable Cluster-based Network-on-Chip Simulator
-------------------------------------------

This project is a C++ code written by MS Visual Studio 2015 (Version 14.0.25123.00 Update2) (in win32 console application mode) which can simulate a 3-dimensional Cluster-based Reconfigurable Network-on-Chip.

This project is under construction, so additional details will be presented soon in this file. Best way for study simulation method is studying the code of simulator. The main file of simulator code is in below path:

RecNoC_simulator2/RecNoC_simulator2.cpp

For beginning, if you want to obtain more information about a Cluster-based Reconfigurable Network-on-Chip please study below paper:

M. Modarressi and H. Sarbazi-Azad, "Reconfigurable Cluster-Based Networks-on-Chip for Application-Specific MPSoCs," 2012 IEEE 23rd International Conference on Application-Specific Systems, Architectures and Processors, Delft, 2012, pp. 153-156.

link of above paper on ieeexplore:
http://ieeexplore.ieee.org/abstract/document/6341466/

Convolutional Neural Networks (CNNs) have a wide range of applications due to their superior performance in image and pattern classification. However, the performance of CNNs comes at the price of high computational load and memory bandwidth usage. Hardware acceleration has become the primary way to tackle this ever-increasing complexity of CNNs. Most of the recent accelerators arrange processing units (PEs) as a many-core accelerator architecture, with the inter-PE connections tailored to the specific dataflow of the CNN layers. The performance of such accelerators is maximized if the input feature map and filter size/dimension matches that of the underlying accelerator. However, current fixed-size accelerator structures lead to sever resource underutilization because the same structure is used to compute CNN layers of varying dimensions. In this paper, we tackle this problem by presenting RC-CNN, a reconfigurable accelerator architecture for CNNs that can adapt the structure of accelerator to the size and dataflow pattern of the running CNN layer. RC-CNN relies on a reconfigurable on-chip interconnection fabric that can organize a sub-set of accelerator’s PEs as a PE set with the same size/dimension of the target CNN layer and customize the inter-PE connections for the layer’s dataflow pattern. Since the area/energy overhead does not justify using a full-fledged packet-switched network in accelerators with fine-grained PEs, we use a reconfigurable network with very simple switches in order to efficiently implement the dynamic reconfiguration capability for many-core fine-grained CNN accelerators. Experimental results show that, based on the CNN size and accelerator structure, RC-CNN yields 37% higher PE utilization over a baseline design, on average. It also improves the PE utilization of the state-of-the-art CNN accelerators we selected for comparison purpose by 18%, on average. The results show that these improvements translate to 9%–41% increase in the accelerator’s throughput. Further, RC-CNN reduces the network latency and energy consumption by 28% and 22%, respectively, compared to the state-of-the-art utilization-aware methods that employ packet-switched networks-on-chip.

for more information you can read below paper:
--------------------------------------------
Arash Firuzan, Mehdi Modarressi, Midia Reshadi, Ahmad Khademzadeh,
Reconfigurable Network-on-Chip based Convolutional Neural Network Accelerator,
Journal of Systems Architecture,
Volume 129,
2022,
102567,
ISSN 1383-7621,
https://doi.org/10.1016/j.sysarc.2022.102567.
(https://www.sciencedirect.com/science/article/pii/S1383762122001175)
--------------------------------------------

Your comments on this simulator can be help us for fixing the probable problems.
Thanks for your attention.

please send us your feedback.
