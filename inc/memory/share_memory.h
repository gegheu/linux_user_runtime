#ifndef __SHARE_MEMORY__
#define __SHARE_MEMORY__

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define RET_ERROR  -1
#define RET_OK     0
#define PERM_RWX 0666

typedef struct share_memory_parameter {
        const char* file_path;
        int id;
        size_t size;
        key_t key;
        int shm_attr;
        int shmflag;
        int shmid;
        char* shm_addr;
} share_memory_parameter;

void* share_memory_malloc(share_memory_parameter* shmp);
void share_memory_free(const void* shm_addr, int shmid);

#endif
