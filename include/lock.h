#ifndef LOCK__H
#define LOCK__H
#include "list.h"
#include "stdint.h"
#include "thread.h"

//信号量
struct semaphore{
    uint32_t value;  
    struct list wait_list; //等待队列
};

//锁
struct lock{
    struct task_struct* owner; //锁拥有者
    struct semaphore sme;  //二元信号量
    uint32_t req_cnt;   //拥有者请求次数
};


void lock_init(struct lock* lock);

void lock_get(struct lock* lock);

void lock_release(struct lock* lock);

#endif