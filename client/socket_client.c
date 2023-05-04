/*********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  p.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/29/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "04/29/2023 04:04:31 PM"
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

#include "time.h"
#include "temp.h"
#include "my_sqlite3.h"
#include "logger.h"
#include "client.h"

int get_serial(char *serial_buf);
static inline void print_usage(char *progname);

typedef struct socket_s
{
	int     fd;
	int     port;
	char    hostname;
} socket_t;


int main(int argc, char **argv)
{
	int                   sockfd = -1;
	int                   rv = -1;
	int                   port = 0;
	int                   itval = 10;
	int                   founds = 0;
	int                   sample = 0;
	long                  collet_time = 0;
	long                  current_time = 0;                 
	int                   ch;
	int                   maxid;
	int                   idx;
	char                  serial_buf[16];
	char                  time_buf[64];
	char                  temp_buf[64];
	char                  send_buf[128];
	char                  buf[512];
	char                  data_buff[512];
	socket_t              sock;
	struct timeval        tv;
	struct tm            *st;
	char                 *servip = NULL;

	struct option       opts[] = {
		{"hostname", required_argument, NULL, 'h'},
		{"port", required_argument, NULL, 'p'},
		{"interval", required_argument, NULL, 't'},
		{"Help", no_argument, NULL, 'H'},
		{NULL, 0, NULL, 0}
	};

	while((ch=getopt_long(argc, argv, "h:p:t:H", opts, &idx)) != -1)
	{
		switch(ch)
		{
			case 'h':
				servip=optarg;
				break;
			case 'p':
				port=atoi(optarg);
				break;
			case 't':
				interval=atoi(optarg);
				break;
			case 'H':
				print_usage(argv[0]);
				return 0;
		}
	}
	if( argc < 3)
	{
		log_error("Program usage: %s [ServerIP] [Port] [Time]\n",argv[0]);
		return 0;
	}

	servip=argv[1];
	port=atoi(argv[2]);

	if( logger_init("stdout", LOG_LEVEL_DEBUG) < 0)
	{
		fprintf(stderr, "initial logger system failure\n");
		return 1;
	}


	/* 将socket初始化 */
	if( socket_client_init(&sock, hostname, port) < 0)
	{
		log_warn("socket initialization failure\n");
		return 0;
	}

	if(get_sqlite_create_db() < 0)
	{
		log_warn("sqlite3 initialization failed\n");
	}
	else
	{
		log_info("sqlite3 initialization successfully\n");
	}
		

	while(1)
	{
		/* 判断到没到采样时间，时间到了就开始采样 */
		gettimeofday(&tv, NULL);
		current_time=tv.tv_sec;
		if( (current_time-collet_time) >= itval )
		{
			sample = 1;
			/* 获取当前的时间 */
			collet_time=get_time(time_buf);
			/* 获取当前的温度 */
			get_temp(temp_buf);
			/* 获取产品序列号 */
			get_serial(serial_buf);
		}

		else
		{
			sample = 0;
			log_info("It's not sampling time\n");
		}

		/* 判断socket连没连上，如果没连上就连接服务器 */
		if(socket_client_judge(sockfd) < 0)
		{
			if( sockfd > 0)
			{
				socket_close(sock);
			}

			if( socket_client_connect(&sock) > 0 )
			{
				log_info("socket client connect againt successfully\n");
			}
			else
			{
				log_error("socket client connect againt failed\n");
				socket_close(sock);
			}
		}

		/* socket断线重连失败，且有采样，把采样数据插入到数据库 */
		if(sockfd<0)
		{
			if( sample )
			{
				if( (rv=sqlite_insert_data(time_buf, serial_buf, temp_buf)) < 0 )
				{
					log_info("Insert data failed\n");
				}
			}
			continue;
		}

		/* socket连上了且数据有采样 */
		if( sample )
		{
			memset(data_buff,0,sizeof(data_buff));
			sprintf(data_buff,"%s/%s/%s",time_buf,serial_buf,temp_buf);
			log_debug("data_buff: %s\n", data_buff);
			rv = write(sockfd,data_buff,sizeof(data_buff));
			if(rv < 0)
			{
				/* 把没发送成功的数据插入到数据库 */
				sqlite_insert_data(time_buf, serial_buf, temp_buf);
				log_error("send data failure: %s\n",strerror(errno));
				socket_close(sock);
			}
			else
			{
				log_info("sample send successfully\n");
			}
		}
		
		/* 如果没有采样的数据但是socket已经连接，将数据库里面的数据发送到服务器 */
		else
		{
			/* 判断数据库里面有没有数据，maxid大于0就是有数据 */
			if(sqlite_maxid(&maxid) > 0)
			{
				/* 把数据库里面最大id数的数据发送到服务器 */
				sqlite_send_data(send_buf);
				rv=write(sockfd,send_buf,sizeof(send_buf));
				/* 把数据库里面最大id数的数据删除 */
				sqlite_delete_data();
				if(rv < 0)
				{
					log_error("send data failure: %s\n",strerror(errno));
					socket_close(sock)
				}
				else
				{
					log_info("sqlite3 data send successfully\n");
				}
			}
			else
			{
				log_info("sqlite has no data\n");
			}
		}
	}

	sqlite_close_db();
	socket_close(sock);
	return 0;
}

int get_serial(char *serial_buf)
{
	int   n = 1;
	memset(serial_buf, 0, sizeof(serial_buf));
	sprintf(serial_buf, "hby%03d",n);
	return 0;
}

static inline void print_usage(char *progname)
{
    log_error("%s usage: \n", progname);
    log_error("-h[hostname ]: sepcify server IP address\n");
    log_error("-p[port     ]: sepcify server port.\n");
    log_error("-t[interval ]: sepcify the time to send.\n");
    log_error("-H[Help     ]: print this help informstion.\n");
    return ;
}

