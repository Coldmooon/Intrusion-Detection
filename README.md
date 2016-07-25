Intrusion Detection based on Optical Flow
===========================

Created by Coldmooon. some codes are from http://blog.csdn.net/zouxy09

# Introduction

IDOF is an intrusion detection system that utilizes Optical Flow features. During the detection, each object's kinetic energy is computed based on Optical Flow. If the energy is over some threshold, IDOF will alarm. When used for vehicle detection, IDOF can reach nearly 100% mean average precision. 

# Installation

Run `./Make.sh` to compile.

Run `./IDOF` to detect through camera.

Run `./IDOF videofile` to detect in a video file  

# Usage

In the pop-up window, press `R or r`, then use your mouse to draw a square. IDOF will detect in the square;

Press 'L or l' to switch to the `line drawing` mode. Use your mouse to draw a line, then IDOF will detect on the line. 

Press `P or p` to pause or `ESC` to exit.

For each region, IDOF assigns a windowID. Press numeric key to delete the corresponding window. 
For example, if a region's windowID is 3, pressing `3` will delete this region. After this, 
if you'd like to make another region, it will be assigned the windowID `3`.