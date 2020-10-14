;
; Section 3.6.1 Extended Memory Access Using Segments.
; A boot sector program that demonstrates how to use the segment registers to
; access an address space larger than that implied by 16 bit addresses.
; 16 bit address space size = 2^16 = 64 KB.
; Segmented address space size = (max segmented address) + 1
; = (0xffff*16 + 0xffff) + 1
; = (0xffff0 + 0xffff) + 1 = (0x10ffef) + 1 = 0x10fff0.
;

mov ah, 0x0e ; int 0x10, ah = 0x0e, scrolling teletype BIOS routine.

mov al, [the_secret]
int 0x10 ; Does this print an X? No.

mov bx, 0x7c0 ; We can't load immediates directly into the segment registers.
              ; This achieves an effect equivalent to using the [org 0x7c00]
              ; assembler directive.
mov ds, bx
mov al, [the_secret] ; The ds segment register is implied here.
int 0x10 ; X? Yes.

mov al, [ds:the_secret]
int 0x10 ; X? Yes.

mov al, [es:the_secret] ; Explicitly specify that the es segment register be
                        ; used for addressing.
int 0x10 ; X? No.

mov bx, 0x7c0 ; See same instruction above.
mov es, bx
mov al, [es:the_secret]
int 0x10 ; X? Yes.

jmp $ ; Infinite loop.

the_secret:
    db 'X' ; Declare byte.

; 0 padding and magic BIOS number.
times 510-($-$$) db 0
dw 0xaa55