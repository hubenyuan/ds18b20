/*********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  time.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(05/02/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "05/02/2023 03:52:39 PM"
 *                 
 ********************************************************************************/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "logger.h"

/*获取采样的时间*/
int get_time(char *time_buf)
{
    time_t       now;
    struct tm   *t; 
    time(&now);
    t = localtime(&now);
    if(!t)
    {   
        log_warn("obtain time failure: %s\n",strerror(errno));
        return -1; 
    }   

    memset(time_buf, 0, sizeof(time_buf));
    sprintf(time_buf, "%04d-%02d-%02d %02d:%02d:%02d",
            t->tm_year+1900,t->tm_mon+1,t->tm_mday,
            t->tm_hour,t->tm_min,t->tm_sec);
    return 0;
} 


