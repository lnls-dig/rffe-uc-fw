/****************************************************************************
 * rffe-board/lpc17_40_userled.c
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

/*
 * Headers
 */

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <debug.h>

#include <arch/board/board.h>

#include "lpc17_40_gpio.h"
#include "mbed.h"

#ifdef CONFIG_ARCH_LEDS

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_userled_initialize
 ****************************************************************************/

void board_userled_initialize(void)
{
  lpc17_40_configgpio(STATUS_LED_R);
  lpc17_40_configgpio(STATUS_LED_G);
}

/****************************************************************************
 * Name: board_userled
 ****************************************************************************/

void board_userled(int led, bool ledon)
{
  switch (led) {
  case 0:
	  lpc17_40_gpiowrite(STATUS_LED_R, ledon);
	  break;
  case 1:
	  lpc17_40_gpiowrite(STATUS_LED_G, ledon);
	  break;
  default:
	  break;
  }
}

/****************************************************************************
 * Name: board_userled_all
 ****************************************************************************/

void board_userled_all(uint8_t ledset)
{
  /* Configure LED1-8 GPIOs for output */

  lpc17_40_gpiowrite(STATUS_LED_R, (ledset & 0x01) != 0);
  lpc17_40_gpiowrite(STATUS_LED_G, (ledset & 0x02) != 0);
}

#endif
