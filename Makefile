.phony: all clean

all: base_image/boot/kernel.elf user_libc boot.img

qemu: boot.img
	qemu-system-i386 -hda boot.img -serial stdio -m 256M

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
KERNEL_FILES := $(call rwildcard,kernel,*.c)
KERNEL_OUT_FILES := kernel/boot.o ${KERNEL_FILES:.c=.o}
LIBK_FILES := $(call rwildcard,libk,*.c)
LIBK_OUT_FILES := ${LIBK_FILES:.c=.o}

kernel/boot.o: kernel/boot.s
	@echo '[ASM] $<' && \
	i686-elf-as kernel/boot.s -o kernel/boot.o

%.o: %.c
	@echo '[CC] $<' && \
	i686-elf-gcc -c $< -o $@ -g -std=gnu99 -ffreestanding -O0 -Wall -Wextra -Iinclude/ -Ilibk/include/

base_image/boot/kernel.elf: $(KERNEL_OUT_FILES) $(LIBK_OUT_FILES)
	@echo '[LINK] $@' && \
	i686-elf-gcc -T kernel/linker.ld -o $@ -ffreestanding -O0 -nostdlib $^ -lgcc

user_libc: FORCE
	@$(MAKE) -C user

boot.img: base_image/boot/kernel.elf user_libc
	sudo bash image.sh

FORCE: ;
clean:
	rm -f boot.img $(KERNEL_OUT_FILES) $(LIBC_OUT_FILES) $(LIBK_OUT_FILES)
	$(MAKE) -C user clean
