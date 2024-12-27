#ifndef __SEMAPHORE__
#define __SEMAPHORE__

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>

union semum {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
};

typedef struct semaphore_parameter {
        const char* file_path;
        int id;
        int sem_members;
        key_t key;
        int attr;
        int sid;
}semaphore_parameter;

void create_sem(semaphore_parameter* sp, int init_val);
int sem_p(semaphore_parameter* sp, int index);
int sem_v(semaphore_parameter* sp, int index);
int delete_sem(semaphore_parameter* sp);
int wait_sem(semaphore_parameter* sp, int index);
#endif
