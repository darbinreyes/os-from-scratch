; Section 3.6.4 Putting it all Together.
; Read some sectors from the boot disk using our disk_read function.
[org 0x7c00] ; For convenience, tell the assembler where BIOS will load us into
; memory, otherwise we have to explicitly use 0x7c00 as a base address for every
; memory access we make.

mov [BOOT_DRIVE], dl ; By convention the BIOS stores the boot disk device number
; in DL. Save that value in memory. (Notice that we are storing a single byte
; here in a single instruction, even though we are in 16 bit real mode.)

mov bp, 0x8000 ; Initialize our stack base pointer to a safe location. The
               ; disk_load function uses the stack a little bit, so does
               ; print_string and print_hex.
mov sp, bp ; Stack is always initially empty.

mov bx, 0x9000 ; By convention, the BIOS int 0x13 routine uses [ES:BX] as the
; destination address in memory for the disk read. ES is 0 by default, so we
; don't need to set it explicitly.

mov dh, 2 ; Argument to disk_load function = number of sectors to read.
mov dl, [BOOT_DRIVE] ; Argument to disk_load function = disk drive number.
call disk_load ; Our function: Read from a disk into memory! This function will
               ; print a message and hang if an error occurs.

; Test if memory contains what we expect, viz. 0xdada and 0xface.

mov dx, [0x9000] ; print_hex function expects its argument to be in DX.
call print_hex

mov dx, [0x9000 + 512]
call print_hex

jmp $ ; Infinite loop.

; Include helper functions.
%include "print_string.asm"
%include "print_hex.asm"
%include "disk_load.asm"

; Global variables
BOOT_DRIVE: db 0 ; Stores the boot disk device number that the BIOS gave us in
                 ; DL.

; 0 padding and BIOS magic number.
times 510-($-$$) db 0
dw 0xaa55

; Add a 2 additional sectors in addition to our boot sector above so we can use
; them to verify that our disk read worked as expected.

times 256 dw 0xdada
times 256 dw 0xface
