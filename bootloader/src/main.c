/****************************************************************************
 * bootloader/src/main.c
 *
 *   Copyright (C) 2020 Augusto Fraga Giachero. All rights reserved.
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

#include <stdint.h>
#include <stdio.h>

#include "LPC176x5x.h"
#include "lpc17_clock.h"
#include "lpc17_pincfg.h"
#include "lpc17_uart.h"
#include "lpc17_iap.h"
#include "start_app.h"

uint32_t* const flash_end_addr = (uint32_t*)0x80000;
uint32_t* const app_start_addr = (uint32_t*)0x10000;
uint32_t* const boot_start_addr = (uint32_t*)0x0000;
uint32_t* const update_start_addr = (uint32_t*)0x48000;
const uint32_t app_size = (uint32_t)update_start_addr - (uint32_t)app_start_addr;
const uint32_t boot_size = (uint32_t)app_start_addr;
const uint8_t boot_start_sec = 0;
const uint8_t boot_end_sec = 15;
const uint8_t app_start_sec = 16;
const uint8_t update_start_sec = 23;
const uint8_t magic_word_sec = 29;

typedef struct
{
    uint8_t version[3];
    uint8_t fw_type;
    uint32_t magic;
} fw_info;

const fw_info* fw_header = (fw_info*)((uint32_t)flash_end_addr - sizeof(fw_info));

__attribute__ ((long_call, noinline, section (".ramtext")))
uint8_t get_sector_number(uint32_t flash)
{
    uint8_t ret = 0;
    if (flash < 0x10000)
    {
        ret = (flash / 0x1000);
    }
    else
    {
        ret = ((flash - 0x10000) / 0x8000) + 16;
    }
    return ret;
}

__attribute__ ((long_call, noinline, section (".ramtext")))
void copy_flash_region(uint32_t* src, uint32_t* dest, size_t len, uint32_t cpu_clk_khz)
{
    uint32_t buffer[64];

    if (len % 256) return;

    for (size_t i = 0; i < len; i += 256, src += 64, dest += 64)
    {
        for (size_t i = 0; i < 64; i++)
        {
            buffer[i] = src[i];
        }

        uint8_t sector = get_sector_number((uint32_t)dest);

        lpc17_iap_prepare_sectors(sector, sector);
        lpc17_iap_copy_ram_flash(buffer, dest, 256, cpu_clk_khz);
    }
}

__attribute__ ((long_call, noinline, section (".ramtext")))
void update_app(uint32_t cpu_clk_khz)
{
    lpc17_iap_prepare_sectors(app_start_sec, update_start_sec - 1);
    lpc17_iap_erase_sectors(app_start_sec, update_start_sec - 1, cpu_clk_khz);

    copy_flash_region(update_start_addr, app_start_addr, app_size, cpu_clk_khz);

    lpc17_iap_prepare_sectors(magic_word_sec, magic_word_sec);
    lpc17_iap_erase_sectors(magic_word_sec, magic_word_sec, cpu_clk_khz);

    /*
     * Jump to application code
     */
    start_app((uint32_t*)0x10000);
}

__attribute__ ((long_call, noinline, section (".ramtext")))
void update_bootloader(uint32_t cpu_clk_khz)
{
    lpc17_iap_prepare_sectors(boot_start_sec, boot_end_sec);
    lpc17_iap_erase_sectors(boot_start_sec, boot_end_sec, cpu_clk_khz);

    copy_flash_region(update_start_addr, boot_start_addr, boot_size, cpu_clk_khz);

    lpc17_iap_prepare_sectors(magic_word_sec, magic_word_sec);
    lpc17_iap_erase_sectors(magic_word_sec, magic_word_sec, cpu_clk_khz);

    /*
     * Jump to application code
     */
    start_app((uint32_t*)0x10000);
}

int main(void)
{
    /*
     * Update flash access time to 4 CPU clocks (required when running
     * from 60 to 80MHz)
     */
    LPC_SYSCON->FLASHCFG &= ~SYSCON_FLASHCFG_FLASHTIM_Msk;
    LPC_SYSCON->FLASHCFG |= (3 << SYSCON_FLASHCFG_FLASHTIM_Pos);

    /*
     * PLL0 output clock Fout = (2 * pll_mul * Fsrc) / pll_div
     * CPU clock Fout / cpu_div
     *
     * For a 72 MHz cpu clock using the internal RC:
     * pll_div = 1;
     * pll_mull = 18;
     * cpu_div = 2;
     */
    lpc17_set_pll0(1, 18, 2, pll0_irc_src);

    /*
     * Configure pins P0.2 as TXD0 and P0.3 as RXD0
     */
    lp17_pincfg(0, 2, pin_mode_none, pin_func_alt1);
    lp17_pincfg(0, 3, pin_mode_none, pin_func_alt1);

    /*
     * Initialize UAR0 (115200bps, 8N1)
     */
    lpc17_uart0_init(115200, 72000000);

    if (fw_header->magic == 0xAAAAAAAA)
    {
        char tmp[128];
        lpc17_uart0_write_str_blocking("[BOOTLOADER] DO NOT TURN OFF WHILE UPDATING!\r\n");

        if (fw_header->fw_type == 1)
        {
            snprintf(tmp, 128, "[BOOTLOADER] New app firmware update found!\r\nUpdating to %d.%d.%d ...\r\n",
                     fw_header->version[0], fw_header->version[1], fw_header->version[2]);
            lpc17_uart0_write_str_blocking(tmp);
            update_app(72000);
        }
        else if (fw_header->fw_type == 2)
        {
            lpc17_uart0_write_str_blocking("[BOOTLOADER] New bootloader firmware update found!\r\n");
            lpc17_uart0_write_str_blocking("[BOOTLOADER] Updating...\r\n");
            update_bootloader(72000);
        }
        else
        {
            snprintf(tmp, 128, "[BOOTLOADER] ERROR: Unknown fw_type %d !\r\n Jumping to application code...\r\n",
                     fw_header->fw_type);
            lpc17_uart0_write_str_blocking(tmp);
        }
    }

    /*
     * Jump to application code
     */
    start_app((uint32_t*)0x10000);
}
