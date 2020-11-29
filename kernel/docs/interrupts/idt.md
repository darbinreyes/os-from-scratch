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

* Intel SDM pointers

- - -

# Evernote IDT Review - Intel SDM xxx

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