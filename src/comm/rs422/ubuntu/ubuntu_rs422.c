#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <comm/can/can.h>
#include "memory/share_memory.h"
#include "queue/queue.h"
#include "log/log.h"
#include "comm/rs422/ubuntu/ubuntu_rs422.h"
#include "comm/can/can_data_covenant/can_data_covenant.h"
#include "comm/rs422/rs422_data_covenant/can_data_covenant.h"
#if OS_UBUNTU
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>

#define RS422_DEV_PATH  "/dev/tty"
#define RS422_DEV_LENGTH  64
#define RS422_RECV_BUFFER_LENGTH 312
#define RS422_READ_BUFFR_LENGTH 16
#define RS422_DATA_LENGTH   256

int ubuntu_rs422_init(rs422_parameter* rp)
{
	int ret = RET_OK;
	char dev_name[RS422_DEV_LENGTH];
        if (pointer_check(rp)) {
                loge("ubuntu_rs422_init fail, rp is null, please check rp\n");
                return RET_ERROR;
        }
	bzero(dev_name, RS422_DEV_LENGTH);
        ret = sprintf(dev_name, "%s%s", rp->file_path, rp->dev_name);//stdio.h
        if (ret < 0) {
                loge("ubuntu_rs422_init fail, sprintf fail, return ret.%d\n", ret);
                return RET_ERROR;
        }
        rp->fd =  open(dev_name,  O_RDWR | O_NOCTTY | O_NDELAY);
        if (rp->fd < 0) {
                loge("ubuntu_rs422_init open %s error, error info:%s(%d)\n", dev_name, strerror(errno), errno);//string.h
                return RET_ERROR;
        }
	return ret;
}

int ubuntu_rs422_set_speed(rs422_parameter* rp)
{
	int ret = RET_OK;
	speed_t speed;
	struct termios tos = { 0 };

        if (pointer_check(rp)) {
                loge("ubuntu_rs422_set_speed fail, rp is null, please check rp\n");
                return RET_ERROR;
        }
	cfmakeraw(&tos);
	tos.c_cflag |= CREAD | CLOCAL;
	switch (rp->baudrate) {
		case 1200:
			speed = B1200;
			break;
		case 1800:
			speed = B1800;
			break;
		case 2400:
                        speed = B2400;
                        break;
		case 4800:
                        speed = B4800;
                        break;
		case 9600:
                        speed = B9600;
                        break;
		case 19200:
                        speed = B19200;
                        break;
		case 38400:
                        speed = B38400;
                        break;
		case 57600:
                        speed = B57600;
                        break;
		case 115200:
                        speed = B115200;
                        break;
		case 230400:
                        speed = B230400;
                        break;
		case 460800:
                        speed = B460800;
                        break;
		case 500000:
                        speed = B500000;
                        break;
		default:
			speed = B115200;
			logi("default baud rate:115200\n");
			break;
	}
	ret = cfsetispeed(&tos, speed);
	if (ret < 0) {
		logi("ubuntu_rs422_set_speed fail, cfsetispeed error:%s\n", strerror(errno));
		close(rp->fd);
		return RET_ERROR;
	}
        ret = tcsetattr(rp->fd, TCSANOW, &tos);
        if (ret < 0) {
                loge("ubuntu_rs422_set_speed fail, tcsetattr error:%s\n", strerror(errno));
                close(rp->fd);
                return RET_ERROR;
        }
	logi("ubuntu_rs422_set_speed sucess, baud rate.%d\n", rp->baudrate);
	return RET_OK;
}

int ubuntu_rs422_write_data(rs422_parameter* rp, unsigned char* data, int data_len)
{
	int len = 0;
	int total_len = 0;

        if (pointer_check(rp) || pointer_check(data)) {
                loge("ubuntu_rs422_write_data fail, rp is null, please check rp\n");
                return RET_ERROR;
        }
	while (total_len != data_len) {
		len = write(rp->fd, data, data_len);
		if (len > 0) {
			total_len = total_len + len;
		}
		logi("total_len.%d \n", total_len);
	}
	total_len = 0;
	logi("ubuntu_rs422_write_data sucess\n");
	return RET_OK;
}

int ubuntu_rs422_read_data(rs422_parameter* rp, unsigned char* data, int data_len)
{
	int len = 0;
	int total_len = 0;
	int i;

        if (pointer_check(rp) || pointer_check(data)) {
                loge("ubuntu_rs422_read_data fail, rp is null, please check rp\n");
                return RET_ERROR;
        }
	logi("ubuntu_rs422_read_data len.%d\n", len);
	while (total_len != data_len) {
		len = read(rp->fd, data, data_len);
                if (len > 0) {
                        total_len = total_len + len;
                }
                logi("total_len.%d \n", total_len);
	}
	total_len = 0;
	for (i = 0; i < data_len; i++) {
		logi("ubuntu_rs422_read_data data:0x%x", data[i]);
	}
	logi("ubuntu_rs422_read_data sucess\n");
	return RET_OK;
}

int ubuntu_rs422_set_parity(rs422_parameter* rp)
{
        int ret = RET_OK;
        struct termios tos = { 0 };

        if (pointer_check(rp)) {
                loge("ubuntu_rs422_set_speed fail, rp is null, please check rp\n");
                return RET_ERROR;
        }
	ret = tcgetattr(rp->fd, &tos);
	if (ret < 0) {
		logi("ubuntu_rs422_set_parity fail, tcgetattr error:%s\n", strerror(errno));
		close(rp->fd);
		return RET_ERROR;
	}
	switch (rp->parity) {
		case 'n':
		case 'N':
			tos.c_cflag &= ~PARENB;
			tos.c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O':
                        tos.c_cflag |= (PARODD | PARENB);
                        tos.c_iflag |= INPCK;
                        break;
		case 'e':
		case 'E':
                        tos.c_cflag |= PARENB;
			tos.c_cflag &= ~PARODD;
                        tos.c_iflag |= INPCK;
                        break;
		default:
                        tos.c_cflag &= ~PARENB;
                        tos.c_iflag &= ~INPCK;
			logi("ubuntu_rs422_set_parity default parity.%c\n", rp->parity);
                        break;
	}
	logi("ubuntu_rs422_set_parity parity.%c\n", rp->parity);
	ret = tcsetattr(rp->fd, TCSANOW, &tos);
	if (ret < 0) {
		loge("ubuntu_rs422_set_parity fail, tcsetattr error:%s\n", strerror(errno));
		close(rp->fd);
		return RET_ERROR;
	}
	logi("ubuntu_rs422_set_parity sucess, parity.%c\n", rp->parity);
	return RET_OK;
}

int ubuntu_rs422_set_parameter(rs422_parameter* rp)
{
        int ret = RET_OK;
        struct termios tos = { 0 };

        if (pointer_check(rp)) {
                loge("ubuntu_rs422_set_parameter fail, rp is null, please check rp\n");
                return RET_ERROR;
        }
        ret = tcgetattr(rp->fd, &tos);
        if (ret < 0) {
                loge("ubuntu_rs422_set_parameter fail, tcgetattr error:%s\n", strerror(errno));
                close(rp->fd);
                return RET_ERROR;
        }
	tos.c_cflag &= ~CSIZE;
	switch (rp->databits) {
		case FIVE:
			tos.c_cflag |= CS5;
			break;
		case SIX :
			tos.c_cflag |= CS6;
			break;
		case SEVEN:
			tos.c_cflag |= CS7;
			break;
		case EIGHT:
			tos.c_cflag |= CS8;
			break;
		default :
			tos.c_cflag |= CS8;
			logi("default data bit size : 8 \n");
			break;
	}
	switch (rp->stopbits) {
		case ONE_STOP_BIT:
			tos.c_cflag &= ~CSTOPB;
			break;
		case TWO_STOP_BIT:
			tos.c_cflag |= CSTOPB;
			break;
		default:
			tos.c_cflag &= ~CSTOPB;
			logi("default stop bit size: 1 \n");
			break;
	}
	ret = tcflush(rp->fd, TCIOFLUSH);
	if (ret < 0) {
		logi("ubuntu_rs422_set_parameter fail, tcflush error:%s\n", strerror(errno));
		close(rp->fd);
		return RET_ERROR;
	}
        ret = tcsetattr(rp->fd, TCSANOW, &tos);
        if (ret < 0) {
                logi("ubuntu_rs422_set_parameter fail, tcsetattr error:%s\n", strerror(errno));
                close(rp->fd);
                return RET_ERROR;
        }
	return RET_OK;
}

int ubuntu_rs422_deinit(rs422_parameter* rp)
{
	 int ret = RET_OK;

        if (pointer_check(rp)) {
                loge("ubuntu_rs422_set_parameter fail, rp is null, please check rp\n");
                return RET_ERROR;
        }
	close(rp->fd);
	return ret;
}

void *ubuntu_rs422_pthread_handle(void * arg)
{
	rs422_parameter rp = *(rs422_parameter *)arg;
	unsigned char recv_buf[RS422_RECV_BUFFER_LENGTH] = {0};
	unsigned char * p_buf = recv_buf;
	unsigned short rs422_len = RS422_DATA_LENGTH;
	int sum_len = 0;
	int tmp_len = 0;
	int i, ret = 0;
#if 0
	ubuntu_rs422_init(&rp);
	ubuntu_rs422_set_speed(&rp);
	ubuntu_rs422_set_parity(&rp);
	ubuntu_rs422_set_parameter(&rp);
#endif
	while (1) {
		tmp_len = read(rp.fd, p_buf, RS422_READ_BUFFR_LENGTH);
		sum_len += tmp_len;
		p_buf += tmp_len;
		if (sum_len > 0) {
			if (recv_buf[0] != FRAME_HEAD) {
				memset(recv_buf, 0, RS422_RECV_BUFFER_LENGTH);
				sum_len = 0;
				p_buf = recv_buf;
				tcflush(rp.fd, TCIFLUSH);
				continue;
			} else {
				if (recv_buf[sum_len - 1] == FRAME_TAIL) {
					rs422_len = sum_len;
					for (i = 0; i < sum_len; i++) {
						logd("data is : %x\n", recv_buf[i]);
					}
				} else {
					if (sum_len >= 4) {
						rs422_len = (recv_buf[2] << 8) + recv_buf[3];
						logd("rs422_len : %d\n", rs422_len);
					}
				}
			}
		} else {
			continue;
		}
		if (sum_len >= rs422_len) {
			logi("rs422_len.%d  sum_len.%d\n", rs422_len, sum_len);
			ret = uart_frame_valid_check(recv_buf, sum_len);
			if (ret == false) {
				loge("uart_frame_valid_check error\n");
			}
			sum_len = 0;
			rs422_len = 0;
			p_buf = recv_buf;
		}
#if 0
		ubuntu_rs422_read_data(&rp, recv_buf, 8);
		logi("ubuntu rs422 read data done\n");
		sleep(1);
		ubuntu_rs422_write_data(&rp, recv_buf, 8);
		logi("ubuntu rs422 write data done\n");
		sleep(1);
#endif
	}
}

void test_ubuntu_rs422(void)
{
        int ret = 0;
        pthread_t rs422_p;
        rs422_parameter rp;

        rp.file_path = "/dev/tty";
        rp.dev_name = "S2";
        rp.baudrate = 115200;
        rp.databits = CS8;
        rp.stopbits = TWO_STOP_BIT;
        rp.parity = 'e';


        ret = pthread_create((pthread_t *)&rs422_p, NULL, ubuntu_rs422_pthread_handle, (rs422_parameter *)&rp);
        if (ret < 0) {
                loge("test_can pthread_create fail, ret.%x\n",ret);
                return;
        }
        logi("test can start ...\n");
        return;
}

#endif
