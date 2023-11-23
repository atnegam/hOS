#include "print.h"
#include "init.h"
#include "interrupt.h"
#include "debug.h"
#include "string.h"
#include "stdvar.h"		

int main(void){
	put_str("\n Welcome to hOS. \n");

	//初始化各子系统
	init();
	
	int_set_state(INT_OFF);  //开中断
	//int_set_state(INT_OFF);

	uint32_t a = 0;
	memset(&a, 1, 4);
	put_int(a);
	
	


	//char* str = NULL;
	//char* str = "hello";
	//int len = strlen(str); //出错悬停
	//put_int(len);

	//int_enable(); 
	//int_disable();
	


	while(1);
	return 0;
}
