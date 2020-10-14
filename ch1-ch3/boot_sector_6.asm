;
; Section 3.4.5 Control Structures. Question 3.
; Remarks:
; * Translating C if-elseif-else into x86.
; * In writing assembly it is useful to start with C code and then translate it
;   into instructions/x86.
; * Using the cmp instruction and conditional jumps.
; * cmp x, y
;   * x </<=/>/>=/==/!= y.
;

mov bx, 41

;
; Test cases.
;
; [x] bx == 30, prints B.
; [x] bx == 39, prints B.
; [x] bx == 4, prints A.
; [x] bx == 3, prints A.
; [x] bx == 40, prints C.
; [x] bx == 41, prints C.
;

; pseudo C code to translate into x86.
;
;if (bx <= 4) {
;    mov al, 'A'
;} else if (bx < 40) {
;    mov al, 'B'
;} else {
;    mov al, 'C'
;}
;

cmp bx, 4
jle print_a

cmp bx, 40
jl print_b

jmp print_c

print_a:
    mov al, 'A'
    jmp done

print_b:
    mov al, 'B'
    jmp done

print_c:
    mov al, 'C'

done:
    mov ah, 0x0e ; int_0x10(ah=0x0e) BIOS scrolling teletype mode.
    int 0x10

jmp $ ; Infinite loop.

; 0 padding to 512 bytes.
times 510-($-$$) db 0 ; db = declare byte.

dw 0xaa55 ; BIOS boot sector magic number.