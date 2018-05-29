#ifndef CDCE906_H_
#define CDCE906_H_

#include "mbed.h"
#include "rtos.h"

class CDCE906
{
public:

    CDCE906( mbed::I2C& i2c, uint8_t addr )
    : _i2c(i2c), _sladdr(addr) {
    }

    int cfg_eth();
private:
    int _read(uint8_t addr, char *buffer, size_t len);
    int _write(uint8_t addr, char *buffer, size_t len);
    I2C& _i2c;
    int _sladdr;
};
#endif
