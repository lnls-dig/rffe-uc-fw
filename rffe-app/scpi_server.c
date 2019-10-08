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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "scpi/scpi.h"
#include "scpi-def.h"

#include "scpi_interface.h"
#include "scpi_rffe_cmd.h"
#include "scpi_tables.h"

static void* handle_client(void* args)
{
    user_data_t* context = (user_data_t*)args;
    int sockfd = context->sockfd;
    int* active_threads = context->active_threads;
    char tcp_buff[16];
    scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];
    char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
    scpi_t scpi_context;
    pthread_mutex_t counter_lock;


    pthread_mutex_lock(&counter_lock);
    (*active_threads)++;
    pthread_mutex_unlock(&counter_lock);

    /* user_context will be pointer to socket */
    SCPI_Init(&scpi_context,
              scpi_commands,
              &scpi_interface,
              scpi_units_def,
              SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
              scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
              scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);

    scpi_context.user_context = context;

    while(1)
    {
        int n = recv(sockfd, tcp_buff, sizeof(tcp_buff), 0);

        if (n == 0)
        {
            printf("Thread %d, connection closed\n", sockfd);
            break;
        }
        else if (n < 0)
        {
            printf("Thread %d, connection error (%d)\n", sockfd, n);
            break;
        }
        SCPI_Input(&scpi_context, tcp_buff, n);
    }

    close(sockfd);

    /*
     * Free client context
     */
    free(context);

    pthread_mutex_lock(&counter_lock);
    (*active_threads)--;
    pthread_mutex_unlock(&counter_lock);
    return NULL;
}

int scpi_server_start(void)
{
    int sockfd, newsockfd;
    int active_threads = 0;
    int ret;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t thread;
    volatile float dac_ac = 0;
    volatile float dac_bd = 0;

    /*
     * Open a socket
     */
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
        return -1;
    }

    ret = listen(sockfd, 4);
    if (ret < 0)
    {
        perror("failed to listen to a socket");
        return -1;
    }

    while (1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        int value = TRUE;
        setsockopt(newsockfd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(int));

        struct timeval tv;

        value = TRUE;
        ret = setsockopt(newsockfd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(int));
        if (ret < 0)
        {
            fprintf(stderr, "setsockopt(SO_KEEPALIVE) failed: %d\n", ret);
        }

        tv.tv_sec  = 5;
        tv.tv_usec = 0;

        ret = setsockopt(newsockfd, SOL_TCP, TCP_KEEPIDLE, &tv, sizeof(struct timeval));
        if (ret < 0)
        {
            fprintf(stderr, "setsockopt(TCP_KEEPIDLE) failed: %d\n", ret);
        }

        tv.tv_sec  = 1;
        tv.tv_usec = 0;

        ret = setsockopt(newsockfd, SOL_TCP, TCP_KEEPINTVL, &tv, sizeof(struct timeval));
        if (ret < 0)
        {
            fprintf(stderr, "setsockopt(TCP_KEEPIDLE) failed: %d\n", ret);
        }

        value = 3;
        ret = setsockopt(newsockfd, SOL_TCP, TCP_KEEPCNT, &value, sizeof(int));

        if (ret < 0)
        {
            fprintf(stderr, "setsockopt(SO_KEEPALIVE) failed: %d\n", ret);
        }

        if (active_threads < 4)
        {
            user_data_t* ccontext = malloc(sizeof(user_data_t));
            ccontext->active_threads = &active_threads;
            ccontext->sockfd = newsockfd;
            ccontext->dac_ac = (float*)&dac_ac;
            ccontext->dac_bd = (float*)&dac_bd;

            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setstacksize(&attr, 1280);
            printf("New connection!\n");
            pthread_create(&thread, &attr, &handle_client, ccontext);
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
