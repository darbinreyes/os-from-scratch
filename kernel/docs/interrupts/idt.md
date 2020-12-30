# Related docs

* @doc [How to call assembly code from C code.]
       (https://littleosbook.github.io/#moving-the-cursor)
  * "The out assembly code instruction can’t be executed directly in C. Therefore
    it is a good idea to wrap out in a function in assembly code which can be
    accessed from C via the cdecl calling standard [25]"
* @doc [System V ABI](https://wiki.osdev.org/System_V_ABI#i386)
  * Functions preserve the registers ebx, esi, edi, ebp, and esp
  * while eax, ecx, edx are scratch registers.
  * The return value is stored in the eax register, or if it is a 64-bit value,
  * then the higher 32-bits go in edx.
  * Functions push ebp such that the caller-return-eip is 4 bytes above it, and
    set ebp to the address of the saved ebp.
  * This allows iterating through the existing stack frames.
* @doc [Macros in NASM and defining interrupt/exception handlers with common
       code.] (https://littleosbook.github.io/#interrupts-and-input).
* @doc [i386 Interrupt Handling w/ Programmable Interrupt Controller]
        (./docs/interrupts/sigops_i386_Interrupt_Handling.pdf)

* NASM pointers
  * @doc [NASM preprocessor multi-line macros](NASM manual ch.4.3)

* Intel SDM pointers
  * @doc [Errors codes and the stack](Intel 64 & IA-32 Arch. SDM Vol.3 Ch.6.13)
  * @doc [Stack Usage on Transfers to Interrupt and Exception-Handling Routines]
         (Intel 64 & IA-32 Arch. SDM Vol.3 Ch.6.12.1 Figure 6-4)

- - -

# @doc [Intel 8259A chip datasheet - Details on control words](./docs/interrupts/intel-8259a-pic.pdf)

* @TODO

- - -

# @doc [Brief recipe for configuring the 8259A PIC](./docs/interrupts/sigops_i386_Interrupt_Handling.pdf)

* When properly configured the 8259A PIC can be used to generate hardware
interrupts that the invoke interrupt handlers in the Interrupt Descriptor
Table (IDT).
* Summary of 8259A functional behavior
  * 8 input pins - a device will use one of these pins to request the CPU's
    attention. These pins are sometimes called "IRQ 0-7" (IRQ = Interrupt
    Request).
  * 1 interrupt line - connected to the CPU, used by the 8259A to request
    the CPU's attention.
  * A mechanism for the CPU to query the 8259A for the IDT vector associated
    with the interrupt that occurred.
  * When the 8259A raises the interrupt line, the net effect is that the CPU
    will invoke the appropriate interrupt handler in the IDT.
  * To support more than 8 distinct interrupts, the 8259A can be connected
    to an additional 8259A in a so called "master/slave" configuration. This
    is the setup we have in bochs. So we actually have 2x 8259A chips, the
    first called the master, the second the slave.

- - -
# @doc [8259 PIC -  concise additional info.](https://wiki.osdev.org/PIC)
* FYI: The "APIC" mechanism supersedes the 8252A PIC. It is designed for
  multiprocessor systems.
* If multiple interrupts occur, they are sent to the CPU in FIFO order.
* When cascaded, the 8259A must give up 1 of its input lines. Hence with 2 PICS,
  15, not 16, interrupts are supported.
  * Using input pin 2 for cascading is an IBM AT convention.
* If the IRQ came from the Master PIC, it is sufficient to issue this command
  only to the Master PIC; however if the IRQ came from the Slave PIC, it is
  necessary to issue the command to both PIC chips.
* io_wait() may be necessary on older machines.
* If you are going to use the processor local APIC and the IOAPIC, you must
  first disable the PIC.
* PIC registers: IMR = interrupt mask register, ISR = In-Service Register,
  IRR = Interrupt Request Register. OCW3 command word to read registers. Note
  on bit 2 being set because slave PIC is connected to that input pin.
  * Distinction: ISR = IRQs have been sent to the CPU vs. IRR = IRQs that have
    been raised and will be sent to the CPU in the future.
* Masking IRQ2 will cause the slave PIC to stop raising IRQs.
* PICs associate a priority to each IRQ. The highest priority is IRQ 0, the
  lowest IRQ 7.
* Spurious IRQs. Sometimes, the PIC will tell the CPU an interrupt occurred but
  the "IRQ disappears" before the PIC sends the CPU the interrupt vector number.
  At this point the CPU is waiting for the interrupt vector number from the PIC,
  but what interrupt vector number should the PIC send for a disappeared IRQ?
  The answer is that by specification, the PIC will send the lowest priority
  interrupt vector number, viz. IRQ 7 for the master PIC, IRQ 15 for the slave
  PIC. Such an IRQ is called a "Spurious IRQ". Causes include sending an EOI at
  the wrong time or noise on IRQ lines.
* Handling Spurious IRQs.
  * The PIC's IS-Register will not be set for spurious IRQs.
    * The interrupt handler should **not** send an EOI to the PIC.
  * We must handle IRQ 7 and IRQ 15 more carefully.
    * If IRQ 7 occurs, first check the IS-Register to determine if it is a real
      vs. spurious interrupt. If it is spurious do not send an EOI and bail on
      any further actions.
    * If IRQ 15 occurs, the handling is similar but we must account for the fact
      that this is from the slave PIC, which is chained. Again, first check the
      IS-Register to determine if it is a real vs. spurious interrupt. If it is
      spurious send an EOI only to the master PIC, do not send an EOI to the
      slave PIC.
* It is useful to maintain a count of the number of spurious IRQs that occur to
  detect software or hardware problems.

- - -
# @doc [Table mapping IRQs to devices](http://www.brokenthorn.com/Resources/OSDev16.html)

---------------------------------------------------------------------------------
 IRQ # | 16-bit mode vector number | 32-bit mode vector number | Description
-------|---------------------------|---------------------------|-----------------
 0     | 0x08                      | 0x20 = 32                 | Timer
 1     | 0x09                      | 0x21 = 33                 | Keyboard
 2     | 0x0A                      | 0x22 = 34                 | Cascade for slave PIC
 3     | 0x0B                      | 0x23 = 35                 | Serial port 2
 4     | 0x0C                      | 0x24 = 36                 | Serial port 1
 5     | 0x0D                      | 0x25 = 37                 | AT systems: Parallel Port 2. PS/2 systems: reserved
 6     | 0x0E                      | 0x26 = 38                 | Diskette drive
 7     | 0x0F                      | 0x27 = 39                 | Parallel Port 1
 8/0   | 0x70                      | 0x28 = 40                 | CMOS Real time clock
 9/1   | 0x71                      | 0x29 = 41                 | CGA vertical retrace
 10/2  | 0x72                      | 0x30 = 42                 | Reserved
 11/3  | 0x73                      | 0x31 = 43                 | Reserved
 12/4  | 0x74                      | 0x32 = 44                 | AT systems: reserved. PS/2: auxiliary device
 13/5  | 0x75                      | 0x33 = 45                 | FPU
 14/6  | 0x76                      | 0x34 = 46                 | Hard disk controller
 15/7  | 0x77                      | 0x35 = 47                 | Reserved
--------------------------------------------------------------------------------
- - -

# Evernotes - Intel SDM Vol.3 Ch.6 - INTERRUPT AND EXCEPTION HANDLING

* Sources of interrupts - external (HW) - software-generated. - interruption of normal program execution.

* External
  * pins on the processor.
  * or
  * through local APIC
    * specifically, the LINT[1:0] pins are connected to the local APIC.
    * local APIC, if enabled, can be programmed via the APIC’s local vector table (LVT), which in turn are mapped to the CPU’s IDT.

* Sources of Exceptions
  * CPU detected program error.
  * SW generated.
  * Machine-check

* Interrupt Descriptor Table (IDT)
  * vector -> gate descriptor
  * 8 bytes each.
  * 256 entries MAX.
  * Empty entries - Present Flag = 0.
  * @IMPORTANT - IDT 8 byte aligned.
  * IDTR register.
  * LIDT - loading requires CPL = 0.
  * Vectors accessed beyond IDT limit cause #GP.

* IDT descriptors
  * Task/Interrupt/Trap gates.
  * Formats - table 6-2.
  * Task gate in IDT, same as task gate in GDT/LDT.
  * Interrupt and trap gates differ in the use of the EFLAGS.IF flag.

* 6.12 Exception and Interrupt Handling
  * No P. level change.
  * Use IRET to return.

* 6.13 Error Code.

- - -

# @doc [INTERRUPT AND EXCEPTION HANDLING](Intel 64 & IA-32 Arch. SDM Vol.3 Ch.6)



