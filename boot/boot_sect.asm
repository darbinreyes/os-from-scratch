; A boot sector program that bootstraps kernel_0.c in 32 bit protected mode.

[org 0x7c00] ; Tell assembler where our boot sector will reside in memory.

KERNEL_OFFSET equ 0x1000 ; This is where we will load our kernel.

mov [BOOT_DRIVE], dl ; By convention, the BIOS sets dl to the boot drive #.
                     ; This drive is also where our kernel resides.

mov bp, 0x9000 ; Stack init.
mov sp, bp

mov bx, MSG_REAL_MODE ; Print.
call print_string

call load_kernel ; Load our kernel from disk to memory.

call switch_to_pm ; Switch from 16 bit real mode to 32 bit protected mode.
                  ; If successful, this function will not return HERE, but
                  ; will jump instead to BEGIN_PM below.

jmp $ ; Infinite loop.


%include "print_string.asm"
%include "disk_load.asm"
%include "gdt.asm"
%include "print_string_pm.asm" ; FYI: Includes a [bits 32] directive.
%include "switch_to_pm.asm" ; FYI: Includes a [bits 32] directive.


[bits 16] ; Some of the includes above contain the [bits 32] assembler directive.

load_kernel: ; Reads 15 sectors after the first sector into memory at address KERNEL_OFFSET.

mov bx, MSG_LOAD_KERNEL ; Print.
call print_string

mov bx, KERNEL_OFFSET ; disk_load function, argument for destination address is passed in ES:BX, this assumes ES == 0.
mov dh, 20 ; disk_load function, argument for the number of sectors to read from the drive is passed in DH. 15 is an arbitrary large number given by the author. So we don't need to adjust this frequently as our kernel code grows.
mov dl, [BOOT_DRIVE] ; disk_load function, argument indicating which drive to read from is passed in DL.

call disk_load ; Note, this function skips the reading first sector, which should be our boot sector.

ret

[bits 32] ; We jump here if we successfully switch into 32 bit protected mode.
BEGIN_PM:

mov ebx, MSG_PROT_MODE
call print_string_pm

call KERNEL_OFFSET ; Jump to our kernel code. It should return after printing an X.

jmp $ ; Infinite loop, in case the jump to our kernel code above fails.

; Global variables.
BOOT_DRIVE db 0
MSG_REAL_MODE db "Started in 16 bit real mode bruh.", 0xa, 0x0d, 0
MSG_PROT_MODE db "We are in 32 bit protected mode. Light weight!", 0
MSG_LOAD_KERNEL db "About to load the kernel into memory son.", 0

; Boot sector 0 padding and magic number.
times 510-($-$$) db 0
dw 0xaa55