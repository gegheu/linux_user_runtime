#include <stdio.h>
#include "log/log.h"
#include "main.h"
#include "refactor/refactor.h"

void *refactor_pthread_handle(void * arg)
{
	logi("refactor pthread create!\n");
	while(refa_flag) {
		if (image_recv_done) {
		    logi("refactor start \n");
		    image_recv_done = false;
		}
	}
	return NULL;
}

