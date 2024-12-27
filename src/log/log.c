#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include "log/log.h"


int log_level = LOG_LEVEL_INFO;


void get_sys_time(void)
{
    struct timeval    tv;
    struct tm         *tm_ptr;

    gettimeofday(&tv, NULL);

    tm_ptr = localtime(&tv.tv_sec);
    printf("[%d-%02d-%02d %02d:%02d:%02d.%ld] ", 1900+tm_ptr->tm_year, 1+tm_ptr->tm_mon,
		tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec, tv.tv_usec/1000);
}
