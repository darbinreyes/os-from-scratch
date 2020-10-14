;
; A boot sector program that demos addressing.
;

mov ah, 0x0e ; scrolling teletype BIOS routine.

; 1st attempt
mov al, the_secret
int 0x10 ; Does this print 'D'?

; 2nd attempt
mov al, [the_secret]
int 0x10

; 3rd attempt
mov bx, the_secret
add bx, 0x7c00
mov al, [bx]
int 0x10

; 4th attempt
mov al, [0x7c1e]
int 0x10

jmp $ ; infinite loop.

the_secret:
    db 'D' ; The author uses double quotes here.

times 510-($-$$) db 0

dw 0xaa55