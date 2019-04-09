#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "mbed.h"

// Based on Simon's Watchdog code from
// http://mbed.org/forum/mbed/topic/508/

class Watchdog {
public:
// Load timeout value in watchdog timer and enable
    void kick(float s) {
        LPC_WDT->WDCLKSEL = 0x1;                // Set CLK src to PCLK
        uint32_t clk = SystemCoreClock / 16;    // WD has a fixed /4 prescaler, PCLK default is /4
        LPC_WDT->WDTC = s * (float)clk;
        LPC_WDT->WDMOD = 0x3;                   // Enabled and Reset
        kick();
    }
// "kick" or "feed" the dog - reset the watchdog timer
// by writing this required bit pattern
    void kick() {
        LPC_WDT->WDFEED = 0xAA;
        LPC_WDT->WDFEED = 0x55;
    }
    int check_overflow_flag() {
        return (LPC_WDT->WDMOD >> 2) & 1;
    }
    void clear_overflow_flag() {
        LPC_WDT->WDMOD &= ~(1 << 2);
    }
};
#endif
