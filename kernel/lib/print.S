;TI_GDT equ 0
;RPL0 equ 0
;SELECTOR_VIDEO equ (0x0003<<3)+TI_GDT+RPL0

put_int_buffer dq 0  ;8字节缓存区

[bits 32]
section .text
global put_int

put_int:
	pushad
	push ebp
	mov ebp, esp
	mov ebx, [ebp+40] ;获取参数
	mov edx, ebx
	mov eax, put_int_buffer
	mov edi, 7 ;buffer中的偏移 
	mov ecx, 8

.do_per_4bit:
	and ebx, 0x000F
	cmp ebx, 9
	jg .do_A_F
	add ebx, '0'
	jmp .save_per_4bit 
.do_A_F:
	sub ebx, 10
	add ebx, 'A'

.save_per_4bit:		
	mov [eax+edi], bl
	dec edi
	shr edx, 4
	mov ebx, edx
	loop .do_per_4bit

.dump0:
	cmp edi, 7 ;至少7个0，直接打印最后一个字符
	je .print_num

	inc edi
	mov bl, [eax+edi]
	cmp bl, '0'
	jz .dump0
	
.print_num:
	;从buffer取出并打印
	mov bl, [eax+edi]
	push ebx
	call put_char
	add esp, 4
	inc edi
	cmp edi, 8
	jl .print_num
	
	pop ebp
	popad
	ret


global put_str
put_str:
	push ebx
	push ecx
	xor ecx,ecx
	mov ebx,[esp+12]
.goon:
	mov cl,[ebx]
	cmp cl,0
	jz .str_over
	push ecx
	call put_char
	add esp,4
	inc ebx
	jmp .goon
.str_over:
	pop ecx
	pop ebx
	ret

global put_char
put_char:
	pushad
	;保证gs中为正确到视频段选择子
	;mov ax,SELECTOR_VIDEO
	;mov gs,ax
	
	;获取当前光标位置
	;获得高8位
	mov dx,0x03d4	;索引寄存器
	mov al,0x0e
	out dx,al
	mov dx,0x03d5
	in al,dx
	mov ah,al
	
	;获得低8位
	mov dx,0x03d4
	mov al,0x0f
	out dx,al
	mov dx,0x03d5
	in al,dx
	
	;将光标存入bx
	mov bx,ax
	
	mov ecx,[esp+36]
	cmp cl,0xd
	jz .is_carriage_return
	cmp cl,0xa
	jz .is_line_feed
	
	cmp cl,0x8
	jz .is_backspace
	jmp .put_other
	
.is_backspace:
	dec bx
	shl bx,1
	mov byte [gs:bx],0x20
	inc bx
	mov byte [gs:bx],0x07
	shr bx,1
	jmp .set_cursor
	
.put_other:
	shl bx,1
	mov [gs:bx],cl
	inc bx
	mov byte [gs:bx],0x07
	shr bx,1
	inc bx
	cmp bx,2000
	jl .set_cursor
	
.is_line_feed:
.is_carriage_return:
;cr(\r)，只要把光标移到首行就行了
	xor dx,dx
	mov ax,bx
	mov si,80
	div si
	sub bx,dx
	
.is_carriage_return_end:
	add bx,80
	cmp bx,2000
.is_line_feed_end:
	jl .set_cursor
	
.roll_screen:
	cld
	mov ecx,960
	mov esi,0xc00b80a0	;第1行行首
	mov edi,0xc00b8000	;第0行行首
	rep movsd
	
	;最后一行填充为空白
	mov ebx,3840
	mov ecx,80
.cls:
	mov word [gs:ebx],0x0720
	add ebx,2
	loop .cls
	mov bx,1920	;最后一行行首
	
.set_cursor:
;将光标设为bx值
	;设置高8位
	mov dx,0x03d4
	mov al,0x0e
	out dx,al
	mov dx,0x03d5
	mov al,bh
	out dx,al
	
	;再设置低8位
	mov dx,0x03d4
	mov al,0x0f
	out dx,al
	mov dx,0x03d5
	mov al,bl
	out dx,al
.put_char_done:
	popad
	ret
