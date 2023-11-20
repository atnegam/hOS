#include "init.h"
#include "interrupt.h"
#include "print.h"
#include "time.h"

void init(){
    put_str("init start......\n");
   
    interrupt_init();
    time_init();
    
    put_str("init done.\n");
}