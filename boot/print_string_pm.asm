; @abstract
; Printing routine for 32-bit protected mode. We cannot use the BIOS ISRs after
; the switch into protected mode.
;
; @discussion
; A very simple printing routine for use in 32 bit protected mode since we won't
; be able to use the BIOS once we make the switch into 32 bit protected mode.
; This function doesn't scroll, all prints start at the upper left corner. The
; address of the first character of the string to be printed should be passed in
; EBX.
;

VIDEO_MEMORY equ 0xb8000 ; Address of the first pair of bytes (ASCII code,
                         ; character attributes) for writing to the screen
                         ; device in text mode. It corresponds to the upper-left
                         ; character cell.

WHITE_ON_BLACK equ 0x0f  ; Character attributes - white foreground, black
                         ; background.

[bits 32]                ; NASM assembler directive - generate code to be
                         ; executed in 32-bit mode.

; @procedure    print_string_pm    Procedure for printing a null terminated
;                                  string to the screen in 32-bit protected
;                                  mode. Printing always starts at the
;                                  upper-left character cell.
;
; @register    EBX                 The address of the first character of a null
;                                  terminated ASCII string.
;
print_string_pm:
    pusha
    mov edx, VIDEO_MEMORY ; Point EDX to start of video memory (in text mode).

    print_string_pm_loop:

    mov al, [ebx]          ; Store the character at EBX in AL. AL := *EBX.
    mov ah, WHITE_ON_BLACK ; Store the character attributes in AH.
                           ; AH := WHITE_ON_BLACK.

    cmp al, 0              ; Check if this is the end of the string.
    je print_string_pm_done

    mov [edx], ax          ; Store the character and its attributes at the
                           ; current character cell.

    add ebx, 1              ; Advance to the next character of the string.
    add edx, 2              ; Advance to the next pair of bytes in video memory.

    jmp print_string_pm_loop

    print_string_pm_done:

    popa
    ret

