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

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <netinet/tcp.h>

#include "packdata.h"
#include "logger.h"

typedef struct socket_s
{
	int       sockfd;
	int       port;
	char     *servip;
} socket_t;

/* 关闭socket客户端并且把sockfd置为-1 */
int socket_close(socket_t *sock)
{
	close(sock->sockfd);
	sock->sockfd = -1;
}

/* socket客户端开始连接服务器 */
int socket_client_connect(socket_t *sock)
{
	int                   cn = -1;
	struct sockaddr_in    servaddr;

	(sock->sockfd) = socket(AF_INET, SOCK_STREAM, 0);
	if( (sock->sockfd) < 0 )
	{
		log_error("Create socket failur %s\n", strerror(errno));
		return -1;
	}
	log_info("Create socket[%d] successfully!\n", (sock->sockfd));

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(sock->port);
	inet_aton( (sock->servip), &servaddr.sin_addr );
	
	cn = connect( (sock->sockfd), (struct sockaddr *)&servaddr, sizeof(servaddr) );

	if(cn < 0)
	{
		log_warn("Connect to server failure: %s\n",strerror(errno));
		socket_close(sock);
	}
	else
	{
		log_info("Connect to server [%s:%d] successfully!\n", sock->servip, sock->port);
	}

	return cn;
}

/* 判断客户端有没有连接服务器 */
int socket_client_judge(int sockfd)
{
	struct tcp_info   info;
	int len = sizeof(info);
	int rv = -1;

	if(sockfd < 0)
	{
		log_error("Get socket status error: %s\n", strerror(errno));
		return -1;
	}

	rv = getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *) &len);
	if(rv < 0)
	{
		log_info("socket connected\n");
		return -2;
	}

	if(info.tcpi_state == 1)
	{
		log_info("socket connected\n");
	}
	else
	{
		log_error("socket disconnected\n");
		return 0;
	}
}

/* 把采集到的数据发送给服务器 */
int socket_client_send(int sockfd, packdata_t packdata)
{
	int          rv = -1;
	char         data_buf[256];

	memset(data_buf, 0, sizeof(data_buf));
	sprintf(data_buf, "%s/%s/%s",packdata.data_time, packdata.data_serial, packdata.data_temp);

	log_debug("data_buf= %s\n", data_buf);
	rv = write(sockfd,data_buf,strlen(data_buf));
	if(rv < 0)
	{
		log_error("Send data to server failure: %s\n",strerror(errno));
	}
	else
	{
		log_info("Send data to server successfully\n");
	}
}
