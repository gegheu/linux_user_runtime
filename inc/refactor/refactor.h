#ifndef __REFACTOR__
#define __REFACTOR__

#include <stdio.h>

extern volatile bool refa_flag;
extern volatile bool image_recv_done;
void *refactor_pthread_handle(void * arg);

#endif
