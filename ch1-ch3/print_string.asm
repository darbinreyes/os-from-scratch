; Print string function , arg = bx
print_string:
    pusha ; Push all register values on the stack
    mov ah, 0x0e ; int 0x10, BIOS scrolling teletype mode.
next_char:
    mov cx, [bx] ; bx register stores our argument, the address of the first
                 ; character to be printed.
    cmp cl, 0    ; Check for the NULL terminator of the string.
    je done      ; Reached end of the string, done.
    mov al, cl
    int 0x10     ; print(al)
    add bx, 1    ; Move on to the next character.
    jmp next_char
done:
    popa ; Pop all register value off the stack.
    ret ; Return to caller.
