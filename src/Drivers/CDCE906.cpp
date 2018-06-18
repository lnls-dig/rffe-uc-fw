#include "CDCE906.h"

int CDCE906::cfg_eth( void )
{
    char cfg[26] = {0};
    int err = 0;

    cfg[0] = 0x1;
    cfg[1] = 9;   /* PLL 1 M */
    cfg[2] = 25;   /* PLL 1 N */
    cfg[3] = 0x0;
    cfg[4] = 9;   /* PLL 2 M */
    cfg[5] = 25; /* PLL 2 N */
    cfg[6] = 0xE0;
    cfg[7] = 9;   /* PLL 3 M */
    cfg[8] = 25;   /* PLL 3 N */
    cfg[9] = (3<<5);
    cfg[10] = 0x0;
    cfg[11] = 0x0;
    cfg[12] = 0x0;
    cfg[13] = 0x2; /* P0 div */
    cfg[14] = 0x1;  /* P1 div */
    cfg[15] = 0x1; /* P2 div */
    cfg[16] = 0x1; /* P3 div */
    cfg[17] = 0x1; /* P4 div */
    cfg[18] = 0x1; /* P5 div */
    cfg[19] = 0x38; /* Y0 */
    cfg[20] = 0;    /* Y1 */
    cfg[21] = 0;    /* Y2 */
    cfg[22] = 0;    /* Y3 */
    cfg[23] = 0;    /* Y4 */
    cfg[24] = 0;    /* Y5 */
    cfg[25] = (3<<4)|0xB;

    err = _write(0, cfg, sizeof(cfg));

    return err;
}

int CDCE906::_write(uint8_t addr, char *buffer, size_t len)
{
    int err;
    char *data = (char *) malloc(len+2);

    data[0] = addr;
    data[1] = len;
    memcpy(&data[2], buffer, len);
    err = _i2c.write(_sladdr, &data[0], len+2);

    free(data);

    return err;
}

int CDCE906::_read(uint8_t addr, char *buffer, size_t len)
{
    int err;
    char data[1];

    data[0] = addr;
    err = _i2c.write(_sladdr, data, 1, true);

    if (err == 0) {
        err = _i2c.read(_sladdr, buffer, len);
    }

    return err;
}
