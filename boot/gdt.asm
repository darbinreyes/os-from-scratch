; @TODO
; - [x] What is bochs physical memory setting? ANS: The default is 32MB.
; - [x] What happens if we try to read above the physical memory limit? ANS: Since we are using segment limit == 4GB, the CPU will not generate an exception.

;
; @header Global Descriptor Table (GDT).
; This file defines the GDT. Defining the GDT is a necessary prerequisite step
; to switching the CPU mode from 16-bit to 32-bit mode. The definition of the
; GDT in this file consists of two segment descriptors. A code segment
; descriptor and a data segment descriptor.
;
; @abstract Definition of the global descriptor table (GDT).
;
; @discussion
; As far as CPU is concerned, memory contains two mutually exclusive kinds of
; contents: data, or code (machine instructions).
; The CPU supports a view of memory such that different regions of memory can
; be assigned various attributes and features depending on they way in which
; that memory used. For example, these attributes may include file-like access
; flags, a region of memory can be marked as read-only. These regions of
; memory consists of contiguous arrays of bytes defined by a base address and
; limit value. For brevity, we call these regions of memory "segments". The GDT
; is Intel's mechanism for the programmer to describe various segments. Each
; entry in the GDT is an 8-byte value called a "segment descriptor".
;
;
; @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter.3 Protected-Mode Memory Management]
; @note Segmentation == mechanism of isolating individual code, data, and stack modules.
; @note In 32-bit mode, some form of segmentation is required to be used. It cannot be disabled.
; @note Linear address space == processor's addressable memory.
; @note Segmentation divides the linear address space into smaller protected address spaces.
; @note A segment can contain: data, code, stack, system data structures (e.g. a TSS or LDT).
; @note All segments are contained in the linear address space.
; @note To locate a byte in a segment use a logical address (a.k.a far pointer).
; @note logical address == pair {16-bit segment selector, 32-bit offset}.
; @note segment selector == unique identifier for a segment == offset into a descriptor table (e.g. the GDT) and other optional info.
; @note Descriptor table entries == segment descriptor.
; @note Segment descriptor == defines the segment.
; @note segment_descriptor.base_address + logical_address.offset == linear address in the linear address space.
; @note If paging is not used, linear address space == physical address space.
; @note "processor begins execution at FFFF_FFF0H."
; @note "initial base address for the DS data segment after reset initialization is 0."
; @note
; * Basic flat model hides the segmentation mechanism to the greatest extent possible.
;   * 2 descriptors. 1. code segment. 2. data segment.
;   * Both descriptors have the same base address 0H and limit of 4GB (= FFFF`FFFFH).
;   * @IMPORTANT Using limit == 4GB here will cause the CPU to silently ignore memory references outside of the physical address space. Normally, such a reference generates an exception.
;   * "ROM (EPROM) is generally located at the top of the physical address space".
;   * "because the processor begins execution at FFFF_FFF0H." Since I only have 32 MB of physical memory, the CPU must be memory mapping this part of the memory space to the ROM chip.
;   * "RAM (DRAM) is placed at the bottom of the address space because the initial base address for the DS data segment after reset initialization is 0."
;   * @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter 3.2.1 Basic Flat Model]
; @note The "protected flat model" might be a worthwhile change to consider to this GDT. @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter 3.2.2 Protected Flat Model]
; @note "normal physical address space" == 4 GB.
; @note "normal physical address space" == address space that the CPU can address on its address bus.
; @note "normal physical address space" is flat, ranging continuously from 0 to FFFF_FFFFH.
; @note "normal physical address space" can be mapped to: read-write memory, read-only memory, and memory mapped I/O.
; @note Every logical address is translated into a linear address.
; @note linear address == 32-bit address in the processor's linear address space.
; @note If no paging is used, a logical address is translated into a linear address. linear address == physical address == the address that goes out on the CPU's address bus.
; @note
;   *A segment selector is 16-bits, points to a segment descriptor in the GDT or LDT. But, it also encodes a little more info:
;     * segment_selector[bit 15:3] == Index into GDT or LDT. Is multiplied by 8 to object the GDT/LDT byte offset.
;     * segment_selector[bit 2]    == TI == Table indicator flag. 0 (GDT). 1 (LDT).
;     * segment_selector[bit 1:0]  == RPL == Requested Privilege Level == Specifies the privilege level of the **selector** (vs. the descriptor). @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter 5.5 Privilege Levels].
; @note The first GDT entry is *not** used. Segment selectors == 0000H == "null segment selector". Loading the CS or SS register with 0000H causes a general protection exception (#GP). This is not the case with the remaining segment registers, unless you try to access memory with them.
; @note "Segment selectors are visible to application programs as part of a **pointer variable**, but the values of selectors are usually assigned or modified by link editors or linking loaders, not application programs."
; @note
;   * The CPU has exactly 6 registers specifically intended to hold segment selectors. "They are called segment registers".
;     * Each segment register "supports a specific kind of memory reference": code, stack, data.
;     * At minimum, it is required that the CS, DS, and SS registers must be loaded with **valid segment selectors**.
;     * The remaining segment registers, ES, FS, GS, may be used optionally as **extra data-segment registers**.
;     * @IMPORTANT The only way to access memory is by having the corresponding segment selector loaded into one of the segment registers. It is worthwhile to note that although the GDT may contain > 8000 segment descriptors, we only have 6 segment registers. To access another segment, we must load its segment selector into one of the 6 segment registers.
; @note Every segment register has a visible part and a hidden part (hidden part a.k.a. descriptor cache/shadow registers). The hidden part caches the details found in the corresponding segment descriptor. @doc [Intel 64 & IA-32 SDM, Vol.3, Figure 3-7 Segment Registers]
; @note Instructions that **explicitly** load the segment registers: MOV, POP, LES, LSS, LGS, LFS.
; @note Instructions that **implicitly** load the **CS** (and possibly **other segment registers**): far pointer versions of CALL, JMP, and RET. Also, SYSENTER, SYSEXIT, IRET, INT n, INTO, INT3, and INT1.
;
; @doc [Writing a Simple Operating System - from Scratch, by Nick Blundell, Chapter 4.2 Understanding the Global Descriptor Table]
; @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter 3.4.5 Segment Descriptors]
;



;
; |@doc [Intel 64 & IA-32 SDM, Vol.3, Figure 3.8 Segment Descriptor]                           |
; |--------------------------------------------------------------------------------------------|
; |31        24| 23|   22| 21|   20|19              16| 15|14 13| 12|11   8|7           0| Byte|
; |------------|---|-----|---|-----|------------------|---|-----|---|------|-------------|-----|
; | Base 31:24 | G | D/B | L | AVL | Seg. Limit 19:16 | P | DPL | S | Type |  Base 23:16 |    4|
; |--------------------------------------------------------------------------------------------|

; |--------------------------------------------------------------------------------------------|
; |31                                               16|15                               0| Byte|
; |---------------------------------------------------|----------------------------------|-----|
; |                                Base Address 15:00 |              Segment Limit 15:00 |    0|
; |--------------------------------------------------------------------------------------------|
; Table 3-1. Code- and Data-Segment Types
; Table 3-2. System-Segment and Gate-Descriptor Types


;
; | Abbrev. |            Name |                                                                                            Meaning |
; |---------|-----------------|----------------------------------------------------------------------------------------------------|
; | S       | Descriptor type | 0 = this is a system descriptor; 1 = this is a code segment descriptor or a data segment descriptor|
;
; @doc [Intel 64 & IA-32 SDM, Vol.3, Table 3-1 Code- and Data-Segment Types]
;
; If GDT.segment_descriptor.S = 1 (code or data). GDT.segment_descriptor.Type
; has the following meaning
; Type[bit 11] | 0 = this is a data segment descriptor; 1 = this is code segment descriptor|
;
; Data segment
; |                 11|                       10|                 9|            8| Description                                                           |
; |-------------------|-------------------------|------------------|-------------|-----------------------------------------------------------------------|
; | Data (0)/Code (1) | expansion-direction (E) | write-enable (W) | accessed (A)|                                                                       |
; |                 0 |                       0 |                1 |           0 | Data, Expand-Up (segment limit is constant), Read/Write, Not Accessed |
; Type[bit 11:8]| 0010B
; @discussion

; |                   10|             9|                    8|
; |---------------------|--------------|---------------------|
; | expansion direction (1 = size of stack needs to be changed dynamically)| write-enable (1 = writable)|            accessed |
;
; Code segment
; |                 11|              10|                9|            8| Description                                     |
; |-------------------|----------------|-----------------|-------------|-------------------------------------------------|
; | Data (0)/Code (1) | conforming (C) | read-enable (R) | accessed (A)|                                                 |
; |                 1 |              0 |               1 |           0 | Code, Execute/Read, Nonconforming, Not Accessed |
; Type[bit 11:8]| 1010B
;   Type[bit 11]  | 1 = This descriptor is for a code segment
;   Type[bit 10] C| 0 = Nonconforming
;   Type[bit  9] R| 1 = Read-enable
;   Type[bit  8] A| 0 = Not accessed
; segment_limit | F`FFFFH
;   segment_limit[bit 19:16] | FH
;   segment_limit[bit 15:0]  | FFFFH
; base_address | 0000`0000H
;   base_address[bit 31:24] | 00H
;   base_address[bit 23:16] | 00H
;   base_address[bit 15:00] | 0000H

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This configures a basic flat model of memory with 2 overlapping segments: a
; code segment and a data segment, after the standard null descriptor.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
gdt_start:

gdt_null: ; the mandatory null descriptor
dd 0x0 ; dd = declare double word = 32 bit values
dd 0x0

gdt_code: ; the code segment descriptor
; base = 0x0000`0000, limit = 0xf`ffff
; type flags:       (code) 1     (conforming) 0 (readable) 1 (accessed) 0   = 1010b
; 1st flags:     (present) 1      (privilege) 00 (descriptor type) 1        = 1001b
; 2nd flags: (granularity) 1 (32 bit default) 1 (64 bit seg) 0 (AVL) 0      = 1100b

dw 0xffff    ; Limit (bits 0-15)                ; 16 bits
dw 0x0000    ; Base (bits 0-15)                 ; 16 bits
db 0x00      ; Base (bits 16-23)                ; 8 bits
db 1001```1010b ; 1st flags and type flags         ; 8 bits
db 11001111b ; 2nd flags and Limit (bits 16-19) ; 8 bits
db 0x00      ; Base (bits 24-31)                ; 8 bits

gdt_data: ; the data segment descriptor
; Same as code segment except for the type flags:
; type flags: (code) 0 (EXPAND DOWN) 0 (WRITABLE) 1 (accessed) 0 = 0010b

dw 0xffff ; Limit (bits 0-15) ; 16 bits
dw 0x0 ; Base (bits 0-15) ; 16 bits
db 0x0 ; Base (bits 16-23) ; 8 bits
db 10010010b ; 1st flags and type flags ; 8 bits
db 11001111b ; 2nd flags and Limit (bits 16-19) ; 8 bits
db 0x0 ; Base (bits 24-31) ; 8 bits

gdt_end: ; This label is here to make it easy to calculate the GDT size which we
         ; need for the GDT descriptor.

; GDT descriptor
gdt_descriptor:

dw gdt_end - gdt_start - 1 ; The size of our GDT, ALWAYS LESS 1 OF THE TRUE SIZE. ; 16 bits
dd gdt_start ; Starting address of our GDT.

; Handy constants for the GDT segment descriptor OFFSETS, which are what segment
; registers must contain when in protected mode.
; For example, when we set DS = 0x10 in protected mode (PM), the CPU knows that
; we mean it to use the segment described at offset 0x10 (= 16 bytes) in our
; GST, which in our case is the DATA segment (0x0 = NULL, 0x08 = CODE, 0x10 = DATA)
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
