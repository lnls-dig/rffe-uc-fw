/****************************************************************************
 * rffe-app/rffe_main.c
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
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/sensors/ioctl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fixedmath.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * hello_main
 ****************************************************************************/

#if defined(BUILD_MODULE)
int main(int argc, FAR char *argv[])
#else
int rffe_main(int argc, char *argv[])
#endif
{
  if (argc != 2) {
    printf("Invalid arguments.\n");
    return 1;
  }

  int fd = open(argv[1], O_RDONLY);
  uint8_t temp_raw[4];

  if(fd < 0)
  {
    printf("File not found.\n");
    return 2;
  }

  read(fd, temp_raw, 4);

  printf("Temperature: %d.%d\n", temp_raw[2], (temp_raw[1] * 10) / 256);

  b16_t temp;

  ioctl(fd, SNIOC_READTCRIT, (unsigned long)&temp);
  printf("Critical temperature: %08X\n", temp);

  ioctl(fd, SNIOC_READTLOW, (unsigned long)&temp);
  printf("Low temperature: %08X\n", temp);

  ioctl(fd, SNIOC_READTHIGH, (unsigned long)&temp);
  printf("High temperature: %08X\n", temp);

  ioctl(fd, SNIOC_READTHYS, (unsigned long)&temp);
  printf("Hysteresis temperature: %08X\n", temp);

  uint8_t conf;
  ioctl(fd, SNIOC_READCONF, (unsigned long)&conf);
  printf("Configuration: %02X\n", conf);

  ioctl(fd, SNIOC_READSTAT, (unsigned long)&conf);
  printf("Status: %02X\n", conf);

  close(fd);
  return 0;
}
