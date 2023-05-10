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
#include "packdata.h"

int sqlite_create_db(void);

int sqlite_insert_data(packdata_t *packdata);

int sqlite_maxid(int *maxid);

int sqlite_select_data(packdata_t *packdata);

int sqlite_delete_data(void);

int sqlite_close_db(void);

#endif

