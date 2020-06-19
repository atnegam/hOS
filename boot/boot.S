;------------------主引导程序--------------------


;-----------------setup.S相关-------------------
SETUP_BASE_ADDR equ 0x8FD
SETUP_START_SECTOR equ 0x2

;-------------16位模式初始化段寄存器---------------
section boot vstart=0x7c00

    mov ax, cs  ;BIOS加载mbr,cs变为0x7c00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00
    mov ax, 0xb800  ;显存基址
    mov gs, ax

;------------------清屏-------------------------
    mov ax, 0600h
    mov bx, 0700h
    mov cx, 0
    mov dx, 184fh
    int 10h

;------------------打印 "MBR"--------------------

;------------------加载setup.S--------------------

    mov eax, SETUP_START_SECTOR
    mov bx, 0x8FD
    
    ; 读取1个扇区
    mov cx, 4
    call rd_disk_m_16       ;调用16位模式加载磁盘到内存函数

    
    jmp SETUP_BASE_ADDR     ;直接跳到setup.S的起始代码执行

;-------------rd_disk_m_16函数--------------------
; eax保存从硬盘读取到的数据的保存地址，ebx为起始扇区，cx为读取的扇区数
rd_disk_m_16:

    mov esi, eax
    mov di, cx

    mov dx, 0x1f2
    mov al, cl
    out dx, al

    mov eax, esi

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
    mov [bx], ax
    add bx, 2
    loop .go_on_read
    ret

;-----------------补齐512B----------------
    times 510-($-$$) db 0
    db 0x55, 0xaa
