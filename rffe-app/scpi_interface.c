/****************************************************************************
 * rffe-app/scpi_interface.c
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
#include <stdio.h>

#include "scpi_interface.h"

size_t SCPI_Write(scpi_t * context, const char * data, size_t len)
{
    user_data_t * u = (user_data_t *) (context->user_context);
    return write(u->sockfd, data, len);
}

int SCPI_Error(scpi_t * context, int_fast16_t err)
{
    printf("RFFE SCPI **ERROR: %ld, \"%s\"\n", (int32_t) err, SCPI_ErrorTranslate(err));
    return 0;
}

scpi_result_t SCPI_Flush(scpi_t * context)
{
    /*
     * Flush not implemented
     */
    return SCPI_RES_OK;
}

scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val)
{
    /*
     * Control not implemented
     */
    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t * context)
{
	/*
	 * Reset not implemented
	 */
    printf("RFFE SCPI **Reset\n");
    return SCPI_RES_OK;
}
