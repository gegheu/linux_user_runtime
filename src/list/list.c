#include <stdio.h>
#include <stdbool.h>
#include "log/log.h"
#include "memory/memory.h"
#include "list/list.h"
#include "queue/queue.h"

list_node* creat_list_node(list_datatype ldt)
{
	list_node* new_node = (list_node *)space_malloc(sizeof(list_node));
	if (NULL == new_node) {
		loge("creat_list_node fail, space_malloc error\n");
		return NULL;
	}
	new_node->data = ldt;
	new_node->next = NULL;
	new_node->prev = NULL;
	logi("list node creat sucess\n");
	return new_node;
}

list_node* list_init()
{
	list_node* phead = creat_list_node(0);
	if (NULL == phead) {
		loge("list_init fail, phead is null\n");
		return NULL;
	}
	phead->next = phead;
	phead->prev = phead;
	logi("list init sucess\n");
	return phead;
}

void list_print(list_node* phead)
{
        if (pointer_check(phead)) {
                loge("list_print fail, phead is null, please check phead\n");
                return;
        }
	list_node* cur = phead->next;
	while (cur != phead) {
		logi("data.%d\n", cur->data);
		cur = cur->next;
	}
	logi("list print sucess\n");
	return;
}

void list_insert_tail(list_node* phead, list_datatype ldt)
{
        if (pointer_check(phead)) {
                loge("list_insert_tail fail, phead is null, please check phead\n");
                return;
        }
	list_node* tail = phead->prev;
	list_node* new_node = creat_list_node(ldt);
        if (NULL == new_node) {
                loge("list_insert_tail fail, new_node is null\n");
		return;
        }
	tail->next = new_node;
	new_node->prev = tail;
	new_node->next = phead;
	phead->prev = new_node;
        logi("list insert tail sucess\n");
        return;
}

void list_delect_tail(list_node* phead)
{
        if (pointer_check(phead)) {
                loge("list_delect_tail fail, phead is null, please check phead\n");
                return;
        }
	if (phead->next == phead) {
		logi("list_delect_tail no node del\n");
		return;
	}
	list_node* tail = phead->prev;
	list_node* tail_prev = tail->prev;
	space_free(tail);
	tail_prev->next = phead;
	phead->prev = tail_prev;
	logi("list delect tail sucess\n");
	return;
}

void list_insert_head(list_node* phead, list_datatype ldt)
{
        if (pointer_check(phead)) {
                loge("list_insert_head fail, phead is null, please check phead\n");
                return;
        }
	list_node* new_node = creat_list_node(ldt);
        if (NULL == new_node) {
                loge("list_insert_head fail, new_node is null\n");
		return;
        }
	list_node* next = phead->next;
	phead->next = new_node;
	new_node->prev = phead;
	next->prev = new_node;
	new_node->next = next;
	logi("list insert head sucess\n");
	return;
}

void list_delect_head(list_node* phead)
{
        if (pointer_check(phead)) {
                loge("list_delect_head fail, phead is null, please check phead\n");
                return;
        }
	if(NULL == phead->next) {
		logi("list_delect_head no node del");
		return;
	}
	list_node* next = phead->next->next;
	space_free(phead->next);
	phead->next = next;
	next->prev = phead;
	logi("list delect head sucess\n");
	return;
}

list_node* list_find(list_node* phead, list_datatype ldt)
{
        if (pointer_check(phead)) {
                loge("list_find fail, phead is null, please check phead\n");
                return NULL;
        }
	list_node* cur = phead->next;
	while (cur != phead) {
		if (cur->data == ldt) {
			logi("list find node\n");
			return cur;
		}
		cur = cur->next;
	}
	logi("list not find node\n");
	return NULL;
}


void list_insert_pos(list_node* pos, list_datatype ldt)
{
        if (pointer_check(pos)) {
                loge("list_insert_pos fail, pos is null, please check pos\n");
                return;
        }
        list_node* new_node = creat_list_node(ldt);
        if (NULL == new_node) {
                loge("list_insert_pos fail, new_node is null\n");
                return;
        }
	list_node* pos_prev = pos->prev;
	pos_prev->next = new_node;
	new_node->prev = pos_prev;
	new_node->next = pos;
	pos->prev = new_node;
	logi("list insert pos sucess\n");
	return;
}

void list_delect_pos(list_node* pos)
{
        if (pointer_check(pos)) {
                loge("list_delect_pos fail, pos is null, please check pos\n");
                return;
        }
	list_node* prev = pos->prev;
	list_node* next = pos->next;
	space_free(pos);
	prev->next = next;
	next->prev = prev;
	logi("list delect pos sucess\n");
	return;
}

bool list_empty(list_node* phead)
{
        if (pointer_check(phead)) {
                loge("list_empty fail, phead is null, please check phead\n");
                return false;
        }
	logi("list empty sucess\n");
	return phead->next == phead;
}

void list_size(list_node* phead, size_t *size)
{
        if (pointer_check(phead)) {
                loge("list_size fail, phead is null, please check phead\n");
                return;
        }
	list_node* tail = phead->next;
	while (tail != phead) {
		*size = *size + 1;
		tail = tail->next;
	}
	logi("list size sucess\n");
	return;
}

void list_destroy(list_node* phead)
{
        if (pointer_check(phead)) {
                loge("list_delect_pos fail, phead is null, please check phead\n");
                return;
        }
	list_node* cur = phead->next;
	while (cur != phead) {
		list_node* next = cur->next;
		space_free(cur);
		cur = next;
	}
	space_free(phead);
}
