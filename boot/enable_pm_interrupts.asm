; . . .

[bits 32]

enable_pm_interrupts:

lidt [idt_register]

sti

ret

%include "idt.asm"