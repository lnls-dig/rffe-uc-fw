/****************************************************************************
 * examples/hello/hello_main.c
 *
 *   Copyright (C) 2008, 2011-2012 Gregory Nutt. All rights reserved.
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
