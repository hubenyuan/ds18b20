/*********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  client.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(05/02/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "05/02/2023 09:41:29 PM"
 *                 
 ********************************************************************************/

#include "logger.h"

typedef struct socket_s
{
	int       fd;
	int       port;
	char     *serverip;
} socket_t;

int socket_close(socket_t *sock)
{
	close(sock->sockfd);
	sock->fd = -1;
}

int socket_client_init(socket_t *sock, char *hostname, int port)
{
	sock->fd = -1;

	/* 域名解析 */
	sock->serverip = hostname;
	
	sock->port = port;

	return 0;
}

int socket_client_connect(socket_t *sock)
{
	int                   cn = -1;
	struct sockaddr_in    servaddr;

	*sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		log_error("Create socket failur %s\n", strerror(errno));
		return -1;
	}
	log_info("Create socket[%d] successfully!\n", *sockfd);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(*port);
	net_aton(servip, &servaddr.sin_addr);
	
	cn = connect( *sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr) );

	if(cn < 0)
	{
		log_warn("Connect to server[%s:%d] failure: %s\n",servip, port, strerror(errno));
		socket_close(sock);
		return -2;
	}
	else
	{
		log_info("Connect to server successfully!\n");
	}

	return cn;
}

int socket_client_judge(int sockfd)
{
	struct tcp_info   info;

	if(sockfd < 0)
	{
		return 0;
	}
	int len = sizeof(info);

	getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *) &len);
	if((info.tcpi_state == 1))
	{
		log_info("socket connected\n");
		return 1;
	}
	else
	{
		log_errno("socket disconnected\n");
		return 0;
	}
}
