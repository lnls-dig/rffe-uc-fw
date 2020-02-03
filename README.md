# RFFE Controller Firmware

Radio Frequency Front End controller firmware, powered by Nuttx, using the Cortex M3 LPC1769 microcontroller.

## Overview

The RFFE device is used for signal conditioning of the RF signal obtained from the BPM antennas. The uC is in charge of temperature monitoring / control and setting the RF attenuation level. The uC listens to TCP port 9001 for SCPI messages.

## Building

First make sure you have installed on your system:
* make
* arm-none-eabi-gcc
* arm-none-eabi-binutils
* arm-none-eabi-newlib
* kconfig-frontends - See instructions bellow
* openocd

To build, you need to configure nuttx and start the compilation:

``` bash
$ ./make.sh configure
$ ./make.sh build
```

If everything goes well, the files ```nuttx/nuttx``` (elf with debug symbols) and ```nuttx/nuttx.bin``` (raw binary) will be available.

## Installing kconfig-frontends

An out-of-tree version of kconfig-frontends is provided under ```tools/kconfig-frontends```. To build it make sure you have the following tools installed on your system:
* automake
* autoconf
* m4
* flex
* bison
* gperf

Enter the ```tools/kconfig-frontends``` directory and execute:
```bash
$ aclocal
$ autoconf
$ automake
$ ./configure --prefix=your_prefix_path
$ make install
```

Make sure to include the ```your_prefix_path/usr/bin``` directory in your ```PATH``` environment variable.

## Flashing

The nuttx.bin image does not contains the bootloader and is expected to be loaded at 0x00010000, so if you are flashing blank uCs, flash the bootloader from legacy/bootloader first.

### Via CMSIS-DAP

To flash using a CMSIS-DAP compatible debugger (via openocd) execute:

```bash
$ ./make.sh flash

```

### Via lpc21isp

To upload via lpc21isp you need an firmware image with the bootloader. This can be done concatenating the bootloader raw binary (64KiB padded) with the nuttx.bin file. After that, connect the USB cable to the RFFE board and flash:

```bash
$ ./lpc21isp -control -bin -wipe firmware_complete.bin <serial_port> 230400 12000
```

The lpc21isp utility can be obtained [here](https://github.com/lnls-dig/lpc21isp).

### Via Remote Update

To update the RFFE firmware via network use the ./utils/fw_update.py script:

```bash
$ ./utils/fw_update.py <ip_addr> nuttx/nuttx.bin <version>
```
