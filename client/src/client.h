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

int socket_client_init(socket_t *sock, char *hostname, int port);

int socket_client_connect(socket_t *sock);

int socket_client_judge(int sockfd);

int socket_close(socket_t *sock);
#endif

