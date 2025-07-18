
#ifndef LM71_H_
#define LM71_H_

#include "mbed.h"

# define CONFIG_LM71_SPI_FREQUENCY 1000000


class LM71
{
    public:
        LM71(mbed::SPI & spi, mbed::DigitalOut* cs, int hz = CONFIG_LM71_SPI_FREQUENCY);

        bool lm71_readtemp(float & out_celsius);
        int lm71_mode(int shutdown);

    private:
        mbed::SPI & _spi;
        mbed::DigitalOut * _cs;
        int _hz;
};

#endif
