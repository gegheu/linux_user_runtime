#ifndef __COMM__
#define __COMM__

#include <stdio.h>

extern volatile bool comm_flag;
extern volatile bool message_come_flag;
void *comm_pthread_handle(void * arg);

#endif
