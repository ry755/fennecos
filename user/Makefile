.phony: all clean

APPS := \
	app/demo.app    \
	app/explode.app \
	sys/console.app \
	sys/init.app    \
	sys/sh.app

all: $(APPS)

USER_GCC_OBJ_FLAGS := -g -std=gnu99 -fpie -T user.ld -Wl,-no-dynamic-linker -ffreestanding -O2 -Wall -Wextra -I../include/ -I../libc/include/
USER_GCC_ELF_FLAGS := -pie -flinker-output=pie -T user.ld -ffreestanding -O2 -nostdlib -lgcc -Wl,--no-warn-rwx-segments

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

USER_FILES := $(wildcard *.c)
USER_OUT_FILES := crt0.o user.o ${USER_FILES:.c=.o}

LIBC_FILES := $(call rwildcard,../libc,*.c)
LIBC_OUT_FILES := ${LIBC_FILES:.c=.o}

%.o: %.c
	@echo '[CC] $<' && \
	i686-elf-gcc -c $< -o $@ $(USER_GCC_OBJ_FLAGS)
%.o: %.s
	@echo '[ASM] $<' && \
	i686-elf-gcc -c $< -o $@ $(USER_GCC_OBJ_FLAGS)

# behold. a stupid hack
.SECONDEXPANSION:
pc := %
%.app: $(USER_OUT_FILES) $(LIBC_OUT_FILES) $$(patsubst $$(pc).c, $$(pc).o, $$(call rwildcard,$$*,*.c))
	@mkdir -p ../base_image/app ../base_image/sys
	@echo '[APP] $@'
	@i686-elf-gcc -o $@.elf $^ $(USER_GCC_ELF_FLAGS)
	@../elf2app.py $@.elf ../base_image/$@
	@rm $@.elf

clean:
	rm -rf $(USER_OUT_FILES) $(LIBC_OUT_FILES) $(addprefix ../base_image/,$(APPS))
	rm -rf $(foreach dir,$(basename $(APPS)),$(call rwildcard,$(dir),*.o))
