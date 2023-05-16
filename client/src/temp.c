/*********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  temp.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(05/02/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "05/02/2023 03:59:25 PM"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "logger.h"
#include "packdata.h"

int get_temperature(float *temp)
{
    int              fd = -1;
    char             buf[128];
    char             w1_path[64]="../../w1/device/";
    char             chip_sn[32];
    int              found = 0;
    char             ds18b20_path[64];
    char            *ptr = NULL;
    DIR             *dirp = NULL;
    struct dirent   *direntp = NULL;
    dirp=opendir(w1_path);

    if(!dirp)
    {
        log_error("open folder %s failure: %s\n",w1_path,strerror(errno));
        return -1;
    }

    while(NULL!=(direntp=readdir(dirp)))
    {
        if(strstr(direntp->d_name,"28-"))
        {
            strncpy(chip_sn,direntp->d_name,sizeof(chip_sn));
            found=1;
        }
    }

    closedir(dirp);

    if(!found)
    {
        log_error("can not find ds18b20_path chipest\n");
        return -2;
    }
    strncat(w1_path,chip_sn,sizeof(w1_path)-strlen(w1_path));
    strncat(w1_path,"/w1_slave",sizeof(w1_path)-strlen(w1_path));
    fd=open(w1_path,O_RDONLY);

    if(fd<0)
    {
        perror("open file failure\n");
        return -3;
    }

    memset(buf,0,sizeof(buf));

    if(read(fd,buf,sizeof(buf))<0)
    {
        log_warn("read data from fd=%d failure: %s\n",fd,strerror(errno));
        return -4;
    }
    ptr=strstr(buf,"t=");

    if(!ptr)
    {
        log_warn("can not find t=string\n");
        return -5;
    }

    ptr+=2;
    *temp=atof(ptr)/1000;
    close(fd);
    return 0;
}

/* 获取温度 */
int get_temp(packdata_t *packdata)
{
    float   temp;
    int     rv = -1;

    rv=get_temperature(&temp);
    if(rv<0)
    {
        log_warn("get temperature failure,return value:%d",rv);
        return -1;
    }
	packdata->temperature = temp;
    return 0;

}
