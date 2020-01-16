/****************************************************************************
 * config/mbed/src/lpc17_40_appinit.c
 *
 *   Copyright (C) 2010, 2016 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdio.h>
#include <syslog.h>
#include <errno.h>

#include <nuttx/board.h>
#include <nuttx/spi/spi.h>
#include <nuttx/spi/spi_bitbang.h>
#include <nuttx/analog/dac.h>
#include <nuttx/sensors/adt7320.h>
#include <nuttx/sensors/lm71.h>
#include <nuttx/i2c/i2c_master.h>
#include <nuttx/eeprom/i2c_xx24xx.h>
#include <nuttx/rf/dat-31r5-sp.h>
#include <nuttx/leds/userled.h>

#include "lpc17_40_gpio.h"
#include "lpc17_40_ssp.h"
#include "lpc17_40_i2c.h"

#include "mbed.h"

static void spi_bitbang_set_mosi(void)
{
  lpc17_40_gpiowrite(DATA_A_DAT31R5SP, 1);
  lpc17_40_gpiowrite(DATA_B_DAT31R5SP, 1);
  lpc17_40_gpiowrite(DATA_C_DAT31R5SP, 1);
  lpc17_40_gpiowrite(DATA_D_DAT31R5SP, 1);
}

static void spi_bitbang_clear_mosi(void)
{
  lpc17_40_gpiowrite(DATA_A_DAT31R5SP, 0);
  lpc17_40_gpiowrite(DATA_B_DAT31R5SP, 0);
  lpc17_40_gpiowrite(DATA_C_DAT31R5SP, 0);
  lpc17_40_gpiowrite(DATA_D_DAT31R5SP, 0);
}

#define SPI_SETSCK  lpc17_40_gpiowrite(CLK_DAT31R5SP, 1)
#define SPI_CLRSCK  lpc17_40_gpiowrite(CLK_DAT31R5SP, 0)
#define SPI_SETMOSI spi_bitbang_set_mosi()
#define SPI_CLRMOSI spi_bitbang_clear_mosi()
#define SPI_GETMISO (0)

#define SPI_BITBAND_LOOPSPERMSEC CONFIG_BOARD_LOOPSPERMSEC

#define SPI_PERBIT_NSEC      100

#include <nuttx/spi/spi_bitbang.c>

static void spi_select(FAR struct spi_bitbang_s *priv, uint32_t devid,
                       bool selected)
{
  switch (devid)
  {
  case SPIDEV_USER(1):
    /* When selected is true, LE = 1, otherwise LE = 0 */
    lpc17_40_gpiowrite(LE_DAT31R5SP, selected);
    break;

  default:
    break;
  }
}

static uint8_t spi_status(FAR struct spi_bitbang_s *priv, uint32_t devid)
{
  if (devid == SPIDEV_USER(1))
    {
      return SPI_STATUS_PRESENT;
    }

  return 0;
}

#ifdef CONFIG_SPI_CMDDATA

static int spi_cmddata(FAR struct spi_bitbang_s *priv, uint32_t devid,
                       bool cmd)
{
  return OK;
}

#endif

void lpc17_40_ssp1select(FAR struct spi_dev_s *dev, uint32_t devid, bool selected)
{
  switch (devid)
  {
  case SPIDEV_TEMPERATURE(0):
    lpc17_40_gpiowrite(CS_ADT7320_AC, !selected);
    break;

  case SPIDEV_TEMPERATURE(1):
    lpc17_40_gpiowrite(CS_ADT7320_BD, !selected);
    break;

  case SPIDEV_USER(0):
    lpc17_40_gpiowrite(CS_DAC7554, !selected);
    break;

  default:
    break;
  }
}

uint8_t lpc17_40_ssp1status(FAR struct spi_dev_s *dev, uint32_t devid)
{
  return 0;
}

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_app_initialize
 *
 * Description:
 *   Perform architecture specific initialization
 *
 * Input Parameters:
 *   arg - The boardctl() argument is passed to the board_app_initialize()
 *         implementation without modification.  The argument has no
 *         meaning to NuttX; the meaning of the argument is a contract
 *         between the board-specific initalization logic and the
 *         matching application logic.  The value cold be such things as a
 *         mode enumeration value, a set of DIP switch switch settings, a
 *         pointer to configuration data read from a file or serial FLASH,
 *         or whatever you would like to do with it.  Every implementation
 *         should accept zero/NULL as a default configuration.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   any failure to indicate the nature of the failure.
 *
 ****************************************************************************/

int board_app_initialize(uintptr_t arg)
{
  int ret;
  uint8_t buf[2];

  struct i2c_master_s *i2c0, *i2c1;
  struct spi_dev_s *ssp1, *spi_att;
  struct dac_dev_s *dac;

  lpc17_40_configgpio(WP_FERAM);
  lpc17_40_configgpio(CS_ADT7320_AC);
  lpc17_40_configgpio(CS_ADT7320_BD);
  lpc17_40_configgpio(CS_DAC7554);
  lpc17_40_configgpio(LE_DAT31R5SP);
  lpc17_40_configgpio(CLK_DAT31R5SP);
  lpc17_40_configgpio(DATA_A_DAT31R5SP);
  lpc17_40_configgpio(DATA_B_DAT31R5SP);
  lpc17_40_configgpio(DATA_C_DAT31R5SP);
  lpc17_40_configgpio(DATA_D_DAT31R5SP);

  i2c0 = lpc17_40_i2cbus_initialize(0);
  if (i2c0 == NULL)
  {
    syslog(LOG_ERR, "ERROR: Failed to get the i2c0 interface\n");
  }
  else
  {
    /* Create the /dev/i2c0 character device */
    i2c_register(i2c0, 0);
  }

  i2c1 = lpc17_40_i2cbus_initialize(1);
  if (i2c1 == NULL)
  {
    syslog(LOG_ERR, "ERROR: Failed to get the i2c0 interface\n");
  }
  else
  {
    /* Associate the FeRAM under the i2c1 bus to /dev/feram0 */
    ret = ee24xx_initialize(i2c1, 0x50, "/dev/feram0", EEPROM_24xx16, 0);
    if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: Failed to initialize the feram\n");
    }
  }

  ssp1 = lpc17_40_sspbus_initialize(1);

  /*
   * Checks if LM71 is present
   */
  SPI_LOCK(ssp1, true);
  SPI_SETMODE(ssp1, SPIDEV_MODE0);
  SPI_SETBITS(ssp1, 8);
  SPI_HWFEATURES(ssp1, 0);
  SPI_SETFREQUENCY(ssp1, 1000000);
  SPI_SELECT(ssp1, SPIDEV_TEMPERATURE(0), true);
  SPI_RECVBLOCK(ssp1, buf, 2);
  SPI_SEND(ssp1, 0xFF);
  SPI_SEND(ssp1, 0xFF);
  SPI_RECVBLOCK(ssp1, buf, 2);
  SPI_SELECT(ssp1, SPIDEV_TEMPERATURE(0), false);
  SPI_LOCK(ssp1, false);

  /*
   * If the ID matches, register the LM71 driver. Otherwise register
   * the ADT7320 driver.
   */
  if (buf[0] == 0x80 && buf[1] == 0x0F)
  {
    lm71_register("/dev/temp_ac", ssp1, SPIDEV_TEMPERATURE(0));
    lm71_register("/dev/temp_bd", ssp1, SPIDEV_TEMPERATURE(1));
  }
  else
  {
    adt7320_register("/dev/temp_ac", ssp1, SPIDEV_TEMPERATURE(0));
    adt7320_register("/dev/temp_bd", ssp1, SPIDEV_TEMPERATURE(1));
  }

  dac = dac7554_initialize(ssp1, SPIDEV_USER(0));
  dac_register("/dev/dac0", dac);

  spi_att = spi_create_bitbang(&g_spiops);

  dat31r5sp_register("/dev/att0",
                     spi_att,
                     SPIDEV_USER(1));

  /* Register the LED driver */

  ret = userled_lower_initialize("/dev/statusleds");
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: userled_lower_initialize() failed: %d\n", ret);
    }
  UNUSED(ret);
  return OK;
}
