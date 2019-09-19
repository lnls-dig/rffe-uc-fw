/****************************************************************************
 * rffe-app/config_file.c
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
#ifndef CONFIG_FILE_H_
#define CONFIG_FILE_H_

#include <stdint.h>
#include <fixedmath.h>
#include <netinet/in.h>

typedef enum
{
    ETH_ADDR_MODE_DHCP,
    ETH_ADDR_MODE_STATIC,
    ETH_ADDR_MODE_NONE,
} eth_addr_mode_t;

/**
 * @brief Read the config file version
 * @param version : A pointer to store the version read
 * @return 0 if success, a negative number otherwise
 */
int config_get_version(const char* path, uint8_t* version);

/**
 * @brief Write the config file version
 * @param version : The version number to be written
 * @return 0 if success, a negative number otherwise
 */
int config_set_version(const char* path, uint8_t version);

/**
 * @brief Read the address mode from the config file (static / dhcp)
 * @param addr_mode : A pointer to store the address mode read
 * @return 0 if success, a negative number otherwise
 */
int config_get_eth_addressing(const char* path, eth_addr_mode_t* addr_mode);

/**
 * @brief Write the address mode to the config file (static / dhcp)
 * @param addr_mode : The address mode
 * @return 0 if success, a negative number otherwise
 */
int config_set_eth_addressing(const char* path, eth_addr_mode_t addr_mode);

/**
 * @brief Read the mac address from the config file
 * @param mac : A pointer to store the mac address read
 * @return 0 if success, a negative number otherwise
 */
int config_get_mac_addr(const char* path, uint8_t mac[6]);

/**
 * @brief Write the mac address to the config file
 * @param mac : A pointer to the the mac address buffer
 * @return 0 if success, a negative number otherwise
 */
int config_set_mac_addr(const char* path, const uint8_t mac[6]);

/**
 * @brief Read the ip address from the config file
 * @param ip : A pointer to store the ip address read
 * @return 0 if success, a negative number otherwise
 */
int config_get_ipv4_addr(const char* path, in_addr_t* ip);

/**
 * @brief Write the ip address to the config file
 * @param ip : The ip address to be saved (network order)
 * @return 0 if success, a negative number otherwise
 */
int config_set_ipv4_addr(const char* path, in_addr_t ip);

/**
 * @brief Read the network mask from the config file
 * @param mask : A pointer to store the network mask read
 * @return 0 if success, a negative number otherwise
 */
int config_get_mask_addr(const char* path, in_addr_t* mask);

/**
 * @brief Write the network mask to the config file
 * @param mask : The mask to be saved (network order)
 * @return 0 if success, a negative number otherwise
 */
int config_set_mask_addr(const char* path, in_addr_t mask);

/**
 * @brief Read the gateway address from the config file
 * @param gateway : A pointer to store the gateway address read
 * @return 0 if success, a negative number otherwise
 */
int config_get_gateway_addr(const char* path, in_addr_t* gateway);

/**
 * @brief Write the gateway address to the config file
 * @param gateway : The gateway address to be saved (network order)
 * @return 0 if success, a negative number otherwise
 */
int config_set_gateway_addr(const char* path, in_addr_t gateway);

/**
 * @brief Read the attenuation value from the config file
 * @param att : A pointer to store attenuation value read
 * @return 0 if success, a negative number otherwise
 */
int config_get_attenuation(const char* path, b16_t *att);

/**
 * @brief Write the address value to the config file
 * @param att : The attenuation value
 * @return 0 if success, a negative number otherwise
 */
int config_set_attenuation(const char* path, b16_t att);

/**
 * @brief Read the PID constants for the AC channel from the config
 * file
 * @param kc: Proportional constant (pointer)
 * @param ti: Integrative constant (pointer)
 * @param td: Derivative constant (pointer)
 * @return 0 if success, a negative number otherwise
 */
int config_get_pid_ac(const char* path, float* kc, float* ti, float* td);

/**
 * @brief Write PID constants for the AC channel to the config file
 * @param kc: Proportional constant
 * @param ti: Integrative constant
 * @param td: Derivative constant
 * @return 0 if success, a negative number otherwise
 */
int config_set_pid_ac(const char* path, float kc, float ti, float td);

/**
 * @brief Read the PID constants for the BD channel from the config
 * file
 * @param kc: Proportional constant (pointer)
 * @param ti: Integrative constant (pointer)
 * @param td: Derivative constant (pointer)
 * @return 0 if success, a negative number otherwise
 */
int config_get_pid_bd(const char* path, float* kc, float* ti, float* tc);

/**
 * @brief Write PID constants for the BD channel to the config file
 * @param kc: Proportional constant
 * @param ti: Integrative constant
 * @param td: Derivative constant
 * @return 0 if success, a negative number otherwise
 */
int config_set_pid_bd(const char* path, float kc, float ti, float td);

/**
 * @brief Read the temperature setpoint for the AC channel from the
 * config file
 * @param setpoint: Temperature setpoint in C (pointer)
 * @return 0 if success, a negative number otherwise
 */
int config_get_setpoint_ac(const char* path, float* setpoint);

/**
 * @brief Write the temperature setpoint for the AC channel to the
 * config file
 * @param setpoint: Temperature setpoint in C
 * @return 0 if success, a negative number otherwise
 */
int config_set_setpoint_ac(const char* path, float setpoint);

/**
 * @brief Read the temperature setpoint for the BD channel from the
 * config file
 * @param setpoint: Temperature setpoint in C (pointer)
 * @return 0 if success, a negative number otherwise
 */
int config_get_setpoint_bd(const char* path, float* setpoint);

/**
 * @brief Write the temperature setpoint for the BD channel to the
 * config file
 * @param setpoint: Temperature setpoint in C
 * @return 0 if success, a negative number otherwise
 */
int config_set_setpoint_bd(const char* path, float setpoint);


#endif
