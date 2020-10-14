; A very simple printing routine for use in 32 bit protected mode since
; we won't be able to use the BIOS once we make the switch into 32 bit protected mode
; This function doesn't scroll, all prints start at the upper left corner.
; The address of the first character of the string to be printed should be passed
; in EBX.

[bits 32]

; Useful constants.
VIDEO_MEMORY equ 0xb8000 ; Address of the 1st pair of bytes (ASCII code +
; character attributes) for the screen device in text mode.
WHITE_ON_BLACK equ 0x0f ; Character attributes - fore/back-ground color.

; print a null terminated string point to by EBX
print_string_pm:
pusha
mov edx, VIDEO_MEMORY ; Point EDX to start of video memory for text mode.

print_string_pm_loop:

mov al, [ebx] ; Store the character at EBX in AL.
mov ah, WHITE_ON_BLACK ; Store the character attributes in AH

cmp al, 0 ; Check if this is the end of the string.
je print_string_pm_done

mov [edx], ax ; Store the character and its attributes at the current character
; cell.

add ebx, 1 ; Advance to the next character of the string.
add edx, 2 ; Advance to the next pair of bytes in video memory text mode.

jmp print_string_pm_loop ; Repeat.

print_string_pm_done:

popa
ret

