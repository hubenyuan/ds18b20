/********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  client.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(05/03/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "05/03/2023 05:12:18 PM"
 *                 
 ********************************************************************************/

#ifndef    CLIENT_H_
#define    CLIENT_H_

typedef struct socket_s
{
    int     fd;
    int     port;
    int     connected;
    char    hostip[32];
} socket_t;

int socket_client_init(socket_t *sock, char *hostname, int port);

int socket_client_connect(socket_t *sock);

int socket_client_judge(socket_t *sock);

int socket_client_send(socket_t *sock, packdata_t packdata);

int socket_close(socket_t *sock);

#endif

