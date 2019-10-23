#ifndef DAC7554_H_
#define DAC7554_H_

#include "mbed.h"
#include "rtos.h"

#define DAC_AC_SEL 0xB
#define DAC_BD_SEL 0xA

class DAC7554
{
public:

DAC7554( mbed::SPI& spi, mbed::DigitalOut& cs, uint8_t dac_channel, float dac_refin )
    : _spi(spi), _cs(cs), channel(dac_channel), refin(dac_refin) {}

    void Write( double vout );

private:
    SPI& _spi;
    DigitalOut& _cs;
    uint8_t channel;
    float refin;
};
#endif
