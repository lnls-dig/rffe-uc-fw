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
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "scpi/scpi.h"
#include "scpi-def.h"

#include "scpi_interface.h"
#include "scpi_rffe_cmd.h"
#include "scpi_tables.h"

static volatile int active_threads = 0;

static int readline_tcp(int sockfd, char* str, int str_size)
{
    int str_ind = 0;
    char c;

    for (; str_ind < (str_size - 1); str_ind++)
    {
        int n = read(sockfd, &c, 1);
        if (n < 1)
        {
            str[str_ind] = 0;
            return n;
        }

        str[str_ind] = c;

        if (c == '\n')
        {
            str_ind++;
            break;
        }
    }

    str[str_ind] = 0;
    return str_ind;
}

static void* handle_client(void* args)
{
    int sockfd = (int)args;
    char line[SCPI_INPUT_BUFFER_LENGTH];
    scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];
    char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
    user_data_t user_data;
    scpi_t scpi_context;
    pthread_mutex_t counter_lock;

    pthread_mutex_lock(&counter_lock);
    active_threads++;
    pthread_mutex_unlock(&counter_lock);

    /* user_context will be pointer to socket */
    SCPI_Init(&scpi_context,
              scpi_commands,
              &scpi_interface,
              scpi_units_def,
              SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
              scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
              scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);

    user_data.sockfd = sockfd;
    scpi_context.user_context = &user_data;

    while(1)
    {
        int n = readline_tcp(sockfd, line, 128);

        if (n == 0)
        {
            printf("Thread %d, connection closed\n", sockfd);
            break;
        }
        else if (n < 0)
        {
            printf("Thread %d, connection error\n", sockfd);
            break;
        }
        printf("Thread %d, received: %s", sockfd, line);
        SCPI_Input(&scpi_context, line, n);
    }

    close(sockfd);

    pthread_mutex_lock(&counter_lock);
    active_threads--;
    pthread_mutex_unlock(&counter_lock);
    return NULL;
}

int scpi_server_start(void)
{
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t thread;

    /*
     * Open a socket
     */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(9001);

    bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    listen(sockfd, 4);

    while (1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (active_threads < 4)
        {
            printf("New connection!\n");
            pthread_create(&thread, NULL, &handle_client, (void*)newsockfd);
            pthread_detach(thread);
        }
        else
        {
            printf("Connection rejected, maximum active connections reached!\n");
            close(newsockfd);
        }
    }

    return 0;
}