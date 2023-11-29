#include "console.h"
#include "print.h"
#include "lock.h"

static struct lock console_lock;

// 初始化终端
void console_init(){
    lock_init(&console_lock);
}

//获取终端锁
void console_get(){
    lock_get(&console_lock);
}

//释放终端锁
void console_release(){
    lock_release(&console_lock);
}

//终端打印字符串
void console_putstr(char* str){
    console_get();
    put_str(str);
    console_release();
}

//终端打印字符
void console_putchat(uint8_t c){
    console_get();
    put_char(c);
    console_release();
}

//终端打印16进制数字
void console_putint(uint32_t value){
    console_get();
    put_int(value);
    console_release();
}