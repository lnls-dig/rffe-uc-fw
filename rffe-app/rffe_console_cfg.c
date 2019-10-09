/****************************************************************************
 * rffe-app/rffe_console_cfg.c
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

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <nuttx/rf/ioctl.h>
#include <nuttx/rf/attenuator.h>

#include "netconfig.h"
#include "config_file.h"

static char* cfg_file = "/dev/feram0";

int rffe_console_cfg(int argc, char *argv[])
{
    struct netifconfig conf;
    in_addr_t netaddr;
    int ret;

    if (argc > 2)
    {
        if (strcmp(argv[1], "get") == 0)
        {
            if (strcmp(argv[2], "ip") == 0)
            {
                config_get_ipv4_addr(cfg_file, &conf.ipaddr.s_addr);
                printf("IP: %s\n", inet_ntoa(conf.ipaddr));
            }
            else if (strcmp(argv[2], "netmask") == 0)
            {
                config_get_mask_addr(cfg_file, &conf.ipaddr.s_addr);
                printf("Netmask: %s\n", inet_ntoa(conf.ipaddr));
            }
            else if (strcmp(argv[2], "gateway") == 0)
            {
                config_get_gateway_addr(cfg_file, &conf.ipaddr.s_addr);
                printf("Gateway: %s\n", inet_ntoa(conf.ipaddr));
            }
            else if (strcmp(argv[2], "mac") == 0)
            {
                uint8_t mac[6];
                config_get_mac_addr(cfg_file, mac);
                printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            }
            else if (strcmp(argv[2], "dhcp") == 0)
            {
                eth_addr_mode_t dhcp;
                config_get_eth_addressing(cfg_file, &dhcp);

                if (dhcp == ETH_ADDR_MODE_DHCP)
                {
                    printf("enabled\n");
                }
                else if (dhcp == ETH_ADDR_MODE_STATIC)
                {
                    printf("disabled\n");
                }
                else
                {
                    printf("none\n");
                }
            }
            else if (strcmp(argv[2], "temp_ac") == 0)
            {
                int fd = open("/dev/temp_ac", O_RDONLY);
                b16_t temp = 0;
                if (fd > 0)
                {
                    read(fd, &temp, 4);
                    close(fd);
                    printf("%f C\n", b16tof(temp));
                }
                else
                {
                    printf("Hardware missing!\n");
                }

            }
            else if (strcmp(argv[2], "temp_bd") == 0)
            {
                int fd = open("/dev/temp_bd", O_RDONLY);
                b16_t temp = 0;
                if (fd > 0)
                {
                    read(fd, &temp, 4);
                    close(fd);
                    printf("%f C\n", b16tof(temp));
                }
                else
                {
                    printf("Hardware missing!\n");
                }
            }
            else if (strcmp(argv[2], "attenuation") == 0)
            {
                b16_t att;
                config_get_attenuation("/dev/feram0", &att);
                printf("%f dB\n", b16tof(att));
            }

        }
        else if (strcmp(argv[1], "set") == 0)
        {
            if (strcmp(argv[2], "ip") == 0)
            {
                int valid_ip = inet_pton(AF_INET, argv[3], &netaddr);
                if (valid_ip)
                {
                    config_set_ipv4_addr(cfg_file, netaddr);
                }
            }
            else if (strcmp(argv[2], "netmask") == 0)
            {
                int valid_ip = inet_pton(AF_INET, argv[3], &netaddr);
                if (valid_ip)
                {
                    config_set_mask_addr(cfg_file, netaddr);
                }
            }
            else if (strcmp(argv[2], "gateway") == 0)
            {
                int valid_ip = inet_pton(AF_INET, argv[3], &netaddr);
                if (valid_ip)
                {
                    config_set_gateway_addr(cfg_file, netaddr);
                }
            }
            else if (strcmp(argv[2], "mac") == 0)
            {
                uint8_t mac[6];
                ret = sscanf(argv[3], "%x:%x:%x:%x:%x:%x",
                             &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
                if (ret == 6)
                {
                    config_set_mac_addr(cfg_file, mac);
                }
                else
                {
                    printf("Invalid mac address!\n");
                }
            }
            else if (strcmp(argv[2], "dhcp") == 0)
            {
                if (strcmp(argv[3], "enable") == 0)
                {
                    config_set_eth_addressing(cfg_file, ETH_ADDR_MODE_DHCP);
                }
                else if (strcmp(argv[3], "disable") == 0)
                {
                    config_set_eth_addressing(cfg_file, ETH_ADDR_MODE_STATIC);
                }
                else
                {
                    printf("Expected parameter: [enable | disable]. Received: %s\n", argv[3]);
                }
            }
            else if (strcmp(argv[2], "attenuation") == 0)
            {
                struct attenuator_control att;
                float num;

                ret = sscanf(argv[3], "%f", &num);
                if (ret == 1)
                {
                    att.attenuation = ftob16(num);

                    int fd = open("/dev/att0", O_RDONLY);
                    ioctl(fd, RFIOC_SETATT, (unsigned long)&att);
                    close(fd);

                    config_set_attenuation("/dev/feram0", att.attenuation);
                }
                else
                {
                    printf("Invalid attenuation value!\n");
                }
            }
        }
        else
        {
            printf("Invalid command: %s\n", argv[1]);
            return -1;
        }
    }
    return 0;
}
