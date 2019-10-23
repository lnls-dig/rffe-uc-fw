/****************************************************************************
 * rffe-app/netconfig.h
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

#include <arpa/inet.h>
#include "netconfig.h"

int netconfig(char* netdev, struct netifconfig* conf, int use_dhcp)
{
    netlib_setmacaddr(netdev, conf->mac);
    netlib_set_dripv4addr(netdev, &conf->default_router);
    netlib_set_ipv4addr(netdev, &conf->ipaddr);
    netlib_set_ipv4netmask(netdev, &conf->netmask);
    netlib_ifup(netdev);

    if(use_dhcp)
    {
        FAR void *handle;
        handle = dhcpc_open(netdev, conf->mac, 6);

        if (handle != NULL)
        {
            struct dhcpc_state ds;
            (void)dhcpc_request(handle, &ds);
            netlib_set_ipv4addr(netdev, &ds.ipaddr);

            conf->ipaddr = ds.ipaddr;

            if (ds.netmask.s_addr != 0)
            {
                netlib_set_ipv4netmask(netdev, &ds.netmask);
                conf->netmask = ds.netmask;
            }

            if (ds.default_router.s_addr != 0)
            {
                netlib_set_dripv4addr(netdev, &ds.default_router);
                conf->default_router = ds.default_router;
            }
            dhcpc_close(handle);
        }
        else
        {
            return -2;
        }
    }
    return 0;
}

void print_netconfig(struct netifconfig* conf)
{
    printf("IP:            %s\n", inet_ntoa(conf->ipaddr));
    printf("Netmask:       %s\n", inet_ntoa(conf->netmask));
    printf("Default route: %s\n", inet_ntoa(conf->default_router));
}
