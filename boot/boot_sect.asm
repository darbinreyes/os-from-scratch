;!
; @header The boot sector program.
; The BIOS transfers control to this program, this program transfers control to
; the kernel.
; The steps taken by this program are:
; 1. Loads the kernel from the floppy a: drive into memory.
; 2. Switches the CPU mode from real-mode to protected-mode. This is necessary
;    because the kernel is compiled in 32-bit code.
; 3. Transfers control to the kernel.
;
; @discussion
; Initially, the machine is under the control of the BIOS. Towards the end of
; its execution, the BIOS will attempt to boot an operating system by following
; some well defined set of rules. The rule is, inspect all devices from which it
; is possible boot, a.k.a. boot devices (e.g. a floppy device or a cd-rom
; device). The devices are inspected in an user-configurable order, called the
; boot order. To inspect a boot device the BIOS loads the first sector (512 byte
; block) from the device into memory. The BIOS transfers control to that sector
; if that sector that ends with the magic number 0xaa55.

[org 0x7c00] ; Tell assembler where this program will be loaded into memory i.e.
             ; the address of the first instruction. All program addresses are
             ; treated as relative to this value. The specific address value is
             ; a BIOS convention.
             ; @doc [BIOS Boot Spec.]
             ; @doc [NASM manual chapter 8.1.1]

SECTOR_READ_COUNT equ 32 ; The number of sectors to read from the boot drive as
                         ; part of loading the kernel into memory.
                         ; @IMPORTANT:
                         ; The size of kernel.bin <= SECTOR_READ_COUNT * 512.

STACK_ADDR    equ 0x9000 ; Initial address of the frame pointer (BP) and stack
                         ; pointer (SP) registers. The value has been chosen
                         ; arbitrarily.

KERNEL_OFFSET equ 0x1000 ; This is the address at which we intend to load our
                         ; kernel. The specific value has been chosen
                         ; arbitrarily.

mov [BOOT_DRIVE], dl  ; By convention, the BIOS stores the boot drive number in
                      ; the DL register. Here, we are storing the boot driver
                      ; number in memory for later use viz. to load our kernel
                      ; from the same drive.

mov bp, STACK_ADDR    ; Initialize frame pointer and stack pointer registers.
mov sp, bp

mov bx, STR_REAL_MODE ; print_string(STR_REAL_MODE).
call print_string

call load_kernel      ; Load our kernel from disk to memory.

call switch_to_pm     ; Switch from 16-bit real mode to 32-bit protected mode.
                      ; If successful, this function will not return here, but
                      ; will jump instead to BEGIN_PM below.

jmp $                 ; Infinite loop.

;
; Include any data and procedures we intend to use.
;
%include "print_string.asm"
%include "disk_load.asm"
%include "gdt.asm"
%include "print_string_pm.asm" ; FYI: Includes a [bits 32] directive.
%include "switch_to_pm.asm"    ; FYI: Includes a [bits 32] directive.

[bits 16] ; NASM assembler directive - generate code to be executed in 16-bit
          ; mode. This directive is necessary because some of the includes above
          ; contain the [bits 32] assembler directive.
          ; @doc [NASM bits assembler directive](NASM manual ch.7.1).

;!
; @procedure    load_kernel    Loads the kernel from BOOT_DRIVE into memory at
;                              address KERNEL_OFFSET. It is assumed that the
;                              kernel is stored in the second sector of the
;                              drive. It is also assumed that the size of the
;                              kernel is less than or equal to SECTOR_READ_COUNT
;                              * 512 bytes.
load_kernel:
    mov bx, STR_LOADING_KERNEL ; print_string(STR_LOADING_KERNEL).
    call print_string

    mov bx, 0                  ; The BIOS int 0x13 ISR reads into address ES:BX.
    mov es, bx                 ; ES := 0.
    mov bx, KERNEL_OFFSET      ; BX := KERNEL_OFFSET.

    mov dh, SECTOR_READ_COUNT  ; DH := SECTOR_READ_COUNT. Number of sectors to
                               ; read @IMPORTANT: See note on SECTOR_READ_COUNT
                               ; above.

    mov dl, [BOOT_DRIVE]       ; DL := [BOOT_DRIVE]. Drive number to read from.

    call disk_load

    ret

[bits 32] ; NASM assembler directive - generate code to be executed in 32-bit
          ; mode.

;!
; @procedure    BEGIN_PM    We jump here if we successfully switch into 32-bit
;                           protected mode.
BEGIN_PM:
    mov ebx, STR_PROTECTED_MODE
    call print_string_pm

    call KERNEL_OFFSET ; Jump to the kernel's entry point.

    jmp $              ; Infinite loop. In case the kernel returns.

;
; Global variables - Misc.
;
BOOT_DRIVE         db 0

;
; Global variables - Strings.
;
STR_REAL_MODE      db "Executing in 16-bit real-address mode.", 0xa, 0x0d, 0
STR_LOADING_KERNEL db "Loading the kernel.", 0xa, 0x0d, 0

; @remark print_string_pm() does not support printing new-line characters.
STR_PROTECTED_MODE db "Executing in 32-bit protected mode. Calling kernel.", 0

; Boot sector zero-padding and BIOS magic number.
;
; @discussion
; The BIOS "magic number", 0xaa55, located at the last two bytes of a 512 byte
; sector is used by the BIOS to identify a boot program stored on a boot device.
times 510-($-$$) db 0
dw 0xaa55