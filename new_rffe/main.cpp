#include "DAT31R5SP.hpp"
#include <iostream>
#include "mbed.h"

#define LE_DAT31R5SP  (P0_15)
#define CLK_DAT31R5SP (P0_17)
#define DATA_A_DAT31R5SP (P0_18)
#define DATA_B_DAT31R5SP (P2_0)
#define DATA_C_DAT31R5SP (P0_1)
#define DATA_D_DAT31R5SP (P0_11)

mbed::DigitalOut le(LE_DAT31R5SP);
mbed::DigitalOut clk(CLK_DAT31R5SP);
mbed::DigitalOut dtA(DATA_A_DAT31R5SP);
mbed::DigitalOut dtB(DATA_B_DAT31R5SP);
mbed::DigitalOut dtC(DATA_C_DAT31R5SP);
mbed::DigitalOut dtD(DATA_D_DAT31R5SP);


int main()
{
    DAT31R5SP att(&le, &clk, &dtA, &dtB, &dtC, &dtD);
    att.set_attenuation(0.0);
    ThisThread::sleep_for(1s);
    std::cout << att.get_attenuation() << std::endl;
}
