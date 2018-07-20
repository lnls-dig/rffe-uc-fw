#include "LM71.h"

double LM71::Read()
{
    int16_t data = 0;
    double temp = 0;

    if (_mode != LM71_MODE_CONVERSION) {
        LM71::Config(_freq, LM71_MODE_CONVERSION);
    }

    _spi.format(16,0);

    _cs = 0;
    Thread::wait(1);
    // Select Temperature value register
    data = _spi.write(0x0000);
    _cs = 1;

    temp = (data >> 2) * LM71_LSB_VALUE;

    /* Check if the current read is within range, if not, return the last valid data */
    if ( (temp >= minTemp) && (temp <= maxTemp) ) {
        lastTemp = temp;
    } else {
        _err++;
        //temp = lastTemp;
    }

    return temp;
}

int LM71::Config( int freq, lm71_mode_t mode )
{
    if (freq > 6250000) {
        return -1;
    }

    _freq = freq;
    _mode = mode;
    _spi.frequency( _freq );
    _cs = 1;

    _spi.format(16,0);

    if (_mode == LM71_MODE_CONVERSION) {
        _cs = 0;
        Thread::wait(1);
        _spi.write(0x0000);
        _spi.write(0x0000);
        _cs = 1;
    } else if (_mode == LM71_MODE_SHUTDOWN) {
        _cs = 0;
        Thread::wait(1);
        _spi.write(0xFFFF);
        _spi.write(0xFFFF);
        _cs = 1;
    } else {
        return -2;
    }

    return 0;
}

uint16_t LM71::ReadID( void )
{
    LM71::Config( _freq, LM71_MODE_SHUTDOWN);
    _mode = LM71_MODE_SHUTDOWN;

    _cs = 0;
    Thread::wait(1);
    uint16_t id = _spi.write(0xFFFF);
    _cs = 1;

    return (id >> 2);
}

uint32_t LM71::err_count( void )
{
    return _err;
}

void LM71::SetRange( double min, double max )
{
    minTemp = min;
    maxTemp = max;
}
