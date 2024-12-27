#ifndef __CAN_DATA_COVENANT__
#define __CAN_DATA_COVENANT__

#include <stdio.h>

#define FRAME_HEAD        0xaa
#define FRAME_TAIL        0xbb

enum CAN_CMD {
        CMD_VAILD         = 0x11,
        FRAME_COUNT_VAILD = 0x22,
        PROC_WRITE_DONE   = 0x33,
        TEMP_VAILD        = 0x44,
        VOLTAGE_VAILD     = 0x55,
        TEMP_INQUIRY      = 0x66,
};

bool can_frame_valid_check(unsigned char* data, int length);
void can_cmd_send(can_parameter* cp, unsigned char* data, enum CAN_CMD cmd);

#endif
