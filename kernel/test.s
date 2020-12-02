;!
; @header
; @discussion
;

;!
; @function    lidt_and_sti
;
; @param    idtr    Pointer to the value to load into the IDT register.
;
; @result Returns `idtr`.
;
; @discussion
; Loads the IDTR register using the LIDTR instruction with the value pointed to
; by `idtr` and enables interrupts using the STI instruction.
;
; @stack  [esp + 4] Arg. #1
;         [esp    ] EIP
;
; @doc [Figure 6-1. Relationship of the IDTR and IDT]
;      (Intel64 & IA-32 Arch. SDM Vol.3 Ch.6.10)
;
; |------------------|-----------|
; |47              16| 15       0|
; |------------------|-----------|
; | IDT Base Address | IDT Limit |
; |------------------|-----------|
;         |                |
;         |                v                   _IDT_
;         |-------------->(+)---------------->[     ] (N - 1) * 8
;         |                                   [     ]
;         |                                    . . .
;         |---------------------------------->[     ] 0 (Byte offset)
;
; @remark The IDT limit is 1 less than the size of the table in bytes.
;         Equivalent to the offset of the last byte of the table. Each entry is
;         8 bytes in size.
;
; `global` makes the label `lidt_and_sti` visible outside this file. When a
; corresponding function declaration is provided in a .h file, this enables you
; to call this assembly procedure from a C program.
; @doc [NASM global assembler directive](NASM manual ch.7.7).
global lidt_and_sti
lidt_and_sti:
    ; @TODO Do we need to use standard push/pop of ebp for gdb to work?
    mov eax, [esp + 4]
    lidt [eax]
    sti
    ret

;-------------------------------------------------------------------------------
; @IMPORTANT
; * Note that the error code is **not** popped when the IRET instruction is
;   executed to return from an exception handler, so the handler must remove the
;   error code before executing a return.
; * Error codes are **not** pushed on the stack for exceptions that are
;   generated externally (with the INTR or LINT[1:0] pins) or the INT n
;   instruction, even if an error code is normally produced for those
;   exceptions.
; * "INTR pins" means external interrupt, therefore this applies to vectors
;   32 - 255.
; * @doc [Errors codes and the stack](Intel64 & IA-32 Arch. SDM Vol.3 Ch.6.13)
;-------------------------------------------------------------------------------

;!
; @function    intr_v%1_handler
;
; @stack [esp + 12]        <- ESP before transfer to handler.
;        [esp + 8 ] EFLAGS
;        [esp + 4 ] CS
;        [esp     ] EIP    <- ESP after transfer to handler.
;
; @discussion
; Interrupt handler entry point with no error code on the stack. The name of the
; macro is intr_handler_no_err_code. The name of the function it defines is
; intr_v%1_handler. This function is not called explicitly, it is called by the
; CPU when the corresponding interrupt occurs.
;
; @doc [Stack Usage on Transfers to Interrupt and Exception-Handling Routines]
;      (Intel64 & IA-32 Arch. SDM Vol.3 Ch.6.12.1 Figure 6-4)
; @doc [NASM preprocessor multi-line macros](NASM manual ch.4.3)
; @doc [Error Code](Intel64 & IA-32 Arch. SDM Vol.3 Ch.6.13 Figure 6-7)
%macro intr_handler_no_err_code 1
global intr_v%1_handler
intr_v%1_handler:
    push dword 0            ; push 0 as the error code.
    push dword %1           ; push vector number.
    jmp intr_common_handler ; jump to common handler.
%endmacro

;!
; @function    intr_v%1_handler
;
; @stack [esp + 16]           <- ESP before transfer to handler.
;        [esp + 12] EFLAGS
;        [esp + 8 ] CS
;        [esp   4 ] EIP
;        [esp     ] Error Code <- ESP after transfer to handler.
;
; @discussion
; Interrupt handler entry point with error code pushed onto the stack. The name
; of the macro is intr_handler_with_err_code. The name of the function it
; defines is intr_v%1_handler. This function is not called explicitly, it is
; called by the CPU when the corresponding interrupt occurs.
%macro intr_handler_with_err_code 1
global intr_v%1_handler
intr_v%1_handler:
    push dword %1           ; push vector number.
    jmp intr_common_handler ; jump to common handler.
%endmacro

;!
; @procedure    intr_common_handler
;
; @stack [esp + 4] Error code.
;        [esp    ] Vector number.
;
; @discussion
; Code common to all interrupt/exception handlers. We jump (not call) here from
; the wrappers intr_handler_no_err_code/intr_handler_err_code.
; The function `intr_handler` is implemented in the .c file.
[extern intr_handler]
intr_common_handler:
    ; Save registers?
    call intr_handler ; Call the C function. Error code and Vector number are on
                      ; the stack.
    ; Restore registers?
    add esp, 8
    iret ; @IMPORTANT: This is not the usual `ret`. Interrupt specific return.

;-------------------------------------------------------------------------------
; @doc [Table 6-1. Protected-Mode Exceptions and Interrupts](Intel64 & IA-32 Arch. SDM Vol.3 Ch.6.2)
; Vector | Mnemonic | Description                                | Type       | Error Code | Source
; -------|----------|--------------------------------------------|------------|------------|--------------------------------------------------------------------
; 0      |  #DE     | Divide Error                               | Fault      | No         | DIV and IDIV instructions.
; 1      |  #DB     | Debug Exception                            | Fault/Trap | No         | Instruction, data, and I/O breakpoints; single-step; and others.
; 2      |  ---     | NMI Interrupt                              | Interrupt  | No         | Nonmaskable external interrupt.
; 3      |  #BP     | Breakpoint                                 | Trap       | No         | INT3 instruction.
; 4      |  #OF     | Overflow                                   | Trap       | No         | INTO instruction.
; 5      |  #BR     | BOUND Range Exceeded                       | Fault      | No         | BOUND instruction.
; 6      |  #UD     | Invalid Opcode (Undefined Opcode)          | Fault      | No         | UD instruction or reserved opcode.
; 7      |  #NM     | Device Not Available (No Math Coprocessor) | Fault      | No         | Floating-point or WAIT/FWAIT instruction.
; 8      |  #DF     | Double Fault                               | Abort      | Yes (zero) | Any instruction that can generate an exception, an NMI, or an INTR.
; 9      |  ---     | CoProcessor Segment Overrun (reserved)     | Fault      | No         | Floating-point instruction.^1
; 10     |  #TS     | Invalid TSS                                | Fault      | Yes        | Task switch or TSS access.
; 11     |  #NP     | Segment Not Present                        | Fault      | Yes        | Loading segment registers or accessing system segments.
; 12     |  #SS     | Stack-Segment Fault                        | Fault      | Yes        | Stack operations and SS register loads.
; 13     |  #GP     | General Protection                         | Fault      | Yes        | Any memory reference and other protection checks.
; 14     |  #PF     | Page Fault                                 | Fault      | Yes        | Any memory reference.
; 15     |  ---     | (Intel reserved. Do not use.)              | ---        | No         | ----
; 16     |  #MF     | x87 FPU Floating-Point Error (Math Fault)  | Fault      | No         | Floating-point or WAIT/FWAIT instruction.
; 17     |  #AC     | Alignment Check                            | Fault      | Yes (zero) | Any data reference in memory.^2
; 18     |  #MC     | Machine Check                              | Abort      | No         | Error codes (if any) and source are model dependent.^3
; 19     |  #XM     | SIMD Floating-Point Exception              | Fault      | No         | SIMD Floating-Point Instruction^4
; 20     |  #VE     | Virtualization Exception                   | Fault      | No         | EPT violations^5
; 21     |  #CP     | Control Protection Exception               | Fault      | Yes        | The RET, IRET, RSTORSSP, and SETSSBSY instructions can generate this
                                                                                             ; exception. When CET indirect branch tracking is enabled, this
                                                                                             ; exception can be generated due to a missing ENDBRANCH instruction
                                                                                             ; at the target of an indirect call or jump.
; 22-31  |  ---     | Intel reserved. Do not use.                | ---        |            | ---
; 32-255 |  ---     | User Defined (Non-reserved) Interrupts     | Interrupt  |            | External interrupt from INTR pin or INT n instruction.
;-------------------------------------------------------------------------------
; NOTES:
; ^1. Processors after the Intel386 processor do not generate this exception.
; ^2. This exception was introduced in the Intel486 processor.
; ^3. This exception was introduced in the Pentium processor and enhanced in the
;     P6 family processors.
; ^4. This exception was introduced in the Pentium III processor.
; ^5. This exception can occur only on processors that support the 1-setting of
;     the "EPT-violation #VE" VM-execution control.
;-------------------------------------------------------------------------------
intr_handler_no_err_code   0
intr_handler_no_err_code   1
intr_handler_no_err_code   2
intr_handler_no_err_code   3
intr_handler_no_err_code   4
intr_handler_no_err_code   5
intr_handler_no_err_code   6
intr_handler_no_err_code   7
intr_handler_with_err_code 8
intr_handler_no_err_code   9
intr_handler_with_err_code 10
intr_handler_with_err_code 11
intr_handler_with_err_code 12
intr_handler_with_err_code 13
intr_handler_with_err_code 14
; 15 - RESERVED
intr_handler_no_err_code   16
intr_handler_with_err_code 17
intr_handler_no_err_code   18
intr_handler_no_err_code   19
intr_handler_no_err_code   20
intr_handler_with_err_code 21
; 22 - 31 RESERVED
intr_handler_no_err_code   32
;-------------------------------------------------------------------------------


; @doc [sigops i386 Interupt Handling.pdf](./docs/interrupts)

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