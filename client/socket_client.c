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

#include "package_data.h"
#include "my_sqlite3.h"
#include "logger.h"


int get_serial(char *serial_buf);

static inline void print_usage(char *progname)
{
    log_error("%s usage: \n", progname);
    log_error("-i[ipaddr ]: sepcify server IP address\n");
    log_error("-p[port   ]: sepcify server port.\n");
    log_error("-t[times  ]: sepcify the time to send.\n");
    log_error("-h[help   ]: print this help informstion.\n");
    return ;
}

int main(int argc, char **argv)
{
	int                   sockfd = -1;
	int                   rv = -1;
	int                   port = 0;
	int                   times = 3;
	char                  buf[512];
	char                  data_buff[512];
	int                   ch;
	int                   maxid;
	int                   idx;
	int                   founds = 0;
	char                  serial_buf[16];
	char                  time_buf[64];
	char                  temp_buf[64];
	char                  send_buf[128];
	struct sockaddr_in    servaddr;
	char                 *servip = NULL;

	struct option       opts[] = {
		{"hostname", required_argument, NULL, 'h'},
		{"port", required_argument, NULL, 'p'},
		{"times", required_argument, NULL, 't'},
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
				times=atoi(optarg);
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
	port = atoi(argv[2]);

	if( logger_init("stdout", LOG_LEVEL_DEBUG) < 0)
	{
		fprintf(stderr, "initial logger system failure\n");
		return 1;
	}

	/*创建连接数据库和并且创建名为packaged_data的表*/
	get_sqlite_create_db();

	while(1)
	{
		/* 创建文件描述符sockfd */
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(sockfd < 0)
		{
			log_error("Create socket failur %s\n", strerror(errno));
			close(sockfd);
			sockfd = -1;
			return -1;
		}
		log_info("Create socket[%d] successfully!\n",sockfd);

		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family=AF_INET;
		servaddr.sin_port = htons(port);
		inet_aton(servip, &servaddr.sin_addr);

		rv = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if(rv < 0)
		{
			log_warn("Connect to server[%s:%d] failure: %s\n",servip, port, strerror(errno));
            /* 获取当前时间 */
			get_time(time_buf);
			/* 获取温度 */ 
			get_temporary(temp_buf);
			/* 获取产品序列号 */
			get_serial(serial_buf);

		    sqlite_insert_data(time_buf, serial_buf, temp_buf);
			close(sockfd);
			sockfd = -1;
			sleep(5);
			continue;
		}
		log_info("Connect to server[%s:%d] successfully!\n", servip, port);

		while(1)
		{
			/* 获取数据库数据最大ID并且判断数据库里面存不存在数据 */
			sqlite_maxid( &maxid );
			/* maxid > 0表示有数据 */
			if(maxid>0)
			{
				memset(send_buf, 0, sizeof(send_buf));
				memset(data_buff, 0, sizeof(data_buff));
				/* 获取数据库表里面的数据并且发送到服务器 */
				sqlite_select_data(send_buf);
				sprintf(data_buff,"%s\n",send_buf);
				rv=write(sockfd, data_buff, strlen(data_buff));
				/* 删除数据库表里面ID最大的数据  */
				sqlite_delete_data();
				continue;
			}
			break;
		}

		while(1)
		{
			/* 获取当前时间*/ 
			get_time(time_buf);
			/* 获取温度*/
			get_temporary(temp_buf);
			/* 获取产品序列号 */
			get_serial(serial_buf);

			memset(data_buff,0,sizeof(data_buff));
			sprintf(data_buff,"%s/%s/%s\n",time_buf,serial_buf,temp_buf);

			rv=write(sockfd, data_buff, strlen(data_buff));
			if(rv < 0 )
			{
				log_error("write to server by socket[%d] failure: %s\n",sockfd,strerror(errno));
				break;
			}

			memset(buf, 0, sizeof(buf));
			rv=read(sockfd, buf, sizeof(buf));	
			if(rv < 0 )
			{
				log_error("Read data from server by sockfd[%d] failure: %s\n",sockfd, strerror(errno));
				break;
			}
			else if( rv == 0 )
			{
				log_error("socket[%d] get disconnected\n",sockfd);
				break;
			}
			else if( rv > 0 )
			{	
				log_info("Read %d bytes data from Server.\n",rv);
			}
			sleep(times);
		}
	}

	sqlite_close_db();
	close(sockfd);
	return 0;
}

int get_serial(char *serial_buf)
{
	int   n = 1;
	memset(serial_buf, 0, sizeof(serial_buf));
	sprintf(serial_buf, "hby%03d",n);
	return 0;
}
