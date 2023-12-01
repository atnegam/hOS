;实模式  1.设置段表和页表  2.加载并解析内核

section setup vstart=0x8FD
    
jmp setup_start


;创建GDT表  一项占8字节
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

;GDT基址指针，用于加载到GDT到GDTR寄存器
gdt_ptr dw GDT_LIMIT        ;GDT界限         
        dd GDT_BASE         ;GDT基址
                 

all_mermory dd 0 ;字节为单位表示物理内存容量
ards_buf times 244 db 0
ards_nr dw 0    ;

setupmsg db 'setup starting...'



;setup.S入口

setup_start:
;test
    mov sp, 0x900
    mov bp, setupmsg
    mov cx, 17
    mov ax, 0x1301
    mov bx, 0x001f
    mov dx, 0x1800
    int 0x10            ;BIOS中断

    call check_memory   ;检查物理内存容量

    ; 开始进入保护模式
    
    in al, 0x92
    or al, 00000010B
    out 0x92, al        ;打开A20地址线

    
    lgdt [gdt_ptr]      ;加载gdt

    
    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax        ;cr0第0位置1

    jmp dword GDT_CODE_SELECT:mode32_start  ;刷新流水线进入保护模式



;检查物理内存容量   BIOS中断调用 int15h
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
mode32_start:
    mov ax, GDT_DATA_SELECT
    mov ds, ax

    mov es, ax
    mov ss, ax

    mov esp, 0x900
    mov ax, GDT_VIDEO_SELECT
    mov gs, ax

    ;加载kernel映像
    mov eax, 0x9   ;KERNEL_START_SECTOR
    mov ebx, 0x70000  ;KERNEL_IMG_BASE

    mov ecx, 200
    call rd_disk_m_32
    
    ;构建页表
    call page_setup

    ;保存gdt表
    sgdt [gdt_ptr]

    ;重新设置gdt描述符， 使虚拟地址指向内核的第一个页表
    mov ebx, [gdt_ptr + 2]
    or dword [ebx + 0x18 + 4], 0xc0000000
    add dword [gdt_ptr + 2], 0xc0000000
    
    add esp, 0xc0000000

    ;页目录基地址寄存器
    mov eax, PAGE_BASE
    mov cr3, eax

    ;打开分页
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    lgdt [gdt_ptr]
    
    ;解析kernel映像
    call kernel_init
    mov esp, 0xc009f000
    jmp 0xc0001500


kernel_init:    ;加载解析kernel
	xor eax,eax
	xor ebx,ebx	;记录程序头表地址（内核地址+程序头表偏移地址）
	xor ecx,ecx	;记录程序头中的数量
	xor edx,edx	;记录程序头表中每个条目的字节大小
	
	mov dx,[0x70000+42]	;偏移文件42字节处是e_phentsize
	mov ebx,[0x70000+28]	;偏移文件28字节处是e_phoff
	add ebx,0x70000
	mov cx,[0x70000+44]	;偏移文件44字节处是e_phnum
	
.each_segment:
	cmp byte [ebx+0],0	;p_type=0,说明此头未使用
	je .PTNULL
	
	push dword [ebx+16]	;p_filesz压入栈(mem_cpy第三个参数)
	mov eax,[ebx+4]
	add eax,0x70000
	push eax		;p_offset+内核地址=段地址(mem_cpy第二个参数)
	push dword [ebx+8]	;p_vaddr(mem_cpy第一个参数)
	call mem_cpy
	add esp,12
.PTNULL:
	add ebx,edx	;ebx指向下一个程序头
	loop .each_segment
	ret
	
;主子拷贝函数（dst,src,size）
mem_cpy:
	cld
	push ebp
	mov ebp,esp
	push ecx
	
	mov edi,[ebp+8]		;dst
	mov esi,[ebp+12]	;src
	mov ecx,[ebp+16]	;size
	rep movsb
	
	pop ecx
	pop ebp
	ret


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
    mov eax, PAGE_BASE  
    add eax, 0x1000      ;eax存放第一个页表的地址
    mov ebx, eax
    or eax, US_U | RW_W | PP
    mov [PAGE_BASE + 0x00], eax
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


;32位下读取硬盘数据函数
rd_disk_m_32:

    mov esi, eax
    mov di, cx

    mov dx, 0x1f2
    mov al, cl
    out dx, al

    mov eax, esi
    ; 保存LBA地址
    mov dx, 0x1f3
    out dx, al

    mov cl, 8
    shr eax, cl
    mov dx, 0x1f4
    out dx, al

    shr eax, cl
    mov dx, 0x1f5
    out dx, al

    shr eax, cl
    and al, 0x0f
    or al, 0xe0
    mov dx, 0x1f6
    out dx, al

    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

.not_ready:
    nop
    in al, dx
    and al, 0x88
    cmp al, 0x08
    jnz .not_ready

    mov ax, di
    mov dx, 256
    mul dx
    mov cx, ax
    mov dx, 0x1f0

.go_on_read:
    in ax, dx
    mov [ds:ebx], ax
    add ebx, 2
    loop .go_on_read
    ret


;gdt表项各位段属性值

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

GDT_CODE_SELECT equ (0x0001 << 3) + TI_GDT + RPL0  
GDT_DATA_SELECT equ (0x0002 << 3) + TI_GDT + RPL0
GDT_VIDEO_SELECT equ (0x0003 << 3) + TI_GDT + RPL0

;---------------------------------二级页表-----------------------------------------

;页目录项结构(PDE)
;31        12 11 9  8  7  6  5   4    3    2   1  0
;-----------------------------------------------------
;| 地址       | AVL |G |0 |D |A |PCD |PWT |US |RW |P |
;-----------------------------------------------------

;页表项结构(PTE)
;31         12            7
;-----------------------------------------------------
;| 地址       |.....同上.| PAT |.................同上  |
;-----------------------------------------------------

PAGE_BASE equ 0x100000

PP  equ 1b
RW_R equ 00b
RW_W equ 10b
US_S equ 000b
US_U equ 100b
