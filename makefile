#$
BUILD_DIR = ./build
HARDWARE = ./hardware
AS = nasm


#.S bulid
$(BUILD_DIR)/boot.bin: boot/boot.S
	$(AS) $< -o $@

$(BUILD_DIR)/setup.bin: boot/setup.S
	$(AS) $< -o $@


#$all

build:$(BUILD_DIR)/boot.bin $(BUILD_DIR)/setup.bin

copy_hd: 
		dd if=$(BUILD_DIR)/boot.bin of=./hardware/hd60M.img bs=512 count=1 conv=notrunc
		dd if=$(BUILD_DIR)/setup.bin of=./hardware/hd60M.img bs=512 count=4 seek=2 conv=notrunc


all: build copy_hd