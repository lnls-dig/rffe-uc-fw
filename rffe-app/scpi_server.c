/****************************************************************************
 * rffe-app/scpi_server.c
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

#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <nuttx/leds/userled.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "scpi/scpi.h"
#include "scpi-def.h"

#include "scpi_interface.h"
#include "scpi_rffe_cmd.h"
#include "scpi_tables.h"

static void handle_client(user_data_t * context)
{
    int sockfd = context->sockfd;
    char tcp_buff[16];
    scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];
    char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
    scpi_t scpi_context;

    int ledfd = open("/dev/statusleds", O_WRONLY);
    ioctl(ledfd, ULEDIOC_SETALL, 0x02);

    SCPI_Init(&scpi_context,
              scpi_commands,
              &scpi_interface,
              scpi_units_def,
              SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
              scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
              scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);

    scpi_context.user_context = context;

    while (1)
    {
        int n = recv(sockfd, tcp_buff, sizeof(tcp_buff), 0);

        if (n == 0)
        {
            printf("Connection closed\n");
            break;
        }
        else if (n < 0)
        {
            printf("Connection error (%d)\n", -errno);
            break;
        }

        SCPI_Input(&scpi_context, tcp_buff, n);
    }
    printf("Closing the socket\n");
    close(sockfd);
    printf("Socket closed\n");

    ioctl(ledfd, ULEDIOC_SETALL, 0x00);
    close(ledfd);
}

int scpi_server_start(float* dac_ac, float* dac_bd)
{
    int sockfd, newsockfd;
    int ret;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("failed to open a socket");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(9001);

    ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        perror("failed to bind a socket");
        close(sockfd);
        return -1;
    }

    ret = listen(sockfd, 1);
    if (ret < 0)
    {
        perror("failed to listen to a socket");
        close(sockfd);
        return -1;
    }

    while (1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        struct timeval tv;

        /*
         * Receive timeout: 30s
         */
        tv.tv_sec  = 30;
        tv.tv_usec = 0;

        ret = setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));

        if (ret < 0)
        {
            fprintf(stderr, "setsockopt(SO_RCVTIMEO) failed: %d\n", ret);
        }

        printf("New connection!\n");

        user_data_t ccontext;
        ccontext.sockfd = newsockfd;
        ccontext.dac_ac = dac_ac;
        ccontext.dac_bd = dac_bd;
        handle_client(&ccontext);
    }

    close(sockfd);
    return 0;
}
