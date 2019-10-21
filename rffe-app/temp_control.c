/****************************************************************************
 * rffe-app/temp_control.c
 *
 *   Copyright (C) 2019 Augusto Fraga Giachero. All rights reserved.
 *   Author: Augusto Fraga Giachero <afg@augustofg.net>
 *   Inspired by Arduino PID Library by Brett Beauregard
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
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "pid.h"
#include "config_file.h"

static const char* cfg_file = "/dev/feram0";
static const char* dac_file = "/dev/dac0";

struct dac_write_back
{
    float* dac_ac;
    float* dac_bd;
};

static void* temp_control_server(void* args)
{
    pid_ctrl_t pid_ac, pid_bd;
    float out_ac, out_bd, temp_ac, temp_bd;
    int temp_ac_fd, temp_bd_fd, dac_fd, ret;
    b16_t temp;
    uint8_t buf[3];
    uint16_t dac_val;
    temp_ctrl_mode_t tctrl;
    struct dac_write_back* dac_out = args;

    temp_ac_fd = open("/dev/temp_ac", O_RDONLY);
    temp_bd_fd = open("/dev/temp_bd", O_RDONLY);
    dac_fd = open(dac_file, O_RDWR);

    if (temp_ac_fd < 0 || temp_bd_fd < 0)
    {
        puts("Temperature control error: temperature sensors not found!\n");
        return NULL;
    }
    if (dac_fd < 0)
    {
        puts("DAC device not found!\n");
        return NULL;
    }

    pid_ac.inte_acc = 0.0;
    pid_ac.last_in = 0.0;
    pid_ac.out_max = 3.3;
    pid_ac.out_min = 0.0;
    pid_ac.sample_time = 0.1;
    pid_bd.inte_acc = 0.0;
    pid_bd.last_in = 0.0;
    pid_bd.out_max = 3.3;
    pid_bd.out_min = 0.0;
    pid_bd.sample_time = 0.1;

    while(1)
    {
        ret = config_get_temp_control_mode(cfg_file, &tctrl);

        if (tctrl == TEMP_CTRL_AUTOMATIC)
        {
            config_get_pid_bd(cfg_file, &pid_bd.kp, &pid_bd.ki, &pid_bd.kd);
            config_get_pid_ac(cfg_file, &pid_ac.kp, &pid_ac.ki, &pid_ac.kd);
            config_get_setpoint_ac(cfg_file, &pid_ac.setpoint);
            config_get_setpoint_bd(cfg_file, &pid_bd.setpoint);

            read(temp_ac_fd, &temp, 4);
            temp_ac = b16tof(temp);
            read(temp_bd_fd, &temp, 4);
            temp_bd = b16tof(temp);

            out_ac = pid_compute(&pid_ac, temp_ac);
            out_bd = pid_compute(&pid_bd, temp_bd);

            *dac_out->dac_ac = out_ac;
            *dac_out->dac_bd = out_bd;

            dac_val = (out_ac / 3.3) * 4095.0;
            buf[0] = 3;
            buf[1] = dac_val & 0xFF;
            buf[2] = (dac_val >> 8) & 0xFF;
            write(dac_fd, buf, 3);

            dac_val = (out_bd / 3.3) * 4095.0;
            buf[0] = 2;
            buf[1] = dac_val & 0xFF;
            buf[2] = (dac_val >> 8) & 0xFF;
            write(dac_fd, buf, 3);

        }
        usleep(100000);
    }

    free(args);
    return NULL;
}

void start_temp_control_server(float* dac_ac, float* dac_bd)
{
    pthread_t thread;
    pthread_attr_t attr;

    struct dac_write_back* arg = malloc(sizeof(struct dac_write_back));

    arg->dac_ac = dac_ac;
    arg->dac_bd = dac_bd;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 768);
    pthread_create(&thread, &attr, &temp_control_server, arg);
    pthread_detach(thread);
}
