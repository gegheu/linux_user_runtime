#ifndef __CAN__
#define __CAN__

#include <stdio.h>
#include <sys/types.h>

#define CAN_MAX_DLEN 8

typedef struct canfilter {
        unsigned int can_id;
        unsigned int can_mask;
	int mode;
} canfilter;

typedef struct canframe {
        unsigned int can_id;
        unsigned char can_dlc;
        unsigned char pad;
        unsigned char res0;
        unsigned char res1;
        unsigned char data[CAN_MAX_DLEN] __attribute__((aligned(8)));
} canframe;

typedef struct can_parameter {
        const char* file_path;
        const char* dev_name;
        int flags;
	int speed;
        mode_t mode;
        int fd;
        canfilter cf;
} can_parameter;
#endif
