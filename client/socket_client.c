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

#include "temp.h"
#include "get_time.h"
#include "my_sqlite3.h"
#include "logger.h"
#include "client.h"
#include "packdata.h"

int get_serial(char *serial_buf);
static inline void print_usage(char *progname);


int main(int argc, char **argv)
{
	int                   sockfd = -1;
	int                   rv = -1;
	int                   port = 8888;    //默认端口
	int                   itval = 6;     //设置上报时间，默认6秒
	int                   sample = 0;     //采样标志符，为0没采样，为1采了样
	time_t                collet_time = 0;     //上次采样时间戳
	time_t                current_time = 0;    //当前时间戳           
	int                   ch;
	int                   maxid;
	int                   idx;
	char                  hostname[64] = "192.168.68.129";   //默认IP
	char                  serial_buf[16];
	char                  time_buf[64];
	char                  temp_buf[64];
	socket_t              sock;
	packdata_t            packdata;

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
				strcpy(hostname,optarg);
				break;
			case 'p':
				port=atoi(optarg);
				break;
			case 't':
				itval=atoi(optarg);
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

	if( logger_init("stdout", LOG_LEVEL_DEBUG) < 0)
	{
		fprintf(stderr, "initial logger system failure\n");
		return 1;
	}

	rv = socket_client_init(&sock, hostname, port);
	if(rv < 0)
	{
		log_warn("socket initialization failure\n");
		return -1;
	}

	/* 创建数据库的表格 */
	if(sqlite_create_db() < 0)
	{
		log_warn("sqlite3 initialization failed\n");
	}

	while(1)
	{
		/* 判断到没到采样时间，时间到了就开始采样 */
		current_time = time(NULL);
		if( (current_time-collet_time) >= itval )
		{
			sample = 1;

			/* 获取当前的时间 */
			get_time(time_buf);
			/* 获取当前的温度 */
			get_temp(temp_buf);
			/* 获取产品序列号 */
			get_serial(serial_buf);

			/* 将采集的数据都放到packdata结构体里面 */
			memset(&packdata, 0, sizeof(packdata));
			strcpy(packdata.data_time, time_buf);
			strcpy(packdata.data_serial, serial_buf);
			strcpy(packdata.data_temp, temp_buf);
			log_debug("data_time= %s,data_serial= %s,data_temp= %s\n",packdata.data_time,packdata.data_serial,packdata.data_temp);

			collet_time = current_time;
		}

		else
		{
			sample = 0;
			log_info("It's not sampling time\n");
		}

		/* 判断socket连没连接，如果没连接就开始连接服务器 */
		if(socket_client_judge(&sock) < 0)
		{
			if( socket_client_connect(&sock) < 0 )
			{
				log_warn("socket client connect againt failed\n");
				socket_close(&sock);
			}
		}

		/* socket断线重连失败，且有采样，把采样数据插入到数据库 */
		if( !sock.connected )
		{
			if( sample )
			{
				if(sqlite_insert_data(&packdata) < 0 )
				{
					log_error("Insert data failed\n");
				}
			}
			continue;
		}

		/* socket连上了且数据有采样 */
		if( sample )
		{
			rv = socket_client_send(&sock, packdata);
			if(rv < 0)
			{
				/* 把没发送成功的数据插入到数据库 */
				sqlite_insert_data(&packdata);
				log_error("send data failure\n");
				socket_close(&sock);
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
			sqlite_maxid(&maxid);
			if( maxid > 0)
			{
				/* 把数据库里面最大id数的数据提取出来 */
				if(sqlite_select_data(&packdata) < 0)
				{
					log_warn("database get data failure\n");
					log_debug("maxid= %d\n", maxid);
				}
				else
				{
					/* 把数据库里面的id数最大的数据发送到服务器 */
					rv = socket_client_send(&sock, packdata);
					if(rv < 0)
					{
						log_error("database send data failure\n");
						socket_close(&sock);
					}
					else
					{
						/* 把数据库里面最大id数的数据删除 */
						if(sqlite_delete_data() < 0)
						{
							log_warn("database delete data failure or no data\n");
						}
					}
				}
			}
			else
			{
				log_info("database has no data\n");
			}
		}
	}

	sqlite_close_db();
	socket_close(&sock);
	return 0;
}

int get_serial(char *serial_buf)
{
	int   n = 1;
	memset(serial_buf, 0, sizeof(serial_buf));
	sprintf(serial_buf, "hby%03d", n);
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

