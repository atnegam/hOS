;主引导程序boot

section boot vstart=0x7c00

    mov ax, cs  ;BIOS加载mbr(boot.S),cs变为0x7c00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00
    mov ax, 0xb800  ;显存基址
    mov gs, ax

    ;清屏
    mov ax, 0600h
    mov bx, 0700h
    mov cx, 0
    mov dx, 184fh
    int 10h


;加载setup.S
    mov eax, 0x2          ;2扇区
    mov bx, 0x8FD
    
    ; 读取4个扇区
    mov cx, 4
    call read_setup       ;加载setup.S到内存

    
    jmp 0x8fD     ;直接跳到setup.S的起始代码执行


;bx:目标内存地址 eax:起始扇区 cx:度入扇区数  
read_setup:

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

    times 510-($-$$) db 0   ;补齐512
    db 0x55, 0xaa           ;mbr魔数
