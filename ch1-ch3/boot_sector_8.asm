;
; Question 5. Implementation of print_hex.
;

[org 0x7c00] ; Tell assembler where we will be loaded into memory.

mov dx, 0x1fb6 ; store the value to print in dx
call print_hex ; call the function


jmp $ ; Infinite loop.

; prints the value of DX as hex.
print_hex: ; FYI: This routine always converts and prints 4 nibbles, even if e.g. dx == 0x0000.
    ; TODO : manipulate chars at HEX_OUT to reflect DX
pusha ; save register values

mov bx, HEX_OUT ; Initialize HEX_OUT pointer
add bx, 5
mov ax, 0 ; Nibbles printed count.


next_nibble:
push dx
and dx, 0x000f ; Get the lower nibble
cmp dx, 10
jl is_digit

cmp dx, 16
jl is_hex

jmp print_hex_done ; Nibble is out of range. Should not occur.

is_hex: ; Store 'a'-'f' in cl
mov cx, dx
sub cx, 10
add cx, 'a' ; cl contains our ASCII character.
jmp update_str

is_digit: ; Store '0'-'9' in cl
mov cx, dx
add cx, '0' ; cl contains our ASCII character.
jmp update_str

update_str:
mov dx, [bx] ; Load before store to preserve the most significant byte.
mov dl, cl   ; cl contains our ASCII character.
mov [bx], dx ; Store ASCII character.

pop dx    ; Prepare for the next nibble.
shr dx, 4
sub bx, 1 ; Update HEX_OUT pointer.
add ax, 1
cmp ax, 4
jl next_nibble

mov bx, HEX_OUT ; print the string pointed to
call print_string ; by BX

print_hex_done:

popa ; restore register values
ret

%include "print_string.asm"

; global variables
HEX_OUT: db '0x0000', 0

; 0 padding and magic number.

times 510-($-$$) db 0
dw 0xaa55