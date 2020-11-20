;!
; @header Calls the kernel's entry point function.
; The purpose of this small assembly program is to ensure that the kernel's
; entry point function is successfully called irrespective of where in the
; kernel binary the kernel's entry point happens to be located.
;
; @discussion This assembly program is taken from @doc [Writing a Simple
; Operating System - from Scratch, by Nick Blundell]. Since the boot sector
; program transfers control to the kernel by jumping to an absolute address.
; Since we don't have direct control over where the kernel's entry point is
; placed in the kernel binary, we use this program as a workaround. The only
; thing this program does is call the kernel's entry point function. It is
; essential for this program to be compiled in ELF format and for the resulting
; program to be the first object file in the list of all object files passed to
; the linker to produce the final kernel binary.
; e.g. to assemble use:
; `nasm kernel_entry.asm -f elf -o kernel_entry.o`
; to link use:
; `i386-elf-ld -o kernel.bin -Ttext 0x1000 kernel_entry.o kernel.o --oformat
;  binary`

[bits 32]
[extern main]

call main
jmp $




