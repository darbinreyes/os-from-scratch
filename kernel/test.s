;  - - - - - - - - -
; How to call assembly code from C code.
; From https://littleosbook.github.io/#moving-the-cursor
; The out assembly code instruction can’t be executed directly in C. Therefore it is a good idea to wrap out in a function in assembly code which can be accessed from C via the cdecl calling standard [25]:
;  - - - - - - - - -
; @ref https://wiki.osdev.org/System_V_ABI#i386
; Functions preserve the registers ebx, esi, edi, ebp, and esp;
;  while eax, ecx, edx are scratch registers.
; The return value is stored in the eax register, or if it is a 64-bit value, then the higher 32-bits go in edx.
; Functions push ebp such that the caller-return-eip is 4 bytes above it, and set ebp to the address of the saved ebp.
; This allows iterating through the existing stack frames.
; - - -  - - -  - - -
; [ ] TODO: Draw stack frame layout here.
; - - -  - - -  - - -

global load_idt_reg  ; make the label load_idt_reg visible outside this file


; load_idt_reg - Load the IDT register.
;        [esp + 4] Address of the value to be loaded into the IDT register.
;        [esp    ] return address
load_idt_reg:
    mov eax, [esp + 4]
    lidt [eax]
    sti
    ret

; - - -  - - -  - - -
; @ref Intel SDM vol.3 ch. 6.13
;
; Note that the error code is **not** popped when the IRET instruction is executed to return from an exception handler, so the handler must remove the error code before executing a return.
;
; * Error codes are **not** pushed on the stack for exceptions that are generated externally (with the INTR or LINT[1:0] pins) or the INT n instruction, even if an error code is normally produced for those exceptions.
;   * "INTR pins" means external interrupt, therefore this applies to vectors 32 - 255.
; - - -  - - -  - - -
; @ref [Macros in NASM and defining interrupt/exception handlers with common code.](https://littleosbook.github.io/#interrupts-and-input).
; - - -  - - -  - - -

; intr_handler_no_err_code - Interrupt handler entry point with no error code on the stack.
;
; @ref Intel SDM Vol.3 Figure 6-4.
;
;        [esp + 12]        <- ESP before transfer to handler.
;        [esp + 8 ] EFLAGS
;        [esp + 4 ] CS
;        [esp     ] EIP    <- ESP after transfer to handler.
%macro intr_handler_no_err_code 1
global intr_v%1_handler
intr_v%1_handler:
    push dword 0                    ; push 0 as the error code.
    ;push ebp
    ;mov  ebp, esp
    push dword %1                   ; push vector number.
    jmp intr_common_handler              ; jump to common handler.
%endmacro

; intr_handler_with_err_code - Interrupt handler entry point with error code pushed onto the stack.
;
; @ref Intel SDM Vol.3 Figure 6-4.
;
;        [esp + 16]           <- ESP before transfer to handler.
;        [esp + 12] EFLAGS
;        [esp + 8 ] CS
;        [esp   4 ] EIP
;        [esp     ] Error Code <- ESP after transfer to handler.
%macro intr_handler_with_err_code 1
global intr_v%1_handler
intr_v%1_handler:
    ;push ebp
    ;mov  ebp, esp
    push dword %1                        ; push vector number.
    jmp intr_common_handler              ; jump to common handler.
%endmacro

[extern intr_handler]

; intr_common_handler - Code common to all interrupt/exception handlers. We jump (not call) here from the wrappers intr_handler_no_err_code/intr_handler_err_code.
;        [esp + 4] Error code.
;        [esp    ] Vector number.
intr_common_handler:
    ; Save registers?


    call intr_handler    ; Call the C function. Error code and Vector number are on the stack.

    ; Restore registers?

    add esp, 8

    iret ; **NOTE: This is not the usual `ret`.** Interrupt specific return.

intr_handler_no_err_code 0
intr_handler_no_err_code 1
intr_handler_no_err_code 2
intr_handler_no_err_code 3
intr_handler_no_err_code 4
intr_handler_no_err_code 5
intr_handler_no_err_code 6
intr_handler_no_err_code 7
intr_handler_no_err_code 8
intr_handler_no_err_code 9
intr_handler_no_err_code 10
intr_handler_no_err_code 11
intr_handler_no_err_code 12
intr_handler_no_err_code 13
intr_handler_no_err_code 14
; intr_handler_no_err_code 15 ; RESERVED
intr_handler_no_err_code 16
intr_handler_no_err_code 17
intr_handler_no_err_code 18
intr_handler_no_err_code 19
intr_handler_no_err_code 20
intr_handler_no_err_code 21

; intr_handler_no_err_code 22 ; RESERVED
; intr_handler_no_err_code 23
; intr_handler_no_err_code 24
; intr_handler_no_err_code 25
; intr_handler_no_err_code 26
; intr_handler_no_err_code 27
; intr_handler_no_err_code 28
; intr_handler_no_err_code 29
; intr_handler_no_err_code 30
; intr_handler_no_err_code 31

intr_handler_no_err_code 32
;intr_handler_no_err_code 33

MASTER_PIC_PORT_A equ 0x0020
MASTER_PIC_PORT_B equ 0x0021

SLAVE_PIC_PORT_A equ 0x00A0
SLAVE_PIC_PORT_B equ 0x00A1

global init_pics

init_pics:
    pusha
    ; ICW1
    mov dx, MASTER_PIC_PORT_A
    mov al, 0x11
    out dx, al
    mov dx, SLAVE_PIC_PORT_A
    out dx, al
    ; ICW2
    mov dx, MASTER_PIC_PORT_B
    mov al, 0x20
    out dx, al
    mov dx, SLAVE_PIC_PORT_B
    mov al, 0x28
    out dx, al
    ; ICW3
    mov dx, MASTER_PIC_PORT_B
    mov al, 0x04
    out dx, al
    mov dx, SLAVE_PIC_PORT_B
    mov al, 0x02
    out dx, al
    ; ICW4
    mov dx, MASTER_PIC_PORT_B
    mov al, 0x05
    out dx, al
    mov dx, SLAVE_PIC_PORT_B
    mov al, 0x01
    out dx, al
    ; Set interrupt mask - which IRQs to listen to and not listen to.
    mov dx, MASTER_PIC_PORT_B
    mov al, 0xfd
    out dx, al ; d = 1101b
    mov dx, SLAVE_PIC_PORT_B
    mov al, 0xff
    out dx, al

    popa
    ret



; Send pic end of interrupt (EOI) byte.
send_pic_eoi:
    pusha
    mov al, 0x20
    mov dx, MASTER_PIC_PORT_A
    out dx, al
    mov dx, SLAVE_PIC_PORT_A
    out dx, al
    popa
    ret

global intr_v33_handler

intr_v33_handler:
    ;pusha
    push dword 0
    push dword 33
    call intr_handler
    mov dx, 0x0060
    in al, dx      ; Read keyboard output buffer.
    call send_pic_eoi
    ;popa
    add esp, 8
    iret