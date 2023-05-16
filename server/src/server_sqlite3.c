/*********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  my_sqlite3.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/18/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "04/18/2023 10:44:26 AM"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "logger.h"

static sqlite3  *db;

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	int    i;
    
	for(i=0; i<argc; i++)
    {
        log_info("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    log_info("\n");
    return 0;
}


/*创建连接数据库并且创建叫产品序列号名字的表*/
int sqlite_init(void)
{
    char             *zErrMsg;
    int               rv;

    rv = sqlite3_open("server.db", &db);

    if( !rv )
    {
    	log_warn("Can't open database: %s\n", zErrMsg);
        return -1;
    }
    else
    {
        log_info("Opened database successfully.\n");
    }
}


/*创建表名并向数据库表里面插入数据*/
int sqlite_insert_data(char *buf)
{
    char       create_buf[128];
	int        rv;
	char       insert_buf[512];
	float      temp;
	char      *zErrMsg;
	char      *time;
	char      *sn;

	time = strtok(buf,"/");
	sn   = strtok(NULL,"/");
	temp = atof(strtok(NULL,"/"));

	memset(create_buf,0,sizeof(create_buf));
	sprintf(create_buf,"CREATE TABLE %s(ID INTEGER PRIMARY KEY, time CHAR(80),serial CHAR(30),temperature CHAR(50));",sn);
	rv = sqlite3_exec(db,create_buf,callback,0,&zErrMsg);
	if( rv != SQLITE_OK )
    {   
        log_warn("failure to create %s: %s\n",sn, zErrMsg);
        sqlite3_free(zErrMsg);
    }   
	else
	{
		log_info("create list %s successfully\n", sn);
	}

	memset(insert_buf, 0, sizeof(insert_buf));
	sprintf(insert_buf,"INSERT INTO %s VALUES( NULL, '%s', '%s', '%f' );", sn, time, sn, temp);

	rv = sqlite3_exec(db, insert_buf, callback, 0, &zErrMsg);

	if(rv != SQLITE_OK)
	{
		
		log_warn("insert data failure: %s\n",zErrMsg);
		sqlite3_free(zErrMsg);
	}

	log_info("Insert  data successfully\n");
	return 0;
}

/* 关闭sqlite3数据库 */
int sqlite_close(void)
{
    char       *zErrMsg;
    int         rc; 

    rc = sqlite3_close(db);
    if(rc != SQLITE_OK)
    {   
        log_warn("close database failure: %s\n",zErrMsg);
        sqlite3_free(zErrMsg);
        return -1; 
    }   
    log_info("close database successfully\n");
    return 0;
}
