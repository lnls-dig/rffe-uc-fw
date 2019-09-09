/****************************************************************************
 * rffe-app/scpi_rffe_cmd.h
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

#ifndef SCPI_CMD_RFFE_H_
#define SCPI_CMD_RFFE_H_

#include "scpi/scpi.h"
#include "scpi-def.h"

scpi_result_t rffe_measure_temp_ac(scpi_t * context);
scpi_result_t rffe_measure_temp_bd(scpi_t * context);
scpi_result_t rffe_set_attenuation(scpi_t * context);
scpi_result_t rffe_self_test(scpi_t * context);

#endif
