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

## This doesn't work with kodi?
to use this with kodi required changing udev rules, because on load kodi grabs all input devices, this includes the rotary encoder event though it doesn't know what to do with any of the events from it. by changing the ownership of that input from `root:input` to `root:root` we can prevent it from being accesible to kodi, allowing this module to work as intended.
on OSMC this can be achieved by editing `/etc/udef/rules.d/998-fix-input.rules` to contain the following:

```
# input
KERNEL=="mouse*|mice|event*", MODE="0660", GROUP="osmc"
KERNEL=="event*", DRIVERS=="rotary-encoder", MODE="0660", GROUP="root"
KERNEL=="ts[0-9]*|uinput",      MODE="0660", GROUP="osmc"
KERNEL=="js[0-9]*",             MODE="0660", GROUP="osmc"

# tty
SUBSYSTEM=="tty", KERNEL=="tty[0-9]*", GROUP="tty", MODE="0666"
```

## Testing
you can test each stage by using evtest, if you have already set up the udev rules then this will need to be run as root.
if you run `evtest` and select the rotary device you should see ea long list of supported keys ending in the volume keys, the driver will never send any other keys than volume up and down, however kodi ignored events from devices with 20 or less keys, so this is currently a hacky workaround to make kodi recognise this module.
when you turn the rotary encoder you should see events come in, with code 9 (the axis we picked) and value of either 1 or -1, the positive events get turned into volume up presses, and negative becomes volume down.

assuming the first test is successful we can then test this module by selecting it with `evtest`, again we should see evets come in when we turn the encoder, either KEY_VOLUMEUP or KEY_VOLUMEDOWN.

## Optional settings
when writing this module I only had a 400 pulse per revolution encoder available, so I wrote in the capability to require multiple "counts" before sending a keypress, this is the `count_per_press` variable [here](https://github.com/JamesGKent/rotary_volume/blob/master/module/rotary_volume.c#L21) reducing this number will result in more events, if you reduce it to 1 and its still not fast enough for your liking you could add either:
```
steps-per-period=2
```
or
```
steps-per-period=4
```
to the line in the config file for the rotary encoder, eg:
```
`dtoverlay=rotary-encoder:relative_axis=1,linux_axis=9,steps-per-period=2`
```
