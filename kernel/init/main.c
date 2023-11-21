#include "print.h"
#include "init.h"
#include "interrupt.h"

int main(void){
	put_str("\n Welcome to hOS. \n");
	
	//初始化各子系统
	init();
	int_enable(); 
	//int_disable();

	while(1);
	return 0;
}
