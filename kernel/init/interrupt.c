#include "stdint.h"
#include "interrupt.h"
#include "env.h"
#include "print.h"
#include "io.h"


//门描述符(中断描述符)结构
/*
;              31~16                    15  14~13 12  11~8  7 6 5             0
;|-----------------------------------------------------------------------------
;|     中断处理程序在目标段偏移的31~16位   | P | DPL | S | TYPE |0|0|0|   未使用    |
;|-----------------------------------------------------------------------------

;               32~16                                   15~0
;|-----------------------------------------------------------------------------
;|         中断程序目标代码段选择子       |    中断处理程序在目标段偏移的0~15位        |   
;|-----------------------------------------------------------------------------
*/

//定义中断描述符  64bit
struct idt_desc {
    
    uint16_t offset_low;
    uint16_t selector; 
    uint8_t count;
    uint8_t attr;
    uint16_t offset_high;

};

#define INT_NUM 0x21

//定义中断描述符表
struct idt_desc IDT[INT_NUM];


//初始化中断描述符
void init_idt_desc(struct idt_desc* cur_desc, interrupt_handler irq_fun, uint8_t _attr){
    
    cur_desc->offset_low = (uint32_t)irq_fun & 0x0000FFFF;
    cur_desc->selector = CODE_K_SELECTOR;
    cur_desc->count = 0;
    cur_desc->attr = _attr;
    cur_desc->offset_high = ((uint32_t)irq_fun & 0xFFFF0000) >> 16;

}

//中断服务程序路由表
extern interrupt_handler int_table[INT_NUM];

//初始化中断描述符表
void init_idt(){
    put_str("init_irq_desc_list start......\n");
    for(int i = 0; i < INT_NUM; i++){
        init_idt_desc(&IDT[i], int_table[i], IRQ_DESC_K_ATTR);
    }
    put_str("init_irq_desc_list done.\n");
}

//中断服务程序入口表
interrupt_handler int_handler_entry[INT_NUM];

//通用中断服务程序
static void exception_handler(uint8_t irq_no){
    if(irq_no == 0x27 || irq_no == 0x2f){
        return;
    }
    put_str("irq_no: ");
    put_int(irq_no);
    put_char('\n');
}

//异常提示内容表
char* exception_str[INT_NUM];

//注册中断服务程序
static void int_handler_reg(){
    for(int i = 0; i < INT_NUM; i++){
        int_handler_entry[i] = exception_handler;
    }
    //注册异常提示
}


//8259A 主(M)/从(S) 片数据与控制端口
#define M_CTRL_PORT 0x20
#define M_DATA_PORT 0x21
#define S_CTRL_PORT 0xa0
#define S_DATA_PORT 0xa1

//初始化8259A
void init_8259A(){
    put_str("init_8259A start......\n");
    //初始化主片
    outb(M_CTRL_PORT, 0x11);
    outb(M_DATA_PORT, 0x20);
    outb(M_DATA_PORT, 0x04);
    outb(M_DATA_PORT, 0x01);

    //初始化从片
    outb(S_CTRL_PORT, 0x11);
    outb(S_DATA_PORT, 0x28); 
    outb(S_DATA_PORT, 0x02);
    outb(S_DATA_PORT, 0x01);

    //接受时钟中断
    //outb(M_DATA_PORT, 0xfe);
    //outb(S_DATA_PORT, 0xff);

    put_str("init_825A done.\n");
}


void interrupt_init(){
    put_str("irq_init start......\n");

    init_idt();
    int_handler_reg();
    init_8259A();

    uint64_t IDTR_DATA = (sizeof(IDT) - 1)|(uint64_t)((uint32_t)IDT << 16);
   // asm volatile ("lidt %0" : : "m" (IDTR_DATA));
    asm volatile("lidt %0" : : "m" (IDTR_DATA));

    put_str("irq_init done.\n");
}