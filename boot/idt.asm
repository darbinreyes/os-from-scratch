; Our interrupt descriptor table (IDT).
; This configures ...


; - - - - - - - Table 6-1. Exceptions and Interrupts - Intel SDM Vol.1.Ch.6.5.1 - - - - - -
;
; Vector | Mnemonic | Description | Source |
; -------|----------|-------------|--------|
; 0        |  #DE     | Divide Error                               | DIV and IDIV instructions.
; 1        |  #DB     | Debug                                      | Any code or data reference.
; 2        |          | NMI Interrupt                              | Non-maskable external interrupt.
; 3        |  #BP     | Breakpoint                                 | INT3 instruction.
; 4        |  #OF     | Overflow                                   | INTO instruction.
; 5        |  #BR     | BOUND Range Exceeded                       | BOUND instruction.
; 6        |  #UD     | Invalid Opcode (Undefined Opcode)          | UD instruction or reserved opcode.
; 7        |  #NM     | Device Not Available (No Math Coprocessor) | Floating-point or WAIT/FWAIT instruction.
; 8        |  #DF     | Double Fault                               | Any instruction that can generate an exception, an NMI, or an INTR.
; 9        |  #MF     | CoProcessor Segment Overrun (reserved)     | Floating-point instruction.^1
; 10       |  #TS     | Invalid TSS                                | Task switch or TSS access.
; 11       |  #NP     | Segment Not Present                        | Loading segment registers or accessing system segments.
; 12       |  #SS     | Stack Segment Fault                        | Stack operations and SS register loads.
; 13       |  #GP     | General Protection                         | Any memory reference and other protection checks.
; 14       |  #PF     | Page Fault                                 | Any memory reference.
; 15       |          | Reserved                                   | ~ ~ ~
; 16       |  #MF     | Floating-Point Error (Math Fault)          | Floating-point or WAIT/FWAIT instruction.
; 17       |  #AC     | Alignment Check                            | Any data reference in memory.^2
; 18       |  #MC     | Machine Check                              | Error codes (if any) and source are model dependent.^3
; 19       |  #XM     | SIMD Floating-Point Exception              | SIMD Floating-Point Instruction^4
; 20       |  #VE     | Virtualization Exception                   | EPT violations^5
; 21       |  #CP     | Control Protection Exception               | The RET, IRET, RSTORSSP, and SETSSBSY instructions can generate this exception. When CET indirect branch tracking is enabled, this exception can be generated due to a missing ENDBRANCH instruction at the target of an indirect call or jump.
; 22 - 31  |          | Reserved                                   | ~ ~ ~
; 32 - 255 |          | Maskable Interrupts                        | External interrupt from INTR pin or INT n instruction.
;
; - - - - - - -
; NOTES:
; ^1. IA-32 processors after the Intel386 processor do not generate this exception.
; ^2. This exception was introduced in the Intel486 processor.
; ^3. This exception was introduced in the Pentium processor and enhanced in the P6 family processors.
; ^4. This exception was introduced in the Pentium III processor.
; ^5. This exception can occur only on processors that support the 1-setting of the “EPT-violation #VE” VM-execution control.
; "~ ~ ~" means no text given by Intel.
; - - - - - - -

; - - - - - - -
; DPL      ; Descriptor Privilege Level.
; Offset   ; Offset to procedure entry point.
; P        ; Segment Present Flag. 0b1 = present. 0b0 = not present.
; Selector ; Segment selector for destination code segment. This should be an offset into the GDT.
; D        ; Size of gate. 1 = 32 bits. 0 = 16-bits.
; - - - - - - -





; A **interrupt** gate descriptor without an exception handler procedure.
; - - - - - - -
; Offset   ; low order ; Offset to procedure entry point ; 0x00`00.
; Selector ; Segment Selector for destination code segment 0x00`08.
; Reserved ; 0b0`0000.
; constant ; 0b000.
; constant ; 0b110.
; D ; Size of gate ; 1 = 32-bits ;  0b1.
; constant ; 0b0.
; DPL ; Descriptor Privilege Level ; 0b00.
; P ; Segment Present Flag ; 0b0.
; Offset ; high order ; Offset to procedure entry point ; 0x00`00.
;- - - - - - -
; A **trap** gate descriptor without an exception handler procedure.
; - - - - - - -
; Offset   ; low order ; Offset to procedure entry point ; 0x00`00.
; Selector ; Segment Selector for destination code segment 0x00`08.
; Reserved ; 0b0`0000.
; constant ; 0b000.
; constant ; 0b111.
; D ; Size of gate ; 1 = 32-bits ;  0b1.
; constant ; 0b0.
; DPL ; Descriptor Privilege Level ; 0b00.
; P ; Segment Present Flag ; 0b0.
; Offset ; high order ; Offset to procedure entry point ; 0x00`00.
;- - - - - - -
; Reminder Intel CPUs are Little-Endian.
;- - - - - - -
;dw 0x0000          ; Offset low (15<-0)
;dw 0x0000          ; Segment Selector (31<-16)
; 4-byte boundary.
;db 00000000b       ; constant (7<-5), Reserved (4<-0)
;db 00001110b       ; P (15)  DPL (14<-13) constant (12)  D (11) constant (10<-8)
;dw 0x0000          ; Offset high (31<-16)


; [ ] .align 8? iSDM.Vol.3.Ch.6.11.
idt_start:
;- - - - - - - vector 0 - - - - - - -;
dw v_0_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 1 - - - - - - -;
dw v_1_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 2 - - - - - - -;
dw v_2_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 3 - - - - - - -;
dw v_3_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 4 - - - - - - -;
dw v_4_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 5 - - - - - - -;
dw v_5_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 6 - - - - - - -;
dw v_6_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 7 - - - - - - -;
dw v_7_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 8 - - - - - - -;
dw v_8_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 9 - - - - - - -;
dw v_9_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 10 - - - - - - -;
dw v_10_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 11 - - - - - - -;
dw v_11_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 12 - - - - - - -;
dw v_12_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 13 - - - - - - -;
dw v_13_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 14 - - - - - - -;
dw v_14_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 15 - - - - - - -; (Reserved)
dw 0x0000
dw 0x0000
; 4-byte boundary.
db 00000000b
db 00001110b
dw 0x0000
;- - - - - - - vector 16 - - - - - - -;
dw v_16_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 17 - - - - - - -;
dw v_17_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 18 - - - - - - -;
dw v_18_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 19 - - - - - - -;
dw v_19_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 20 - - - - - - -;
dw v_20_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 21 - - - - - - -;
dw v_21_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 22 - - - - - - -;(Reserved)
dw 0x0000
dw 0x0000
; 4-byte boundary.
db 00000000b
db 00001110b
dw 0x0000
;- - - - - - - vector 23 - - - - - - -;(Reserved)
dw 0x0000
dw 0x0000
; 4-byte boundary.
db 00000000b
db 00001110b
dw 0x0000
;- - - - - - - vector 24 - - - - - - -;(Reserved)
dw 0x0000
dw 0x0000
; 4-byte boundary.
db 00000000b
db 00001110b
dw 0x0000
;- - - - - - - vector 25 - - - - - - -;(Reserved)
dw 0x0000
dw 0x0000
; 4-byte boundary.
db 00000000b
db 00001110b
dw 0x0000
;- - - - - - - vector 26 - - - - - - -;(Reserved)
dw 0x0000
dw 0x0000
; 4-byte boundary.
db 00000000b
db 00001110b
dw 0x0000
;- - - - - - - vector 27 - - - - - - -;(Reserved)
dw 0x0000
dw 0x0000
; 4-byte boundary.
db 00000000b
db 00001110b
dw 0x0000
;- - - - - - - vector 28 - - - - - - -;(Reserved)
dw 0x0000
dw 0x0000
; 4-byte boundary.
db 00000000b
db 00001110b
dw 0x0000
;- - - - - - - vector 29 - - - - - - -;(Reserved)
dw 0x0000
dw 0x0000
; 4-byte boundary.
db 00000000b
db 00001110b
dw 0x0000
;- - - - - - - vector 30 - - - - - - -;(Reserved)
dw 0x0000
dw 0x0000
; 4-byte boundary.
db 00000000b
db 00001110b
dw 0x0000
;- - - - - - - vector 31 - - - - - - -;(Reserved)
dw 0x0000
dw 0x0000
; 4-byte boundary.
db 00000000b
db 00001110b
dw 0x0000
;- - - - - - - vector 32 - - - - - - -;
dw v_32_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - vector 33 - - - - - - -;
dw v_33_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
idt_end:

; IDT register
idt_register: ; 16-bit IDT limit and 32-bit IDT base.

dw idt_end - idt_start - 1 ; Limit is ALWAYS LESS 1 OF THE TRUE SIZE.
dd idt_start ; Starting address of our IDT.


; Issue special instruction to load the IDT register.
load_idtr:
lidt [idt_register]
sti
;int 0
;int 1
;int 20
;int 21
ret

; /* ICW1 */
; outb( 0x11, 0x20 ); /* Master port A */
; outb( 0x11, 0xA0 ); /* Slave port A */

; /* ICW2 */
; outb( 0x20, 0x21 ); /* Master offset of 0x20 in the IDT */
; outb( 0x28, 0xA1 ); /* Master offset of 0x28 in the IDT */

; /* ICW3 */
; outb( 0x04, 0x21 ); /* Slaves attached to IR line 2 */
; outb( 0x02, 0xA1 ); /* This slave in IR line 2 of master */

; /* ICW4 */
; outb( 0x05, 0x21 ); /* Set as master */
; outb( 0x01, 0xA1 ); /* Set as slave */

; /* Only listen to irqs 0, 1, and 2 */
; outb( 0xf8, 0x21 ); /* master PIC */
; outb( 0xff, 0xA1 ); /* slave PIC */


; /* Send EOI to both master and slave */
; outb( 0x20, 0x20 ); /* master PIC */
; outb( 0x20, 0xA0 ); /* slave PIC */


    ; __asm__("in al, dx" : "=a" (result) : "d" (port) );
    ; __asm__("out dx, al" : :"a" (data), "d" (port) );

MASTER_PIC_PORT_A equ 0x0020
MASTER_PIC_PORT_B equ 0x0021

SLAVE_PIC_PORT_A equ 0x00A0
SLAVE_PIC_PORT_B equ 0x00A1

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

[extern v_0_print]
v_0_handler_procedure:
call v_0_print
iret

[extern v_1_print]
v_1_handler_procedure:
call v_1_print
iret

[extern v_2_print]
v_2_handler_procedure:
call v_2_print
iret

[extern v_3_print]
v_3_handler_procedure:
call v_3_print
iret

[extern v_4_print]
v_4_handler_procedure:
call v_4_print
iret

[extern v_5_print]
v_5_handler_procedure:
call v_5_print
iret

[extern v_6_print]
v_6_handler_procedure:
call v_6_print
iret

[extern v_7_print]
v_7_handler_procedure:
call v_7_print
iret

[extern v_8_print]
v_8_handler_procedure:
call v_8_print
iret

[extern v_9_print]
v_9_handler_procedure:
call v_9_print
iret

[extern v_10_print]
v_10_handler_procedure:
call v_10_print
iret

[extern v_11_print]
v_11_handler_procedure:
call v_11_print
iret

[extern v_12_print]
v_12_handler_procedure:
call v_12_print
iret

[extern v_13_print]
v_13_handler_procedure:
call v_13_print
iret

[extern v_14_print]
v_14_handler_procedure:
call v_14_print
iret

; vector 15 reserved.

[extern v_16_print]
v_16_handler_procedure:
call v_16_print
iret

[extern v_17_print]
v_17_handler_procedure:
call v_17_print
iret

[extern v_18_print]
v_18_handler_procedure:
call v_18_print
iret

[extern v_19_print]
v_19_handler_procedure:
call v_19_print
iret

[extern v_20_print]
v_20_handler_procedure:
call v_20_print
iret

[extern v_21_print]
v_21_handler_procedure:
call v_21_print
iret

; vectors 22 - 31 reserved.

[extern v_32_print]
v_32_handler_procedure:
call v_32_print
iret

[extern v_33_print]
v_33_handler_procedure:
pusha
call v_33_print
; port 0x60
mov dx, 0x0060
in al, dx
call send_pic_eoi
popa
iret
