/**
 *    @file     scpi_cmd_rffe.hpp
 *    @author   João Victor Santos (joao.ssantos@lnls.br)
 *
 *    @brief Header file declaring SCPI command handlers for the RFFE module.
 *
 *    @details
 *      Provides SCPI command interfaces for controlling and monitoring the
 *      RFFE (Radio Frequency Front-End) module. Supported functionality includes:
 *        - Setting and querying attenuation values
 *        - Configuring network parameters (IP, gateway, netmask, DHCP)
 *        - Resetting the device
 *        - Reading firmware version
 *        - Measuring internal temperatures from different sensors
 *
 *      Each function defined here is implemented in `scpi_cmd.cpp` and follows
 *      the SCPI standard command/response mechanism.
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

#ifndef SCPI_CMD_RFFE_H_
#define SCPI_CMD_RFFE_H_

#include "SCPIIntfModule.hpp"
#include "GlobalRFFE.hpp"

extern "C" {
    #include "scpi/scpi.h"
}

/**
 * @brief Set attenuation value in dB for the RFFE.
 *
 * SCPI command: SET:ATTEnuation <value>
 */
scpi_result_t rffe_set_attenuation(scpi_t* context);

/**
 * @brief Get the current attenuation value in dB from the RFFE.
 *
 * SCPI command: GET:ATTEnuation?
 */
scpi_result_t rffe_get_attenuation(scpi_t* context);

/**
 * @brief Set IPv4 address of the RFFE device.
 *
 * SCPI command: SET:IPAddr <addr>
 */
scpi_result_t rffe_set_ip_addr(scpi_t* context);

/**
 * @brief Get IPv4 address of the RFFE device.
 *
 * SCPI command: GET:IPAddr?
 */
scpi_result_t rffe_get_ip_addr(scpi_t* context);

/**
 * @brief Set gateway IPv4 address.
 *
 * SCPI command: SET:GATEwayaddr <addr>
 */
scpi_result_t rffe_set_gateway_addr(scpi_t* context);

/**
 * @brief Get gateway IPv4 address.
 *
 * SCPI command: GET:GATEwayaddr?
 */
scpi_result_t rffe_get_gateway_addr(scpi_t* context);

/**
 * @brief Set network mask (subnet mask).
 *
 * SCPI command: SET:NETMask <addr>
 */
scpi_result_t rffe_set_netmask(scpi_t* context);

/**
 * @brief Get network mask (subnet mask).
 *
 * SCPI command: GET:NETMask?
 */
scpi_result_t rffe_get_netmask(scpi_t* context);

/**
 * @brief Enable or disable DHCP mode.
 *
 * SCPI command: SET:DHCPMode <0|1>
 *  - 0: Static addressing
 *  - 1: DHCP enabled
 */
scpi_result_t rffe_set_dhcp_mode(scpi_t* context);

/**
 * @brief Query current DHCP mode.
 *
 * SCPI command: GET:DHCPMode?
 *  - 0: Static addressing
 *  - 1: DHCP enabled
 */
scpi_result_t rffe_get_dhcp_mode(scpi_t* context);

/**
 * @brief Get firmware or hardware version string.
 *
 * SCPI command: GET:VERsion?
 */
scpi_result_t rffe_get_version(scpi_t* context);

/**
 * @brief Reset the RFFE device via system reset.
 *
 * SCPI command: SYSTem:RESet
 */
scpi_result_t rffe_reset(scpi_t* context);

/**
 * @brief Measure temperature at AC sensor location.
 *
 * SCPI command: MEASure:TEMPerature:AC?
 */
scpi_result_t rffe_measure_temp_ac(scpi_t* context);

/**
 * @brief Measure temperature at BD sensor location.
 *
 * SCPI command: MEASure:TEMPerature:BD?
 */
scpi_result_t rffe_measure_temp_bd(scpi_t* context);

#endif // SCPI_CMD_RFFE_H_
