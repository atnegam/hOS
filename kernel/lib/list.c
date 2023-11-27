#include "list.h"
#include "interrupt.h"


//init
void list_init(struct list* cur_list){
    cur_list->head.next = &cur_list->tail;
    cur_list->head.pre = NULL;
    cur_list->tail.pre = &cur_list->head;
    cur_list->tail.next = NULL;
}

//node节点之前插入cur_node
void list_insert(struct list_node* node, struct list_node* cur_node){
    enum int_state state = int_disable(); //暂时用关中断来同步
    /*
        1.断被插节点的前节点指向被插节点的链
        2.断被插节点指向前节点的链
    */
    node->pre->next = cur_node;
    cur_node->pre = node->pre;
    node->pre = cur_node;
    cur_node->next = node;
    int_set_state(state);
}

//头插
void head_insert(struct list* cur_list, struct list_node* cur_node){
    list_insert(cur_list->head.next, cur_node);
}

//尾插
void tail_insert(struct list* cur_list, struct list_node* cur_node){
    list_insert(&cur_list->tail, cur_node);
}

//删除节点
void list_delete(struct list_node* cur_node){
    enum int_state state = int_disable();
    cur_node->pre->next = cur_node->next;
    cur_node->next->pre = cur_node->pre;
    int_set_state(state);
}

//pop头元素
struct list_node* list_pop(struct list* cur_list){
    struct list_node* cur_node = cur_list->head.next;
    list_delete(cur_node);
    return cur_node;
}

//查找节点
bool list_find(struct list* cur_list, struct list_node* node){
    struct list_node* cur_node = cur_list->head.next;
    while(cur_node != &cur_list->tail){
        if(node == cur_node) return true;
        cur_node = cur_node->next;
    }
    return false;
}

//计算链表长度
uint32_t list_len(struct list* cur_list){
    int len = 0;
    struct list_node* cur_node = cur_list->head.next;
    while(cur_node != &cur_list->tail){
        len++;
        cur_node = cur_node->next;
        
    }
    return len;
}

//判空
bool list_empty(struct list* cur_list){
    return list_len(cur_list) > 0? false : true;
}

//查找符合条件的节点
struct list_node* find_condition(struct list* cur_list, func fun, int arg){
    if(!list_empty(cur_list)) return NULL;

    struct list_node* cur_node = cur_list->head.next;
    while(cur_node != &cur_list->tail){
        if(fun(cur_node, arg)) return cur_node;
        cur_node = cur_node->next;
    }
    return NULL;
}