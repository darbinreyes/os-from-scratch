; @abstract
; Performs the final steps of switching the CPU mode from 16-bit real-address
; mode to 32-bit protected-mode (PM). @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter
; 2.2, 9.8 Mode Switching]
;
; @remark
; The initial steps consist in setting up the global descriptor table (GDT)
; which is currently done in gdt.asm.

STACK_ADDR_PM equ 0x90000 ; Address used to initialize the frame pointer (EBP)
                          ; and stack pointer (ESP) registers after switching
                          ; the CPU mode to 32-bit protected-mode.

[bits 16] ; NASM assembler directive - generate code to be executed in 16-bit
          ; mode.


; @procedure    switch_to_pm    Procedure to switch the CPU mode from 16-bit
;                               real-address mode to 32-bit protected mode.
;
; @label    gdt_descriptor    The address of the value to load into the global
;                             descriptor table register (GDTR).
;
; @constant    CODE_SEG    The byte offset of the code segment descriptor in the
;                          GDT.
;
; @constant    DATA_SEG    The byte offset of the data segment descriptor in the
;                          GDT.
switch_to_pm:
    cli ; Disable interrupts. Interrupts work completely differently in 32-bit
        ; protected mode. We need to configure things before we turn them on
        ; again. Not doing this will lead to a crash.

    lgdt [gdt_descriptor] ; Load the global descriptor table register (GDTR).
                          ; @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter 2.4,
                          ; Memory-Management Registers]

    mov eax, cr0 ; The lowest order bit in the CR0 control register does the
                 ; actual switch. Like the segment registers, we cannot set CR0
                 ; directly.

    or eax, 0x1  ; Set the lowest order bit.
    mov cr0, eax

    jmp CODE_SEG:init_pm ; The Intel SDM specifies that a far jump immediately
                         ; follow the MOV CR0 instruction in its description of
                         ; switching to protected mode. @doc [Intel 64 & IA-32
                         ; SDM, Vol.3, Chapter 9.9.1] This is the NASM notation
                         ; for a far jump, note the "CODE_SEG:" prefixed to the
                         ; label being jumped to. @IMPORTANT This instruction
                         ; implicitly loads the CS register.

[bits 32] ; NASM assembler directive - generate code to be executed in 32-bit
          ; mode.

; @procedure    init_pm    Procedure jumped to immediately following the CPU
;                          mode switch into 32-bit protected mode. This
;                          procedure loads all segment registers, except the
;                          code segment (CS) register, with the data segment
;                          descriptor offset into the GDT.
;
; @constant    DATA_SEG    The byte offset of the data segment descriptor in the
;                          GDT.
;
; @label    BEGIN_PM    The label of the procedure to CALL that continues 32-bit
;                       protected mode execution.
init_pm:

    mov ax, DATA_SEG ; Now in PM, our old segments are meaningless, so we point
                     ; our segment registers to the data segment descriptor in
                     ; our GDT. Note the use of the 16-bit AX register here.

    mov ds, ax ; Notice that the code segment (CS) register does not appear
               ; here. At this point the CS register should already be loaded
               ; with the code segment descriptor offset into the GDT via a
               ; `jmp CODE_SEG:init_pm` instruction. @doc [Writing a Simple
               ; Operating System - from Scratch, by Nick Blundell, Chapter 4.4]
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, STACK_ADDR_PM ; Stack init.
    mov esp, ebp

    call BEGIN_PM ; Call the procedure that continues 32-bit protected mode
                  ; execution. If no errors occur, this procedure should not
                  ; return. It should transfer control to the kernel.
