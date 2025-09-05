#include "scpi_cmd.hpp"


scpi_result_t rffe_set_attenuation(scpi_t* context)
{
    float value;
    if(!SCPI_ParamFloat(context, &value, TRUE))
    {
        SCPI_ErrorPush(context, SCPI_ERROR_MISSING_PARAMETER);
        return SCPI_RES_ERR;
    }

    auto* holder    = static_cast<SCPICtx*>(context->user_context);
    auto* rffe = static_cast<GlobalRFFE*>(holder->ctx);
    rffe->config_set_attenuation(value);

    return SCPI_RES_OK;
}

scpi_result_t rffe_get_attenuation(scpi_t* context)
{
    float att;

    auto* holder    = static_cast<SCPICtx*>(context->user_context);
    auto* rffe = static_cast<GlobalRFFE*>(holder->ctx);

    att = rffe->config_get_attenuation();
    SCPI_ResultFloat(context, att);

    return SCPI_RES_OK;
}


scpi_result_t rffe_set_ip_addr(scpi_t* context)
{
    char ip[16];
    size_t copy_len;
    if(!SCPI_ParamCopyText(context, ip, sizeof(ip), &copy_len, FALSE))
    {
        SCPI_ErrorPush(context, SCPI_ERROR_MISSING_PARAMETER);
        return SCPI_RES_ERR;
    }

    auto* holder    = static_cast<SCPICtx*>(context->user_context);
    auto* rffe = static_cast<GlobalRFFE*>(holder->ctx);
    if(rffe->config_set_ipv4_addr(ip) != 0)
    {
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rffe_get_ip_addr(scpi_t* context)
{
    auto* holder = static_cast<SCPICtx*>(context->user_context);
    auto* rffe   = static_cast<GlobalRFFE*>(holder->ctx);

    char ip[16];
    size_t n = rffe->config_get_ipv4_addr(ip, sizeof(ip));
    if (n == 0) return SCPI_RES_ERR;

    SCPI_ResultCharacters(context, ip, static_cast<size_t>(n));

    return SCPI_RES_OK;
}

scpi_result_t rffe_set_gateway_addr(scpi_t* context)
{
    char gateway[16];
    size_t copy_len;
    if(!SCPI_ParamCopyText(context, gateway, sizeof(gateway), &copy_len, FALSE))
    {
        SCPI_ErrorPush(context, SCPI_ERROR_MISSING_PARAMETER);
        return SCPI_RES_ERR;
    }

    auto* holder    = static_cast<SCPICtx*>(context->user_context);
    auto* rffe = static_cast<GlobalRFFE*>(holder->ctx);
    if(rffe->config_set_gateway_addr(gateway) != 0)
    {
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rffe_get_gateway_addr(scpi_t* context)
{
    auto* holder = static_cast<SCPICtx*>(context->user_context);
    auto* rffe   = static_cast<GlobalRFFE*>(holder->ctx);

    char gateway[16];
    size_t n = rffe->config_get_gateway_addr(gateway, sizeof(gateway));
    if (n == 0) return SCPI_RES_ERR;

    SCPI_ResultCharacters(context, gateway, static_cast<size_t>(n));

    return SCPI_RES_OK;
}

scpi_result_t rffe_set_netmask(scpi_t* context)
{
    char mask[16];
    size_t copy_len;
    if(!SCPI_ParamCopyText(context, mask, sizeof(mask), &copy_len, FALSE))
    {
        SCPI_ErrorPush(context, SCPI_ERROR_MISSING_PARAMETER);
        return SCPI_RES_ERR;
    }

    auto* holder    = static_cast<SCPICtx*>(context->user_context);
    auto* rffe = static_cast<GlobalRFFE*>(holder->ctx);
    if(rffe->config_set_mask_addr(mask) != 0)
    {
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rffe_get_netmask(scpi_t* context)
{
    auto* holder = static_cast<SCPICtx*>(context->user_context);
    auto* rffe   = static_cast<GlobalRFFE*>(holder->ctx);

    char mask[16];
    size_t n = rffe->config_get_mask_addr(mask, sizeof(mask));
    if (n == 0) return SCPI_RES_ERR;


    SCPI_ResultCharacters(context, mask, static_cast<size_t>(n));
    return SCPI_RES_OK;
}

scpi_result_t rffe_set_dhcp_mode(scpi_t *context)
{
    auto* holder = static_cast<SCPICtx*>(context->user_context);
    auto* rffe   = static_cast<GlobalRFFE*>(holder->ctx);

    scpi_bool_t value;

    if(SCPI_ParamBool(context, &value, TRUE))
    {
        if (value)
        {
            rffe->config_set_eth_addressing(ETH_ADDR_MODE_DHCP);
        }
        else
        {
            rffe->config_set_eth_addressing(ETH_ADDR_MODE_STATIC);
        }
    }
    else
    {
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rffe_get_dhcp_mode(scpi_t *context)
{
    auto* holder = static_cast<SCPICtx*>(context->user_context);
    auto* rffe   = static_cast<GlobalRFFE*>(holder->ctx);

    eth_addr_mode_t dhcp;

    rffe->config_get_eth_addressing(&dhcp);

    if (dhcp == ETH_ADDR_MODE_STATIC)
    {
        SCPI_ResultInt32(context, 0);
    }
    else
    {
        SCPI_ResultInt32(context, 1);
    }

    return SCPI_RES_OK;
}


scpi_result_t rffe_get_version(scpi_t *context)
{
    return scpi_result_t();
}

scpi_result_t rffe_reset(scpi_t *context)
{
    NVIC_SystemReset();
    return SCPI_RES_OK;
}

scpi_result_t rffe_measure_temp_ac(scpi_t *context)
{
    float temp;

    auto* holder    = static_cast<SCPICtx*>(context->user_context);
    auto* rffe = static_cast<GlobalRFFE*>(holder->ctx);

    rffe->get_measure_temp_ac(temp);
    SCPI_ResultFloat(context, temp);

    return SCPI_RES_OK;
}

scpi_result_t rffe_measure_temp_bd(scpi_t *context)
{
    float temp;

    auto* holder    = static_cast<SCPICtx*>(context->user_context);
    auto* rffe = static_cast<GlobalRFFE*>(holder->ctx);

    rffe->get_measure_temp_bd(temp);
    SCPI_ResultFloat(context, temp);

    return SCPI_RES_OK;
}
