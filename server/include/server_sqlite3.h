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
#ifndef  SERVER_SQLITE3_H
#define  SERVER_SQLITE3_H

int sqlite_init(void);

int sqlite_insert_data(char *buf);

int sqlite_close(void);

#endif

