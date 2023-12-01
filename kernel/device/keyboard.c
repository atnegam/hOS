#include "keyboard.h"
#include "interrupt.h"
#include "print.h"
#include "io.h"
#define buffer_port 0x60

void keyboard_int_handler(){
    put_char('k');
    inb(buffer_port);
}

void keyboard_init(){
    put_str("keyboard_init start...\n");
    int_handler_reg(0x21, keyboard_int_handler);
    put_str("keyboard_init done.\n");
}