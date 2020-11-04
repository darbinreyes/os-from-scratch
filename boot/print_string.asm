;
; @function.label    print_string    Procedure that prints a null terminated string at memory address BX. Uses the BIOS's interrupt service routine (ISR) via an int 0x10 instruction.
;
; @param.register    BX              The address of the first character of the string to be printed.
;

print_string:
    pusha        ; Push all register values on the stack.
    mov ah, 0x0e ; int 0x10, BIOS interrupt service routine usage convention, AH := 0x0e,
                 ; indicates BIOS scrolling teletype mode.

next_char:
    mov cl, [bx] ; Get the first, or next, character of the string. CL := *BX.

    cmp cl, 0    ; Check for the null terminator of the string.
    je done      ; Reached end of the string, done.

    mov al, cl   ; int 0x10, BIOS interrupt service routine usage convention, AL := CL,
                 ; indicates the ASCII code of the character to be printed.
    int 0x10     ; print(AL).


    add bx, 1    ; Move on to the next character. BX++.
    jmp next_char
done:
    popa         ; Pop all register value off the stack.
    ret          ; Return to caller.
