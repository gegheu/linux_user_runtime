#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "comm/can/can.h"
#include "queue/queue.h"
#include "log/log.h"
#include "comm/can/can_data_covenant/can_data_covenant.h"
#if  OS_UBUNTU
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <pthread.h>
#include <fcntl.h>

#define CMD_CAN_PARAMS_LENGTH  128
#define FRAME_DATA_NUM         8

void ubuntu_can_init(can_parameter* cp)
{
	int ret = 0;
	char *strdes = NULL;
	struct ifreq ifr;
	struct sockaddr_can addr;

        if (pointer_check(cp)) {
                loge("ubuntu_can_init fail, cp is null, please check cp\n");
                return;
        }
	cp->fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (cp->fd < 0) {
		loge("ubuntu_can_init fail, socket fail, return ret.%d\n", ret);
		return;
	}
	strdes = strcpy(ifr.ifr_name, cp->dev_name);
	if (strdes == NULL) {
		loge("ubuntu_can_init fail, strcpy fail, return ret.%d\n", ret);
		close(cp->fd);
		cp->fd = 0;
		return;
	}
	ret = ioctl(cp->fd, SIOCGIFINDEX, &ifr);
	if (ret < 0) {
		loge("ubuntu_can_init fail, ioctl fail, return ret.%d\n", ret);
		close(cp->fd);
		cp->fd = 0;
		return;
	}
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	ret = bind(cp->fd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		loge("ubuntu_can_init fail, bind fail, return ret.%d\n", ret);
		close(cp->fd);
		cp->fd = 0;
		return;
	}
	logi("ubuntu_can_init sucess\n");
        return;
}

void ubuntu_set_can_params(can_parameter* cp)
{
	int ret;
	char cmd_can_params[CMD_CAN_PARAMS_LENGTH];

        if (pointer_check(cp)) {
                loge("ubuntu_set_can_params fail, cp is null, please check cp\n");
                return;
        }
        bzero(cmd_can_params, CMD_CAN_PARAMS_LENGTH); //string.h
        ret = sprintf(cmd_can_params, "ifconfig %s down", cp->dev_name);
        if (ret < 0) {
                loge("ubuntu_set_can_params fail, sprintf fail, return ret.%d\n", ret);
                return;
        }
        logi("%s\n", cmd_can_params);
        ret = system(cmd_can_params);
        if (ret != 0) {
                loge("ubuntu_set_can_params fail, system fail, cmd:%s return ret.%d\n", cmd_can_params, ret);
                return;
        }
	bzero(cmd_can_params, CMD_CAN_PARAMS_LENGTH); //string.h
	ret = sprintf(cmd_can_params, "/mnt/iproute2-5.9.0/ip/ip link set %s up type can bitrate %d triple-sampling on", cp->dev_name, cp->speed);//stdio.h
	if (ret < 0) {
		loge("ubuntu_set_can_params fail, sprintf fail, return ret.%d\n", ret);
		return;
	}
	logi("%s\n", cmd_can_params);
	ret = system(cmd_can_params);
	if (ret != 0) {
		loge("ubuntu_set_can_params fail, system fail, cmd:%s return ret.%d\n", cmd_can_params, ret);
		return;
	}
	bzero(cmd_can_params, CMD_CAN_PARAMS_LENGTH); //string.h
	sprintf(cmd_can_params, "ifconfig %s up", cp->dev_name);
        if (ret < 0) {
                loge("ubuntu_set_can_params fail, sprintf fail, return ret.%d\n", ret);
                return;
        }
	logi("%s\n", cmd_can_params);
        ret = system(cmd_can_params);
        if (ret != 0) {
                loge("ubuntu_set_can_params fail, system fail, cmd:%s return ret.%d\n", cmd_can_params, ret);
                return;
        }
	logi("ubuntu_set_can_params sucess\n");
	return;
}

void ubuntu_can_set_filter(can_parameter* cp)
{
	return;
}

void ubuntu_can_write_data(can_parameter* cp, canframe* cf)
{
	ssize_t cnt_bytes = 0;
	ssize_t total_bytes = 0;

        if (pointer_check(cp) || pointer_check(cf)) {
                loge("ubuntu_can_write_data fail, cp or cf  is null, please check cp or cf\n");
                return;
        }
	while (total_bytes != sizeof(canframe)) {
		cnt_bytes = write(cp->fd, cf, sizeof(canframe));
		if (cnt_bytes > 0) {
			total_bytes = total_bytes + cnt_bytes;
		}
		logi("total_bytes.%d \n", total_bytes);
	}
	total_bytes = 0;
	logi("ubuntu_can_write_data sucess\n");
	return;
}

void ubuntu_can_multiframe_write(can_parameter* cp, unsigned char id, unsigned char* data, unsigned int length)
{
	unsigned int index = 0;
	unsigned int i;
	canframe cf;

        if (pointer_check(cp) || pointer_check(data)) {
                loge("ubuntu_can_multiframe_write fail, cp or data  is null, please check cp or data\n");
                return;
        }
	cf.can_id = id;
	cf.can_dlc = FRAME_DATA_NUM;
	for(index = 0; index < length/FRAME_DATA_NUM + 1; index++) {
		for(i = 0; i < FRAME_DATA_NUM; i++) {
			cf.data[i] = data[index * FRAME_DATA_NUM + i];
		}
		ubuntu_can_write_data(cp, &cf);
	}
	cf.can_dlc = length % FRAME_DATA_NUM;
	for (i = 0; i < FRAME_DATA_NUM; i++) {
		if (i < cf.can_dlc) {
			cf.data[i] = data[(length/FRAME_DATA_NUM + 1) * FRAME_DATA_NUM + i];
		} else {
			cf.data[i] = 0;
		}
	}
	ubuntu_can_write_data(cp, &cf);
	return;

}
void ubuntu_can_read_data(can_parameter* cp, canframe* cf)
{
        ssize_t cnt_bytes = 0;
	ssize_t total_bytes = 0;

        if (pointer_check(cp) || pointer_check(cf)) {
                loge("ubuntu_can_read_data fail, cp or cf  is null, please check cp or cf\n");
                return;
        }
	while (total_bytes != sizeof(canframe)) {
		cnt_bytes = read(cp->fd, cf, sizeof(canframe));
		if (cnt_bytes > 0) {
			total_bytes = total_bytes + cnt_bytes;
		}
		logi("total_bytes.%d \n", total_bytes);
	}
	total_bytes = 0;
        logi("ubuntu_read_data sucess\n");
	return;
}

void ubuntu_can_deinit(can_parameter* cp)
{
	close(cp->fd);
	return;
}

void ubuntu_can_frame_info_show(canframe* cf)
{
	logi("ID:%X, DLC:%d, data:%x %x %x %x %x %x %x %x \n",
			cf->can_id,
			cf->can_dlc,
			cf->data[0],
			cf->data[1],
			cf->data[2],
			cf->data[3],
			cf->data[4],
			cf->data[5],
			cf->data[6],
			cf->data[7]);
	return;
}

void *ubuntu_can_pthread_handle(void * arg)
{
	can_parameter cp = *(can_parameter *)arg;
	canframe cf;

	ubuntu_can_init(&cp);
	ubuntu_set_can_params(&cp);
	ubuntu_can_set_filter(&cp);

	while (1) {
		ubuntu_can_read_data(&cp, &cf);
		logi("ubuntu can read data done\n");
		ubuntu_can_frame_info_show(&cf);
		can_frame_valid_check(cf.data, cf.can_dlc);
		sleep(1);
	}
}

void test_ubuntu_can(void)
{
	int ret = 0;
	pthread_t can_p;
	can_parameter cp;

	cp.dev_name = "can0";
	cp.speed = 100000;


	ret = pthread_create((pthread_t *)&can_p, NULL, ubuntu_can_pthread_handle, (can_parameter *)&cp);
	if (ret < 0) {
		loge("test_can pthread_create fail, ret.%x\n",ret);
		return;
	}
	logi("test can start ...\n");
	return;
}
#endif
