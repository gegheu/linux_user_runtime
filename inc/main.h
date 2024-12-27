#ifndef __MAIN__
#define __MAIN__

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct pthreadcreatparameter
{
	pthread_t *tidp;
	const pthread_attr_t *attr;
	void *(*start_rtn)(void*);
	void *arg;
} pthreadcreatparameter;

#endif
