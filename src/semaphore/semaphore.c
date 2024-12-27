#include  <stdio.h>
#include "log/log.h"
#include "queue/queue.h"
#include "memory/share_memory.h"
#include "semaphore/semaphore.h"

void create_sem(semaphore_parameter* sp, int init_val)
{
	int index;
	union semum semopts;
	int ret = RET_ERROR;

        if (pointer_check(sp)) {
                loge("create semaphore fail, sp is null, please check sp\n");
                return;
        }
        sp->key = ftok(sp->file_path, sp->id);
        if (RET_ERROR == sp->key) {
                loge("ftok fail, ret key.%d\n", sp->key);
                return;
        }
        sp->attr = IPC_CREAT | IPC_EXCL | PERM_RWX;
        sp->sid = semget(sp->key, sp->sem_members, sp->attr);
        if (RET_ERROR == sp->sid) {
                logi("semget fail, may semaphore group already create sp->attr.%d\n",sp->attr);
                sp->attr = IPC_CREAT | PERM_RWX;
                sp->sid = semget(sp->key, sp->sem_members, sp->attr);
                if (RET_ERROR == sp->sid) {
                        loge("create_sem fail, semget fail sp->attr.%d\n",sp->attr);
                        return;
                }
		logi("semaphore group already create\n");
		return;
        }
	semopts.val=init_val;
	for (index = 0; index < sp->sem_members; index++) {
		ret = semctl(sp->sid, index, SETVAL, semopts);
		if (RET_ERROR == ret) {
			loge("create_sem fail, semctl fail, set val error, semaphore index.%d\n", index);
			continue;
		}
	}
	logi("semaphore group create sucess\n");
	return;
}

int sem_p(semaphore_parameter* sp, int index)
{
	struct sembuf buf = { 0, -1, IPC_NOWAIT };
	int ret = RET_ERROR;

        if (pointer_check(sp)) {
                loge("sem_p fail, sp is null, please check sp\n");
                return ret;
        }
	if (sp->sem_members < index || index < 0) {
		loge("sem_p fail, index out off,please check sp->sem_members.%d index.%d\n", sp->sem_members, index);
		return ret;
	}
	buf.sem_num = index;
	ret = semop(sp->sid, &buf, 1);
	if (RET_ERROR == ret) {
		loge("sem_p fail, semop fail, semaphore pose fail\n");
		return ret;
	}
	logi("semaphore pose sucess\n");
	return RET_OK; 
}

int sem_v(semaphore_parameter* sp, int index)
{
        struct sembuf buf = { 0, +1, IPC_NOWAIT };
        int ret = RET_ERROR;

        if (pointer_check(sp)) {
                loge("sem_v fail, sp is null, please check sp\n");
                return ret;
        }
        if (sp->sem_members < index || index < 0) {
                loge("sem_v fail, index out off,please check sp->sem_members.%d index.%d\n", sp->sem_members, index);
                return ret;
        }
        buf.sem_num = index;
        ret = semop(sp->sid, &buf, 1);
        if (RET_ERROR == ret) {
                loge("sem_v fail, semop fail, semaphore v fail\n");
                return ret;
        }
	logi("semaphore v sucess\n");
        return RET_OK;
}

int delete_sem(semaphore_parameter* sp)
{
	int ret = RET_ERROR;

        if (pointer_check(sp)) {
                loge("delete_sem fail, sp is null, please check sp\n");
                return ret;
        }
	ret = semctl(sp->sid, 0, IPC_RMID);
	if (RET_ERROR == ret) {
		loge("delete_sem fail\n");
		return ret;
	}
	logi("delete semaphore sucess\n");
	return RET_OK;
}

int wait_sem(semaphore_parameter* sp, int index)
{
        int ret = RET_ERROR;

        if (pointer_check(sp)) {
                loge("wait_sem fail, sp is null, please check sp\n");
                return ret;
        }
        if (sp->sem_members < index || index < 0) {
                loge("wait_sem fail, index out off,please check sp->sem_members.%d index.%d\n", sp->sem_members, index);
                return ret;
        }
	while (semctl(sp->sid,index, GETVAL) == 0) {
		usleep(1000);
	}
	logi("wait semaphore sucess\n");
	return RET_OK;
}
