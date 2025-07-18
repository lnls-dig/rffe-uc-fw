#ifndef GLOBALRFFE_H_
#define GLOBALRFFE_H_
#include <cstdint>
#include <cstring>
#include "EthernetInterface.h"
#include "nsapi_types.h"
#include "DAT31R5SP.hpp"
#include "LM71.hpp"
#include "mbed.h"
#include "I2CEeprom.hpp"


// DAT31R5SP
#define LE_DAT31R5SP  P0_15
#define CLK_DAT31R5SP P0_17
#define DATA_A_DAT31R5SP P0_18
#define DATA_B_DAT31R5SP P2_0
#define DATA_C_DAT31R5SP P0_1
#define DATA_D_DAT31R5SP P0_11

// FERAM
#define FERAM_SDA  P0_19
#define FERAM_SCL P0_20
#define FERAM_WP P0_21

// LM71
# define LM71_SCLK1 P0_7
# define LM71_MISO1 P0_8
# define LM71_MOSI1 P0_9
# define LM71_CS_AC P2_1
# define LM71_CS_BD P0_16


typedef enum {
    ETH_ADDR_MODE_DHCP,
    ETH_ADDR_MODE_STATIC,
    ETH_ADDR_MODE_NONE,
} eth_addr_mode_t;

typedef enum {
    TEMP_CTRL_MANUAL,
    TEMP_CTRL_AUTOMATIC,
} temp_ctrl_mode_t;


class GlobalRFFE
{
    public:
        GlobalRFFE();

        ~GlobalRFFE() = default;

        // FERAM
        nsapi_error_t config_get_eth_mode(EthernetInterface & net);

        int config_get_version();

        int config_get_eth_addressing(eth_addr_mode_t* addr_mode);
        int config_set_eth_addressing(eth_addr_mode_t addr_mode);

        int config_get_mac_addr(uint8_t * mac, size_t len);
        int config_set_mac_addr(const char * mac);

        int config_get_ipv4_addr(char * ip, size_t len);
        int config_set_ipv4_addr(const char * ip);

        int config_get_mask_addr(char * mask, size_t len);
        int config_set_mask_addr(const char * mask);

        int config_get_gateway_addr(char * gateway, size_t len);
        int config_set_gateway_addr(const char * gateway);

        float config_get_attenuation();
        int config_set_attenuation(float att);

        // Just SCPI communication
        int stream_feram_raw();
        int get_measure_temp_ac(float &value);
        int get_measure_temp_bd(float &value);

    private:
        uint8_t _version;
        eth_addr_mode_t _addr_mode;
        uint8_t _mac[6];
        in_addr_t _ip;
        float _att;

        mbed::DigitalOut _le;
        mbed::DigitalOut _clk;
        mbed::DigitalOut _dtA;
        mbed::DigitalOut _dtB;
        mbed::DigitalOut _dtC;
        mbed::DigitalOut _dtD;

        mbed::DigitalOut _feram_wp;

        DAT31R5SP _dat31r5sp;
        I2CEeprom _feram;

        mbed::DigitalOut _cs_ac;
        mbed::DigitalOut _cs_bd;
        mbed::SPI _spi;
        LM71 _lm71_ac;
        LM71 _lm71_bd;

        // offsets
        static constexpr int _mac_addr_offset = 0;
        static constexpr int _config_version_offset = 0x0F;
        static constexpr int _ip_addr_offset = 0x10;
        static constexpr int _mask_addr_offset = 0x20;
        static constexpr int _gateway_addr_offset = 0x30;
        static constexpr int _attenuation_offset = 0x40;
        static constexpr int _eth_addr_offset = 0x50;
        static constexpr int _temp_ctrl_mode_offset = 0x51;
        static constexpr int _pid_ac_kc_offset = 0x60;
        static constexpr int _pid_ac_ti_offset = 0x64;
        static constexpr int _pid_ac_td_offset = 0x68;
        static constexpr int _pid_bd_kc_offset = 0x6C;
        static constexpr int _pid_bd_ti_offset = 0x70;
        static constexpr int _pid_bd_td_offset = 0x74;
        static constexpr int _pid_ac_set_point__offset = 0x78;
        static constexpr int _pid_bd_set_point__offset = 0x7C;
};
#endif
