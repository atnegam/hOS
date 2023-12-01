#include "print.h"
#include "init.h"
#include "interrupt.h"
#include "debug.h"
#include "string.h"
#include "stdvar.h"		
#include "memory.h"
#include "thread.h"
#include "list.h"
#include "lock.h"
#include "console.h"

int a = 0;

void testA(void* arg);
void testB(void* arg);

int main(void){
	put_str("\n Welcome to hOS. \n");

	//初始化各子系统
	init();
	// int_set_state(INT_OFF);

	//int_set_state(INT_OFF);
	
	// testmm();

	// void* arr = kernel_page_get(3);
	// int len = strlen(arr);
	// put_int((uint32_t)arr);

	//char* str = NULL;
	//char* str = "hello";
	//int len = strlen(str); //出错悬停
	//put_int(len);

	//int_enable(); 
	//int_disable();

	thread_create("testA", 10, testA, "argA");
	thread_create("testB", 10, testB, "argB");

	// int_set_state(INT_NO);  //开中断
	int_enable();


	while(1){
		// console_putstr("Main ");
		// int_disable();
		// put_str("Main");
		// put_int(b++);
		// int_enable();
	};
	return 0;
}

void testA(void* arg){
	static int a = 0;
	while(1){
		console_putstr("argA ");
		// int_disable();
		// put_str("argA");
		// int_enable();
	}
}

void testB(void* arg){
	while(1){
		console_putstr("argB ");
		// int_disable();
		// put_str("argB ");
		// int_enable();
	}
}