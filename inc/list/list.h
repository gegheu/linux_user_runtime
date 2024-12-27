#ifndef __LIST__
#define __LIST__

#include <stdio.h>

typedef int list_datatype;

typedef struct list_node {
        list_datatype data;
        struct list_node* next;
        struct list_node* prev;
}list_node;

list_node* creat_list_node(list_datatype ldt);
list_node* list_init();
void list_print(list_node* phead);
void list_insert_tail(list_node* phead, list_datatype ldt);
void list_delect_tail(list_node* phead);
void list_insert_head(list_node* phead, list_datatype ldt);
void list_delect_head(list_node* phead);
list_node* list_find(list_node* phead, list_datatype ldt);
void list_insert_pos(list_node* pos, list_datatype ldt);
void list_delect_pos(list_node* pos);
bool list_empty(list_node* phead);
void list_size(list_node* phead, size_t *size);
void list_destroy(list_node* phead);

#endif
