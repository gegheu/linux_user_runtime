#ifndef __UBUNTU_I2C__
#define __UBUNTU_I2C__

#include <stdio.h>

#define I2C_DEV_LENGTH           64
#define CHAR_BITS_NUM            8
#define I2C_ADDR_BITS_NUM        7
#define I2C_ADDR_BITS_NUM_MAX    10
#define MSB_VALUE_GET            0x0000FF00
#define LSB_VALUE_GET            0x000000FF
#define I2C_SEND_MAX_LEN         128
#define I2C_RECV_MAX_LEN         128

typedef struct i2c_parameter {
        const char* file_path;
        const char* dev_name;
        int fd;
        char dev[I2C_DEV_LENGTH];
        unsigned int slave_addr;
        unsigned int slave_addr_bits;
} i2c_parameter;

struct i2c_addr_value {
        unsigned int reg_addr;
        unsigned int reg_value;
        unsigned int def_value;
};

int ubuntu_i2c_init(i2c_parameter* ip, char* dev, unsigned int i2c_slave_addr, unsigned int i2c_addr_bits);
int ubuntu_i2c_send(i2c_parameter* ip, unsigned int reg_addr, unsigned int data);
int ubuntu_i2c_recv(i2c_parameter* ip, unsigned int reg_addr, unsigned int* data);
#endif
