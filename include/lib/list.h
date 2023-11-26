#ifndef LIST__H
#define LIST__H
#include "stdvar.h"
#include "stdint.h"

//链表节点
struct list_node{
    struct list_node* pre;
    struct list_node* next;
};

//链表
struct list{
    struct list_node head;
    struct list_node tail;
};

typedef bool (func)(struct list_node* node, int arg);

void list_init(struct list* cur_list);
void list_insert(struct list_node* node, struct list_node* cur_node);
void head_insert(struct list* cur_list, struct list_node* cur_node);
void list_delete(struct list_node* cur_node);
struct list_node* list_pop(struct list* cur_list);
bool list_find(struct list* cur_list, struct list_node* node);
uint32_t list_len(struct list* cur_list);
bool list_empty(struct list* cur_list);
struct list_node* find_condition(struct list* cur_list, func fun, int arg);

#endif