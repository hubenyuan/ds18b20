/********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  get_sqlite_create.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(04/17/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "04/17/2023 10:02:08 PM"
 *                 
 ********************************************************************************/
#ifndef  MY_SQLITE3_H
#define  MY_SQLITE3_H

int get_sqlite_create_db();

int sqlite_insert_data(char *time_buf, char *serial_buf, char *temp_buf);

int sqlite_maxid(int *maxid);

int sqlite_select_data(char *send_buf);

int sqlite_delete_data();

int sqlite_close_db(void);

#endif

