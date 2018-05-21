#ifndef FERAM_H_
#define FERAM_H_

#include "mbed.h"
#include "rtos.h"

class FeRAM
{
public:

    FeRAM( mbed::I2C& i2c, mbed::DigitalOut& wp, uint8_t sladdr = 0xA0 )
    : _i2c(i2c), _wp(wp), _sladdr(sladdr) {
    }

    int read(uint16_t addr, char *buffer, size_t len); /* 11bit address */
    int write(uint16_t addr, char *buffer, size_t len);

    int get_mac_addr(char *mac_str);
    int get_ip_addr(char *ip_str);
    int get_mask_addr(char *mask_str);
    int get_gateway_addr(char *gateway_str);
private:
    I2C& _i2c;
    mbed::DigitalOut& _wp;
    int _sladdr;

};
#endif