#include "thread.h"
#include "string.h"
#include "stdint.h"
#include "memory.h"
#include "interrupt.h"
#include "list.h"
#include "debug.h"
#include "print.h"

struct task_struct* main_thread;
struct list ready_list;
struct list global_list;
static struct list_node* thread_tag;

extern void switch_to(struct task_struct* thread, struct task_struct* next);

//获取当前运行线程pcb地址
struct task_struct* cur_thread(){
    uint32_t esp;
    asm ("mov %%esp, %0" : "=g"(esp));
    return (struct task_struct*)(esp & 0xfffff000);
}

//内核线程函数
static void kernel_thread(thread_fuc* fuc, void* fuc_arg){
    int_enable();
    fuc(fuc_arg);
}

//初始化main线程PCB
static void init_main_thread(){
    main_thread = cur_thread();
    memset(main_thread->task_name, 0, sizeof(*main_thread));
    strcpy(main_thread->task_name, "main_thread");
    main_thread->state = RUNING;
    main_thread->piro = 30;
    main_thread->kthread_stack = (uint32_t*)((uint32_t)main_thread + PAGE_SIZE);
    main_thread->ticks = main_thread->piro;
    main_thread->total_ticks = 0;
    main_thread->pg_addr = NULL;
    main_thread->magic_num = 0x19970405;
    ASSERT(!list_find(&global_list, &main_thread->global_tag));
    tail_insert(&global_list, &main_thread->global_tag);
}

//初始化PCB
struct task_struct* task_struct_init(struct task_struct* ts, char* name, int piro, thread_fuc* fuc, void* fuc_arg){
    //PCB信息
    memset(ts, 0, sizeof(*ts));
    strcpy(ts->task_name, name);
    ts->piro = piro;
    ts->state = READING;
    ts->kthread_stack = (uint32_t*)((uint32_t)ts + PAGE_SIZE); //kthread_stack存放指向栈顶的值
    ts->ticks = piro;
    ts->total_ticks = 0;
    // ts->ready_tag = 0;
    // ts->global_tag = 0;
    ts->pg_addr = NULL;
    ts->magic_num = 0x19970405;
    //内核栈
    ts->kthread_stack -= sizeof(struct int_stack);
    ts->kthread_stack -= sizeof(struct fuc_stack);
    struct fuc_stack* cur_fuc_stack = (struct fuc_stack*)ts->kthread_stack;
    cur_fuc_stack->eip = kernel_thread;
    cur_fuc_stack->fuc = fuc;
    cur_fuc_stack->fuc_arg = fuc_arg;
    cur_fuc_stack->esi = 0;
    cur_fuc_stack->edi = 0;
    cur_fuc_stack->ebx = 0;
    cur_fuc_stack->ebp = 0; 

    return ts;
}

//线程阻塞
void thread_block(enum task_state blocked){
    enum int_state state = int_disable();
    struct task_struct* cur = cur_thread();
    cur->state = blocked; //设置为阻塞状态
    // put_str("scheduler start...\n");
    scheduler();
    int_set_state(state);
}

//线程唤醒
void thread_unblock(struct task_struct* ready_thread){
    enum int_state state = int_disable();
    if(ready_thread->state != READING){
        ASSERT(!list_find(&ready_list ,&ready_thread->ready_tag));
        head_insert(&ready_list ,&ready_thread->ready_tag);//插入就绪队头，优先调用
        ready_thread->state = READING;
    }
    int_set_state(state); 
}


//线程调度
void scheduler(){
    ASSERT(int_get_state() == INT_OFF); //时钟中断时处理器自动关中断
    struct task_struct* thread = cur_thread();
    if(thread->state == RUNING){//时间片耗尽   "这个 “=” bug 找了半天 (´･_･`)"
        ASSERT(!list_find(&ready_list, &thread->ready_tag));
        tail_insert(&ready_list, &thread->ready_tag);
        thread->ticks == thread->piro;
        thread->state = READING;
    }else{//阻塞
        //
        // put_str("waiting...\n");
    }

    //取next线程
    ASSERT(!list_empty(&ready_list));
    
    thread_tag = NULL;
    thread_tag = list_pop(&ready_list);
    struct task_struct* next = node2entry(struct task_struct, ready_tag, thread_tag);
    next->state = RUNING;
    switch_to(thread, next);
}

//创建线程
struct task_struct* thread_create(char* name, int prio, thread_fuc* fuc, void* fuc_arg){
    struct task_struct* ts = (struct task_struct*)kernel_page_get(1); //为PCB分配一页
    // put_int((uint32_t)ts);
    task_struct_init(ts, name, prio, fuc, fuc_arg);

    //加入线程队列
    ASSERT(!list_find(&ready_list, &ts->ready_tag));
    tail_insert(&ready_list, &ts->ready_tag);

    ASSERT(!list_find(&global_list, &ts->global_tag));
    tail_insert(&global_list, &ts->global_tag);
    // put_str("tail_insert.\n");

    // put_str("task_struct_init done.\n");
    // asm volatile ("movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret"\
    //  :  : "g"(ts->kthread_stack) : "memory");
    // put_str("thread_create done.\n");
    return ts;
}

//初始化线程环境
void thread_init(void){
    put_str("thread_init start...\n");

    list_init(&ready_list);
    list_init(&global_list);
    init_main_thread();
    
    put_str("thread_init done.\n");
}
