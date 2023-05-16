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

int get_devsn(packdata_t *packdata);
static inline void print_usage(char *progname);


int main(int argc, char **argv)
{
	int                   rv = -1;
	int                   port = 8888;    //默认端口
	int                   itval = 6;     //设置上报时间，默认6秒
	int                   sample = 0;     //采样标志符，为0没采样，为1采了样
	int                   ch;
	int                   idx;
	int                   loglevel = LOG_LEVEL_INFO;
	int                   debug = 0;
	time_t                last_time = 0;     //上次采样时间戳
	time_t                current_time = 0;    //当前时间戳           
	char                  Hostname[64] = "192.168.68.129";   //默认IP
	socket_t              sock;
	packdata_t            packdata;
//	char                  *logfile = "client.log";    //定义一个client.log的文件用来后台输出

	struct option       opts[] = {
		{"Hostname", required_argument, NULL, 'h'},
		{"port", required_argument, NULL, 'p'},
		{"interval", required_argument, NULL, 't'},
	//	{"debug", no_argument, NULL, 'd'},
		{"help", no_argument, NULL, 'H'},
		{NULL, 0, NULL, 0}
	};

	while((ch=getopt_long(argc, argv, "H:p:t:dh", opts, &idx)) != -1)
	{
		switch(ch)
		{
			case 'H':
				strcpy(Hostname,optarg);
				break;
			case 'p':
				port=atoi(optarg);
				break;
			case 't':
				itval=atoi(optarg);
				break;
/*          case 'd':
				debug=1;
				logfile="stdout";
				loglevel=LOG_LEVEL_DEBUG;
*/			
			case 'h':
				print_usage(argv[0]);
				return 0;
		}
	}

/* 把输出放到client.log里面打印 */
/* 	if( logger_init(logfile, loglevel) < 0)
	{
		fprintf(stderr, "initial logger system failure\n");
		return -1;
	}
	if(!debug)
	{
		fprintf(stderr, "Initial logger file '%s' failure: %s\n", logfile, strerror(errno));
		daemon(1,1);
	}
*/

	if(logger_init("stdout", LOG_LEVEL_DEBUG) < 0)
	{
		fprintf(stderr, "initial logger system failure\n");
		return -1;
	}
	rv = socket_client_init(&sock, Hostname, port);
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
		if( (current_time-last_time) >= itval )
		{
			sample = 1;

			/* 获取当前的时间 */
			get_time(&packdata);
			/* 获取当前的温度 */
			get_temp(&packdata);
			/* 获取产品序列号 */
			get_devsn(&packdata);
			log_debug("time= %s,devsn= %s,temperature= %f\n",packdata.time, packdata.devsn,packdata.temperature);

			last_time = current_time;
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
			/* 把数据库里面第一条数据提取出来 */
			if(sqlite_select_data(&packdata) < 0)
			{
				log_warn("database get data failure or has no data\n");
			}
			else
			{
				/* 把数据库里面第一条数据发送到服务器 */
				rv = socket_client_send(&sock, packdata);
				if(rv < 0)
				{
					log_error("database send data failure\n");
					socket_close(&sock);
				}
				else
				{
					/* 把数据库里面第一条数据删除 */
					if(sqlite_delete_data() < 0)
					{
						log_warn("database delete data failure or no data\n");
					}
				}
			}
		}
	}

	sqlite_close_db();
	socket_close(&sock);
	return 0;
}

int get_devsn(packdata_t *packdata)
{
	int   n = 1;
	memset(packdata->devsn, 0, sizeof(packdata->devsn));
	sprintf(packdata->devsn, "hby%03d", n);
	return 0;
}

static inline void print_usage(char *progname)
{
    log_error("%s usage: \n", progname);
    log_error("-H[Hostname ]: sepcify server IP address\n");
    log_error("-p[port     ]: sepcify server port.\n");
    log_error("-t[interval ]: sepcify the time to send.\n");
    log_error("-d[debug    ]: run as debug mode.\n");
    log_error("-h[help     ]: print this help informstion.\n");
    return ;
}

