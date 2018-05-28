#ifndef ADT7320_H_
#define ADT7320_H_

#include "mbed.h"
#include "rtos.h"

#define ADT7320_CFG_13_BITS 0
#define ADT7320_CFG_16_BITS 1

class ADT7320
{
public:

    ADT7320( mbed::SPI& spi, mbed::DigitalOut& cs, int spi_freq, int resolution, int config, double min, double max )
        : _spi(spi), _cs(cs), _freq(spi_freq) {
        ADT7320::Config( _freq, resolution, config );
        ADT7320::SetRange( min, max );
    }

    double Read( void );
    void Config( int freq, int res, int cfg );
    void SetRange( double min, double max );

private:
    SPI& _spi;
    DigitalOut& _cs;

    int _freq;

    double lastTemp;
    double minTemp, maxTemp;
};
#endif
