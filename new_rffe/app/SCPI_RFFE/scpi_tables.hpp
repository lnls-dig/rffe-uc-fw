/**
 *    @file     scpi_tables.hpp
 *    @author   João Victor Silva e Santos (joao.ssantos@lnls.br)
 *
 *    @brief Header file declaring the SCPI command table for the RFFE module.
 *
 *    @details
 *      This file provides the global SCPI command table (`scpi_commands`) that
 *      maps command patterns to their corresponding handler functions defined
 *      in `scpi_cmd.hpp` / `scpi_cmd.cpp`.
 *
 *      The supported SCPI commands include:
 *        - SET:ATTEnuation / GET:ATTEnuation? → Configure or read attenuation
 *        - SET:IPAddr / GET:IPAddr? → Configure or read device IPv4 address
 *        - SET:GATEwayaddr / GET:GATEwayaddr? → Configure or read gateway address
 *        - SET:NETMask / GET:NETMask? → Configure or read subnet mask
 *        - SET:DHCPMode / GET:DHCPMode? → Configure or query DHCP/static mode
 *        - GET:VERsion? → Query firmware version
 *        - SYSTem:RESet → Reset the device
 *        - MEASure:TEMPerature:AC? / BD? → Measure temperatures at AC and BD sensors
 *
 *      This table is used by the SCPI parser to route incoming commands to the
 *      correct implementation.
 *
 *    @date 2025
 *
 *    @copyright
 *      Copyright (C) 2025 CNPEM (cnpem.br)
 *
 *    rffe-uc-fw is free software: you can redistribute it and/or modify
*     it under the terms of the GNU General Public License as published by
*     the Free Software Foundation, either version 3 of the License, or
*     (at your option) any later version.
*
*     rffe-uc-fw is distributed in the hope that it will be useful,
*     but WITHOUT ANY WARRANTY; without even the implied warranty of
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*     GNU General Public License for more details.
*
*     You should have received a copy of the GNU General Public License
*     along with RFFE.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SCPI_TABLES_H_
#define SCPI_TABLES_H_

#include "scpi_cmd.hpp"

extern "C" {
    #include "scpi/scpi.h"
}

/**
 * @brief Global SCPI command table mapping command patterns to handlers.
 */
extern const scpi_command_t scpi_commands[];

#endif
