#ifndef LM71_H_
#define LM71_H_

#include "mbed.h"
#include "rtos.h"

#define LM71_LSB_VALUE 0.03125

typedef enum {
    LM71_MODE_CONVERSION,
    LM71_MODE_SHUTDOWN
} lm71_mode_t;

class LM71
{
public:

    LM71( mbed::SPI& spi, mbed::DigitalOut& cs, int spi_freq, lm71_mode_t mode, double min, double max )
        : _spi(spi), _cs(cs), _freq(spi_freq), _mode(mode) {
        LM71::Config( _freq, _mode );
        LM71::SetRange( min, max );
    }

    double Read( void );
    int Config( int freq, lm71_mode_t mode );
    void SetRange( double min, double max );
    uint16_t ReadID( void );

    uint32_t err_count( void );
private:
    SPI& _spi;
    DigitalOut& _cs;

    int _freq;
    lm71_mode_t _mode;

    uint32_t _err;

    double lastTemp;
    double minTemp, maxTemp;
};
#endif
