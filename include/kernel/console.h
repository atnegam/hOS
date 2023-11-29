#ifndef CONSOLE__H
#define CONSOLE__H
#include "stdint.h"

void console_init();
void console_putstr(char* str);
void console_putchat(uint8_t c);
void console_putint(uint32_t value);

#endif