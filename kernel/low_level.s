;!
;     @function    inb
;
;     @discussion C wrapper for the `in` instruction.
;
;     @param    port    The 16-bit  I/O port number to read from.
;
; @stack  [esp + 8] @param port
;         [esp + 4] EIP
;         [esp    ] EBP
;
global inb
inb:
    push ebp
    mov ebp, esp
    mov dx, word [esp + 8]
    xor eax, eax
    in al, dx              ; reg. <- port#
    mov esp, ebp
    pop ebp
    ret

;     @function    outb
;
;     @discussion C wrapper for the `out` instruction.
;
;     @param    port    The 16-bit I/O port number to write to.
;     @param    data    The 8-bit value to write.
;
; @stack  [esp + 12] @param data
;         [esp + 8]  @param port
;         [esp + 4]  EIP
;         [esp    ]  EBP
;
global outb
outb:
    push ebp
    mov ebp, esp
    xor eax, eax
    mov dx, word [esp + 8]
    mov al, byte [esp + 12]
    out dx, al             ; port# -> reg.
    mov esp, ebp
    pop ebp
    ret