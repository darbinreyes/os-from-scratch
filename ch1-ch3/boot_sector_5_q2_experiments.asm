; This started as my version of the figure 3.6 program. When my I discovered my
; answer to question 2 was wrong, comments and test code pilled up while trying
; to figure things out. Full details in Evernote.
;
; Messy code I wrote as part of determining the stack layout in memory and
; how little endianess works in x86 with push/pop, mov al/h/bl/h in 16 bit mode.

;
; A boot sector program that uses the stack.
;

mov ah, 0x0e ; int 0x10, scrolling teletype BIOS routine.

; FYI: Since the BIOS always loads the boot sector at 0x7c00 and is always 512
; bytes in size, it always ends at 0x7e00. Thus 0x8000 used below is safe to
; use.

mov bp, 0x8000 ; Set the base/frame pointer of the stack a tad above our boot
               ; sector. Notice that the lowest order bit is 0, so this address
               ; is aligned to a 2 byte boundary, as required by x86 in 16 bit mode.

mov sp, bp ; By convention, our current stack/frame is empty when the top of the
           ; stack and the bottom of the stack point to the same memory address. Hence the
           ; stack is empty when sp - bp == 0. If sp - bp is greater than 0, the difference
           ; is equal to the size in bytes of whole stack.

; Push characters onto the stack. We are in 16-bit mode, in this mode x86 push
; and pop instructions always use 16-bit (2 bytes) values. When we push a single
; character, the assembler assumes we want a 0 in the high order byte.

push 'A' ; I have replaced the original characters A, B, C with my initials, for
         ; fun.
push 'B'
push 'C'

; (after the print immediately below) R will be printed first, then E.

; This is not part of the original given boot sector program. This is to answer
; part of question 2, which asks us to predict and confirm the absolute memory
; address of character 'C'. Note above, I have replaced 'C' with 'R'

; 1 - works
;mov al, 'X' ; Ensure sure R isn't already stored in al. Put an X there.
;mov al, [0x7ffa] ; Absolute address of R on the stack/in memory.
;int 0x10 ; print(al)

; 2
;mov al, 'X'
;mov bx, [0x7ffb]
;mov al, bl
;int 0x10

; 3 - worked
;mov al, 'X'
;mov bx, [0x7ffa]
;mov al, bl
;int 0x10

;;;; - works - prints C B A.

; mov al, 'X'
; mov al, [0x7ffa]
; int 0x10

; mov al, 'X'
; mov al, [0x7ffc]
; int 0x10

; mov al, 'X'
; mov al, [0x7ffe]
; int 0x10

;;;

;;;;

; prints a blank - probably a 0 byte - expected C.
mov al, 'X'
mov bx, [0x7ffb]
mov al, bl
int 0x10

; prints a B - expected C.
mov al, 'X'
mov bx, [0x7ffb]
mov al, bh
int 0x10

; expect B - since address is 2 byte aligned - and since little endian means 0x4200 will be read into bx - prints blank.
mov al, 'X'
mov bx, [0x7ffc]
mov al, bh
add al, 0x44 ; !!! - add 'D'
int 0x10

; same as above but with bl in place of bh. - prints B.
mov al, 'X'
mov bx, [0x7ffc]
mov al, bl
int 0x10
; The previous two prints indicate that
; mov bx, [0x7ffc]
; loads 0x0042 = 0x00,'B' into bx.
; Does this make sense?

; expect A - correct!
mov al, 'X'
mov bx, [0x7ffe]
mov al, bl
int 0x10

; expect C - correct.
mov al, 'X'
mov bx, [0x7ffa]
mov al, bl
int 0x10

;;;

; Pop the value of stack into bx. Again, in 16-bit mode, pop is pops a 16 bit
; value. Since we don't want to overwrite the value in ah, we use bx as a
; temporary register. We use bl to get the character value we want to print.
pop bx
mov al, bl
int 0x10 ; Print! - print(al)

; Repeat above.

pop bx
mov al, bl
int 0x10

jmp $ ; Infinite loop.

; 0 padding to 512 bytes and the magic boot sector number.

times 510-($-$$) db 0

dw 0xaa55



