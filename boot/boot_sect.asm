;
; @header The boot sector program.
; The BIOS transfers control to this program, called the boot sector program,
; and the boot sector program ultimately transfers control to the kernel. This
; program does the following:
; 1. Loads the kernel from the floppy a: drive into memory.
; 2. Switches the CPU mode from real-mode to protected-mode. This allows use to
;    go from executing 16-bit instructions to 32-bit instructions. The kernel is
;    compiled in 32-bit code.
; 3. Transfers control to the kernel.
;
; @discussion
; Initially, the machine is under the control of the BIOS. Towards the end of its
; execution, the BIOS will attempt to boot an operating system by following some
; well defined set of rules. The rule is, inspect all devices from which it is
; possible boot, a.k.a. boot device (e.g. a floppy device or a cd-rom device). The
; devices are inspected in an user-configurable order, called the boot order.
; To inspect a boot device the BIOS loads the first sector (512 byte block) from
; the device into memory. The BIOS transfers control to that sector (now in memory)
; if that sector that ends with the magic number 0xaa55.
;

[org 0x7c00] ;;;
; Tell assembler where this program will be loaded into memory i.e. the address
; of the first instruction. All program addresses are treated as relative to
; this value. The specific address value used here is defined by convention in
; the @doc BIOS boot spec. .

SECTOR_READ_COUNT equ 20 ;;;
; The number of sectors to read from the boot drive as part of loading the
; kernel into memory. IMPORTANT: The size of kernel must be less then this!
; The size of kernel.bin < SECTOR_READ_COUNT sectors (= SECTOR_READ_COUNT * 512
; bytes).

STACK_ADDR    equ 0x9000 ;;;
; Initial address of the frame pointer (BP) and stack pointer (SP) registers.
; The value has been chosen arbitrarily.

KERNEL_OFFSET equ 0x1000 ;;;
; This is the address at which we intend to load our kernel program. The
; specific value has been chosen arbitrarily.

mov [BOOT_DRIVE], dl ;;;
; By convention in the @doc BIOS boot spec., before transferring control to this
; program, the BIOS stores the boot drive number in the DL register. Here, we
; are storing the boot driver number in memory for later use viz. to load our
; kernel from the same drive.

mov bp, STACK_ADDR
mov sp, bp ;;;
; Initialize our frame pointer and stack pointer registers.

mov bx, STR_REAL_MODE
call print_string     ; print_string(STR_REAL_MODE).

call load_kernel ; Load our kernel from disk to memory.

call switch_to_pm ; Switch from 16 bit real mode to 32 bit protected mode.
                  ; If successful, this function will not return HERE, but
                  ; will jump instead to BEGIN_PM below.

jmp $ ; Infinite loop.

; Include any procedures we intend to use.
%include "print_string.asm"
%include "disk_load.asm"
%include "gdt.asm"
%include "print_string_pm.asm" ; FYI: Includes a [bits 32] directive.
%include "switch_to_pm.asm" ; FYI: Includes a [bits 32] directive.


[bits 16] ;;;
; This directive is necessary because some of the includes above contain the
; [bits 32] assembler directive.

load_kernel: ;;;
; Reads SECTOR_READ_COUNT sectors after the first sector into memory at address
; ES:KERNEL_OFFSET.

    mov bx, STR_LOADING_KERNEL
    call print_string          ; print_string(STR_LOADING_KERNEL).

    mov bx, 0
    mov es, bx ; ES, disk_load function argument.
    mov bx, KERNEL_OFFSET ; BX, disk_load function argument.

    mov dh, SECTOR_READ_COUNT ;;;
; DH, disk_load function argument. IMPORTANT: See note on SECTOR_READ_COUNT
; above.

    mov dl, [BOOT_DRIVE] ; DL, disk_load function argument.

    call disk_load

    ret

[bits 32] ; We jump here if we successfully switch into 32 bit protected mode.
BEGIN_PM:

mov ebx, STR_PROTECTED_MODE
call print_string_pm

call KERNEL_OFFSET ; Jump to our kernel code. It should return after printing an X.

jmp $ ; Infinite loop, in case the jump to our kernel code above fails.

; Global variables - Misc.
BOOT_DRIVE      db 0
; Global variables - Strings.
STR_REAL_MODE      db "Started in 16 bit real mode bruh.", 0xa, 0x0d, 0
STR_PROTECTED_MODE db "We are in 32 bit protected mode. Light weight!", 0
STR_LOADING_KERNEL db "About to load the kernel into memory son.", 0xa, 0x0d, 0

; Boot sector 0 padding and magic number.
times 510-($-$$) db 0
dw 0xaa55