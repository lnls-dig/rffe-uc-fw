/****************************************************************************
 * rffe-app/rffe_main.c
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

/*
 * Headers
 */

#include <nuttx/config.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "netutils/netlib.h"
#include "netutils/dhcpc.h"

#include "cdce906.h"
#include "netconfig.h"
#include "scpi_server.h"

/*
 * Main function
 */

#if defined(BUILD_MODULE)
int main(int argc, FAR char *argv[])
#else
int rffe_main(int argc, char *argv[])
#endif
{
    int ret;
    struct netifconfig conf;

    /*
     * Initialize the ethernet PHY PLL (50MHz)
     */
    ret = cdce906_init("/dev/i2c0");
    if (ret < 0)
    {
        printf("ERROR: PLL couldn't be initialized!\n");
        return -1;
    }

    /*
     * Read the mac address from the FeRAM
     */
    int eeprom_fd = open("/dev/feram0", O_RDONLY);
    read(eeprom_fd, conf.mac, 6);
    close(eeprom_fd);

    printf("Configuring network...\n");

    conf.default_router.s_addr = 0;
    conf.dnsaddr.s_addr = 0;
    conf.ipaddr.s_addr = 0;
    conf.netmask.s_addr = 0x00FFFFFF;

    /*
     * Configure the network interface, use DHCP
     */
    netconfig("eth0", &conf, 1);
    print_netconfig(&conf);

    scpi_server_start();

    return 0;
}
