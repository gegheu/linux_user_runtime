#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "queue/queue.h"
#include "log/log.h"
#include "comm/can/can.h"
#include "comm/can/ubuntu/can_ubuntu.h"
#include "comm/rs422/ubuntu/ubuntu_rs422.h"
#include "comm/can/can_data_covenant/can_data_covenant.h"

#define TMP_BUF_LENGTH    16
#define COUNT_FRAME_DATA_NUM  6
#define TEMP_FRAME_DATA_NUM   5
#define CMD_PROC_FRAME_DATA_NUM   3

#pragma pack(1)
struct can_frame_covenant { 
	unsigned char  head;
	unsigned char  cmd;
	unsigned short v7_voltage;
	unsigned short voltage_810A1;
	unsigned short voltage_810A2;
	unsigned short voltage_810A3;
	unsigned short voltage_810A4;
	unsigned short frame_total_num;
	unsigned char  crc;
	unsigned char  tail;
};

struct can_frame_voltage_covenant {
        unsigned char  head;
        unsigned char  cmd;
        unsigned short v7_voltage;
        unsigned short voltage_810A1;
        unsigned short voltage_810A2;
        unsigned short voltage_810A3;
        unsigned short voltage_810A4;
        unsigned char  crc;
        unsigned char  tail;
};
#pragma pack()

extern rs422_parameter rp;

bool can_frame_valid_check(unsigned char *data, int length)
{
	int ret = 0;

        if (pointer_check(data)) {
                loge("can_frame_valid_check fail, data is null, please check data\n");
                return false;
        }
	if(data[0] == FRAME_HEAD  && data[2] == FRAME_TAIL) {
		ret = ubuntu_rs422_write_data(&rp, data, length);
		if (ret < 0) {
			loge("can_frame_valid_check ok, ubuntu_rs422_write_data fail\n");
			return true;//todo frame vaild ok
		}
		return true;
	} else {
		return false; 
	}
}

void can_cmd_send(can_parameter* cp, unsigned char* data, enum CAN_CMD cmd)
{
	canframe cf;
	struct can_frame_covenant *cfc;
	struct can_frame_voltage_covenant cfvc;

        if (pointer_check(data) || pointer_check(cp)) {
                loge("can_cmd_send fail, data or cp is null, please check data or cp\n");
                return;
        }
	cfc = (struct can_frame_covenant *)data;
	cf.can_id = 0x01; //todo macro
	cf.data[0] = FRAME_HEAD;
	cf.data[1] = cmd;
	cfvc.head = FRAME_HEAD;
	cfvc.cmd  = cmd;
	logi("can_cmd_send cmd.%d\n",cmd);
/* todo check endian big or little change follow code*/
	switch (cmd) {
		case VOLTAGE_VAILD :
			cfvc.v7_voltage    = ntohs(cfc->v7_voltage);
			cfvc.voltage_810A1 = ntohs(cfc->voltage_810A1);
			cfvc.voltage_810A2 = ntohs(cfc->voltage_810A2);
			cfvc.voltage_810A3 = ntohs(cfc->voltage_810A3);
			cfvc.voltage_810A4 = ntohs(cfc->voltage_810A4);
			cfvc.crc = cfc->crc;
			cfvc.tail = FRAME_TAIL;
			ubuntu_can_multiframe_write(cp, cf.can_id, (unsigned char *)&cfvc, sizeof(struct can_frame_voltage_covenant));
			break;
		case FRAME_COUNT_VAILD :
			cf.can_dlc = COUNT_FRAME_DATA_NUM;
			*(unsigned short *)&cf.data[2] = ntohs(cfc->frame_total_num);
			cf.data[4] = cfc->crc;
			cf.data[5] = FRAME_TAIL;
			ubuntu_can_write_data(cp, &cf);
			break;
		case TEMP_INQUIRY :
			cf.can_dlc = TEMP_FRAME_DATA_NUM;
			*(unsigned short *)&cf.data[2] = ntohs(*(unsigned short *)&data[2]);
			cf.data[4] = cfc->crc;
			cf.data[5] = FRAME_TAIL;
			ubuntu_can_write_data(cp, &cf);
			break;
		case CMD_VAILD :
		case PROC_WRITE_DONE :
			cf.can_dlc = CMD_PROC_FRAME_DATA_NUM;
			cf.data[2] = FRAME_TAIL;
                        ubuntu_can_write_data(cp, &cf);
                        break;
		default:
			loge("can_cmd_send cmd.%d not exit\n", cmd);
			return;
	}
	return;
}
