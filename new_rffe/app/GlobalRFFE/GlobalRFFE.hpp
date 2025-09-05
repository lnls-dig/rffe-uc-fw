/**
 *    @file     GlobalRFFE.hpp
 *    @author   João Victor Silva e Santos (joao.ssantos@lnls.br)
 *
 *    @brief Global context and services for the RFFE (Radio Frequency Front-End).
 *
 *    @details
 *      Provides the central control object for the RFFE system, wiring up:
 *        - Digital attenuator Mini-Circuits DAT-31R5-SP+ (via GPIO bit-banging)
 *        - FRAM / I²C EEPROM access for persistent configuration (network, attenuation, etc.)
 *        - Ethernet configuration helper (DHCP/static fallback logic)
 *        - Dual LM71 temperature sensors over shared SPI (chip-select per sensor)
 *
 *      Non-volatile parameters are stored in FRAM at fixed offsets. Network setters parse
 *      and validate textual addresses before persisting them. Temperature reads implement
 *      range checking and a retry strategy. Attenuation values are written to hardware and
 *      saved in FRAM as 16.16 fixed-point.
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

 /* ----------------------------- Hardware pinout ----------------------------- */
 /** @name DAT-31R5-SP+ (attenuator) pins */
 ///@{
 #define LE_DAT31R5SP      P0_15  /**< Latch Enable */
 #define CLK_DAT31R5SP     P0_17  /**< Shift Clock   */
 #define DATA_A_DAT31R5SP  P0_18  /**< DATA line A   */
 #define DATA_B_DAT31R5SP  P2_0   /**< DATA line B   */
 #define DATA_C_DAT31R5SP  P0_1   /**< DATA line C   */
 #define DATA_D_DAT31R5SP  P0_11  /**< DATA line D   */
 ///@}

 /** @name FRAM / I²C EEPROM pins */
 ///@{
 #define FERAM_SDA  P0_19
 #define FERAM_SCL  P0_20
 #define FERAM_WP   P0_21  /**< Write Protect (active level depends on device) */
 ///@}

 /** @name LM71 temperature sensor (shared SPI) */
 ///@{
 #define LM71_SCLK1  P0_7
 #define LM71_MISO1  P0_8
 #define LM71_MOSI1  P0_9
 #define LM71_CS_AC  P2_1   /**< Chip-select for AC sensor */
 #define LM71_CS_BD  P0_16  /**< Chip-select for BD sensor */
 ///@}

 /* ------------------------------- Enumerations ------------------------------ */

 /**
  * @brief Ethernet addressing mode persisted in FRAM.
  */
 typedef enum {
     ETH_ADDR_MODE_DHCP,   /**< Obtain IP via DHCP */
     ETH_ADDR_MODE_STATIC, /**< Use static IP configuration */
     ETH_ADDR_MODE_NONE,   /**< Unspecified; library will attempt static then DHCP */
 } eth_addr_mode_t;

 /**
  * @brief Temperature control mode (reserved for future use).
  */
 typedef enum {
     TEMP_CTRL_MANUAL,
     TEMP_CTRL_AUTOMATIC,
 } temp_ctrl_mode_t;

 /* --------------------------------- Class ---------------------------------- */

 /**
  * @class GlobalRFFE
  * @brief Aggregates hardware drivers and persistent configuration for the RFFE.
  *
  * @details
  *   Constructs GPIO/SPI/I²C peripherals, exposes FRAM-backed getters/setters for
  *   network and attenuation parameters, and provides helper routines used by the
  *   SCPI layer. See each method for exact persistence format and error returns.
  */
 class GlobalRFFE
 {
 public:
     /**
      * @brief Initialize hardware interfaces and driver objects.
      *
      * Initializes DigitalOuts for the attenuator and FRAM WP, sets up FRAM
      * (I²C address 0xA0, capacity 2 KiB), configures shared SPI for LM71 sensors,
      * and binds chip-selects for AC/BD sensors.
      */
     GlobalRFFE();

     ~GlobalRFFE() = default;

     /* ------------------------- Ethernet configuration ------------------------- */

     /**
      * @brief Apply persisted Ethernet mode to an `EthernetInterface`.
      *
      * @details
      *   - Reads addressing mode from FRAM.
      *   - If STATIC, loads IP/Mask/Gateway/MAC from FRAM and configures them.
      *     On failure, **falls back to DHCP** and returns the underlying error.
      *   - If DHCP, enables DHCP.
      *   - If NONE/unknown, attempts STATIC; on failure, enables DHCP.
      *
      * @param net Reference to the network interface to be configured.
      * @return nsapi_error_t NSAPI_ERROR_OK on success; NSAPI error code otherwise.
      */
     nsapi_error_t config_get_eth_mode(EthernetInterface & net);

     /**
      * @brief Read configuration version byte from FRAM.
      * @return int 0 on success; -1 on FRAM read failure.
      */
     int config_get_version();

     /**
      * @brief Get persisted Ethernet addressing mode.
      * @param[out] addr_mode Output variable filled with the mode.
      * @return int 0 on success; -1 on FRAM read failure.
      */
     int config_get_eth_addressing(eth_addr_mode_t* addr_mode);

     /**
      * @brief Persist Ethernet addressing mode to FRAM.
      * @param addr_mode Mode to persist.
      * @return int 0 on success; -1 on FRAM write failure.
      */
     int config_set_eth_addressing(eth_addr_mode_t addr_mode);

     /* ------------------------------ MAC address ------------------------------ */

     /**
      * @brief Get MAC address from FRAM.
      * @param[out] mac Buffer to receive 6-byte MAC.
      * @param len Size of @p mac; must be >= 6 to receive all bytes.
      * @return int Number of bytes copied (len) on success; -1 on error.
      */
     int config_get_mac_addr(uint8_t * mac, size_t len);

     /**
      * @brief Set MAC address in FRAM from a string.
      *
      * @details
      *   Accepts standard hex pairs separated by `:` (e.g., `"00:11:22:33:44:55"`).
      *   The string is parsed and validated before persisting.
      *
      * @param mac String with MAC address.
      * @return int 6 on success; -1 on parse or write error.
      */
     int config_set_mac_addr(const char * mac);

     /* ------------------------------ IPv4 address ----------------------------- */

     /**
      * @brief Read IPv4 address from FRAM and format as dotted-quad.
      * @param[out] ip Destination buffer (null-terminated on success).
      * @param len Size of destination buffer.
      * @return int Number of chars written (excluding null) on success; -1 on error.
      */
     int config_get_ipv4_addr(char * ip, size_t len);

     /**
      * @brief Parse and persist an IPv4 address into FRAM.
      * @param ip String like `"192.168.0.10"`.
      * @return int 0 on success; -1 on parse or write error.
      */
     int config_set_ipv4_addr(const char * ip);

     /* -------------------------------- Netmask -------------------------------- */

     /**
      * @brief Read subnet mask from FRAM as dotted-quad.
      * @param[out] mask Destination buffer (null-terminated on success).
      * @param len Size of destination buffer.
      * @return int Number of chars written (excluding null) on success; -1 on error.
      */
     int config_get_mask_addr(char * mask, size_t len);

     /**
      * @brief Parse and persist subnet mask into FRAM.
      * @param mask String like `"255.255.255.0"`.
      * @return int Number of bytes written on success (4); -1 on error.
      *
      * @note Persistence offset should be `_mask_addr_offset`.
      */
     int config_set_mask_addr(const char * mask);

     /* ------------------------------- Gateway --------------------------------- */

     /**
      * @brief Read default gateway from FRAM as dotted-quad.
      * @param[out] gateway Destination buffer (null-terminated on success).
      * @param len Size of destination buffer.
      * @return int Number of chars written (excluding null) on success; -1 on error.
      */
     int config_get_gateway_addr(char * gateway, size_t len);

     /**
      * @brief Parse and persist gateway into FRAM.
      * @param gateway String like `"192.168.0.1"`.
      * @return int Number of bytes written on success (4); -1 on error.
      *
      * @note Persistence offset should be `_gateway_addr_offset`.
      */
     int config_set_gateway_addr(const char * gateway);

     /* ----------------------------- Attenuation (dB) -------------------------- */

     /**
      * @brief Get the persisted attenuation value (in dB).
      *
      * @details
      *   Reads a 32-bit signed fixed-point value from FRAM (Q16.16),
      *   converts it to float and returns it. On FRAM error, returns -1.
      *
      * @return float Attenuation in dB, or -1.0f on FRAM read failure.
      */
     float config_get_attenuation();

     /**
      * @brief Set attenuation in hardware and persist to FRAM.
      *
      * @details
      *   - Validates DAT31R5SP pins, applies value via bit-banging.
      *   - Stores the value in FRAM as Q16.16 fixed-point.
      *
      * @param att Attenuation in dB (0.0–31.5, step 0.5 typical).
      * @return int 0 on success; -1 on FRAM write failure.
      */
     int config_set_attenuation(float att);

     /* --------------------------- SCPI-facing helpers ------------------------- */

     /**
      * @brief Dump entire FRAM contents as hex (16 bytes per line).
      * @return int Number of bytes dumped on success (2048); -1 on first read error.
      */
     int stream_feram_raw();

     /**
      * @brief Read LM71 temperature at the AC sensor location.
      *
      * @details
      *   Puts the sensor in mode 0, reads one sample, validates range
      *   [-55°C, 150°C], and if out-of-range sleeps ~270 ms and retries once.
      *
      * @param[out] value Temperature in °C on success.
      * @return int 0 on success; -1 if both reads are out-of-range.
      */
     int get_measure_temp_ac(float &value);

     /**
      * @brief Read LM71 temperature at the BD sensor location.
      *
      * @details
      *   Same semantics as @ref get_measure_temp_ac.
      *
      * @param[out] value Temperature in °C on success.
      * @return int 0 on success; -1 if both reads are out-of-range.
      */
     int get_measure_temp_bd(float &value);

 private:
     float _att;
     /* ------------------------------- GPIO lines --------------------------------- */
     mbed::DigitalOut _le, _clk, _dtA, _dtB, _dtC, _dtD;
     mbed::DigitalOut _feram_wp;

     /* ---------------------------- Device instances ------------------------------ */
     DAT31R5SP       _dat31r5sp;
     I2CEeprom       _feram;

     /* ----------------------------- Temperature IO ------------------------------- */
     mbed::DigitalOut _cs_ac, _cs_bd;
     mbed::SPI        _spi;
     LM71             _lm71_ac, _lm71_bd;

     /* --------------------------- FRAM layout (offsets) -------------------------- */
     static constexpr int _mac_addr_offset          = 0x00; /**< 6 bytes */
     static constexpr int _config_version_offset    = 0x0F; /**< 1 byte  */
     static constexpr int _ip_addr_offset           = 0x10; /**< 4 bytes */
     static constexpr int _mask_addr_offset         = 0x20; /**< 4 bytes */
     static constexpr int _gateway_addr_offset      = 0x30; /**< 4 bytes */
     static constexpr int _attenuation_offset       = 0x40; /**< int32 Q16.16 */
     static constexpr int _eth_addr_offset          = 0x50; /**< 1 byte (mode) */
     static constexpr int _temp_ctrl_mode_offset    = 0x51;
     static constexpr int _pid_ac_kc_offset         = 0x60;
     static constexpr int _pid_ac_ti_offset         = 0x64;
     static constexpr int _pid_ac_td_offset         = 0x68;
     static constexpr int _pid_bd_kc_offset         = 0x6C;
     static constexpr int _pid_bd_ti_offset         = 0x70;
     static constexpr int _pid_bd_td_offset         = 0x74;
     static constexpr int _pid_ac_set_point__offset = 0x78;
     static constexpr int _pid_bd_set_point__offset = 0x7C;
 };

 #endif /* GLOBALRFFE_H_ */
