#include "ADT7320.h"

double ADT7320::Read()
{
    uint16_t data;
    int reference = 0;
    double delta = 128;
    double temp;

    _cs = 1;
    _spi.frequency( _freq );
    _spi.format(16,3);

    _cs = 0;
    ThisThread::sleep_for(1);
    // Select Temperature value register
    _spi.write(0x0050);
    data = _spi.write(0x0000);
    _cs = 1;

    temp = (float(data)-reference)/delta;

    /* Check if the current read is within range, if not, return the last valid data */
    if ( (temp >= minTemp) && (temp <= maxTemp) ) {
	lastTemp = temp;
    } else {
	temp = lastTemp;
    }

    return temp;
}

void ADT7320::Config( int freq, int res, int cfg )
{
    int cfg_byte = 0;

    _freq = freq;

    _spi.frequency( _freq );
    _cs = 1;

    /* Reseting SPI interface - Write 32 1's to the IC */
    _spi.format(16,3);
    _cs = 0;
    ThisThread::sleep_for(1);
    _spi.write(0xFFFF);
    _spi.write(0xFFFF);
    ThisThread::sleep_for(1);
    _cs = 1;

    // Configuration process
    _spi.format(8,3);
    _cs = 0;
    ThisThread::sleep_for(1);

    // Select CONFIGURATION REGISTER – 0x01
    _spi.write(0x08);

    // Write data to configuration register ( 16-bits resolution + continuous conversion )
    cfg_byte = (res == ADT7320_CFG_16_BITS)? (1 << 7) : 0;

    // Additional configuration bits
    cfg_byte |= cfg;

    _spi.write( cfg_byte );

    ThisThread::sleep_for(1);
    _cs = 1;
    ThisThread::sleep_for(1);
}

void ADT7320::SetRange( double min, double max )
{
    minTemp = min;
    maxTemp = max;
}
