
section setup vstart=0x8FD
    
jmp setup_start

;--------------------创建GDT表-------------------------------------------------
;一项占8字节
GDT_BASE: dd 0x00000000
          dd 0x00000000

CODE_DESC: dd 0x0000FFFF
           dd GDT_CODE_HIGH_32

DATA_STACK_DESC: dd 0x0000FFFF
                 dd GDT_DATA_HIGH_32

VIDEO_DESC: dd 0x80000007
            dd GDT_VIDEO_HIGH_32

GDT_SIZE equ $ - GDT_BASE   ;GDT表大小
GDT_LIMIT equ GDT_SIZE - 1  ;GDT表界限

times 60 dq 0

;GDT基址指针，用于加载GDT到GDTR寄存器
gdt_ptr dw GDT_LIMIT        ;GDT界限         
        dd GDT_BASE         ;GDT基址
                 

all_mermory dd 0 ;字节为单位表示物理内存容量
ards_buf times 244 db 0
ards_nr dw 0    ;

setupmsg db 'setup starting...'



;-----------------------setup入口程序------------------

setup_start:
;test
    mov sp, 0x900
    mov bp, setupmsg
    mov cx, 17
    mov ax, 0x1301
    mov bx, 0x001f
    mov dx, 0x1800
    int 0x10

    call check_memory   ;检查物理内存容量

;------------------------进入保护模式----------------
;   1.打开A20   2.加载GDT表   3.将cr0 pe置1

    ; 开始进入保护模式
    ; 打开A20地址线
    in al, 0x92
    or al, 00000010B
    out 0x92, al

    ; 加载gdt
    lgdt [gdt_ptr]

    ; cr0第0位置1
    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax

    jmp dword GDT_CODE_SELECT:protect_start ; 刷新流水线进入保护模式



;检查物理内存容量   BUOS中断调用 int15h
check_memory:

    xor ebx, ebx
    mov edx, 0x534d4150
    mov di, ards_buf

.e820_mem_get_loop:
    mov eax, 0x0000e820
    mov ecx, 20
    int 0x15
    
    jc .e820_mem_get_failed
    
    add di, cx
    inc word [ards_nr]
    cmp ebx, 0
    jnz .e820_mem_get_loop

    mov cx, [ards_nr]
    mov ebx, ards_buf
    xor edx, edx

.find_max_mem_area:
    mov eax, [ebx]
    add eax, [ebx + 8]
    add ebx, 20
    cmp edx, eax
    jge .next_ards
    mov edx, eax

.next_ards:
    loop .find_max_mem_area
    jmp .mem_get_ok

.e820_mem_get_failed:
    mov byte [gs:0], 'f'
    mov byte [gs:2], 'a'
    mov byte [gs:4], 'i'
    mov byte [gs:6], 'l'
    mov byte [gs:8], 'e'
    mov byte [gs:10], 'd'
    ;检查失败卡死
    jmp $

.mem_get_ok:
    mov [all_mermory], edx  ;检查成功，将总容量值存储于all_mermory

    ret


;第一个保护模式函数
[bits 32]
protect_start:
    mov ax, GDT_DATA_SELECT
    mov ds, ax

    mov es, ax
    mov ss, ax

    mov esp, 0x900
    mov ax, GDT_VIDEO_SELECT
    mov gs, ax

    call page_setup

    ; 保存gdt表
    sgdt [gdt_ptr]

    ; 重新设置gdt描述符， 使虚拟地址指向内核的第一个页表
    mov ebx, [gdt_ptr + 2]
    or dword [ebx + 0x18 + 4], 0xc0000000
    add dword [gdt_ptr + 2], 0xc0000000
    
    add esp, 0xc0000000

    ; 页目录基地址寄存器
    mov eax, PAGE_BASE
    mov cr3, eax

    ; 打开分页
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    lgdt [gdt_ptr]
    mov byte [gs:160], 'V'
    jmp $	; 通过死循环使程序悬停在此


;创建二级页表
page_setup:

    mov ecx, 4096
    mov esi, 0
.pde_dir_clear:
    mov byte [PAGE_BASE + esi], 0
    inc esi
    loop .pde_dir_clear ;页目录表清0


;一个PDE分得空间(1*1024*1024*4K)，
.pde_create:
    mov eax, PAGE_BASE  ;
    add eax, 0x1000      ;eax存放第一个页表的地址
    mov ebx, eax

    or eax, US_U | RW_W | PP
    mov [PAGE_BASE + 0x0], eax
    mov [PAGE_BASE + 0xc00], eax
    
    sub eax, 0x1000
    mov [PAGE_BASE + 4092], eax   ;最后一个表项指向自己,动态调整页表

    mov ecx, 256    ;先创建1M/4K=256个页
    mov esi, 0
    mov edx, US_U | RW_W | PP
;一个PTE(1*1024*4k=4M)
.pte_create:
    mov [ebx+esi*4], edx
    add edx, 4096   ;前进一个物理页
    inc esi
    loop .pte_create

;高1G-4M 内核所需PDE
    mov eax, PAGE_BASE
    add eax, 0x2000
    or eax, US_U | RW_W | PP
    mov ebx, PAGE_BASE
    mov ecx, 254
    mov esi, 769

.hight_1G_pde_create:
    mov [ebx+esi*4], eax
    inc esi
    add eax, 0x1000
    loop .hight_1G_pde_create
    ret



;-----------------gdt表项各位段属性值---------------------------------------------

;GDT全局段描述符(8B=64bit) 平坦模式段空间=段界限*G=0xFFFFF*4K=0xFFFFFFFF=2^32-1=4G
;31~24          23  22   21   20    19~16  15  14~13 12   11~8    7~0
;|-----------------------------------------------------------------------------
;|段基址(31-24)| G | D/B | L | AVL | 段界限 | P | DPL | S | TYPE | 段基址(23-16) | 高32位
;|-----------------------------------------------------------------------------

;               32~16                                   15~0
;|-----------------------------------------------------------------------------
;|             段基址(15~0)            |                 段界限(15~0)           | 低32位   
;|-----------------------------------------------------------------------------

    
GDT_TYPE_DATA    equ 0010b    ;数据段,x=0,e=0,w=1,a=0    不可执行 向上扩展 可写 未访问
GDT_TYPE_CODE    equ 1000b    ;代码段,x=1,c=0,r=0,a=0    可执行   非一致性 不可读 未访问

GDT_S_CODE  equ 1b       ;代码段
GDT_S_DATA  equ 1b       ;数据段
GDT_S_SYS   equ 0b       ;系统段

GDT_DPL_0   equ 00b      ;0特权级
GDT_DPL_1   equ 01b      ;1特权级
GDT_DPL_2   equ 10b      ;2特权级
GDT_DPL_3   equ 11b      ;3特权级
    
GDT_P       equ 1b       ;段存在否

GDT_LIMIT_C_D_HING  equ 1111b   ;在高32位处的code和data段界限值
GDT_LIMIT_V_HIGH    equ 0000b   ;在高32位处的显存段界限值

GDT_AVL     equ 0b       ;空位

GDT_L       equ 0b       ;0 不支持64位代码

GDT_D_32    equ 1b       ;1 支持32位代码

GDT_G_4K    equ 1b       ;1 4k粒度    


GDT_CODE_HIGH_32 equ (0x00 << 24)+(GDT_G_4K << 23)+(GDT_D_32 << 22)+(GDT_L << 21)+ \
(GDT_AVL << 20)+(GDT_LIMIT_C_D_HING << 16)+(GDT_P << 15)+(GDT_DPL_0 << 13)+ \
(GDT_S_CODE << 12)+(GDT_TYPE_CODE << 8)+(0x00)  ;代码段描述符高32位

GDT_DATA_HIGH_32 equ (0x00 << 24)+(GDT_G_4K << 23)+(GDT_D_32 << 22)+(GDT_L << 21)+ \
(GDT_AVL << 20)+(GDT_LIMIT_C_D_HING << 16)+(GDT_P << 15)+(GDT_DPL_0 << 13)+ \
(GDT_S_DATA << 12)+(GDT_TYPE_DATA << 8)+(0x00)  ;数据段描述符高32位

GDT_VIDEO_HIGH_32 equ (0x00 << 24)+(GDT_G_4K << 23)+(GDT_D_32 << 22)+(GDT_L << 21)+ \
(GDT_AVL << 20)+(GDT_LIMIT_V_HIGH << 16)+(GDT_P << 15)+(GDT_DPL_0 << 13)+ \
(GDT_S_DATA << 12)+(GDT_TYPE_DATA << 8)+(0x0b)  ;显存段描述符高32位         



;--------------------GDT表选择子-----------------------------------------------

;选择子结构 TI:1为LDT  2为GDT       RPL:特权级相关
; 15                    3   2        0  
;|------------------------------------|
;|   描述符索引值         |TI |   RPL   |
;|------------------------------------|
    
RPL0  equ   00b
RPL1  equ   01b
RPL2  equ   10b
RPL3  equ   11b
TI_GDT	 equ   000b
TI_LDT	 equ   100b                   

GDT_CODE_SELECT equ (0x0001 << 3) + TI_GDT + RPL0   ;
GDT_DATA_SELECT equ (0x0002 << 3) + TI_GDT + RPL0
GDT_VIDEO_SELECT equ (0x0003 << 3) + TI_GDT + RPL0

;---------------------------------二级页表-----------------------------------------

;页目录项结构(PDE)
;31        12 11 9  8  7  6  5   4    3    2   1  0
;-------------------------------------------------------
;| 地址       | AVL |G |0 |D |A |PCD |PWT |US |RW |P |
;-------------------------------------------------------

;页表项结构(PTE)
;31         12            7
;-------------------------------------------------------
;| 地址       |.....同上.| PAT |.................同上  |
;-------------------------------------------------------

PAGE_BASE equ 0x100000

PP  equ 1b
RW_R equ 00b
RW_W equ 10b
US_S equ 000b
US_U equ 100b