#ifndef __LOG__
#define __LOG__

#include <stdio.h>
#define DEBUG  1
extern int log_level;

void get_sys_time(void);

enum LOG_LEVEL
{
	LOG_LEVEL_OFF   = -3,
	LOG_LEVEL_ERROR = -2,
	LOG_LEVEL_WARN  = -1,
	LOG_LEVEL_INFO  =  0,
	LOG_LEVEL_DEBUG =  1,
	LOG_LEVEL_ALL   =  2,
};

#if DEBUG
#define logd(format, ...) \
do {\
	if (log_level >= LOG_LEVEL_DEBUG) {\
	    get_sys_time(); \
            printf("[%s] [%d] [DEBUG]  "format"", __FILE__, __LINE__, ##__VA_ARGS__);	\
	} \
} while(0)

#define logi(format, ...) \
do {\
        if (log_level >= LOG_LEVEL_INFO) {\
            get_sys_time(); \
            printf("[%s] [%d] [INFO]  "format"", __FILE__, __LINE__, ##__VA_ARGS__);      \
        } \
} while(0)

#define logw(format, ...) \
do {\
        if (log_level >= LOG_LEVEL_WARN) {\
            get_sys_time(); \
            printf("[%s] [%d] [WARN]  "format"", __FILE__, __LINE__, ##__VA_ARGS__);      \
        } \
} while(0)

#define loge(format, ...) \
do {\
        if (log_level >= LOG_LEVEL_ERROR) {\
            get_sys_time(); \
            printf("[%s] [%d] [ERROR]  "format"", __FILE__, __LINE__, ##__VA_ARGS__);      \
        } \
} while(0)
#else
#define log(frm, args)
#endif

#endif
