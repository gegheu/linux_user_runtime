#ifndef __UBUNTU_RS422__
#define __UBUNTU_RS422__

#include <stdio.h>
#include <sys/types.h>

typedef struct rs422_parameter {
        const char* file_path;
        const char* dev_name;
        int fd;
        int baudrate;
        int databits;
        int stopbits;
        char parity;
} rs422_parameter;

enum DATA_SIZE {
        FIVE = 5,
        SIX,
        SEVEN,
        EIGHT,
};

enum STOP_BIT_NUM {
        ONE_STOP_BIT = 1,
        TWO_STOP_BIT,
};

#if OS_UBUNTU
int ubuntu_rs422_init(rs422_parameter* rp);
int ubuntu_rs422_set_speed(rs422_parameter* rp);
int ubuntu_rs422_write_data(rs422_parameter* rp, unsigned char* data, int data_len);
int ubuntu_rs422_read_data(rs422_parameter* rp, unsigned char* data, int data_len);
int ubuntu_rs422_set_parity(rs422_parameter* rp);
int ubuntu_rs422_set_parameter(rs422_parameter* rp);
int ubuntu_rs422_deinit(rs422_parameter* rp);
void *ubuntu_rs422_pthread_handle(void * arg);
void test_ubuntu_rs422(void);
#endif
#endif
