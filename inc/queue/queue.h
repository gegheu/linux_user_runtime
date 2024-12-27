#ifndef __QUEUE__
#define __QUEUE__

#include <stdio.h>
#include <stdbool.h>

typedef int  queue_datatype;

typedef struct queue_node {
        queue_datatype val;
        struct queue_node* next;
}queue_node;

typedef struct queue {
        queue_node* head;
        queue_node* tail;
}queue;

bool pointer_check(void* p);
void* space_malloc(size_t size);
void queue_init(queue* pq);
void queue_destory(queue* pq);
void queue_push(queue* pq, queue_datatype qd);
void queue_pop(queue* pq);
void queue_front(queue* pq, queue_datatype* qd);
void queue_empty(queue* pq, bool* empty);
void queue_size(queue* pq, int* count);
#endif
