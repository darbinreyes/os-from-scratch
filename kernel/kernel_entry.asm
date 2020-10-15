; An assembly program that jumps to our kernel's entry point viz. main().
; To assemble use:
; `nasm kernel_entry.asm -f elf -o kernel_entry.o`

[bits 32] ; If we reached this code, we are already in 32-bit protected mode.

[extern main] ; Declare that we will be referencing the external symbol "main".
              ; The linker will substitute the final address.

call en_ints

; To link this program with the kernel use (order of .o files is essential):
; i386-elf-gcc -ffreestanding -c kernel.c -o kernel.o
; i386-elf-ld -o kernel.bin -Ttext 0x1000 kernel_entry.o kernel.o --oformat binary

call main ; Call the main function of our C kernel.

jmp $ ; Infinite loop if/when the kernel returns.

en_ints:
lidt [idt_register]
sti
;int 21
ret

def_handler_procedure:
mov ebx, MSG_IDT
call print_string_pm
iret

%include "boot/idt.asm"
%include "boot/print_string_pm.asm"

MSG_IDT     db "vector 21 was called    ", 0