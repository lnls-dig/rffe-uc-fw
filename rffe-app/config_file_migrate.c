/****************************************************************************
 * rffe-app/config_file_migrate.c
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

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fixedmath.h>
#include <string.h>

struct __attribute__((__packed__)) config_v0
{
    uint8_t mac[6];
    uint8_t __unused1[9];
    uint8_t version;
    uint8_t ipv4[4];
    uint8_t __unused2[12];
    uint8_t netmask[4];
    uint8_t __unused3[12];
    uint8_t gateway[4];
    uint8_t __unused4[12];
    uint8_t attenuation[4];
    uint8_t __unused5[12];
    uint8_t eth_addr_mode;
};

struct __attribute__((__packed__)) config_v1
{
    uint8_t mac[6];
    uint8_t __unused1[9];
    uint8_t version;
    uint8_t ipv4[4];
    uint8_t __unused2[12];
    uint8_t netmask[4];
    uint8_t __unused3[12];
    uint8_t gateway[4];
    uint8_t __unused4[12];
    b16_t attenuation;
    uint8_t __unused5[12];
    uint8_t eth_addr_mode;
    uint8_t temp_control_manual;
    uint8_t __unused6[14];
    float pid_ac_kc;
    float pid_ac_ti;
    float pid_ac_td;
    float pid_bd_kc;
    float pid_bd_ti;
    float pid_bd_td;
    float pid_ac_set_point;
    float pid_bd_set_point;
};

int config_migrate_latest(const char* path)
{
    struct config_v0 confv0;
    struct config_v1 confv1;

    int fd = open(path, O_RDWR);

    if (fd < 0)
    {
        return -2;
    }

    read(fd, &confv0, sizeof(confv0));
    if (confv0.version == 0)
    {
        /*
         * Save a backup at the end of FERAM
         */
        lseek(fd, 2048 - 256, SEEK_SET);
        write(fd, &confv0, sizeof(confv0));

        memset(&confv1, 0, sizeof(confv1));
        memcpy(confv1.mac, confv0.mac, 6);
        memcpy(confv1.ipv4, confv0.ipv4, 4);
        memcpy(confv1.netmask, confv0.netmask, 4);
        memcpy(confv1.gateway, confv0.gateway, 4);
        confv1.eth_addr_mode = confv0.eth_addr_mode;

        int32_t att = confv0.attenuation[3] | confv0.attenuation[2] << 8 |
            confv0.attenuation[1] << 16 | confv0.attenuation[0] << 24;
        confv1.attenuation = itob16(att) >> 1;

        confv1.pid_ac_kc = 1.0;
        confv1.pid_ac_ti = 1.0;
        confv1.pid_ac_td = 1.0;
        confv1.pid_bd_kc = 1.0;
        confv1.pid_bd_ti = 1.0;
        confv1.pid_bd_td = 1.0;
        confv1.pid_ac_set_point = 50.0;
        confv1.pid_bd_set_point = 50.0;
        confv1.temp_control_manual = 1;

        confv1.version = 1;
        lseek(fd, 0, SEEK_SET);
        write(fd, &confv1, sizeof(confv1));
    }
    else if (confv0.version > 0x7F)
    {
        memset(&confv1, 0, sizeof(confv1));
        memcpy(confv1.mac, confv0.mac, 6);

        confv1.ipv4[0] = 192;
        confv1.ipv4[1] = 168;
        confv1.ipv4[2] = 1;
        confv1.ipv4[3] = 101;

        confv1.netmask[0] = 255;
        confv1.netmask[1] = 255;
        confv1.netmask[2] = 255;
        confv1.netmask[3] = 0;

        confv1.gateway[0] = 192;
        confv1.gateway[1] = 168;
        confv1.gateway[2] = 1;
        confv1.gateway[3] = 1;
        confv1.eth_addr_mode = 1;

        confv1.attenuation = 0;

        confv1.pid_ac_kc = 1.0;
        confv1.pid_ac_ti = 1.0;
        confv1.pid_ac_td = 1.0;
        confv1.pid_bd_kc = 1.0;
        confv1.pid_bd_ti = 1.0;
        confv1.pid_bd_td = 1.0;
        confv1.pid_ac_set_point = 50.0;
        confv1.pid_bd_set_point = 50.0;
        confv1.temp_control_manual = 0;

        confv1.version = 1;
        lseek(fd, 0, SEEK_SET);
        write(fd, &confv1, sizeof(confv1));
    }
    else
    {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}
