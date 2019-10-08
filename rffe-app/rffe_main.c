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
#include "config_file_migrate.h"
#include "config_file.h"
#include "rffe_console_cfg.h"
#include "fw_update.h"

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
    const char* cfg_file = "/dev/feram0";
    eth_addr_mode_t dhcp;

    /*
     * Migrate the FeRAM contents to the last format if necessary
     */
    config_migrate_latest(cfg_file);

    /*
     * If there are arguments to be read, call rffe_console_cfg. This
     * is used for reading and writing configuration parameters from
     * the FeRAM
     */
    if (argc > 1)
    {
        return rffe_console_cfg(argc, argv);
    }

    /*
     * Initialize the ethernet PHY PLL (50MHz)
     */
    ret = cdce906_init("/dev/i2c0");
    if (ret < 0)
    {
        printf("ERROR: PLL couldn't be initialized!\n");
        return -1;
    }

    printf("Configuring network...\n");

    /*
     * Get the network configuration from the FeRAM
     */
    config_get_mac_addr(cfg_file, conf.mac);
    config_get_ipv4_addr(cfg_file, &conf.ipaddr.s_addr);
    config_get_mask_addr(cfg_file, &conf.netmask.s_addr);
    config_get_gateway_addr(cfg_file, &conf.default_router.s_addr);
    conf.dnsaddr.s_addr = 0;

    config_get_eth_addressing(cfg_file, &dhcp);

    if (dhcp == ETH_ADDR_MODE_STATIC)
    {
        /*
         * Configure the network interface, static IP
         */
        netconfig("eth0", &conf, 0);
    }
    else
    {
        /*
         * Configure the network interface, use DHCP
         */
        netconfig("eth0", &conf, 1);
    }


    print_netconfig(&conf);

    /*
     * Initialize the RFFE scpi server
     */
    scpi_server_start();

    return 0;
}
