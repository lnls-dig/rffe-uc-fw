/****************************************************************************
 * rffe-app/cdce906.c
 *
 *   Copyright (C) 2019 Augusto Fraga Giachero. All rights reserved.
 *   Author: Augusto Fraga Giachero <afg@augustofg.net>
 *
 * This file is part of the RFFE firmware.
 *
 * RFFE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RFFE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RFFE.  If not, see <https://www.gnu.org/licenses/>.
 *
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <nuttx/i2c/i2c_master.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include "cdce906.h"

static uint8_t cdce906_config[] =
{
    0, /* Starting address */
    25, /* Length */
    0x1,
    9,   /* PLL 1 M */
    25,   /* PLL 1 N */
    0x0,
    9,   /* PLL 2 M */
    25, /* PLL 2 N */
    0xE0,
    9,   /* PLL 3 M */
    25,   /* PLL 3 N */
    (2<<5),
    0x0,
    0x0,
    0x0,
    0x2, /* P0 div */
    0x1,  /* P1 div */
    0x1, /* P2 div */
    0x1, /* P3 div */
    0x1, /* P4 div */
    0x1, /* P5 div */
    0x38, /* Y0 */
    0,    /* Y1 */
    0,    /* Y2 */
    0,    /* Y3 */
    0,    /* Y4 */
    0,    /* Y5 */
    (3<<4)|0xB,
};

int cdce906_init(char* devfile)
{
    struct i2c_msg_s i2c_msg[2];
    struct i2c_transfer_s i2c_transfer;
    int fd;

    fd = open("/dev/i2c0", O_RDONLY);
    if (fd < 0)
	{
		return -errno;
    }

    i2c_msg[0].addr   = 0x69;
    i2c_msg[0].flags  = 0;
    i2c_msg[0].buffer = cdce906_config;
    i2c_msg[0].length = sizeof(cdce906_config);
    i2c_msg[0].frequency = 100000;

    i2c_transfer.msgv = (struct i2c_msg_s *)i2c_msg;
    i2c_transfer.msgc = 1;

    ioctl(fd, I2CIOC_TRANSFER, (unsigned long)&i2c_transfer);

    return 0;
}
