# High-ranking stereo matching algorithm
Found at Middlebury evaluation [benchmark](https://vision.middlebury.edu/stereo/eval3/).  Currently ranked in 19th place.  Execution time on KITTI data is about 1 second per frame.

Another stereo algorithm that seems accurate and does not require nvidia hardware is [LocalExpStereo](https://github.com/t-taniai/LocalExpStereo) [ranked #2].

# Segment-based Disparity Refinement with Occlusion Handling for Stereo Matching
Please cite the [[paper](https://ieeexplore.ieee.org/document/8661596)] if you find it useful
```
@ARTICLE{8661596, 
author={T. {Yan} and Y. {Gan} and Z. {Xia} and Q. {Zhao}}, 
journal={IEEE Transactions on Image Processing}, 
title={Segment-Based Disparity Refinement With Occlusion Handling for Stereo Matching}, 
year={2019}, 
volume={28}, 
number={8}, 
pages={3885-3897}, 
doi={10.1109/TIP.2019.2903318}, 
ISSN={1057-7149}, 
month={Aug},}
```
# Workflow
![SDR](workflow.png)

# Dependency
-OpenCV 3  
-Eigen

# Usage on Mac
```bash
# Install cmake
brew install cmake
# Modified CMakeLists.txt to point to brew's opencv
# set(OpenCV_DIR "/usr/local/Cellar/opencv@3/3.4.12/share/OpenCV")

# Install opencv3 into /usr/local/Cellar/opencv@3/3.4.12
brew install opencv@3

# Change the permissions to allow execution of the shell scripts
cd SDR
chmod +x demo.sh
chmod +x demo_KITTI.sh

# Compile and make code
cd SDR
mkdir build
cd build
cmake ..
make -j4

# Download test data
# The data can be obtained from  
#   https://drive.google.com/file/d/1W3Ea8SOGWC5xEXvEFiJWtU25RQFeza8M/view?usp=sharing
# The file structure after decompressing should be    
#   SDR/data/MiddV3  
#   SDR/data/KITTI

# Run the demo
cd SDR
./demo-KITTI.sh
```

To run the demo
- on Windows:  
  double-click demo.bat
- on Mac & Ubuntu:  
  ./demo.sh  and ./demo-KITTI.sh

TODO: create pybind11 interface to call from python

## 
You will obtain the same results as in our paper on Windows. Results on Mac is silghtly different due to the graph-based segmentation generates different number of superpixels on Mac and Windows.
