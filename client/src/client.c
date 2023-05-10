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
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
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
	int                   get_back = -1;
	struct addrinfo       hints;     //定义一个结构体
	struct sockaddr_in    servaddr;
    struct addrinfo      *res;      //定义函数返回的结构体链表的指针
    struct addrinfo      *read;     //定义一个遍历链表的指针
	struct sockaddr_in   *addr;     //定义一个存储返回域名IP信息的结构体指针

	/* 域名解析 */
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE; 
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;
	hints.ai_protocol = 0;

	get_back = getaddrinfo(sock->hostip, NULL, &hints, &res);  //调用函数

	if(get_back != 0)
	{
		log_error("Analyze failure: %s\n", strerror(errno));
		return -1;
	}
	log_info("Analyze successfully\n");    //调用函数成功

	for(read = res; read != NULL; read = read->ai_next)  //遍历链表每一个节点，查询关于存储返回的IP的信息
	{
		addr = (struct sockaddr_in *)read->ai_addr;    //将返回的IP信息存储在addr指向的结构体中
		log_info("IP address: %s\n", inet_ntoa(addr->sin_addr));   //inet_ntoa函数将字符串类型IP地址转化为点分十进制
	}

	/* 开始连接服务器 */
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

	freeaddrinfo(res);    //释放getaddrinfo函数调用动态获取的空间
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
