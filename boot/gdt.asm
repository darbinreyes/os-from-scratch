; @TODO
; - [x] What is bochs physical memory setting? ANS: The default is 32MB.
; - [x] What happens if we try to read above the physical memory limit? ANS: Since we are using segment limit == 4GB, the CPU will not generate an exception.
; - [ ] How do I tell NASM to align things?

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
; @remark For brevity, where the Intel SDM use "processor" I use "CPU".
;
;
; @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter.3 Protected-Mode Memory Management - through Chapter.3.4.4]
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
;   * A segment selector is 16-bits, points to a segment descriptor in the GDT or LDT. But, it also encodes a little more info:
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
;

; @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter 3.4.5 Segment Descriptors]
; @note Each entry in the GDT/LDT is called a "segment descriptor".
; @note A segment descriptor contains all the details about a segment that the CPU needs.
; @note Every segment descriptor is exactly 8 bytes in size.
; @note Every segment descriptor has the same general format. @doc [Intel 64 & IA-32 SDM, Vol.3, Figure 3.8 Segment Descriptor].



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
;
; The full names of the fields in Figure 3.8 are as follows:
;
; | Abbrev. | Name
; |---------|-----------------
; | LIMIT   | Segment limit
; | BASE    | Segment base address
; | TYPE    | Segment type
; | S       | Descriptor type
; | DPL     | Descriptor privilege level
; | P       | Segment present
; | L       | 64-bit code segment (IA-32e mode only)
; | D/B     | Default operation size (0 = 16-bit segment; 1 = 32-bit segment)
; | G       | Granularity
;

; @note
; * Segment limit
;   * The Intel SDM uses the phrases like "the SS segment", which is somewhat ambiguous. It means "a memory access that uses the SS segment register to specify the segment selector part of a logical address".
;   * The segment limit is a 20-bit value that specifies the size of the segment.
;   * The size is interpreted as having units of 1 Byte or 4 KB depending on the Granularity (G) setting.
;     * if (G == 0) "size is in 1 Byte units"; Min. segment size = 1 Byte. Max = 2^20 Bytes = 1 MB. Since the max 20-bit integer is = 2^20 - 1, from this we can infer that, LIMIT == 0 defines a segment whose size is exactly 1 Byte. LIMIT == F_FFFFH defines a segment whose size is 2^20 - 1 + 1 = 2^20 = 1 MB. So the size of the segment is always segment limit + 1.
;     * if (G == 1) "size is in 4 KB units"; Min. segment size = 4 KB. Max = 4 GB = 2^20 * 4 KB = 2^20 * 2^12 = 2^32 = 4 GB.
;   * If the segment type == data segment descriptor type, TYPE[bit 10] == "expansion-direction (E)". @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter.3.4.5.1 Code- and Data-Segment Descriptor Types]
;     * The CPU uses the segment limit in one of two ways, depending on the "expansion-direction (E)" setting.
;     * 1.
;       if (TYPE[bit 11] == 0 && TYPE[bit 10] == 0 ) { // (Is data segment descriptor type) && (Expansion-direction is expand-up)
;           if (!(0 <= logical_address.offset <= segment_limit)) {
;               if ("is SS segment")
;                   "generate a #SS";
;               else
;                   "generate a #GP";
;           }
;       }
;     * 2. "the segment limit has the reverse function"
;       if (TYPE[bit 11] == 0 && TYPE[bit 10] == 1 ) { // (Is data segment descriptor type) && (Expansion-direction is expand-down)
;           if (D/B == 0) segment_upper_bound = FFFFH;
;           if (D/B == 1) segment_upper_bound = FFFF_FFFFH;
;           if (!(segment_limit + 1 <= logical_address.offset <= segment_upper_bound)) { // Here the segment limit value defines the lower bound of the logical_address.offset. As the segment limit is increased, the size of the segment decreases. Notice that the upper limit check can never trigger a fault because the logical_address.offset is 32-bits.
;
;               "generate a #GP or a #SS"; // "Offsets less than or equal to the segment limit generate a #GP or a #SS."
;               // @note "for an expand-down segment"  "Decreasing the value in the segment limit field allocates new memory at the bottom of the segment's address space"
;               // @IMPORTANT "IA-32 architecture stacks always grow downwards, making this mechanism convenient for expandable stacks.
;               // @note Here, to get a 1 Byte segment, we set the segment limit to FFFF_FFFFH - 1 = FFFF_FFFEH. In general, for a size n segment, set the segment limit to FFFF_FFFFH - n.
;           }
;       }
;
; @note
; * Base address
;   * 32-bit address of byte 0 of the segment withing the 4 GB linear address space.
;   * @IMPORTANT Segment bases addresses **should** aligned to 16-byte boundaries. This is equivalent to having the lowest order 4 bits of the base address == 0000B = 0H. This alignment is not required but ensuring it will maximize performance.
;
; @note
; * Type
;   * Segment descriptors are classified into 2 broad categories:
;   * 1. Code or data segment descriptors (a.k.a application segment descriptors). These sorts of descriptors will be familiar to most programmers.
;   * 2. System segment descriptors. These sorts of descriptors are involved in the use of lesser known CPU features.
;   * if (segment_descriptor.S == 1) descriptor is a #1 descriptor.
;   * if (segment_descriptor.S == 0) descriptor is a #2 descriptor.
;   * The particular values in Type[bit 11:8] depends on the individual segment descriptor type. The Intel SDM devotes an entire section to each type.
;
; @note
;   * S - descriptor type.
;     * if (segment_descriptor.S == 0) segment descriptor is for a system segment.
;     * if (segment_descriptor.S == 1) segment descriptor is for a code or data segment.
;
; @note
;   * DPL - descriptor privilege level
;     * "Specifies the privilege level of the segment." "used to control access to the segment." @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter 5.5 Privilege Levels].
;
; @note
;   * P - segment-present
;     * 1 = the segment is present in memory.
;     * 0 = not present in memory.
;     * if (segment register is loaded with a segment_selector.P == 0) "generates a segment-not-present exception (#NP)."
;     * "use this flag to control which segments are actually loaded into physical memory at a given time". "It offers control in addition to paging for managing virtual memory." @remark We don't have a secondary store at the moment, so where would a segment reside if it is not present in physical memory?
;     * @IMPORTANT When P == 0, the segment descriptor's interpretation changes drastically. See Figure 3-9.
;       * "Available" == "the OS is free to use the locations marked Available to store its own data". @remark To me, this strongly implies that we would use these bits to store the location on secondary store of a page on disk under a demand-paging memory management scheme.
;
; |@doc [Intel 64 & IA-32 SDM, Vol.3, Figure 3-9 Segment Descriptor When Segment-Present Flag is Clear]
; |------------------------------------------------------------------------------------------------|
; |31                                               16|     15|14 13| 12|11   8|7           0| Byte|
; |---------------------------------------------------|-------|-----|---|------|-------------|-----|
; |                                         Available | (P) 0 | DPL | S | Type |   Available |    4|
; |------------------------------------------------------------------------------------------------|

; |------------------------------------------------------------------------------------------------|
; |31                                                                                       0| Byte|
; |------------------------------------------------------------------------------------------|-----|
; |                                                                                Available |    0|
; |------------------------------------------------------------------------------------------------|
;
; @note
;   * D/B - default operation size/default stack pointer size and/or upper bound.
;     * 1 = 32-bit code or data segment. 0 = 16-bit code or data segment.
;     * The effect of this bit is different according to the particular kind of segment. 3 kinds of segments apply here. Each case calls the flag either the "D flag" or the "B flag", hence the name "D/B".
;       1. Executable code segment. D flag.
;       2. Stack segment. (A stack segment is simply a read/write data segment pointed to by the SS registers). B (big) flag.
;       3. Expand-down data segment. B (big) flag.

;   * if (is case #1)
;     * Called the D flag. "indicates the default length of effective addresses and operands referenced by instructions in the segment."
;     * The 32-bit offset part of a logical address is also called an "effective address". @doc [Intel 64 & IA-32 SDM, Vol.3, Figure 3-5 Logical Address to Linear Address Translation]
;     * 1 = 32-bit **addresses** and 32-bit or 8-bit **operands** are assumed.
;     * 0 = 16-bit **addresses** and 16-bit or 8-bit **operands** are assumed.
;     * "The instruction prefix 66H" - used to select an **operand** size other than the default.
;     * "The instruction prefix 67H" - used to select an **address** size other than the default.
;   * if (is case #2)
;     * Called B (big) flag. "specifies the size of the stack **pointer** used for **implicit** stack operations" (e.g. PUSH, POP, CALL).
;     * 1 = "32-bit stack pointer is used, stored in the ESP register"
;     * 0 = "16-bit stack pointer is used, stored in the SP register"
;     * if (segment_descriptor.Type.E == "Expansion-direction is expand-down" == 1) {
;       * if (B == 0) segment_upper_bound = FFFFH;
;       * if (B == 1) segment_upper_bound = FFFF_FFFFH; // See segment limit definition.
;       }
;   * if (is case #3)
;     * Called the B (big) flag. "specifies the **upper bound** of the segment"
;       * if (B == 0) segment_upper_bound = FFFFH;
;       * if (B == 1) segment_upper_bound = FFFF_FFFFH; // See segment limit definition.
;
; @note
;   * G - granularity.
;     * "Determines the scaling of the segment limit field."
;     * 0 = byte units. a.k.a. "byte granular"
;     * 1 = 4 KB units.
;       * if (G == 1) "low order 12-bits of a logical_address.offset are not tested when checking the offset against the segment_descriptor.limit"
;         * e.g. if (segment_descriptor.limit == 0) a valid offsets satisfy ( 0 <= logical_address.offset <= 4095)
;           * @remark Why not test those 12-bits? ANS: We can think of a logical_address.offset as a pair {20-bit 4 KB page number, 12-bit page offset}, paging isn't involved, its just a convenient name to use. The logical_address.offset can range from {00000H, 000H} to {00000H, FFFH}. Since FFFH == 4095, it will never be out of range with respect to locating a byte inside a 4KB unit, therefore, we need not test that part of the logical address against the segment limit. All we need to test is that [logical_address.page_number <= segment_descriptor.limit].
;     * @note This flag does not affect the granularity of the segment_descriptor.base_address. It is always byte granular.
;
; @note
;   * L - 64-bit codes segment flag.
;     * if (CPU mode == ***IA-32e mode**)
;       * 1 = **code segment** contains native 64-bit code.
;       * 0 = "code segment executed in compatibility mode".
;       * if (L == 1) "segment_descriptor.D/B must == 0".
;     * else
;           L should always == 0.
;     * @note I'm not using 64-bit mode any time soon.
;
; @note
;   * AVL - Available and reserved bits.
;     * The AVL bit "is available for use by system software." @remark I assume this means the OS can set/clear this bit at will, and assign to it whatever meaning it wishes. Perhaps it could be used as a dirty bit indicator under a paging memory management scheme.


;--------------------------------------------------------------------------------------------------------------------|
; @dijkstra "Our proof format". "Everywhere operator" e.g. [X]. "boolean scalars: true, false.".
; @remark Notes taken as part of understanding the segment limit field of a segment descriptor.
; I call this the "absorption of the one's rule". For all integers x, y:
;-----------------------------------------------------
;  x + 1 <= y
; = {}
;  x <= y - 1
; = {}
;  x < y // Example:  x <= 5 - 1, solutions: x = 0, 1, 2, 3, 4; | x < 5, has identical solutions: x = 0, 1, 2, 3, 4; |
;-----------------------------------------------------
;  !(x + 1 <= y)
; = {}
;   (x + 1 > y)
; = {}
;   (y < x + 1)
; = {}
;   (y <= x) // Example: y < 4 + 1, solutions: y = 0, 1, 2, 3, 4; | y <= 4, solutions: y = 0, 1, 2, 3, 4; |
;-----------------------------------------------------
; Summary
;-----------------------------------------------------
;  x + 1 <= y
; = {Symbol dynamics: `+ 1` elimination of the `=`}
;  x < y
;-----------------------------------------------------
;  x - 1 <= y  // Example x - 1 <= 3, solutions x = 0, 1, 2, 3, 4; | x < 3 + 2, solutions x = 0, 1, 2, 3, 4; |
; = {}
;  x + 1 <= y + 2
; = {Symbol dynamics: `+ 1` elimination of the `=`}
;  x < y + 2
;-----------------------------------------------------
;  x < y + 1
; = {Symbol dynamics: `+ 1` formation of the `=`}
;  x <= y
;-----------------------------------------------------
;  x < y - 1 // Example: x < 3 - 1, solutions x = 0, 1; x <= 1, solutions x = 0, 1; |
; = {}
;  x + 2 < y + 1
; = {Symbol dynamics: `+ 1` formation of the `=`}
;  x + 2 <= y // Check: x = 0, [0 + 2 <= 3], x = 1, [1 + 2 <= 3] |
;--------------------------------------------------------------------------------------------------------------------|

; @doc [Intel 64 & IA-32 SDM, Vol.3, Table 3-1. Code- and Data-Segment Types]
; @doc [Intel 64 & IA-32 SDM, Vol.3, Table 3-2. System-Segment and Gate-Descriptor Types]

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
db 10011010b ; 1st flags and type flags         ; 8 bits
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
