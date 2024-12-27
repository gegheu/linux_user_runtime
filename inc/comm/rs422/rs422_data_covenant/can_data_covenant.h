#ifndef __RS422_DATA_COVENANT__
#define __RS422_DATA_COVENANT__

#include <stdio.h>

bool uart_frame_valid_check(unsigned char* data, unsigned short len);

#endif
