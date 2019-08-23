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

#include "scpi_rffe_cmd.h"

scpi_result_t rffe_measure_temp_ac(scpi_t * context)
{
    uint8_t temp_raw[4];

    int fd = open("/dev/temp_ac", O_RDONLY);
    read(fd, temp_raw, 4);
    close(fd);

    SCPI_ResultDouble(context, (temp_raw[1] + (temp_raw[2] << 8)) / 256.0);
    return SCPI_RES_OK;
}

scpi_result_t rffe_measure_temp_bd(scpi_t * context)
{
    uint8_t temp_raw[4];

    int fd = open("/dev/temp_bd", O_RDONLY);
    read(fd, temp_raw, 4);
    close(fd);

    SCPI_ResultDouble(context, (temp_raw[1] + (temp_raw[2] << 8)) / 256.0);
    return SCPI_RES_OK;
}

scpi_result_t rffe_self_test(scpi_t * context)
{

    /*
     * Self-test not implemented
     */

    SCPI_ResultInt32(context, 0);

    return SCPI_RES_OK;
}
