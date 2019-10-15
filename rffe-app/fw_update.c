/****************************************************************************
 * rffe-app/fw_update.c
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
#include <nuttx/progmem.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <sys/boardctl.h>
#include <sys/time.h>

static void* fw_update_server(void* args)
{
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int sockfd, ret;
    uint8_t tcp_buf[256];

    /*
     * Open a socket
     */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("failed to open a socket");
        return NULL;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(9090);

    ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        perror("failed to bind a socket");
        return NULL;
    }

    ret = listen(sockfd, 4);
    if (ret < 0)
    {
        perror("failed to listen to a socket");
        return NULL;
    }

    while (1)
    {
        clilen = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd > 0)
        {
            printf("Firmware update server: new connection\n");
        }
        else continue;

        struct timeval tv;

        /*
         * Receive timeout: 30s
         */
        tv.tv_sec  = 30;
        tv.tv_usec = 0;
        ret = setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));

        while (1)
        {
            int n = recv(newsockfd, tcp_buf, 1, MSG_WAITALL);

            if (n == 0)
            {
                printf("Firmware update server: connection closed\n");
                break;
            }
            else if (n < 0)
            {
                printf("Firmware update server: connection error (%d)\n", n);
                break;
            }

            switch (tcp_buf[0])
            {
            case 'e':
                /*
                 * Erase sectors 23 to 29 (0x00048000 - 0x0007FFFF)
                 */
                for (int i = 0; i < 7; i++)
                {
                    up_progmem_eraseblock(i);
                }
                write(newsockfd, "1", 1);
                break;

            case 'w':
            {
                n = recv(newsockfd, tcp_buf, 4, MSG_WAITALL);
                if (n != 4) break;

                uint32_t start_addr = tcp_buf[0] | tcp_buf[1] << 8 |
                    tcp_buf[2] << 16 | tcp_buf[3] << 24;

                n = recv(newsockfd, tcp_buf, 256, MSG_WAITALL);
                if (n != 256) break;

                if (start_addr >= 0x48000 && start_addr <= (0x80000 - 256))
                {
                    up_progmem_write(start_addr, tcp_buf, 256);
                    write(newsockfd, "1", 1);
                }
                else
                {
                    write(newsockfd, "0", 1);
                }
            }
                break;

            case 'r':
                boardctl(BOARDIOC_RESET, 0);
                break;

            default:
                printf("Firmware update server: invalid command '%c'\n", tcp_buf[0]);
                write(newsockfd, "0", 1);
                break;
            }
        }

        /*
         * Close the socket
         */
        close(newsockfd);
    }
    return NULL;
}

void start_fw_update_server(void)
{
    pthread_t thread;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1280);
    pthread_create(&thread, &attr, &fw_update_server, NULL);
    pthread_detach(thread);
}
