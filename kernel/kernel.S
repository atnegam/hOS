; 内核例程

%define ZERO push 0
extern put_str

section .data
irq_str db "interrupt occur!", 0xa, 0

global irq_table

irq_table:
;中断程序模版宏
%macro vector 2
section .text
irq%1entry:
    %2
    
    push irq_str
    call put_str
    mov esp, 4

    mov al, 0x20
    out 0xa0, al
    out 0x20, al

    add esp, 4 ;
    iret

section .data
    dd irq%1entry

%endmacro


vector 0x00, ZERO
vector 0x01, ZERO
vector 0x02, ZERO
vector 0x03, ZERO
vector 0x04, ZERO
vector 0x05, ZERO
vector 0x06, ZERO
vector 0x07, ZERO
vector 0x08, ZERO
vector 0x09, ZERO
vector 0x0A, ZERO
vector 0x0B, ZERO
vector 0x0C, ZERO
vector 0x0D, ZERO
vector 0x0E, ZERO
vector 0x0F, ZERO
vector 0x10, ZERO
vector 0x11, ZERO
vector 0x12, ZERO
vector 0x13, ZERO
vector 0x14, ZERO
vector 0x15, ZERO
vector 0x16, ZERO
vector 0x17, ZERO
vector 0x18, ZERO
vector 0x19, ZERO
vector 0x1A, ZERO
vector 0x1B, ZERO
vector 0x1C, ZERO
vector 0x1D, ZERO
vector 0x1E, ZERO
vector 0x1F, ZERO
vector 0x20, ZERO