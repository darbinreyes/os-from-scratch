;
; A boot sector program that uses the stack.
; ALSO SEE  boot_sector_5_q2_experiments.asm.
;

mov ah, 0x0e ; int 0x10, scrolling teletype BIOS routine.

; FYI: Since the BIOS always loads the boot sector at 0x7c00 (see figure 3.4)
; and is always 512 bytes in size, it always ends at 0x7e00. Thus 0x8000, used
; below, is safe to use.

mov bp, 0x8000
               ; Set the stack-base/frame-pointer register of the stack to an
               ; address above our boot sector, which is free memory (see figure
               ; 3.4). Notice that the lowest order bit is 0, so this address is
               ; aligned to a 2 byte boundary, as required[??] by x86 in 16 bit
               ; mode.

mov sp, bp ; By convention, our current stack is empty when the top of the
           ; stack and the bottom of the stack point to the same memory address. Hence the
           ; stack is empty when sp - bp == 0. If sp - bp is greater than 0, the difference
           ; is equal to the size in bytes of whole stack.

; Push characters onto the stack. We are in 16-bit mode, in this mode x86 push
; and pop instructions always use 16-bit (2 bytes) values. When we push a single
; character, the assembler assumes we want a 0 in the most significant byte while
; the character itself is taken as the least significant byte.

;push 'A' ; Push characters A, B, C
;push 'B'
;push 'C'

;push 0x0041
;push 0x0042
;push 0x0043

push 0x4441 ; D, A
push 0x4542 ; E, B
push 0x4643 ; F, C

;;; v2 tests.

; expect C - correct.
mov al, 'X'
mov bx, [0x7ffa]
mov al, bl
int 0x10

; expect A - correct.
mov al, 'X' ; something other than what we expect.
mov bx, [0x7ffd] ; This address is NOT 2 byte aligned. bx == 0x4100 == ‘A’, 0x00
mov al, bh
int 0x10

; expect B - correct.
; Can avoid the use of bx as a temporary register? ANS: YES. Does this print B? ANS: YES.
mov al, 'X' ; something other than what we expect.
mov al, [0x7ffc]
int 0x10

; expect F - correct.
mov al, 'X'
mov bx, [0x7ffa]
mov al, bh
int 0x10

; expect E - correct.
mov al, 'X'
mov bx, [0x7ffd]
mov al, bl
int 0x10

; expect D - correct.
mov al, 'X'
mov bx, [0x7ffe]
mov al, bh
int 0x10

; expect A - correct.
mov al, 'X'
mov bx, [0x7ffe]
mov al, bl
int 0x10

;;; End of v2 tests.

; Pop the value off stack into bx. Again, in 16-bit mode, pop, pops, a 16 bit
; value. Since we don't want to overwrite the value in ah, we use bx as a
; temporary register. We use bl to get the character value we want to print.

pop bx
mov al, bl
int 0x10 ; Print! - print(al) - prints C.

; Repeat above.

pop bx
mov al, bl
int 0x10 ; prints B.

jmp $ ; Infinite loop.

; 0 padding to 512 bytes and the magic boot sector number.

times 510-($-$$) db 0

dw 0xaa55



