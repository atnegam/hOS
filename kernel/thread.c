#include "thread.h"
#include "string.h"
#include "stdint.h"
#include "memory.h"

//内核线程函数
static void kernel_thread(thread_fuc* fuc, void* fuc_arg){
    fuc(fuc_arg);
}

//初始化PCB
static void task_struct_init(struct task_struct* ts, char* name, int piro, thread_fuc* fuc, void* fuc_arg){
    //PCB信息
    memset(ts, 0, sizeof(*ts));
    strcpy(ts->task_name, name);
    ts->piro = piro;
    ts->state = RUNING;
    ts->kthread_stack = (uint32_t*)((uint32_t)ts + PAGE_SIZE); //kthread_stack存放指向栈顶的值
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

}

//创建线程
struct task_struct* thread_create(char* name, int prio, thread_fuc* fuc, void* fuc_arg){
    struct task_struct* ts = (struct task_struct*)kernel_page_get(1); //为PCB分配一页
    task_struct_init(ts, name, prio, fuc, fuc_arg);
    asm volatile ("movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret"\
     :  : "g"(ts->kthread_stack) : "memory");
    return ts;
}

