#include "feram.h"

int FeRAM::write(uint16_t addr, char *buffer, size_t len)
{
    int err;
    char *data = (char *) malloc(len+1);

    uint8_t page_sladdr = ((_sladdr & 0xF0) | ( ( (addr >> 8) & 0x7) << 1));
    data[0] = (addr & 0xFF);
    memcpy(&data[1], buffer, len);

    _wp = 0;
    /* Upper 3 bits from addr are the bits [3:1] from the slave address */
    err = _i2c.write(page_sladdr, &data[0], len+1);
    _wp = 1;

    free(data);

    return err;
}

int FeRAM::read(uint16_t addr, char *buffer, size_t len)
{
    int err;
    char data[1];

    uint8_t page_sladdr = ((_sladdr & 0xF0) | ( ( (addr >> 8) & 0x7) << 1));
    data[0] = (addr & 0xFF);

    err = _i2c.write(page_sladdr, data, 1, true);

    if (err == 0) {
        err = _i2c.read(page_sladdr, buffer, len);
    }

    return err;
}

int FeRAM::get_mac_addr(char *mac_str, char* mac_buf)
{
    int err;
    char mac[6] = {0};

    err = FeRAM::read(0, mac, 6);

    snprintf(mac_str, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if (mac_buf) {
        memcpy(mac_buf, mac, 6);
    }
    return err;
}

int FeRAM::get_ip_addr(char *ip_str)
{
    int err;
    char ip[4] = {0};

    err = FeRAM::read(0x10, ip, 4);

    snprintf(ip_str, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    return err;
}

int FeRAM::get_mask_addr(char *mask_str)
{
    int err;
    char mask[4] = {0};

    err = FeRAM::read(0x20, mask, 4);

    snprintf(mask_str, 16, "%d.%d.%d.%d", mask[0], mask[1], mask[2], mask[3]);

    return err;
}


int FeRAM::get_gateway_addr(char *gateway_str)
{
    int err;
    char gateway[4] = {0};

    err = FeRAM::read(0x30, gateway, 4);

    snprintf(gateway_str, 16, "%d.%d.%d.%d", gateway[0], gateway[1], gateway[2], gateway[3]);

    return err;
}
