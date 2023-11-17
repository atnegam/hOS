#include "print.h"
#include "init.h"

int main(void){
	put_str("\n Welcome to hOS. \n");
	
	//初始化各子系统
	init();
	
	asm volatile ("sti");

	while(1);
	return 0;
}
