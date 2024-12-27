#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "memory/share_memory.h"
#include "comm/rs422/ubuntu/ubuntu_rs422.h"
#include "driver/i2c/ubuntu_i2c.h"
#include "queue/queue.h"
#include "log/log.h"
#if 1
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

unsigned char i2c_send_buf[I2C_SEND_MAX_LEN];
unsigned char i2c_recv_buf[I2C_RECV_MAX_LEN];

int ubuntu_i2c_init(i2c_parameter* ip, char* dev, unsigned int i2c_slave_addr, unsigned int i2c_addr_bits)
{
	char dev_name[I2C_DEV_LENGTH];
	int dev_length;

        if (pointer_check(ip) || pointer_check(dev)) {
                loge("ubuntu_i2c_init fail, ip or dev is null, please check ip or dev\n");
                return RET_ERROR;
        }
	dev_length = strlen(dev);
	if (dev_length > I2C_DEV_LENGTH) {
		loge("ubuntu_i2c_init fail, dev name length over, please cleck length\n");
		return RET_ERROR;
	}
	bzero(dev_name, I2C_DEV_LENGTH);
	ip->slave_addr = i2c_slave_addr;
	ip->slave_addr_bits = i2c_addr_bits;
	memcpy(ip->dev, dev, I2C_DEV_LENGTH);
	return RET_OK;
}

int ubuntu_i2c_send(i2c_parameter* ip, unsigned int reg_addr, unsigned int data)
{
	int ret = RET_OK;
	struct i2c_rdwr_ioctl_data cmd;
	struct i2c_msg msg;

	memset(&msg, 0, sizeof(struct i2c_msg));
	unsigned char reg_addr_msb = (reg_addr & MSB_VALUE_GET) >> CHAR_BITS_NUM;
	unsigned char reg_addr_lsb = (reg_addr & LSB_VALUE_GET);
	unsigned char send_data[2] = { ((data & MSB_VALUE_GET) >> CHAR_BITS_NUM), (data & LSB_VALUE_GET)};

        if (pointer_check(ip)) {
                loge("ubuntu_i2c_send fail, ip is null, please check ip\n");
                return RET_ERROR;
        }
	ip->fd = open(ip->dev, O_RDWR);
	if (ip->fd < 0) {
		loge("ubuntu_i2c_send fail, open i2c fail\n");
		return RET_ERROR;
	}
	ret = ioctl(ip->fd, I2C_SLAVE, ip->slave_addr);
	if (ret < 0) {
		loge("ubuntu_i2c_send fail, ioctl I2C_SLAVE cmd fail\n");
		close(ip->fd);
		return RET_ERROR;
	}
	cmd.nmsgs = 1;
	cmd.msgs = &msg;

	if (ip->slave_addr_bits == I2C_ADDR_BITS_NUM) {
		memset(i2c_send_buf, 0, sizeof(i2c_send_buf));
		i2c_send_buf[0] = reg_addr_msb;
		i2c_send_buf[1] = reg_addr_lsb;
		memcpy(i2c_send_buf + 2, send_data, 2);
		msg.buf = i2c_send_buf;
		msg.len = 4;
		msg.flags = 0;
		msg.addr = ip->slave_addr;
	
		ret = ioctl(ip->fd, I2C_RDWR, &cmd);
		if (ret < 0) {
			loge("ubuntu_i2c_send fail, ioctl I2C_RDWR cmd fail\n");
			close(ip->fd);
			return RET_ERROR;
		}
	} else if (ip->slave_addr_bits == I2C_ADDR_BITS_NUM_MAX) {
		logi("ubuntu_i2c_send ip->slave_addr_bits.%d\n", ip->slave_addr_bits);
	}
	close(ip->fd);
	return RET_OK;
}

int ubuntu_i2c_recv(i2c_parameter* ip, unsigned int reg_addr, unsigned int* data)
{
        int ret = RET_OK;
        struct i2c_rdwr_ioctl_data cmd;
        struct i2c_msg msg[2];

        memset(msg, 0, sizeof(msg));
        unsigned char reg_addr_msb = (reg_addr & MSB_VALUE_GET) >> CHAR_BITS_NUM;
        unsigned char reg_addr_lsb = (reg_addr & LSB_VALUE_GET);

        if (pointer_check(ip)) {
                loge("ubuntu_i2c_recv fail, ip is null, please check ip\n");
                return RET_ERROR;
        }
        ip->fd = open(ip->dev, O_RDWR);
        if (ip->fd < 0) {
                loge("ubuntu_i2c_recv fail, open i2c fail\n");
                return RET_ERROR;
        }
        ret = ioctl(ip->fd, I2C_SLAVE, ip->slave_addr);
        if (ret < 0) {
                loge("ubuntu_i2c_recv fail, ioctl I2C_SLAVE cmd fail\n");
                close(ip->fd);
                return RET_ERROR;
        }
        cmd.nmsgs = 2;
        cmd.msgs = msg;

        if (ip->slave_addr_bits == I2C_ADDR_BITS_NUM) {
                memset(i2c_send_buf, 0, sizeof(i2c_send_buf));
                i2c_send_buf[0] = reg_addr_msb;
                i2c_send_buf[1] = reg_addr_lsb;

                msg[0].buf = i2c_send_buf;
                msg[0].len = 2;
                msg[0].flags = 0;
                msg[0].addr = ip->slave_addr;

		memset(i2c_recv_buf, 0, sizeof(i2c_recv_buf));

                msg[1].buf = i2c_recv_buf;
                msg[1].len = 2;
                msg[1].flags = 1;
                msg[1].addr = ip->slave_addr;

                ret = ioctl(ip->fd, I2C_RDWR, &cmd);
                if (ret < 0) {
                        loge("ubuntu_i2c_recv fail, ioctl I2C_RDWR cmd fail\n");
                        close(ip->fd);
                        return RET_ERROR;
                }
        } else if (ip->slave_addr_bits == I2C_ADDR_BITS_NUM_MAX) {
                logi("ubuntu_i2c_recv ip->slave_addr_bits.%d\n", ip->slave_addr_bits);
        }
	*data = i2c_recv_buf[0] << CHAR_BITS_NUM | i2c_recv_buf[1];
        close(ip->fd);
        return RET_OK;
}
#endif
