#include "lock.h"
#include "thread.h"
#include "interrupt.h"
#include "list.h"
#include "debug.h"


//初始化信号量
void semaphore_init(struct semaphore* sme){
    sme->value = 1;
    list_init(&sme->wait_list);
}

//初始化锁
void lock_init(struct lock* lock){
    lock->owner = NULL;
    lock->req_cnt = 0;
    semaphore_init(&lock->sme);
}

//semaphore down操作
void semaphore_down(struct semaphore* sme){
    enum int_state state = int_disable();

    // put_str("downing...\n");
    
    while(sme->value == 0){
        // put_str("while...");
        struct task_struct* cur = cur_thread();
        ASSERT(!list_find(&sme->wait_list, &cur->ready_tag));
        // put_str("tail_insert start...\n");
        tail_insert(&sme->wait_list, &cur->ready_tag);
        // put_int(list_len(&sme->wait_list));
        // put_str("block start...");
        // put_int(list_len(&sme->wait_list));
        // put_char('\n');
        thread_block(WAITING); //自我阻塞
    }
    // put_str("sme->value--\n");
    sme->value--;  //总会执行到这
    ASSERT(sme->value == 0);
    int_set_state(state);
}

//semaphore up操作
void semaphore_up(struct semaphore* sme){
    enum int_state state = int_disable();
    ASSERT(sme->value == 0);
    // put_str("unblock start...");
    // put_int(list_len(&sme->wait_list));
    if(!list_empty(&sme->wait_list)){
        // put_str("unblock start...");
        struct list_node* tag = list_pop(&sme->wait_list);
        struct task_struct* ready_thread = node2entry(struct task_struct, ready_tag, tag); //semaphore只负责value和wait_list
        thread_unblock(ready_thread); //后续交给相关函数处理
    }
    // put_str("sme->value++\n");
    sme->value++;
    ASSERT(sme->value == 1);
    int_set_state(state);
}

//获取锁
void lock_get(struct lock* lock){
    // enum int_state state = int_disable();
    if(lock->owner != cur_thread()){
        // put_str("down start...\n");

        semaphore_down(&lock->sme); //成功往下执行，否则阻塞
        // enum int_state state = int_disable();
        // put_str("lock_get");
        // int_set_state(state);
        lock->owner = cur_thread();
        lock->req_cnt = 1;
    }else{
        lock->req_cnt++;
    }
    // int_set_state(state);
}

//释放锁
void lock_release(struct lock* lock){
    // enum int_state state = int_disable();
    ASSERT(lock->owner == cur_thread());
    if(lock->req_cnt > 1){
        lock->req_cnt--;
        return;
    }
    ASSERT(lock->req_cnt == 1);
    lock->owner = NULL;
    lock->req_cnt = 0;

    semaphore_up(&lock->sme);
    // enum int_state state = int_disable();
    // put_str("lock_release");
    // put_char('\n');
    // int_set_state(state);
}

//