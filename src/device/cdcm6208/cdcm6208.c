#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "memory/share_memory.h"
#include "comm/rs422/ubuntu/ubuntu_rs422.h"
#include "driver/i2c/ubuntu_i2c.h"
#include "queue/queue.h"
#include "log/log.h"
#include "device/cdcm6208/cdcm6208.h"

int check_6208_config(i2c_parameter* ip, unsigned int* already_flag)
{
        int ret = 0, i;
        unsigned int value_reg21 = 0;
        unsigned int value_reg0 = 0;
        unsigned int value_reg5 = 0;
        unsigned int value_reg6 = 0;
        unsigned int value_reg7 = 0;
        struct i2c_addr_value iav[] = {
                {I2C_6208_REG21, value_reg21, 0x1},
                {I2C_6208_REG0, value_reg0,   0x1b1},
                {I2C_6208_REG5, value_reg5,   0x22},
                {I2C_6208_REG6, value_reg6,   0x09},
                {I2C_6208_REG7, value_reg7,   0x22},
        };

        if (pointer_check(ip)) {
                loge("check_6208_config fail, ip is null, please check ip\n");
                return RET_ERROR;
        }
        ret = ubuntu_i2c_init(ip, ip->dev, ip->slave_addr, ip->slave_addr_bits);
        if (ret < 0) {
                loge("check_6208_config fail, ubuntu_i2c_init fail\n");
                return RET_ERROR;
        }
        logi("6208 info : dev name:%s, slave_addr.%x\n", ip->dev, ip->slave_addr);
        for (i = 0; i < sizeof(iav)/sizeof(iav[0]); i++) {
                ret = ubuntu_i2c_recv(ip, iav[i].reg_addr, &iav[i].reg_value);
                if (ret < 0) {
                        loge("check_6208_config fail, ubuntu_i2c_recv fail\n");
                        return RET_ERROR;
                }
		logd("check_6208_config num.%d, i.%d, addr.%x, tmp.%x\n", sizeof(iav)/sizeof(iav[0]), i, iav[i].reg_addr, iav[i].reg_value);
		if (iav[i].reg_value != iav[i].def_value) {
                        *already_flag = 0;
                        return RET_OK;
                }
        }
        *already_flag = 1;
        return RET_OK;
}

int chip6208_set_clk_for_board(i2c_parameter* ip, rs422_parameter* rp)
{
        int ret = 0, i;
	unsigned int already_flag = 0;
        unsigned int value_reg21 = 0;
        unsigned int value_reg40 = 0;
        unsigned char data[3] = { 0xAA, 0xCC, 0xBB};

        struct i2c_addr_value write_iav[] = {
                {I2C_6208_REG3, 0x0091,   0x0091},
                {I2C_6208_REG4, 0x30af,   0x30af},
                {I2C_6208_REG0, 0x1b1,    0x1b1},
                {I2C_6208_REG1, 0x0,      0x0},
                {I2C_6208_REG2, 0x18,     0x18},
                {I2C_6208_REG5, 0x22,     0x22},
                {I2C_6208_REG6, 0x09,     0x09},
                {I2C_6208_REG7, 0x22,     0x22},
                {I2C_6208_REG8, 0x09,     0x09},
                {I2C_6208_REG9, 0x02,     0x02},
                {I2C_6208_REG10, 0x30,     0x30},
                {I2C_6208_REG11, 0x00,     0x00},
                {I2C_6208_REG12, 0x02,     0x02},
                {I2C_6208_REG13, 0x30,     0x30},
                {I2C_6208_REG14, 0x00,     0x00},
                {I2C_6208_REG15, 0x02,     0x02},
                {I2C_6208_REG16, 0x30,     0x30},
                {I2C_6208_REG17, 0x00,     0x00},
                {I2C_6208_REG18, 0x02,     0x02},
                {I2C_6208_REG19, 0x30,     0x30},
                {I2C_6208_REG20, 0x00,     0x00},
                {I2C_6208_REG21, 0x00,     0x00},
                {I2C_6208_REG40, 0x00,     0x00},
                {I2C_6208_REG3,  0x00f5,   0x00f5},
        };

        struct i2c_addr_value read_iav[] = {
                {I2C_6208_REG21, value_reg21, 0x00},
                {I2C_6208_REG40, value_reg40,  0x00},
        };

        if (pointer_check(ip)) {
                loge("chip6208_set_clk_for_board fail, ip is null, please check ip\n");
                return RET_ERROR;
        }
	ret = check_6208_config(ip, &already_flag);
        if (ret < 0) {
                loge("chip6208_set_clk_for_board fail, check_6208_config\n");
                return RET_ERROR;
        }
	logi("already_flag.%d\n", already_flag);
	if (already_flag == 1) {
		logi("6208 already config\n");
		return RET_OK;
	}
	logi("start config 6208....\n");
        for (i = 0; i < sizeof(write_iav)/sizeof(write_iav[0]); i++) {
                ret = ubuntu_i2c_send(ip, write_iav[i].reg_addr, write_iav[i].reg_value);
                if (ret < 0) {
                        loge("chip6208_set_clk_for_board fail, ubuntu_i2c_send fail\n");
                        return RET_ERROR;
                }
                usleep(100000);
        }
        for (i = 0; i < sizeof(read_iav)/sizeof(struct i2c_addr_value); i++) {
                ret = ubuntu_i2c_recv(ip, read_iav[i].reg_addr, &read_iav[i].reg_value);
                if (ret < 0) {
                        loge("chip6208_set_clk_for_board fail, ubuntu_i2c_recv fail\n");
                        return RET_ERROR;
                }
                logi("reg_addr.%x, reg_value.%x\n", read_iav[i].reg_addr, read_iav[i].reg_value);
        }
	logi("end config 6208....\n");
        ret = ubuntu_rs422_write_data(rp, data, sizeof(data)/sizeof(char));
        if (ret < 0) {
                loge("chip6208_set_clk_for_board fail, ubuntu_rs422_write_data fail\n");
                return RET_ERROR;
        }
        return RET_OK;
}
