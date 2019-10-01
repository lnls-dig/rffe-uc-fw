/****************************************************************************
 * rffe-app/scpi_rffe_cmd.c
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

#include <netinet/in.h>
#include <sys/boardctl.h>
#include <sys/ioctl.h>
#include <nuttx/rf/ioctl.h>
#include <nuttx/rf/attenuator.h>
#include <arpa/inet.h>

#include "scpi_rffe_cmd.h"
#include "scpi_interface.h"
#include "config_file.h"

static const char* cfg_file = "/dev/feram0";
static const char* dac_file = "/dev/dac0";

scpi_result_t rffe_measure_temp_ac(scpi_t* context)
{
    b16_t temp = 0;
    scpi_result_t ret = SCPI_RES_OK;

    int fd = open("/dev/temp_ac", O_RDONLY);
    if (fd > 0)
    {
        read(fd, &temp, 4);
        close(fd);
        SCPI_ResultFloat(context, b16tof(temp));
    }
    else
    {
        SCPI_ResultFloat(context, 0);
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_measure_temp_bd(scpi_t* context)
{
    b16_t temp = 0;
    scpi_result_t ret = SCPI_RES_OK;

    int fd = open("/dev/temp_bd", O_RDONLY);
    if (fd > 0)
    {
        read(fd, &temp, 4);
        close(fd);
        SCPI_ResultFloat(context, b16tof(temp));
    }
    else
    {
        SCPI_ResultFloat(context, 0);
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_set_attenuation(scpi_t* context)
{
    struct attenuator_control att;
    scpi_number_t par;
    scpi_result_t ret = SCPI_RES_OK;

    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &par, TRUE))
    {
        SCPI_ErrorPush(context, SCPI_ERROR_MISSING_PARAMETER);
        ret = SCPI_RES_ERR;
    }
    else
    {
        att.attenuation = ftob16(par.content.value);

        int fd = open("/dev/att0", O_RDONLY);
        ioctl(fd, RFIOC_SETATT, (unsigned long)&att);
        close(fd);

        config_set_attenuation(cfg_file, att.attenuation);
    }

    return ret;
}

scpi_result_t rffe_get_attenuation(scpi_t* context)
{
    b16_t att;

    config_get_attenuation(cfg_file, &att);
    SCPI_ResultFloat(context, b16tof(att));

    return SCPI_RES_OK;
}

scpi_result_t rffe_self_test(scpi_t* context)
{

    /*
     * Self-test not implemented
     */

    SCPI_ResultInt32(context, 0);

    return SCPI_RES_OK;
}

scpi_result_t rffe_set_temp_ac(scpi_t* context)
{
    scpi_number_t par;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamNumber(context, scpi_special_numbers_def, &par, TRUE))
    {
        config_set_setpoint_ac(cfg_file, par.content.value);
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_set_temp_bd(scpi_t* context)
{
    scpi_number_t par;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamNumber(context, scpi_special_numbers_def, &par, TRUE))
    {
        config_set_setpoint_bd(cfg_file, par.content.value);
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_get_temp_ac(scpi_t* context)
{
    float temp;

    config_get_setpoint_ac(cfg_file, &temp);
    SCPI_ResultFloat(context, temp);

    return SCPI_RES_OK;
}

scpi_result_t rffe_get_temp_bd(scpi_t* context)
{
    float temp;

    config_get_setpoint_bd(cfg_file, &temp);
    SCPI_ResultFloat(context, temp);

    return SCPI_RES_OK;
}

scpi_result_t rffe_set_pid_kc_ac(scpi_t* context)
{
    float kc, ti, td;
    scpi_number_t par;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamNumber(context, scpi_special_numbers_def, &par, TRUE))
    {
        config_get_pid_ac(cfg_file, &kc, &ti, &td);
        kc = par.content.value;
        config_set_pid_ac(cfg_file, kc, ti, td);
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_set_pid_ti_ac(scpi_t* context)
{
    float kc, ti, td;
    scpi_number_t par;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamNumber(context, scpi_special_numbers_def, &par, TRUE))
    {
        config_get_pid_ac(cfg_file, &kc, &ti, &td);
        ti = par.content.value;
        config_set_pid_ac(cfg_file, kc, ti, td);
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_set_pid_td_ac(scpi_t* context)
{
    float kc, ti, td;
    scpi_number_t par;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamNumber(context, scpi_special_numbers_def, &par, TRUE))
    {
        config_get_pid_ac(cfg_file, &kc, &ti, &td);
        td = par.content.value;
        config_set_pid_ac(cfg_file, kc, ti, td);
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_set_pid_kc_bd(scpi_t* context)
{
    float kc, ti, td;
    scpi_number_t par;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamNumber(context, scpi_special_numbers_def, &par, TRUE))
    {
        config_get_pid_bd(cfg_file, &kc, &ti, &td);
        kc = par.content.value;
        config_set_pid_bd(cfg_file, kc, ti, td);
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_set_pid_ti_bd(scpi_t* context)
{
    float kc, ti, td;
    scpi_number_t par;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamNumber(context, scpi_special_numbers_def, &par, TRUE))
    {
        config_get_pid_bd(cfg_file, &kc, &ti, &td);
        ti = par.content.value;
        config_set_pid_bd(cfg_file, kc, ti, td);
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_set_pid_td_bd(scpi_t* context)
{
    float kc, ti, td;
    scpi_number_t par;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamNumber(context, scpi_special_numbers_def, &par, TRUE))
    {
        config_get_pid_bd(cfg_file, &kc, &ti, &td);
        td = par.content.value;
        config_set_pid_bd(cfg_file, kc, ti, td);
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_get_pid_kc_ac(scpi_t* context)
{
    float kc, ti, td;

    config_get_pid_ac(cfg_file, &kc, &ti, &td);
    SCPI_ResultFloat(context, kc);

    return SCPI_RES_OK;
}

scpi_result_t rffe_get_pid_ti_ac(scpi_t* context)
{
    float kc, ti, td;

    config_get_pid_ac(cfg_file, &kc, &ti, &td);
    SCPI_ResultFloat(context, ti);

    return SCPI_RES_OK;
}

scpi_result_t rffe_get_pid_td_ac(scpi_t* context)
{
    float kc, ti, td;

    config_get_pid_ac(cfg_file, &kc, &ti, &td);
    SCPI_ResultFloat(context, td);

    return SCPI_RES_OK;
}

scpi_result_t rffe_get_pid_kc_bd(scpi_t* context)
{
    float kc, ti, td;

    config_get_pid_bd(cfg_file, &kc, &ti, &td);
    SCPI_ResultFloat(context, kc);

    return SCPI_RES_OK;
}

scpi_result_t rffe_get_pid_ti_bd(scpi_t* context)
{
    float kc, ti, td;

    config_get_pid_bd(cfg_file, &kc, &ti, &td);
    SCPI_ResultFloat(context, ti);

    return SCPI_RES_OK;
}

scpi_result_t rffe_get_pid_td_bd(scpi_t* context)
{
    float kc, ti, td;

    config_get_pid_bd(cfg_file, &kc, &ti, &td);
    SCPI_ResultFloat(context, td);

    return SCPI_RES_OK;
}

scpi_result_t rffe_set_temp_control(scpi_t* context)
{
    scpi_bool_t value;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamBool(context, &value, TRUE))
    {
        if (value)
        {
            config_set_temp_control_mode(cfg_file, TEMP_CTRL_AUTOMATIC);
        }
        else
        {
            config_set_temp_control_mode(cfg_file, TEMP_CTRL_MANUAL);
        }
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_get_temp_control(scpi_t* context)
{
    temp_ctrl_mode_t mode;

    config_get_temp_control_mode(cfg_file, &mode);

    if (mode == TEMP_CTRL_MANUAL)
    {
        SCPI_ResultInt32(context, 0);
    }
    else
    {
        SCPI_ResultInt32(context, 1);
    }

    return SCPI_RES_OK;
}

scpi_result_t rffe_set_dac_output_ac(scpi_t* context)
{
    user_data_t* user_context = (user_data_t*)context->user_context;
    uint16_t dac_val;
    uint8_t buf[3];
    scpi_number_t par;
    scpi_result_t ret = SCPI_RES_OK;

    int fd = open(dac_file, O_RDWR);
    if (fd < 0)
    {
        SCPI_ResultFloat(context, 0);
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
        ret = SCPI_RES_ERR;
    }
    else
    {
        if (SCPI_ParamNumber(context, scpi_special_numbers_def, &par, TRUE))
        {
            *(user_context->dac_ac) = par.content.value;
            dac_val = (par.content.value / 3.3) * 4095.0;

            if (dac_val > 4095)
            {
                SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
                ret = SCPI_RES_ERR;
            }

            buf[0] = 3;
            buf[1] = dac_val & 0xFF;
            buf[2] = (dac_val >> 8) & 0xFF;
            write(fd, buf, 3);
        }
        else
        {
            ret = SCPI_RES_ERR;
        }
        close(fd);
    }

    return ret;
}

scpi_result_t rffe_set_dac_output_bd(scpi_t* context)
{
    user_data_t* user_context = (user_data_t*)context->user_context;
    uint16_t dac_val;
    uint8_t buf[3];
    scpi_number_t par;
    scpi_result_t ret = SCPI_RES_OK;

    int fd = open(dac_file, O_RDWR);
    if (fd < 0)
    {
        SCPI_ResultFloat(context, 0);
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
        ret = SCPI_RES_ERR;
    }
    else
    {
        if (SCPI_ParamNumber(context, scpi_special_numbers_def, &par, TRUE))
        {
            *(user_context->dac_bd) = par.content.value;
            dac_val = (par.content.value / 3.3) * 4095.0;

            if (dac_val > 4095)
            {
                SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
                ret = SCPI_RES_ERR;
            }

            buf[0] = 2;
            buf[1] = dac_val & 0xFF;
            buf[2] = (dac_val >> 8) & 0xFF;
            write(fd, buf, 3);
        }
        else
        {
            ret = SCPI_RES_ERR;
        }
        close(fd);
    }

    return ret;
}

scpi_result_t rffe_get_dac_output_ac(scpi_t* context)
{
    user_data_t* user_context = (user_data_t*)context->user_context;

    SCPI_ResultFloat(context, *(user_context->dac_ac));

    return SCPI_RES_OK;
}

scpi_result_t rffe_get_dac_output_bd(scpi_t* context)
{
    user_data_t* user_context = (user_data_t*)context->user_context;

    SCPI_ResultFloat(context, *(user_context->dac_bd));

    return SCPI_RES_OK;
}

scpi_result_t rffe_set_ip_addr(scpi_t* context)
{
    char buf[32];
    size_t copy_len;
    in_addr_t ipaddr;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamCopyText(context, buf, sizeof (buf), &copy_len, FALSE))
    {
        int valid_ip = inet_pton(AF_INET, buf, &ipaddr);
        if (valid_ip)
        {
            config_set_ipv4_addr(cfg_file, ipaddr);
        }
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_get_ip_addr(scpi_t* context)
{
    struct in_addr ipaddr;

    config_get_ipv4_addr(cfg_file, &ipaddr.s_addr);
    char* ipstr = inet_ntoa(ipaddr);
    SCPI_ResultCharacters(context, ipstr, strlen(ipstr));

    return SCPI_RES_OK;
}

scpi_result_t rffe_set_gateway_addr(scpi_t* context)
{
    char buf[32];
    size_t copy_len;
    in_addr_t gateway;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamCopyText(context, buf, sizeof (buf), &copy_len, FALSE))
    {
        int valid_ip = inet_pton(AF_INET, buf, &gateway);
        if (valid_ip)
        {
            config_set_gateway_addr(cfg_file, gateway);
        }
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_get_gateway_addr(scpi_t* context)
{
    struct in_addr gatewayaddr;

    config_get_gateway_addr(cfg_file, &gatewayaddr.s_addr);
    char* ipstr = inet_ntoa(gatewayaddr);

    SCPI_ResultCharacters(context, ipstr, strlen(ipstr));

    return SCPI_RES_OK;
}

scpi_result_t rffe_set_netmask(scpi_t* context)
{
    char buf[32];
    size_t copy_len;
    in_addr_t netmask;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamCopyText(context, buf, sizeof (buf), &copy_len, FALSE))
    {
        int valid_ip = inet_pton(AF_INET, buf, &netmask);
        if (valid_ip)
        {
            config_set_mask_addr(cfg_file, netmask);
        }
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_get_netmask(scpi_t* context)
{
    struct in_addr netmask;

    config_get_mask_addr(cfg_file, &netmask.s_addr);
    char* ipstr = inet_ntoa(netmask);
    SCPI_ResultCharacters(context, ipstr, strlen(ipstr));

    return SCPI_RES_OK;
}

scpi_result_t rffe_set_dhcp_mode(scpi_t* context)
{
    scpi_bool_t value;
    scpi_result_t ret = SCPI_RES_OK;

    if (SCPI_ParamBool(context, &value, TRUE))
    {
        if (value)
        {
            config_set_eth_addressing(cfg_file, ETH_ADDR_MODE_DHCP);
        }
        else
        {
            config_set_eth_addressing(cfg_file, ETH_ADDR_MODE_STATIC);
        }
    }
    else
    {
        ret = SCPI_RES_ERR;
    }

    return ret;
}

scpi_result_t rffe_get_dhcp_mode(scpi_t* context)
{
    eth_addr_mode_t dhcp;

    config_get_eth_addressing(cfg_file, &dhcp);

    if (dhcp == ETH_ADDR_MODE_STATIC)
    {
        SCPI_ResultInt32(context, 0);
    }
    else
    {
        SCPI_ResultInt32(context, 1);
    }

    return SCPI_RES_OK;
}

scpi_result_t rffe_reset(scpi_t* context)
{
    boardctl(BOARDIOC_RESET, 0);

    return SCPI_RES_OK;
}
