mbed compile -j 4 -t GCC_ARM -m LPC1768 --source bootloader/ --source mbed-os/ --build BUILD/bootloader
mbed compile -j 4 -t GCC_ARM -m LPC1768 --source app/ --source mbed-os/ -N rffe-uc-firmware --build BUILD/rffe-uc-firmware
