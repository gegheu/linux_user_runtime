#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "log/log.h"
#include "queue/queue.h"
#include "memory/memory.h"

bool pointer_check(void* p)
{
	if (NULL == p) {
		return true;
	} else {
		return false;
	}
}

void queue_init(queue* pq)
{
	if (pointer_check(pq)) {
		loge("queue_init fail, pq is null, please check pq\n");
		return;
	}
	pq->head = NULL;
	pq->tail = NULL;
	logi("queue init sucess\n");
	return;
}

void queue_destory(queue* pq)
{
	if (pointer_check(pq)) {
		loge("queue_destory fail, pq is null, please check pq\n");
                return;
        }
	queue_node* cur = pq->head;
	while (NULL != cur) {
		queue_node* next = cur->next;
		space_free(cur);
		cur = next;
	}
	pq->head = NULL;
	pq->tail = NULL;
	logi("queue destory sucess\n");
	return;
}

void queue_push(queue* pq, queue_datatype qd)
{
        if (pointer_check(pq)) {
                loge("queue_push fail, pq is null, please check pq\n");
                return;
        }
	queue_node* new_node = (queue_node *)space_malloc(sizeof(queue_node));
	if (NULL == new_node) {
		loge("queue_push fail,space malloc error\n");
		return;
	}
	new_node->val = qd;
	new_node->next = NULL;
	if (pq->tail == NULL) {
		if (true == pointer_check(pq->head)) {
			pq->head = new_node;
			pq->tail = new_node;
		} else {
			loge("queue_push fail pq->head != NULL\n");
			space_free(new_node);
			return;
		}
	} else {
		pq->tail->next = new_node;
		pq->tail = new_node;
	}
	logi("queue push sucess\n");
	return;
}

void queue_pop(queue* pq)
{
        if (pointer_check(pq)) {
                loge("queue_pop fail, pq is null, please check pq\n");
                return;
        }
	if ((NULL == pq->head) && (NULL == pq->tail)) {
		loge("queue_pop queue is empty, please check\n");
		return;
	}
	if (pq->head->next == NULL) {
		space_free(pq->head);
		pq->tail = NULL;
		pq->head = NULL;
	} else {
		queue_node* next = pq->head->next;
		space_free(pq->head);
		pq->head = next;
	}
	logi("queue pop sucess\n");
	return;
}

void queue_front(queue* pq, queue_datatype* qd)
{
        if (pointer_check(pq)) {
                loge("queue_front fail, pq is null, please check pq\n");
                return; //todo 
        }
        if (pointer_check(qd)) {
                loge("queue_front fail, qd is null, please check pq\n");
                return; //todo 
        }
        if ((NULL == pq->head) && (NULL == pq->tail)) {
                loge("queue_front queue is empty, please check\n");
                return; //todo
        }
	*qd = pq->head->val;
	logi("queue front sucess\n");
	return;

}

void queue_empty(queue* pq, bool* empty)
{
        if (pointer_check(pq)) {
                loge("queue_empty fail, pq is null, please check pq\n");
                return; //todo
        }
        if (pointer_check(empty)) {
                loge("queue_empty fail, empty is null, please check pq\n");
                return; //todo
        }
	if (NULL == pq->head) {
		*empty = true;
	} else {
		*empty = false;
	}
	logi("queue empty sucess\n");
	return;
}

void queue_size(queue* pq, int* count)
{

        if (pointer_check(pq)) {
                loge("queue_size fail, pq is null, please check pq\n");
                return; //todo
        }
        if (pointer_check(count)) {
                loge("queue_size fail, count is null, please check pq\n");
                return; //todo
        }
	queue_node* cur = pq->head;
	*count = 0;
	while (NULL != cur) {
		cur = cur->next;
		*count = *count + 1;
	}
	logi("queue size sucess\n");
	return;
}
