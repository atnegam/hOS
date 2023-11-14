#$
BUILD_DIR = ./build
HARDWARE = ./hardware
kernel_entry = 0xc0001500
AS = nasm
CC = gcc
LD = ld
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/print.o

#.S bulid
$(BUILD_DIR)/print.o: kernel/lib/print.S
	$(AS) -f elf $< -o $@

#.C build
$(BUILD_DIR)/main.o: kernel/init/main.c
	$(CC) -m32 -I include/ -c $^ -o $@



#.bin build
$(BUILD_DIR)/boot.bin: boot/boot.S
	$(AS) $< -o $@

$(BUILD_DIR)/loader.bin: boot/loader.S
	$(AS) $< -o $@

$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) -m elf_i386 -Ttext $(kernel_entry) -e main $^ -o $@


#$all

build: $(BUILD_DIR)/boot.bin $(BUILD_DIR)/loader.bin $(BUILD_DIR)/kernel.bin

copy_hd: 
		dd if=$(BUILD_DIR)/boot.bin of=./hardware/hd60M.img bs=512 count=1 conv=notrunc
		dd if=$(BUILD_DIR)/loader.bin of=./hardware/hd60M.img bs=512 count=4 seek=2 conv=notrunc
		dd if=$(BUILD_DIR)/kernel.bin of=./hardware/hd60M.img bs=512 count=200 seek=9 conv=notrunc

all: build copy_hd

clean: 
		rm -rf $(BUILD_DIR)/*

