###########################
# The first thing the compiler will do is preprocess the file. We can tell clang to show us what it looks like if we stop after that step:
# i386-elf-gcc  -Wall -Wextra -Werror -O0 -ffreestanding -E kernel/idt.c > idtpp.c
# Next up: parsing and code generation. We can tell clang to output the resulting assembly code like so:
# i386-elf-gcc  -Wall -Wextra -Werror -O0 -ffreestanding -S kernel/idt.c
###########################
# Build the kernel binary.
# $^ = all target dependency files.
# $< = first target dependency file.
# $@ = target file.
# THE FIRST TARGET = THE DEFAULT TARGET.
# Some of the suggestions by the author did not work e.g. his use of ${OBJ}.
# RTFM: https://www.gnu.org/software/make/manual/

# IMPORTANT: Always disable optimizations.
# IMPORTANT: Always turn on all warnings with -Wall gcc flag. "This enables all the warnings about constructions"

# Automatically expand to a list of existing files that
# match the patterns.
C_SOURCES = $(wildcard kernel/*.c drivers/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h)
OBJ = ${C_SOURCES:.c=.o}

all: os-image

run: all
	bochs -q -f bochsrc.txt # run bochs installed by mac ports.

rundbg: all
	./bochs/bochs -q -f bochsrc.txt # run bochs compiled from source. Required to use bochs' debugging features.

runq: all
	qemu-system-i386 -drive file=os-image,if=floppy,format=raw

# This is the actual disk image that the computer loads,
# which is the combination of our compiled boot sector and kernel.
os-image: boot_sect.bin kernel.bin
	cat $^ > $@

# Assemble the boot sector to raw machine code
#	The -I option tells nasm where to find our useful assembly
#	routines that we include in boot_sect.asm.
#	NOTE: nasm reports file not found errors with "fatal: unable to open include file"
boot_sect.bin: boot/boot_sect.asm boot/print_string.asm boot/disk_load.asm boot/gdt.asm boot/print_string_pm.asm boot/switch_to_pm.asm
	nasm -O0 $< -I 'boot/' -f bin -o $@

# @IMPORTANT kernel_entry.o must go first here.
kernel.bin: kernel_entry.o kernel.o screen.o low_level.o idt.o test.o
	i386-elf-ld -O0 -o $@ -Ttext 0x1000 $^ --oformat binary

idt.o: kernel/idt.c kernel/idt.h
	i386-elf-gcc  -Wall -Wextra -Werror -O0 -ffreestanding -c $< -o $@

test.o: kernel/test.s kernel/test.h
	nasm -O0 $< -f elf -o $@

#    Build the kernel object file.
#    kernel.o: kernel/kernel.c
#    i386-elf-gcc -ffreestanding -c $< -o $@
# Generic rule for building 'somefile.o' from 'somefile.c'.
# IMPORTANT: The % operator does not match sub-directories, hence `kernel/%.c`.
# kernel/%.c
kernel.o: kernel/kernel.c
	i386-elf-gcc -Wall -Wextra -Werror -O0 -ffreestanding -c $< -o $@

screen.o: drivers/screen.c drivers/screen.h
	i386-elf-gcc -Wall -Wextra -Werror -O0 -ffreestanding -c $< -o $@

keyboard.o: drivers/keyboard.c drivers/keyboard.h
	i386-elf-gcc -Wall -Wextra -Werror -O0 -ffreestanding -c $< -o $@

ps_2_ctlr.o: drivers/ps_2_ctlr.c drivers/ps_2_ctlr.h
	i386-elf-gcc -Wall -Wextra -Werror -O0 -ffreestanding -c $< -o $@

# The use of -masm=intel below changes the syntax for the inline assembly from GAS to NASM.
low_level.o: kernel/low_level.c kernel/low_level.h
	i386-elf-gcc -Wall -Wextra -Werror -O0 -ffreestanding -c $< -o $@

# Build the kernel entry object file.
# [] Add .asm files as dep. and test make reports changes.
kernel_entry.o: kernel/kernel_entry.asm
	nasm -O0 $< -f elf -o $@

# Disassemble our kernel - might be useful for debugging.
kernel.dis: kernel.bin
	ndisasm -b 32 $< > $@

clean:
	rm -Rf *.bin *.o os-image
