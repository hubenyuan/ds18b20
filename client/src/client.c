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
#include "client.h"


/* 初始化socket客户端 */
int socket_client_init(socket_t *sock, char *hostname, int port)
{
	sock->fd = -1;

	strcpy(sock->hostip, hostname);

	sock->port = port; 

	sock->connected = 0;

	return 0;

}

/* 关闭socket客户端并且把fd置为-1 */
int socket_close(socket_t *sock)
{
	close(sock->fd);
	sock->fd = -1;
	return 0;
}

/* socket客户端开始连接服务器 */
int socket_client_connect(socket_t *sock)
{
	int                   rv = -1;
	struct sockaddr_in    servaddr;

	(sock->fd) = socket(AF_INET, SOCK_STREAM, 0);
	if( (sock->fd) < 0 )
	{
		log_error("Create socket failur %s\n", strerror(errno));
		return -1;
	}
	log_info("Create socket[%d] successfully!\n", (sock->fd));

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(sock->port);
	inet_aton( (sock->hostip), &servaddr.sin_addr );
	
	rv=connect( (sock->fd), (struct sockaddr *)&servaddr, sizeof(servaddr) );

	if(rv < 0)
	{
		log_warn("Connect to server failure: %s\n",strerror(errno));
		socket_close(sock);
	}
	else
	{
		log_info("Connect to server [%s:%d] successfully!\n", sock->hostip, sock->port);
	}

	return rv;
}

/* 判断客户端有没有连接服务器 */
int socket_client_judge(socket_t *sock)
{
	struct tcp_info   info;
	int len = sizeof(info);
	int rv = -1;

	if(sock->fd < 0)
	{
		log_error("Get socket status error: %s\n", strerror(errno));
		return -1;
	}

	rv=getsockopt(sock->fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *) &len);
	if(rv < 0)
	{
		log_info("socket connected\n");
		return -2;
	}

	if(info.tcpi_state == 1)
	{
		log_info("socket connected\n");
		sock->connected = 1;
	}
	else
	{
		log_error("socket disconnected\n");
		sock->connected = 0;
		return -3;
	}

	return 0;
}

/* 把采集到的数据发送给服务器 */
int socket_client_send(socket_t *sock, packdata_t packdata)
{
	int          rv = -1;
	char         data_buf[256];

	memset(data_buf, 0, sizeof(data_buf));
	sprintf(data_buf, "%s/%s/%s",packdata.data_time, packdata.data_serial, packdata.data_temp);

	log_debug("data_buf= %s\n", data_buf);
	rv = write(sock->fd,data_buf,strlen(data_buf));
	if(rv < 0)
	{
		log_warn("Send data to server failure: %s\n",strerror(errno));
	}
	else
	{
		log_info("Send data to server successfully\n");
	}
	return 0;
}
