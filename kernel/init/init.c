#include "init.h"
#include "interrupt.h"
#include "print.h"
#include "time.h"
#include "memory.h"
#include "thread.h"
#include "console.h"
#include "keyboard.h"

void init(){
    put_str("init start......\n");

    interrupt_init();   //初始化中断管理
    time_init();        //初始化计数器/定时器
    mem_init();         //初始化内存管理
    thread_init();      //初始化线程管理
    console_init();     //初始化终端
    keyboard_init();    //初始化键盘
    
    put_str("init done.\n");
}