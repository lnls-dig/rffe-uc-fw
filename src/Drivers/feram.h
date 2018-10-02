#ifndef FERAM_H_
#define FERAM_H_

#include "mbed.h"
#include "rtos.h"
#include "bsmp.h"

#define FERAM_MAC_ADDR_OFFSET     0
#define FERAM_IP_ADDR_OFFSET      0x10
#define FERAM_MASK_ADDR_OFFSET    0x20
#define FERAM_GATEWAY_ADDR_OFFSET 0x30
#define FERAM_ATTENUATION_OFFSET  0x40

class FeRAM
{
public:

    FeRAM( mbed::I2C& i2c, mbed::DigitalOut& wp, uint8_t sladdr = 0xA0 )
    : _i2c(i2c), _wp(wp), _sladdr(sladdr) {
    }

    int read(uint16_t addr, uint8_t *buffer, size_t len); /* 11bit address */
    int write(uint16_t addr, uint8_t *buffer, size_t len);

    int get_mac_addr(char *mac_str, char *mac_buf);
    int set_mac_addr(char *mac_str);
    int get_ip_addr(char *ip_str);
    int set_ip_addr(char *ip_str);
    int get_mask_addr(char *mask_str);
    int set_mask_addr(char *mask_str);
    int get_gateway_addr(char *gateway_str);
    int set_gateway_addr(char *gateway_str);
    int get_attenuation(double *att);
    int set_attenuation(double att);
private:
    I2C& _i2c;
    mbed::DigitalOut& _wp;
    int _sladdr;

};
#endif
