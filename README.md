Intrusion Detection based on Optical Flow
===========================

Created by Coldmooon.

# Introduction

IDOF is an intrusion detection system based on Optical Flow. The system can sound the alarm in several ways if there is any object breaking into a detection region. When used for vehicle detection, IDOF can reach nearly 100% mean average precision. During detection, each vehicle's kinetic energy is computed with Optical Flow. If the energy is over some threshold, IDOF will sound the alarm.

# Installation

The code depends on [Opencv2.4](http://opencv.org/) and [SFML](http://www.sfml-dev.org/). Please install `Opencv` and `SFML` first. 

## Linux

Simply run the following line to compile `main.cpp`.

```
g++ `pkg-config --cflags opencv` main.cpp -o IDOF `pkg-config --libs opencv` -lsfml-audio
```

If you compiled `opencv` with `cuda` support, you may get this:

```
/usr/bin/ld: cannot find -lcufft
/usr/bin/ld: cannot find -lnpps
/usr/bin/ld: cannot find -lnppi
/usr/bin/ld: cannot find -lnppc
/usr/bin/ld: cannot find -lcudart
```
In this case, you'll need to tell the `linker` where to find the `cuda` libraries.

```
g++ `pkg-config --cflags opencv` main.cpp -o IDOF `pkg-config --libs opencv` -L/usr/local/cuda/lib64 -lsfml-audio
```

## OS X

Following [here](http://brew.sh/) to install `homebrew` first. Then, 
 
```
brew install opencv
brew install sfml
clang++ `pkg-config --cflags opencv` main.cpp -o IDOF `pkg-config --libs opencv` -lsfml-audio
```

When running `./IDOF`, you may get the following error. 

```
dyld: Library not loaded: @rpath/libsfml-system.2.4.dylib
    Referenced from: /usr/local/opt/sfml/lib/libsfml-audio.2.4.dylib
    Reason: image not found
```

This is because `@rpath` is not set properly. To solve this, run

```
install_name_tool -add_rpath /usr/local/opt/sfml/lib IDOF
```
See `man install_name_tool` for details.

# Usage

run `./IDOF` to detect through camera.

run `./IDOF videofile` to detect in a video file  

This system supports two modes of intrusion detection: `line` and `rectangle`.


In the pop-up window, press `R` or `r` will enter the `rectangle` mode. Then use your mouse to draw a rectangle. IDOF will detect all the objects breaking into this region;

Press `L` or `l` to switch to the `line` mode. In this case, use the mouse to draw a line and IDOF will detect on the line. 

Press `p` to pause.

Press `ESC` to exit.

In addition, IDOF assigns a `windowID` for each region which is corresponding to the numeric key `1~9` on the keyboard. 

Pressing a numeric key will remove the corresponding region. For example, if you have four regions indexed by `1~4` respectively, pressing `3` will delete the third region. After this, 
if you'd like to make another region, it will be assigned the missing `3`.
