/****************************************************************************
 * rffe-app/scpi_tables.c
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

#include "scpi_rffe_cmd.h"

const scpi_command_t scpi_commands[] =
{
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    { .pattern = "*CLS", .callback = SCPI_CoreCls,},
    { .pattern = "*ESE", .callback = SCPI_CoreEse,},
    { .pattern = "*ESE?", .callback = SCPI_CoreEseQ,},
    { .pattern = "*ESR?", .callback = SCPI_CoreEsrQ,},
    { .pattern = "*IDN?", .callback = SCPI_CoreIdnQ,},
    { .pattern = "*OPC", .callback = SCPI_CoreOpc,},
    { .pattern = "*OPC?", .callback = SCPI_CoreOpcQ,},
    { .pattern = "*RST", .callback = SCPI_CoreRst,},
    { .pattern = "*SRE", .callback = SCPI_CoreSre,},
    { .pattern = "*SRE?", .callback = SCPI_CoreSreQ,},
    { .pattern = "*STB?", .callback = SCPI_CoreStbQ,},
    { .pattern = "*TST?", .callback = rffe_self_test,},
    { .pattern = "*WAI", .callback = SCPI_CoreWai,},

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {.pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ,},
    {.pattern = "SYSTem:ERRor:COUNt?", .callback = SCPI_SystemErrorCountQ,},
    {.pattern = "SYSTem:VERSion?", .callback = SCPI_SystemVersionQ,},

    {.pattern = "STATus:QUEStionable[:EVENt]?", .callback = SCPI_StatusQuestionableEventQ,},
    {.pattern = "STATus:QUEStionable:ENABle", .callback = SCPI_StatusQuestionableEnable,},
    {.pattern = "STATus:QUEStionable:ENABle?", .callback = SCPI_StatusQuestionableEnableQ,},

    {.pattern = "STATus:PRESet", .callback = SCPI_StatusPreset,},

	/*
	 * Specific commands for the RFFE
	 */
    {.pattern = "MEASure:TEMPerature:AC?", .callback = rffe_measure_temp_ac,},
    {.pattern = "MEASure:TEMPerature:BD?", .callback = rffe_measure_temp_bd,},
    {.pattern = "SET:ATTEnuation", .callback = rffe_set_attenuation,},
    {.pattern = "GET:ATTEnuation?", .callback = rffe_get_attenuation,},
    {.pattern = "SET:TEMPerature:SETPoint:AC", .callback = rffe_set_temp_ac,},
    {.pattern = "SET:TEMPerature:SETPoint:BD", .callback = rffe_set_temp_bd,},
    {.pattern = "GET:TEMPerature:SETPoint:AC?", .callback = rffe_get_temp_ac,},
    {.pattern = "GET:TEMPerature:SETPoint:BD?", .callback = rffe_get_temp_bd,},
    {.pattern = "SET:PID:Kc:AC", .callback = rffe_set_pid_kc_ac,},
    {.pattern = "SET:PID:Ti:AC", .callback = rffe_set_pid_ti_ac,},
    {.pattern = "SET:PID:Td:AC", .callback = rffe_set_pid_td_ac,},
    {.pattern = "SET:PID:Kc:BD", .callback = rffe_set_pid_kc_bd,},
    {.pattern = "SET:PID:Ti:BD", .callback = rffe_set_pid_ti_bd,},
    {.pattern = "SET:PID:Td:BD", .callback = rffe_set_pid_td_bd,},
    {.pattern = "GET:PID:Kc:AC?", .callback = rffe_get_pid_kc_ac,},
    {.pattern = "GET:PID:Ti:AC?", .callback = rffe_get_pid_ti_ac,},
    {.pattern = "GET:PID:Td:AC?", .callback = rffe_get_pid_td_ac,},
    {.pattern = "GET:PID:Kc:BD?", .callback = rffe_get_pid_kc_bd,},
    {.pattern = "GET:PID:Ti:BD?", .callback = rffe_get_pid_ti_bd,},
    {.pattern = "GET:PID:Td:BD?", .callback = rffe_get_pid_td_bd,},
    {.pattern = "SET:TEMPControl:AUTOmatic", .callback = rffe_set_temp_control,},
    {.pattern = "GET:TEMPControl:AUTOmatic?", .callback = rffe_get_temp_control,},
    {.pattern = "SET:DAC:OUTput:AC", .callback = rffe_set_dac_output_ac,},
    {.pattern = "SET:DAC:OUTput:BD", .callback = rffe_set_dac_output_bd,},
    {.pattern = "GET:DAC:OUTput:AC?", .callback = rffe_get_dac_output_ac,},
    {.pattern = "GET:DAC:OUTput:BD?", .callback = rffe_get_dac_output_bd,},
    {.pattern = "SET:IPAddr", .callback = rffe_set_ip_addr,},
    {.pattern = "GET:IPAddr?", .callback = rffe_get_ip_addr,},
    {.pattern = "SET:GATEwayaddr", .callback = rffe_set_gateway_addr,},
    {.pattern = "GET:GATEwayaddr?", .callback = rffe_get_gateway_addr,},
    {.pattern = "SET:NETMask", .callback = rffe_set_netmask,},
    {.pattern = "GET:NETMask?", .callback = rffe_get_netmask,},
    {.pattern = "SET:DHCPMode", .callback = rffe_set_dhcp_mode,},
    {.pattern = "GET:DHCPMode?", .callback = rffe_get_dhcp_mode,},
    {.pattern = "GET:VERsion?", .callback = rffe_get_version,},
    {.pattern = "SYSTem:RESet", .callback = rffe_reset,},

    SCPI_CMD_LIST_END
};

scpi_interface_t scpi_interface =
{
    .error = SCPI_Error,
    .write = SCPI_Write,
    .control = SCPI_Control,
    .flush = SCPI_Flush,
    .reset = SCPI_Reset,
};
