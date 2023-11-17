#$
BUILD_DIR = ./build
HARDWARE = ./hardware
kernel_entry = 0xc0001500
AS = nasm
ASFLAG = -f elf
CC = gcc
CCFLAG = -m32 -I include/ -c -fno-stack-protector -fno-builtin
LD = ld
LDFLAG = -m elf_i386 -Ttext $(kernel_entry) -e main 
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/print.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
		$(BUILD_DIR)/kernel.o


#.S bulid
$(BUILD_DIR)/print.o: kernel/lib/print.S
	$(AS) $(ASFLAG) $^ -o $@

$(BUILD_DIR)/kernel.o: kernel/kernel.S
	$(AS) $(ASFLAG) $^ -o $@

#.C build
$(BUILD_DIR)/main.o: kernel/init/main.c
	$(CC) $(CCFLAG) $^ -o $@

$(BUILD_DIR)/init.o: kernel/init/init.c
	$(CC) $(CCFLAG) $^ -o $@

$(BUILD_DIR)/interrupt.o: kernel/init/interrupt.c
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

