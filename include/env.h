#ifndef ENV_H
#define ENV_H

//kernel code selector
/*
;选择子结构 TI:1为LDT  2为GDT       RPL:特权级相关
; 15                    3   2        0  
;|------------------------------------|
;|   描述符索引值         |TI |   RPL   |
;|------------------------------------|
*/
#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT 0
#define TI_LDT 1

#define CODE_IND 1

#define CODE_K_SELECTOR (CODE_IND << 3) + (TI_GDT << 2) + RPL0


//irq_desc attribute
#define IDT_DESC_P 1
#define IDT_DESC_DPL0 0
#define IDT_DESC_DPL3 3
#define IDT_DESC_TYPE 0xE

#define IRQ_DESC_K_ATTR (IDT_DESC_P << 7) + (IDT_DESC_DPL0 << 5) + IDT_DESC_TYPE //0级
#define IRQ_DESC_U_ATTR (IDT_DESC_P << 7) + (IDT_DESC_DPL1 << 5) + IDT_DESC_TYPE



#endif