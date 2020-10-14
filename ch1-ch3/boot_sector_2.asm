;
; A boot sector that prints to the screen using a BIOS routine.
;

mov ah, 0x0e ; int 0x10, ah = 0x0e specifies the scrolling teletype BIOS routine.

mov al, 'D' ; Set al to the character we want to print
int 0x10    ; Raise the 0x10 interrupt.

mov al, 'i'
int 0x10

mov al, 'j'
int 0x10

mov al, 'k'
int 0x10

mov al, 's'
int 0x10

mov al, 't'
int 0x10

mov al, 'r'
int 0x10

mov al, 'a'
int 0x10

jmp $ ; Jump to the current instruction - infinite loop.

;
; Padding to ensure the final binary is 512 bytes in size and then end with the
; magic number 0xaa55, which tells the BIOS this is a boot sector.
;

times 510-($-$$) db 0 ; 0 Padding

dw 0xaa55 ; The magic number, "we are a boot sector".