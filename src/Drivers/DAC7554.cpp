#include "DAC7554.h"

void DAC7554::Write( double vout )
{
    // Control bits         Data bits   DAC     Function
    // 1 0 0 0 (0x8000)     12 bits      A      Input register and DAC register updated, output updated
    // 1 0 0 1 (0x9000)     12 bits      B      Input register and DAC register updated, output updated
    // 1 0 1 0 (0xA000)     12 bits      C      Input register and DAC register updated, output updated
    // 1 0 1 1 (0xB000)     12 bits      D      Input register and DAC register updated, output updated
    // vout = refin * data  / 4095
    // data = vout * 4095 / refin

    uint16_t data;
    uint16_t cfg;

    // SPI config
    _spi.frequency(1000000);
    _spi.format(16,2);
    _cs = 1;

    // Calculating data to vout
    data = (uint16_t)(vout*4095/refin);
    cfg = ( channel << 12 ) | ( data & 0x0FFF );

    // Transmition
    _cs = 0;
    Thread::wait(1);
    // control - write data to voutA
    _spi.write( cfg );
    Thread::wait(1);
    _cs = 1;
    Thread::wait(1);
}
