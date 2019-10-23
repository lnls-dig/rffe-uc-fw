/****************************************************************************
 * rffe-app/pid.h
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

#ifndef PID_H_
#define PID_H_

typedef struct pid_ctrl_t
{
    float kp;
    float ki;
    float kd;
    float setpoint;
    float last_in;
    float inte_acc;
    float out_max;
    float out_min;
    float sample_time;
} pid_ctrl_t;

void pid_init(pid_ctrl_t* pid, float kp, float ki, float kd, float setpoint, float out_max, float out_min, float sample_time);
float pid_compute(pid_ctrl_t* pid, float input);

#endif
