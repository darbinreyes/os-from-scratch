;
; Question 4. A boot sector that prints a string using our function.
;
[org 0x7c00] ; Tell the assembler where this code will be loaded.

mov bx, HELLO_MSG ; Use BX as a parameter to our function, so
                  ; we can specify the address of the string.
call print_string


mov bx, GOODBYE_MSG
call print_string

jmp $ ; infinite loop.

%include "print_string.asm"

; Data
HELLO_MSG:
    db 'Hello, World', 0 ; NULL terminated string.

GOODBYE_MSG:
    db 'Goodbye!', 0 ; NULL terminated string.

; Padding and magic number.
times 510-($-$$) db 0
dw 0xaa55