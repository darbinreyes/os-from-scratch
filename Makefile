SHELL = /bin/sh
###########################
# To see the C pre-processor output:
# i386-elf-gcc  -Wall -Wextra -Werror -O0 -ffreestanding -E kernel/idt.c > idtpp.c
# To see the generated assembly:
# i386-elf-gcc  -Wall -Wextra -Werror -O0 -ffreestanding -S kernel/idt.c
###########################
# Build the kernel binary.
# $^ = all target dependency files.
# $< = first target dependency file.
# $@ = target file.
# THE FIRST TARGET = THE DEFAULT TARGET.

# @IMPORTANT Always disable optimizations.
# @IMPORTANT Always turn on all warnings with -Wall gcc flag. "This enables all
#            the warnings about constructions"
#C_SOURCES = $(wildcard include/*.c)
#OBJ = ${C_SOURCES:.c=.o}

CC := i386-elf-gcc
CC_FLAGS = -Wall -Wextra -Werror -O0 -ffreestanding
LD := i386-elf-ld
# Use `make TEST_MODE=1` for test mode.
ifdef TEST_MODE
TEST_OBJ_FILES := test_all.o test_assert.o test_stdlib.o test_stdio.o assert.o\
test_idt.o stdlib.o stdio.o string.o
else
TEST_OBJ_FILES :=
endif

all: os-image

testksize:
	./testksize.sh

run: all
	bochs -q -f bochsrc.txt # run bochs installed by mac ports.

rundbg: all
	./bochs/bochs -q -f bochsrc.txt # run bochs compiled from source. Required
									# to use bochs' debugging features.

runq: all
	qemu-system-i386 -drive file=os-image,if=floppy,format=raw

clean:
	rm -Rf *.bin *.o os-image

# This is the actual disk image that the computer loads, which is the
# combination of our compiled boot sector and kernel.
os-image: boot_sect.bin kernel.bin
	./testksize.sh
	cat $^ > $@

# Assemble the boot sector to raw machine code.
# @remark nasm reports file not found errors with "fatal: unable to open include
# file"
boot_sect.bin:	boot/boot_sect.s boot/print_string.s boot/disk_load.s
	nasm -O0 $< -I 'boot/' -f bin -o $@

# @IMPORTANT kernel_entry.o must go first here. The -lgcc and -L options
# workaround the `__udivdi3` undefined error.
kernel.bin: kernel_entry.o kernel.o screen.o low_level.o idt.o idt_asm.o stdio.o \
			assert.o i8259a_pic.o $(TEST_OBJ_FILES)
	$(LD) -O0 -o $@ -Ttext 0x1000 $^ --oformat binary -e 0x1000 -static -lgcc -L /opt/local/lib/gcc/i386-elf/9.2.0/

kernel_entry.o: kernel/kernel_entry.s
	nasm -O0 $< -f elf -o $@

# @IMPORTANT:The % operator does not match sub-directories, hence `kernel/%.c`.
# To compile in test mode use `make TEST_MODE=1`.
kernel.o: kernel/kernel.c
ifdef TEST_MODE
	$(CC) -DTEST_MODE $(CC_FLAGS) -c $< -o $@
else
	$(CC) $(CC_FLAGS) -c $< -o $@
endif

idt.o: kernel/idt.c kernel/idt.h
	$(CC)  $(CC_FLAGS) -c $< -o $@

idt_asm.o: kernel/idt_asm.s kernel/idt_asm.h
	nasm -O0 $< -f elf -o $@

%.o: drivers/%.c drivers/%.h
	$(CC) $(CC_FLAGS) -c $< -o $@

low_level.o: kernel/low_level.s kernel/low_level.h
	nasm -O0 $< -f elf -o $@

i8259a_pic.o: kernel/i8259a_pic.c kernel/i8259a_pic.h
	$(CC) $(CC_FLAGS) -c $< -o $@

# Disassemble our kernel - might be useful for debugging.
kernel.dis: kernel.bin
	ndisasm -b 32 $< > $@

%.o: tests/%.c tests/%.h
	$(CC) $(CC_FLAGS) -c $< -o $@

%.o: include/%.c include/%.h
	$(CC) $(CC_FLAGS) -c $< -o $@
