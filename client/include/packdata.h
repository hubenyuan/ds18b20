/********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  packdata.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(05/05/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "05/05/2023 03:47:13 PM"
 *                 
 ********************************************************************************/

#ifndef PACKDATA_H_
#define PACKDATA_H_

typedef struct packdata_st
{
	char    data_time[64];
	char    data_serial[64];
	char    data_temp[64];
} packdata_t;

#endif
