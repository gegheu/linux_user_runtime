#include <stdio.h>
#include<string.h>
#include <stdbool.h>
#include "queue/queue.h"
#include "log/log.h"
#include "comm/can/can.h"
#include "comm/can/can_data_covenant/can_data_covenant.h"

#define RS422_VALID_CODE_NUM    14
#define V7TO810_DATA_LENGTH     243
#define DATA_ROW_NUM            27
#define DATA_COL_NUM            9

extern can_parameter cp;

#pragma pack(push,1)
typedef struct
{
	unsigned short head;
	unsigned short length;
	unsigned char flag;
	unsigned short file_size;
	unsigned char data_size;
	unsigned char frame_count;
	unsigned char data[V7TO810_DATA_LENGTH];
	unsigned short sum;
	unsigned short tail;
} v7to810_data;
#pragma pack(pop)

bool uart_frame_valid_check(unsigned char* data, unsigned short len)
{
	unsigned char cmd_code = 0;
	unsigned char sum = 0;
	int i, j;

        if (pointer_check(data)) {
                loge("uart_frame_valid_check fail, data is null, please check data\n");
                return false;
        }
	if (data[0] == FRAME_HEAD && data[15] ==  FRAME_TAIL) {
		for (i = 1; i < RS422_VALID_CODE_NUM; i++) {
			sum += data[i];
		}
		if (data[RS422_VALID_CODE_NUM] == sum) {
			cmd_code = data[1];
			logi("recv cmd.%d\n", cmd_code);
			can_cmd_send(&cp, data, cmd_code);
			return true;
		}
	} else if (data[0] == FRAME_HEAD && data[1] == VOLTAGE_VAILD && data[len - 1] == 0x0a && data[len - 2] == 0x0d) {
		v7to810_data recv_buf;
		memcpy((unsigned char*)(&recv_buf), data, sizeof(v7to810_data));
		logi("head.%x length.%x flag.%x file_size.%x data_size.%x frame_count.%x\n", recv_buf.head, recv_buf.length, recv_buf.flag, recv_buf.file_size, recv_buf.data_size, recv_buf.frame_count);
		for (i = 0; i < DATA_ROW_NUM; i++) {
			for (j = 0; j < DATA_COL_NUM; j++) {
				logi("data %x", recv_buf.data[i * DATA_COL_NUM + j]);
			}
			logi("\n");
		}
		logi("sum %x tail %x\n", recv_buf.sum, recv_buf.tail);
		return true;
	}
	return false;
}	
