/********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  logger.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(04/29/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "04/29/2023 07:04:00 PM"
 *                 
 ********************************************************************************/

#ifndef _LOGGER_H_
#define _LOGGER_H_

enum
{
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARN,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_MAX,
};

#define LOG_STRING_ERROR   "ERROR"
#define LOG_STRING_WARN    "WARN"
#define LOG_STRING_INFO    "INFO"
#define LOG_STRING_DEBUG   "DEBUG"


int logger_init(char *filename, int loglevel);
void logger_term(void);

void log_error(char *format, ...);
void log_warn(char *format, ...);
void log_info(char *format, ...);
void log_debug(char *format, ...);

#endif
