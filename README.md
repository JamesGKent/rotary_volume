# Rotary Volume
This repo contains a kernel module to use a rotary encoder connected to gpio pins as a volume control. rather than completely reinvent the wheel this makes use of the rotary_encoder module from the linux source tree.
This has been tested on a raspberry pi running both raspbian and OSMC, however as this module and the rotary_encoder module use nothing more than the sysfs gpio interface this should work on other SBC's with gpio capability.

## Prerequisites
 - rotary_encoder module, can test with `sudo modprobe rotary-encoder` if it completes without error then this module should work.
 - kernel headers for the version of kernel you are running (commands for install on raspbian and OSMC will follow)
 - make (either installed directly or as part of pkg-tools)
 
## Common instructions
either:  
a. clone this repo  
b. copy the source and makefile from the `module` subfolder  
Then open terminal in the folder containing the source and makefile

## Raspbian
1. edit `/boot/config.txt` to add the following line:  
`dtoverlay=rotary-encoder:relative_axis=1,linux_axis=9`
2. sudo apt-get install raspberypi-kernel-headers
3. sudo ln -s /usr/src/linux-headers-$(uname -r) /lib/modules/$(uname -r/build
4. make
5. sudo make install

## OSMC
1. edit `/boot/config.txt` to add the following line:  
`dtoverlay=rotary-encoder-overlay:relative_axis=1,linux_axis=9`
2. sudo apt-get install rbp2-headers-$(uname -r)
3. sudo ln -s /usr/src/rbp2-headers-$(uname -r) /lib/modules/$(uname -r/build
4. make
5. sudo make install

## Load module at boot
currently the only way to load the module on boot is to add it to the `/etc/modules` file, this must be on it's own line:  
`rotary_volume`  
Ideally this would be replaced by a device tree overlay that would also enable the encoder directly, just requiring a single dtoverlay entry in the config file, but this is not currently implemented.

## What this does:
1. enable the device tree overlay for the rotary encoder module, using the default pins (on pi: BCM 4 and 17) as a relative axis using the code for the misc axis.
2. install the kernel headers, this would probably be different for other OS's
3. create a symlink from the header files into the /lib/modules/(kernel name) folder so it's in a known location
4. build the module
5. install the module:
5a. copy the module into the correct subfolder
5b. ensure module is owned by root
5c. rebuild module dependency files so module can be found by modprobe etc

## I don't have the rotary_encoder module? what can I do?
If you are on OSMC then the rotary encoder is available in the dev branch as I write this, requiring kernel 4.14.30-2 or newer.
otherwise you could download the source directly [here](https://github.com/raspberrypi/linux/blob/rpi-4.16.y/drivers/input/misc/rotary_encoder.c), you could either try the latest branch, or get the version that matches your kernel.
then use the makefile from the extras folder [here](https://github.com/JamesGKent/rotary_volume/blob/master/extras/rotary_encoder/Makefile)
