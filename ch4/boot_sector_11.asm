; A boot sector program that switches from 16 bit real mode to 32 bit
; protected mode.
[org 0x7c00]

mov bp, 0x9000 ; Init. the stack.
mov sp, bp

mov bx, MSG_REAL_MODE
call print_string

call switch_to_pm ; It all goes well, this doesn't return.

jmp $ ; Infinite loop.


%include "ch1-ch3/print_string.asm"
%include "gdt.asm"
%include "print_string_pm.asm"
%include "switch_to_pm.asm"

[bits 32]

; This is were we land after switching to 32 bit protected mode.

BEGIN_PM:

mov ebx, MSG_PROT_MODE
call print_string_pm ; Our 32 bit print routine.

jmp $ ; Infinite loop.

; Global variables
MSG_REAL_MODE db "Started in 16 bit Real Mode", 0
MSG_PROT_MODE db "Successfully landed in 32 bit Protected Mode", 0

; Boot sector 0 padding and magic number.

times 510-($-$$) db 0
dw 0xaa55