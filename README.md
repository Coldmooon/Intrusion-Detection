Intrusion Detection based on Optical Flow
===========================

Created by Coldmooon. I borrowed some codes from http://blog.csdn.net/zouxy09

# Introduction

IDOF is an intrusion detection system that combines Optical Flow feature extraction. When used for vehicle detection, it can reach nearly 100% mean average precision. In this case, each vehicle's kinetic energy is computed based on Optical Flow. If the energy is over some threshold, IDOF will sound the alarm (play a sound and draw a bounding box).