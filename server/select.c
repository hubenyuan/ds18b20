/*********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  o.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/29/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "04/29/2023 12:20:46 PM"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>
#include <libgen.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "server_sqlite3.h"
#include "logger.h"

#define ARRAY_SIZE(x)     (sizeof(x)/sizeof(x[0]))

static inline void msleep(unsigned long ms);
static inline void print_usage(char *progname);
int  socket_server_init(char *listen_ip, int listen_port);

int main(int argc, char **argv)
{
	int                       listenfd, connfd;
	int                       serv_port;
	int                       daemon_run = 0;
	char                     *progname = NULL;
	int                       opt;
	int                       rv;
	int                       i, j;
	int                       found;
	int                       maxfd = 0;
	char                      buf[512];
	int                       fds_array[1024];
	fd_set                    rdset;

	struct option         long_options[]=
	{
		{"daemon", no_argument, NULL, 'b'},
		{"port", required_argument, NULL, 'p'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	progname = basename(argv[0]);

	while( (opt = getopt_long(argc, argv,"bp:h",long_options, NULL)) != -1)
	{
		switch (opt)
		{
			case 'b':
				daemon_run = 1;
				break;
			case 'p':
				serv_port = atoi(optarg);
				break;
			case 'h':
				print_usage(progname);
				return EXIT_SUCCESS;

			default:
				break;
		}
	}

	if( !serv_port )
	{
		print_usage(progname);
		return -1;
	}

	if( logger_init("stdout", LOG_LEVEL_DEBUG) < 0)
	{
		fprintf(stderr, "initial logger system failure\n");
        return 1;
    }

	/* 创建连接数据库并且创建名为server_data的表 */
	get_sqlite_create_db();


	if( (listenfd=socket_server_init( NULL, serv_port)) < 0 )
	{
		log_error("ERROR: %s server listen on port %d failure\n",argv[0],serv_port);
		return -2;
	}
	log_info("%s server start to listen on port %d\n",argv[0],serv_port);

	if( daemon_run )
	{
		daemon(0, 0);
	}

	for(i=0; i<ARRAY_SIZE(fds_array); i++)
	{
		fds_array[i] = -1;	
	}
	fds_array[0] = listenfd;

	for( ; ; )
	{
		FD_ZERO(&rdset);
		for(i=0; i<ARRAY_SIZE(fds_array); i++)
		{
			if( fds_array[i] < 0 )
				continue;

			maxfd = fds_array[i]>maxfd ? fds_array[i] : maxfd;
			FD_SET(fds_array[i], &rdset);
		}

		rv = select(maxfd+1, &rdset, NULL, NULL, NULL);

		if( rv < 0 )
		{
			log_error("select failure: %s\n",strerror(errno));
			break;
		}
		else if( rv == 0 )
		{
			log_error("select get timeout: %s\n",strerror(errno));
			continue;
		}
		
		if( FD_ISSET(listenfd, &rdset) )
		{
			if( (connfd = accept(listenfd,(struct sockaddr *)NULL, NULL)) < 0 )
			{
				log_warn("accept new client failure: %s\n",strerror(errno));
				continue;
			}

			found = 0;

			for(i=0; i<ARRAY_SIZE(fds_array); i++)
			{
				if( fds_array[i] < 0 )
				{
					log_info("accept new client[%d] and add it into array\n",connfd);
					fds_array[i] = connfd;
					found = 1;
					break;
				}
			}

			if( !found )
			{
				log_error("accept new client[%d] but full,so refuse it\n",connfd);
				close(connfd);
			}
		}
		else
		{
			memset(buf, 0, sizeof(buf));
			for(i=0; i<ARRAY_SIZE(fds_array); i++)
			{
				if( fds_array[i]<0 || !FD_ISSET(fds_array[i], &rdset))
					continue;
				if( (rv=read(fds_array[i],buf,sizeof(buf))) <= 0 )
				{
					log_error("socket[%d] read failure or get disconnect.\n",fds_array[i]);
					close(fds_array[i]);
					fds_array[i] = -1;
				}
				else
				{

					log_info("socket[%d] read get %d bytes data: %s\n", fds_array[i], rv, buf);

					/* 向数据库表里面插入数据 */
					sqlite_insert_data(buf);
					
					for(j=0; j<rv; j++)
						buf[j]=toupper(buf[j]);
					if( write(fds_array[i], buf, rv) < 0 )
					{
						log_error("socket[%d] write failure: %s\n",fds_array[i], strerror(errno));
						close(fds_array[i]);
						fds_array[i] = -1;
					}
				}
			}
		}
	}

CleanUp:
	close(listenfd);
	sqlite_close_db();
	return 0;
}


static inline void msleep(unsigned long ms)
{
	struct timeval         tv;

	tv.tv_sec = ms/1000;
	tv.tv_usec = (ms%1000)*1000;

	select(0, NULL, NULL, NULL, &tv);
}

static inline void print_usage(char *progname)
{
        log_error("Usage: %s [OPTION]...\n", progname);
        log_error(" %s is a socket server progname which used to verify client ad echo back string from it.\n",progname);
        log_error("\nMandatory arguments to long options are mandatory for short options too:\n");

        log_error(" -b[daemon ] set program running on backgroud\n");
        log_error(" -p[port   ] Socket server port address\n");
        log_error(" -h[help   ] Display this help information\n");

        log_error("\nExample: %s -b -p 8900\n", progname);
        return ;

}

int socket_server_init(char *listen_ip, int listen_port)
{
	struct sockaddr_in      servaddr;
	int                     rv = 0;
	int                     on = 1;
	int                     listenfd;

	if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		log_error("Use socket() to create a TCP socket failure: %s\n",strerror(errno));
		return -1;
	}

	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(listen_port);


	if( !listen_ip )
	{
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		if(inet_pton(AF_INET, listen_ip, &servaddr.sin_addr) <= 0 )
		{
			log_error("inet_pton() set listen IP address failure.\n");
			rv = -2;
			goto CleanUp;
		}
	}

	if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 )
	{
		log_error("Use bind() to bind the TCP socket failure: %s\n", strerror(errno));
		rv = -3;
		goto CleanUp;
	}

	if( listen(listenfd, 13) < 0 )
	{
		log_error("Use bind() to bind the Tcp socket failure: %s\n", strerror(errno));
		rv = -4;
		goto CleanUp;
	}
CleanUp:
	if( rv < 0 )
		close(listenfd);
	else
		rv = listenfd;

	return rv;
}
