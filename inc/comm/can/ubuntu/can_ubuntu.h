#ifndef __CAN_UBUNTU__
#define __CAN_UBUNTU__

#include <stdio.h>
#include <sys/types.h>

#if OS_UBUNTU
void ubuntu_can_init(can_parameter* cp);
void ubuntu_set_can_params(can_parameter* cp);
void ubuntu_can_set_filter(can_parameter* cp);
void ubuntu_can_write_data(can_parameter* cp, canframe* cf);
void ubuntu_can_read_data(can_parameter* cp, canframe* cf);
void ubuntu_can_deinit(can_parameter* cp);
void ubuntu_can_frame_info_show(canframe* cf);
void *ubuntu_can_pthread_handle(void * arg);
void ubuntu_can_multiframe_write(can_parameter* cp, unsigned char id, unsigned char* data, unsigned int length);
void test_ubuntu_can(void);
#endif
#endif
