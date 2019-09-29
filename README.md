# StereoMatching-Census-SemiGlobal
Implementation of stereo matching with census transform and semi-global matching (image processing course project)

## References
1. Hirschmüller, H., Accurate and Efficient Stereo Processing by Semi-Global Matching and Mutual Information, IEEE Conference on Computer Vision and Pattern Recognition, June 2005
2. Zabih, R., Woodfill, J., Nonparametric Local Transforms for Computing Visual Correspondence

## Linux dependencies (Ubuntu 18.04)
* Installing [OpenCV](https://pydeeplearning.com/opencv/install-opencv-with-c-on-ubuntu-18-04/)

`sudo apt-get update`

`sudo apt-get upgrade`

`sudo apt-get install libopencv-dev`

* *g++* (tested with version 7.4.0)

## Running on Linux (Ubuntu 18.04)
You can use the `Makefile` provided in the project repo:

`make` -> build the application

`make run` -> run the application (it also builds the application if there is no executable)

`make clean` -> clean the project

**TIP:** If you want to save the result images on disk you can put the `SAVE_TO_DISK` to `1` in the `OpenCVApplication.cpp` file.
