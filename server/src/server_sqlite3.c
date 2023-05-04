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
int get_sqlite_create_db(char *buf)
{
    char              create_buf[128];
    char             *zErrMsg;
    int               rc;
	char             *s1,*s2,*s3;
	s1 = strtok(buf,"/");
	s2 = strtok(NULL,"/");
	s3 = strtok(NULL,"/");

    rc = sqlite3_open("server.db", &db);

    if( rc )
    {
    	log_warn("Can't open database: %s\n", strerror(errno));
        return -1;
    }
    else
    {
        log_info("Opened database successfully.\n");
    }

	memset(create_buf,0,sizeof(create_buf));
	sprintf(create_buf,"CREATE TABLE %s(ID INTEGER PRIMARY KEY, time CHAR(80),serial CHAR(30),temperature CHAR(50));",str2);
	rc = sqlite3_exec(db,create_buf,callback,0,&zErrMsg);

	if( rc != SQLITE_OK )
    {
        log_warn("failure to create %s: %s\n",list_name,zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
    else
    {
        log_info("create %s successfully\n",list_name);
    }

    return 0;
}

/*向数据库表里面插入数据*/
int sqlite_insert_data(char *buf)
{
	char      *zErrMsg;
	int        rc;
	char       insert_buf[512];
	int        rd;
	char      *str1,*str2,*str3;
	str1 = strtok(buf,"/");
	str2 = strtok(NULL,"/");
	str3 = strtok(NULL,"/");

	memset(insert_buf, 0, sizeof(insert_buf));
	sprintf(insert_buf,"INSERT INTO %s VALUES( NULL, '%s', '%s', '%s' );", list_name,str1, str2, str3);

	rc = sqlite3_exec(db, insert_buf, callback, 0, &zErrMsg);

	if(rc != SQLITE_OK)
	{
		
		log_warn("insert data failure: %s\n",zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	log_info("Insert  data successfully\n");
	return 0;
}

/* 关闭sqlite3数据库 */
int sqlite_close_db(void)
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
