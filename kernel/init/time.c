#include "io.h"
#include "print.h"
#include "time.h"

#define COUNT_NO 0 //选择计数器
#define COUNT_PORT 0x40 //计数器端口
#define WORK_MODE 2 //计数器工作模式
#define OUT_FREQUENCY 100 //计算器输入频率
#define INPUT_FREQUENCY 1193180 //计算器输出频率
#define INIT_VALUE INPUT_FREQUENCY / OUT_FREQUENCY  //计算器初始值
#define CTRL_PORT 0x43 //控制字寄存器端口
#define RW_METHOD 3

static void time_set(uint8_t ctrl_port, uint8_t count_no, uint8_t rw_method, uint8_t work_mode, uint8_t count_port, uint16_t init_value){
    //设置控制字寄存器
    uint8_t CTRL_FLAG = (count_no << 6)|(rw_method << 4)|(work_mode << 1);
    outb(ctrl_port, CTRL_FLAG);

    //设置计算器初始值
    outb(count_port, init_value);
    outb(count_port, init_value >> 8);
}


void time_init(){
    put_str("time_init start...");
    time_set(CTRL_PORT, COUNT_NO, RW_METHOD, WORK_MODE, COUNT_PORT, INIT_VALUE);
    put_str("time_init done.");
}