#include "print.h"
#include "init.h"
#include "interrupt.h"
#include "debug.h"
#include "string.h"

int main(void){
	put_str("\n Welcome to hOS. \n");
	
	//初始化各子系统
	init();
	
	int_set_state(INT_NO);  //开中断
	//int_set_state(INT_OFF);

	char* str = NULL;
	//char* str = "hello";
	int len = strlen(str); //出错悬停
	put_int(len);

	//int_enable(); 
	//int_disable();
	


	while(1);
	return 0;
}
