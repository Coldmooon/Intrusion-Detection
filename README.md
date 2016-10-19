Intrusion Detection based on Optical Flow
===========================
Created by Coldmooon.

# Introduction

IDOF is an intrusion detection system based on Optical Flow. The system can alarm in several ways if there is any object breaking into a detection region. When used for vehicle detection, IDOF can reach nearly 100% mean average precision. During detection, each vehicle's kinetic energy is computed with Optical Flow. If the energy is over some threshold, IDOF will alarm.

# Install

The code depends on `Opencv2.4`. Please install `Opencv` first. If you have `cuda`, make a soft link of `cuda` to `/usr/local/cuda`. Then,

run `./Make.sh` to compile.

run `./IDOF` to detect through camera.

run `./IDOF videofile` to detect in a video file  

# Usage
This project supports two modes of intrusion detection: `line` and `rectangle`.

In the pop-up window, press `R` or `r` will enter the `rectangle` mode. Then use your mouse to draw a rectangle. IDOF will detect all the objects breaking into this region;

Press `L` or `l` to switch to the `line drawing` mode. In this case, use the mouse to draw a line and IDOF will detect on the line. 

Press `p` to pause.

Press `ESC` to exit.

In addition, IDOF assigns a `windowID` for each region which is corresponding to the numeric key `1~9` on the keyboard. 

Pressing a numeric key will remove the corresponding region. For example, if you have four regions indexed by `1~4` respectively, pressing `3` can delete the third region. After this, 
if you'd like to make another region, it will be assigned the missing `3`.