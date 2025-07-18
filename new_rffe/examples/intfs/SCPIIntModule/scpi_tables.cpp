#include "scpi_tables.hpp"

const scpi_command_t scpi_commands[] =
{
    {.pattern = "SET:ATTEnuation", .callback = rffe_set_attenuation, },
    {.pattern = "GET:ATTEnuation?", .callback = rffe_get_attenuation, },
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
    {.pattern = "MEASure:TEMPerature:AC?", .callback = rffe_measure_temp_ac,},
    {.pattern = "MEASure:TEMPerature:BD?", .callback = rffe_measure_temp_bd,},

    SCPI_CMD_LIST_END
};
