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

    // set_cursor(0);
    // int cnt = 0;
    // while(cnt < 400){
    //     put_char(' ');
    //     cnt++;
    // }
    // set_cursor(0);
    if(irq_no == 14){
        uint32_t page_fault_addr = 0;
        asm ("movl %%cr2, %0" : "=r" (page_fault_addr));
        put_char('\n');
        put_str("page_fault_addr is ");
        put_int(page_fault_addr);
    }else{
        put_str("irq_no: ");
        put_int(irq_no);
        put_char('\n');
    }
    while(1);
}

//异常提示内容表
char* exception_str[INT_NUM];

//注册异常服务程序
static void exception_reg(){
    for(int i = 0; i < INT_NUM; i++){
        int_handler_entry[i] = exception_handler;
    }
    //注册异常提示
}

//注册中断服务程序
void int_handler_reg(uint8_t irq_no, interrupt_handler fuc){
    int_handler_entry[irq_no] = fuc;
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

   // 接受时钟中断
    outb(M_DATA_PORT, 0xfe);
    outb(S_DATA_PORT, 0xff);

    put_str("init_825A done.\n");
}


#define IF 0x00000200   //eflag IF位的值为1
#define GET_FLAG(FLAG_VAL) asm volatile ("pushfl; popl %0" : "=g"(FLAG_VAL)) //取IF位值

//获取中断状态
enum int_state int_get_state(){
    uint32_t eflag = 0;
    GET_FLAG(eflag);
    return eflag & IF? INT_NO:INT_OFF;
}

//开中断
enum int_state int_enable(){
    enum int_state old_state = int_get_state();
    if(old_state == INT_OFF){
        asm volatile ("sti");       
    }
    return old_state;
}

//关中断
enum int_state int_disable(){
    enum int_state old_state = int_get_state();
    if(old_state == INT_NO){
        asm volatile ("cli" : : :"memory");
    }
    return old_state;
}

//设置中断状态
enum int_state int_set_state(enum int_state state){
    return state == INT_NO? int_enable() : int_disable();
}

//中断子系统初始化
void interrupt_init(){
    put_str("irq_init start......\n");

    init_idt();
    exception_reg();
    init_8259A();

    uint64_t IDTR_DATA = (uint64_t)((uint32_t)IDT << 16)|(sizeof(IDT) - 1);
    asm volatile("lidt %0" : : "m" (IDTR_DATA));

    put_str("irq_init done.\n");
}