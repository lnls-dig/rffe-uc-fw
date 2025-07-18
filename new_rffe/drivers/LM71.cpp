#include "LM71.hpp"

LM71::LM71(mbed::SPI & spi, mbed::DigitalOut * cs, int hz) : _spi(spi), _cs(cs), _hz(hz)
{
    *_cs = 1;
    _spi.frequency(_hz);
    _spi.format(16,1);

    lm71_mode(0);
}

bool LM71::lm71_readtemp(float &out_celsius)
{
    *_cs = 0;
    uint16_t raw = static_cast<uint16_t>(_spi.write(0x0000));
    *_cs = 1;

    int16_t t14 = static_cast<int16_t>(raw >> 2);

    if (t14 & 0x2000) {
        t14 |= 0xC000;
    }

    out_celsius = static_cast<float>(t14) * 0.03125f;
    return true;
}

int LM71::lm71_mode(int shutdown)
{
    *_cs = 0;
    _spi.write(0x0000);

    uint16_t mode = shutdown ? 0xFFFF : 0x0000;
    _spi.write(mode);

    if(!shutdown)
    {
        *_cs = 1;
        return 0;
    }

    uint16_t id = _spi.write(0x0000);
    *_cs = 1;
    return id;
}

