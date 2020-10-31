; ##################
; How to call assembly code from C code.
; From https://littleosbook.github.io/#moving-the-cursor
; The out assembly code instruction can’t be executed directly in C. Therefore it is a good idea to wrap out in a function in assembly code which can be accessed from C via the cdecl calling standard [25]:
; ##################
global load_idt_reg  ; make the label load_idt_reg visible outside this file

    ; outb - send a byte to an I/O port
    ; stack: [esp + 8] the data byte
    ;        [esp + 4] the I/O port
    ;        [esp    ] return address


    ; load_idt_reg - Load the IDT register.
    ;        [esp + 4] Address of the value to be loaded into the IDT register.
    ;        [esp    ] return address
load_idt_reg:
    mov eax, [esp + 4]
    lidt [eax]
    ;sti
    ;int 0
    ret