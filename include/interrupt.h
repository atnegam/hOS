#ifndef INTERRUPT__H
#define INTERRUPT__H

typedef void* interrupt_handler;
void interrupt_init();

enum int_state{ //中断状态
    INT_NO,
    INT_OFF
};
enum int_state int_get_state();
enum int_state int_enable();
enum int_state int_disable();
enum int_state int_set_state(enum int_state state);

#endif