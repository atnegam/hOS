#include "init.h"
#include "interrupt.h"
#include "print.h"

void init(){
    put_str("init start......\n");
    
    irq_init();
    
    put_str("init done.\n");
}