; Performs the final steps of switching the CPU from 16 bit real mode to
; 32 bit protected mode.
[bits 16] ; We start in 16 bit real mode.
switch_to_pm:

cli ; Disable interrupts. Interrupts work completely differently in 32 bit
; protected mode. We need to configure things before we turn them on again.
; Not doing this will lead to a crash.

lgdt [gdt_descriptor] ; Load our basic flat model global descriptor table.

mov eax, cr0 ; The lowest order bit in the CR0 control register does the actual switch.
; Like the segment registers, we cannot set CR0 directly.
or eax, 0x1 ; Set the lowest order bit.
mov cr0, eax

jmp CODE_SEG:init_pm ; Use a far jump to trigger a pipeline flush, this is a
; safety precaution that is standard procedure when switching CPU modes. Recall
; that CODE_SEG is just an offset into our GDT.

[bits 32] ; Tell the assembler to output 32 bit encoded instructions.

; Initialize segment registers and the stack once in protected mode (PM).

init_pm:

mov ax, DATA_SEG; Now in PM, our old segments are meaningless, so we point our segment
; registers to the data segment descriptor in our GDT.
; Note the use of the 16 bit AX register here.
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

; Stack init.
mov ebp, 0x90000
mov esp, ebp

call BEGIN_PM ; Finally, call some well known label.
