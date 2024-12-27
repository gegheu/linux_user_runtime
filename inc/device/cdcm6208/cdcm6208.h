#ifndef __CDCM6208__
#define __CDCM6208__

#include <stdio.h>
#include "driver/i2c/ubuntu_i2c.h"
#include "comm/rs422/ubuntu/ubuntu_rs422.h"

#define I2C_SLAVE_6208_ADDR      (0x54)
#define I2C_SLAVE_1848_ADDR      (0x60)
/* 6208 dev info */
#define I2C_6208_REG0            0x0
#define I2C_6208_REG1            0x1
#define I2C_6208_REG2            0x2
#define I2C_6208_REG3            0x3
#define I2C_6208_REG4            0x4
#define I2C_6208_REG5            0x5
#define I2C_6208_REG6            0x6
#define I2C_6208_REG7            0x7
#define I2C_6208_REG8            0x8
#define I2C_6208_REG9            0x9
#define I2C_6208_REG10           0xA
#define I2C_6208_REG11           0xB
#define I2C_6208_REG12           0xC
#define I2C_6208_REG13           0xD
#define I2C_6208_REG14           0xE
#define I2C_6208_REG15           0xF
#define I2C_6208_REG16           0x10
#define I2C_6208_REG17           0x11
#define I2C_6208_REG18           0x12
#define I2C_6208_REG19           0x13
#define I2C_6208_REG20           0x14
#define I2C_6208_REG21           0x15
#define I2C_6208_REG40           0x28

int check_6208_config(i2c_parameter* ip, unsigned int* already_flag);
int chip6208_set_clk_for_board(i2c_parameter* ip, rs422_parameter* rp);
#endif
