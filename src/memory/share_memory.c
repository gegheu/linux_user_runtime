#include <stdio.h>
#include "log/log.h"
#include "queue/queue.h"
#include "memory/share_memory.h"

void* share_memory_malloc(share_memory_parameter* shmp) 
{

        if (pointer_check(shmp)) {
                loge("share_memory_malloc fail, shmp is null, please check shmp\n");
                return NULL;
        }
	shmp->key = ftok(shmp->file_path, shmp->id);
	if (RET_ERROR == shmp->key) {
		loge("ftok fail, ret key.%d\n", shmp->key);
		return NULL;
	}
	shmp->shm_attr = IPC_CREAT | IPC_EXCL | PERM_RWX;
	shmp->shmid = shmget(shmp->key, shmp->size, shmp->shm_attr);
	if (RET_ERROR == shmp->shmid) {
                logi("shmget fail, may share memeroy already creat shmp->shm_attr.0x%x\n",shmp->shm_attr);
                shmp->shm_attr = IPC_CREAT | PERM_RWX;
                shmp->shmid = shmget(shmp->key, shmp->size, shmp->shm_attr);
                if (RET_ERROR == shmp->shmid) {
                        loge("shmget fail, shmget fail shmp->shm_attr.0x%x\n",shmp->shm_attr);
                        return NULL;
		}
	}
	shmp->shm_addr = shmat(shmp->shmid, NULL, shmp->shmflag);
	if ((void *)RET_ERROR == shmp->shm_addr) {
		loge("shmat fail, please ckeeck error\n");
		return NULL;
	}
	logi("share memory malloc sucess, shm_addr.%p\n",shmp->shm_addr);
	return shmp->shm_addr;
}

void share_memory_free(const void* shm_addr, int shmid)
{
	int ret = RET_ERROR;

	ret = shmdt(shm_addr);
	if (RET_ERROR == ret) {
		loge("share_memory_free fail, shmdt fail ret.%d\n", ret);
		return;
	}
	ret = shmctl(shmid, IPC_RMID, NULL);
	if (RET_ERROR == ret) {
		loge("share_memory_free fail, shmctl fail ret.%d\n", ret);
		return;
	}
	logi("share memory free sucess\n");
	return;
}
