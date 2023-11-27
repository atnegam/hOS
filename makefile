#$
BUILD_DIR = ./build
HARDWARE = ./hardware
kernel_entry = 0xc0001500
AS = nasm
ASFLAG = -f elf
CC = gcc
CCFLAG = -m32 -I include/lib -I include/ -I include/kernel -c -fno-stack-protector -fno-builtin
LD = ld
LDFLAG = -m elf_i386 -Ttext $(kernel_entry) -e main 
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/print.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
		$(BUILD_DIR)/kernel.o $(BUILD_DIR)/time.o $(BUILD_DIR)/debug.o $(BUILD_DIR)/string.o \
		$(BUILD_DIR)/bitmap.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/thread.o $(BUILD_DIR)/list.o \
		$(BUILD_DIR)/switch.o


#.S bulid
$(BUILD_DIR)/print.o: kernel/lib/print.S
	$(AS) $(ASFLAG) $^ -o $@

$(BUILD_DIR)/kernel.o: kernel/kernel.S
	$(AS) $(ASFLAG) $^ -o $@

$(BUILD_DIR)/switch.o: kernel/switch.S
	$(AS) $(ASFLAG) $^ -o $@

#.C build
$(BUILD_DIR)/main.o: kernel/main.c
	$(CC) $(CCFLAG) $^ -o $@

$(BUILD_DIR)/init.o: kernel/init/init.c
	$(CC) $(CCFLAG) $^ -o $@

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c
	$(CC) $(CCFLAG) $^ -o $@

$(BUILD_DIR)/time.o: kernel/time.c
	$(CC) $(CCFLAG) $^ -o $@

$(BUILD_DIR)/debug.o: kernel/lib/debug.c
	$(CC) $(CCFLAG) $^ -o $@

$(BUILD_DIR)/string.o: kernel/lib/string.c
	$(CC) $(CCFLAG) $^ -o $@

$(BUILD_DIR)/bitmap.o: kernel/lib/bitmap.c
	$(CC) $(CCFLAG) $^ -o $@

$(BUILD_DIR)/memory.o: kernel/memory.c
	$(CC) $(CCFLAG) $^ -o $@

$(BUILD_DIR)/thread.o: kernel/thread.c
	$(CC) $(CCFLAG) $^ -o $@

$(BUILD_DIR)/list.o: kernel/lib/list.c
	$(CC) $(CCFLAG) $^ -o $@

#.bin build
$(BUILD_DIR)/boot.bin: boot/boot.S
	$(AS) $< -o $@

$(BUILD_DIR)/loader.bin: boot/loader.S
	$(AS) $< -o $@

$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAG) $^ -o $@

#$all

build: $(BUILD_DIR)/boot.bin $(BUILD_DIR)/loader.bin $(BUILD_DIR)/kernel.bin

copy_hd: 
		dd if=$(BUILD_DIR)/boot.bin of=./hardware/hd60M.img bs=512 count=1 conv=notrunc
		dd if=$(BUILD_DIR)/loader.bin of=./hardware/hd60M.img bs=512 count=4 seek=2 conv=notrunc
		dd if=$(BUILD_DIR)/kernel.bin of=./hardware/hd60M.img bs=512 count=200 seek=9 conv=notrunc

all: build copy_hd

clean: 
		rm -rf $(BUILD_DIR)/*

