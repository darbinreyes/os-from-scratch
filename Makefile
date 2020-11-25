###########################
# The first thing the compiler will do is preprocess the file. We can tell clang
# to show us what it looks like if we stop after that step:
# i386-elf-gcc  -Wall -Wextra -Werror -O0 -ffreestanding -E kernel/idt.c > idtpp.c
# Next up: parsing and code generation. We can tell clang to output the
# resulting assembly code like so:
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

C_SOURCES = $(wildcard kernel/*.c drivers/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h)
OBJ = ${C_SOURCES:.c=.o}
CC := i386-elf-gcc
CC_FLAGS = -Wall -Wextra -Werror -O0 -ffreestanding
LD := i386-elf-ld
# Use make TEST_MODE=1 for test mode.
ifdef TEST_MODE
TEST_OBJ_FILES := test_all.o test_assert.o test_stdlib.o assert.o stdlib.o
else
TEST_OBJ_FILES :=
endif

all: os-image

run: all
	bochs -q -f bochsrc.txt # run bochs installed by mac ports.

rundbg: all
	./bochs/bochs -q -f bochsrc.txt # run bochs compiled from source. Required
									# to use bochs' debugging features.

runq: all
	qemu-system-i386 -drive file=os-image,if=floppy,format=raw

# This is the actual disk image that the computer loads, which is the
# combination of our compiled boot sector and kernel.
os-image: boot_sect.bin kernel.bin
	cat $^ > $@

# Assemble the boot sector to raw machine code.
# @remark nasm reports file not found errors with "fatal: unable to open include
# file"
boot_sect.bin:	boot/boot_sect.asm boot/print_string.asm boot/disk_load.asm\
				$(TEST_OBJ_FILES)
	nasm -O0 $< -I 'boot/' -f bin -o $@

# @IMPORTANT kernel_entry.o must go first here.
kernel.bin: kernel_entry.o kernel.o screen.o low_level.o idt.o test.o\
			$(TEST_OBJ_FILES)
	$(LD) -O0 -o $@ -Ttext 0x1000 $^ --oformat binary -e 0x1000

idt.o: kernel/idt.c kernel/idt.h
	$(CC)  $(CC_FLAGS) -c $< -o $@

test.o: kernel/test.s kernel/test.h
	nasm -O0 $< -f elf -o $@

# @IMPORTANT:The % operator does not match sub-directories, hence `kernel/%.c`.
kernel.o: kernel/kernel.c
ifdef TEST_MODE
	$(CC) -DTEST_MODE $(CC_FLAGS) -c $< -o $@
else
	$(CC) $(CC_FLAGS) -c $< -o $@
endif

%.o: drivers/%.c drivers/%.h
	$(CC) $(CC_FLAGS) -c $< -o $@

# @remark The use of -masm=intel changes the syntax for the inline assembly from
# GAS to NASM.
low_level.o: kernel/low_level.c kernel/low_level.h
	$(CC) $(CC_FLAGS) -c $< -o $@

kernel_entry.o: kernel/kernel_entry.asm
	nasm -O0 $< -f elf -o $@

# Disassemble our kernel - might be useful for debugging.
kernel.dis: kernel.bin
	ndisasm -b 32 $< > $@

clean:
	rm -Rf *.bin *.o os-image

# @TODO add stdlib.c etc.

############## Testing #########################################################
%.o: tests/%.c tests/%.h
	$(CC) $(CC_FLAGS) -c $< -o $@

%.o: include/%.c include/%.h
	$(CC) $(CC_FLAGS) -c $< -o $@
################################################################################