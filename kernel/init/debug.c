#include "debug.h"
#include "print.h"
#include "interrupt.h"

void assert(char* filename, int line, char* func, char* condition){  

    int_set_state(INT_OFF);

    put_char('\n');
    put_str("error:");
    put_str(filename);
    put_char(',');
    put_str("0x");
    put_int(line);
    put_char(',');
    put_str(func);
    put_char(':');
    put_str(condition);

    int_set_state(INT_NO);
}