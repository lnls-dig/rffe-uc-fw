/****************************************************************************
 * rffe-app/config_file.c
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

#include "config_file.h"

static const int mac_addr_offset = 0;
static const int config_version_offset = 0x0F;
static const int ip_addr_offset = 0x10;
static const int mask_addr_offset = 0x20;
static const int gateway_addr_offset = 0x30;
static const int attenuation_offset = 0x40;
static const int eth_addr_offset = 0x50;
static const int temp_ctrl_mode_offset = 0x51;
static const int pid_ac_kc_offset = 0x60;
static const int pid_ac_ti_offset = 0x64;
static const int pid_ac_td_offset = 0x68;
static const int pid_bd_kc_offset = 0x6C;
static const int pid_bd_ti_offset = 0x70;
static const int pid_bd_td_offset = 0x74;
static const int pid_ac_set_point_offset = 0x78;
static const int pid_bd_set_point_offset = 0x7C;

int config_get_version(const char* path, uint8_t* version)
{
    int fd = open(path, O_RDONLY);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, config_version_offset, SEEK_SET);
    ret = read(fd, version, 1);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_set_version(const char* path, uint8_t version)
{
    int fd = open(path, O_RDWR);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, eth_addr_offset, SEEK_SET);
    ret = write(fd, &version, 1);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_get_eth_addressing(const char* path, eth_addr_mode_t* addr_mode)
{
	int fd = open(path, O_RDONLY);
    int ret;
    uint8_t buf;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, eth_addr_offset, SEEK_SET);
    ret = read(fd, &buf, 1);
    if (ret > 0) ret = 0;

    switch (buf)
    {
    case 0:
        *addr_mode = ETH_ADDR_MODE_STATIC;
        break;
    case 1:
        *addr_mode = ETH_ADDR_MODE_DHCP;
        break;
    default:
        *addr_mode = ETH_ADDR_MODE_NONE;
        break;
    }

    close(fd);
    return ret;
}

int config_set_eth_addressing(const char* path, eth_addr_mode_t addr_mode)
{
    int fd = open(path, O_RDWR);
    int ret;
    uint8_t buf;

    if (fd < 0)
    {
        return fd;
    }

    switch (addr_mode)
    {
    case ETH_ADDR_MODE_STATIC:
        buf = 0;
        break;
    case ETH_ADDR_MODE_DHCP:
        buf = 1;
        break;
    default:
        buf = 2;
        break;
    }

    lseek(fd, eth_addr_offset, SEEK_SET);
    ret = write(fd, &buf, 1);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_get_mac_addr(const char* path, uint8_t mac[6])
{
    int fd = open(path, O_RDONLY);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, mac_addr_offset, SEEK_SET);
    ret = read(fd, mac, 6);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_set_mac_addr(const char* path, const uint8_t mac[6])
{
    int fd = open(path, O_RDWR);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, mac_addr_offset, SEEK_SET);
    ret = write(fd, mac, 6);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_get_ipv4_addr(const char* path, in_addr_t* ip)
{
    int fd = open(path, O_RDONLY);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, ip_addr_offset, SEEK_SET);
    ret = read(fd, ip, 4);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_set_ipv4_addr(const char* path, in_addr_t ip)
{
    int fd = open(path, O_RDWR);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, ip_addr_offset, SEEK_SET);
    ret = write(fd, &ip, 4);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_get_mask_addr(const char* path, in_addr_t* mask)
{
    int fd = open(path, O_RDONLY);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, mask_addr_offset, SEEK_SET);
    ret = read(fd, mask, 4);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_set_mask_addr(const char* path, in_addr_t mask)
{
    int fd = open(path, O_RDWR);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, mask_addr_offset, SEEK_SET);
    ret = write(fd, &mask, 4);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_get_gateway_addr(const char* path, in_addr_t* gateway)
{
    int fd = open(path, O_RDONLY);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, gateway_addr_offset, SEEK_SET);
    ret = read(fd, gateway, 4);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_set_gateway_addr(const char* path, in_addr_t gateway)
{
    int fd = open(path, O_RDWR);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, gateway_addr_offset, SEEK_SET);
    ret = write(fd, &gateway, 4);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_get_attenuation(const char* path, b16_t *att)
{
    int fd = open(path, O_RDONLY);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, attenuation_offset, SEEK_SET);
    ret = read(fd, att, 4);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_set_attenuation(const char* path, b16_t att)
{
    int fd = open(path, O_RDWR);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, attenuation_offset, SEEK_SET);
    ret = write(fd, &att, 4);
    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_get_pid_ac(const char* path, float* kc, float* ti, float* td)
{
    int fd = open(path, O_RDONLY);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, pid_ac_kc_offset, SEEK_SET);
    ret += read(fd, kc, 4);

    lseek(fd, pid_ac_ti_offset, SEEK_SET);
    ret += read(fd, ti, 4);

    lseek(fd, pid_ac_td_offset, SEEK_SET);
    ret += read(fd, td, 4);

    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_set_pid_ac(const char* path, float kc, float ti, float td)
{
    int fd = open(path, O_RDWR);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, pid_ac_kc_offset, SEEK_SET);
    ret += write(fd, &kc, 4);

    lseek(fd, pid_ac_ti_offset, SEEK_SET);
    ret += write(fd, &ti, 4);

    lseek(fd, pid_ac_td_offset, SEEK_SET);
    ret += write(fd, &td, 4);

    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_get_pid_bd(const char* path, float* kc, float* ti, float* td)
{
    int fd = open(path, O_RDONLY);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, pid_bd_kc_offset, SEEK_SET);
    ret += read(fd, kc, 4);

    lseek(fd, pid_bd_ti_offset, SEEK_SET);
    ret += read(fd, ti, 4);

    lseek(fd, pid_bd_td_offset, SEEK_SET);
    ret += read(fd, td, 4);

    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_set_pid_bd(const char* path, float kc, float ti, float td)
{
    int fd = open(path, O_RDWR);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, pid_bd_kc_offset, SEEK_SET);
    ret += write(fd, &kc, 4);

    lseek(fd, pid_bd_ti_offset, SEEK_SET);
    ret += write(fd, &ti, 4);

    lseek(fd, pid_bd_td_offset, SEEK_SET);
    ret += write(fd, &td, 4);

    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_get_setpoint_ac(const char* path, float* setpoint)
{
    int fd = open(path, O_RDONLY);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, pid_ac_set_point_offset, SEEK_SET);
    ret += read(fd, setpoint, 4);

    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_set_setpoint_ac(const char* path, float setpoint)
{
    int fd = open(path, O_RDWR);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, pid_ac_set_point_offset, SEEK_SET);
    ret += write(fd, &setpoint, 4);

    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_get_setpoint_bd(const char* path, float* setpoint)
{
    int fd = open(path, O_RDONLY);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, pid_bd_set_point_offset, SEEK_SET);
    ret += read(fd, setpoint, 4);

    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_set_setpoint_bd(const char* path, float setpoint)
{
    int fd = open(path, O_RDWR);
    int ret;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, pid_bd_set_point_offset, SEEK_SET);
    ret += write(fd, &setpoint, 4);

    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_get_temp_control_mode(const char* path, temp_ctrl_mode_t* mode)
{
    int fd = open(path, O_RDONLY);
    int ret;
    char buf;

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, temp_ctrl_mode_offset, SEEK_SET);
    ret += read(fd, &buf, 1);

    if (buf)
    {
        *mode = TEMP_CTRL_MANUAL;
    }
    else
    {
        *mode = TEMP_CTRL_AUTOMATIC;
    }

    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}

int config_set_temp_control_mode(const char* path, temp_ctrl_mode_t mode)
{
    int fd = open(path, O_RDWR);
    int ret;
    char buf = (mode == TEMP_CTRL_MANUAL);

    if (fd < 0)
    {
        return fd;
    }

    lseek(fd, temp_ctrl_mode_offset, SEEK_SET);
    ret += write(fd, &buf, 1);

    if (ret > 0) ret = 0;

    close(fd);
    return ret;
}
