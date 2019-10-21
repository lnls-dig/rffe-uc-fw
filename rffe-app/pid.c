/****************************************************************************
 * rffe-app/pid.c
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
#include "pid.h"

void pid_init(pid_ctrl_t* pid, float kp, float ki, float kd, float setpoint, float out_max, float out_min, float sample_time)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->last_in = 0;
    pid->inte_acc = 0;
    pid->out_max = out_max;
    pid->out_min = out_min;
    pid->sample_time = sample_time;
}

float pid_compute(pid_ctrl_t* pid, float input)
{
    float error = pid->setpoint - input;
    float ki_eq = pid->ki * pid->sample_time;
    float kd_eq = pid->kd / pid->sample_time;
    float din, output;

    pid->inte_acc += (ki_eq * error);

    if (pid->inte_acc > pid->out_max)
    {
        pid->inte_acc = pid->out_max;
    }
    else if (pid->inte_acc < pid->out_min)
    {
        pid->inte_acc = pid->out_min;
    }

    din = input - pid->last_in;
    output = pid->kp * error + pid->inte_acc - (kd_eq * din);

    if (output > pid->out_max)
    {
        output = pid->out_max;
    }
    else if (output < pid->out_min)
    {
        output = pid->out_min;
    }

    pid->last_in = input;
    return output;
}
