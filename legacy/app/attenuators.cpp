#include "attenuators.h"

DigitalOut dataA(P0_18); // Data line to attenuator. LVTTL, low = reset, init = low.Set first attenuators (Att)
DigitalOut dataC(P0_1); // Data line to attenuator. LVTTL, low = reset, init = low.Set first attenuators (Att)
DigitalOut dataB(P2_0); // Data line to attenuator. LVTTL, low = reset, init = low. Set RF attenuators (Att)
DigitalOut dataD(P0_11); // Data line to attenuator. LVTTL, low = reset, init = low. Set RF attenuators (Att)

DigitalOut clk(P0_17); // Digital control attenuation. LVTTL, low = reset, init = low.Digital control attenuation
DigitalOut LE(P0_15); // Chip select for RFFE attenuators (all channels). LVTTL, low = reset, init = low.Digital control calibration

double attenuators_update( double new_att )
{
    bool attVec[6];

    /* Check value boundaries */
    if (new_att < MIN_ATTENUATION) {
        new_att = MIN_ATTENUATION;
    }

    if (new_att > MAX_ATTENUATION) {
        new_att = MAX_ATTENUATION;
    }

    /* Checking and setting attenuators value to fisable values (0.5 dB step) */
    new_att = (float) (int(new_att*2))/2;

    /* Save new value to FeRAM */
    feram.set_attenuation(new_att);

    /* Convert to binary vector (multiply by 2 because the LSB is 0.5dB) */
    int2bin6(int(new_att*2), attVec);

    LE = 0;
    clk = 0;

    /* Send serial data to all attenuators */
    for (int i = 5; i >= 0; i--) {
        dataA = attVec[i];
        dataB = attVec[i];
        dataC = attVec[i];
        dataD = attVec[i];
        clk = 1;
        wait_us(0.1);
        clk = 0;
        wait_us(0.1);
    }

    /* Falling edge on Latch Enable pin */
    LE = 1;
    wait_us(0.1);
    LE = 0;

    return new_att;
}
