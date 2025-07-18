#ifndef SCPI_CMD_RFFE_H_
#define SCPI_CMD_RFFE_H_

// #include "Dispatcher.hpp"
#include "SCPIIntModule.hpp"
#include "GlobalRFFE.hpp"

extern "C" {
    #include "scpi/scpi.h"
}

scpi_result_t rffe_set_attenuation(scpi_t* context);
scpi_result_t rffe_get_attenuation(scpi_t* context);

scpi_result_t rffe_set_ip_addr(scpi_t* context);
scpi_result_t rffe_get_ip_addr(scpi_t* context);

scpi_result_t rffe_set_gateway_addr(scpi_t* context);
scpi_result_t rffe_get_gateway_addr(scpi_t* context);

scpi_result_t rffe_set_netmask(scpi_t* context);
scpi_result_t rffe_get_netmask(scpi_t* context);

scpi_result_t rffe_set_dhcp_mode(scpi_t* context);
scpi_result_t rffe_get_dhcp_mode(scpi_t* context);

scpi_result_t rffe_get_version(scpi_t* context);
scpi_result_t rffe_reset(scpi_t* context);

scpi_result_t rffe_measure_temp_ac(scpi_t* context);
scpi_result_t rffe_measure_temp_bd(scpi_t* context);

#endif
