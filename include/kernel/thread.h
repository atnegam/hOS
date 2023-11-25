#ifndef THREAD__H
#define THREAD__H

#include "stdint.h"
typedef void thread_fuc(void*);

//中断栈
struct int_stack{
    //中断程序压入
    uint32_t vec_no;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_int;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    //用户态触发中断时使用
    uint32_t error_code;
    void (*eip)(void*);
    uint32_t cs;
    uint32_t eflag;
    void* esp;
    uint32_t ss;
    
};

//程序调用栈
struct fuc_stack{
     
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    void (*eip)(thread_fuc* fuc, void* fuc_arg);

    //新创建线程所需参数
    void (*nop)(void*);
    thread_fuc* fuc;
    void* fuc_arg;

};

//线程状态
enum task_state{
    READING, //就绪
    RUNING,  //运行
    WAITING, //阻塞
    STOPED   //停止
};

//PCB结构
struct task_struct{
    uint32_t* kthread_stack;       //线程内核栈顶(中断栈+程序调用栈)
    enum task_state state;    //状态
    uint32_t piro;            //优先级
    char task_name[20];              //线程name
    uint32_t magic_num;            //检测内核栈溢出魔数  布局:PCB｜magic_num|kthread_stack
};

struct task_struct* thread_create(char* name, int prio, thread_fuc* fuc, void* fuc_arg);

#endif