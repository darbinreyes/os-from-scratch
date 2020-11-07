; @TODO
; - [x] What is bochs physical memory setting? ANS: The default is 32MB.
; - [x] What happens if we try to read above the physical memory limit? ANS: Since we are using segment limit == 4GB, the CPU will not generate an exception.
; - [ ] How do I tell NASM to align things? // @IMPORTANT The base address of the GDT should be 8-byte aligned. //  @IMPORTANT Segment bases addresses **should** aligned to 16-byte boundaries. This is equivalent to having the lowest order 4 bits of the base address == 0000B = 0H.

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
; @remark I have adopted the Intel SDM notational conventions. @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter.1 About This Manual]
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
;   * Both descriptors have the same base address 0H and limit of 4GB (= FFFF_FFFFH).
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
;     * segment_selector[bit 1:0]  == RPL == Requested Privilege Level == Specifies the privilege level of the **selector** (vs. the descriptor). @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter.5.5 Privilege Levels].
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







; @doc [Intel 64 & IA-32 SDM, Vol.3, Table 3-1 Code- and Data-Segment Types]
;   * This section applies to the cases in which segment_descriptor.S == 1.
; * Our data segment uses Type[bit 11:8] == 0010B.
; @note @doc [Intel 64 & IA-32 SDM, Vol.3, Table 3-1. Code- and Data-Segment Types]
; |                 11|                       10|                 9|            8| Description                                                                   |
; |-------------------|-------------------------|------------------|-------------|-------------------------------------------------------------------------------|
; | Data (0)/Code (1) | expansion-direction (E) | write-enable (W) | accessed (A)|                                                                               |
; |                 0 |                       0 |                1 |           0 | Data segment, Expand-Up (segment limit is constant), Read/Write, Not Accessed |
; |--------------------------------------------------------------------------------------------------------------------------------------------------------------|
; @note A stack segment is a data segment which must be read/write, otherwise loading the SS register with a nonwritable segment selector will generate a #GP.
; @note For a stack whose size needs to be changed dynamically, set E == 1 == expand-down. Then, to change the size of the stack, change the segment_descriptor.limit. For a fixed sized stack, the E flag is a "don't care".
; @note The A - accessed flag, "indicates whether the segment has been accessed since the last time the OS cleared the bit". @remark I suspect this is used for page replacement algorithms if paging is used.
;   * The CPU doesn't set this when a byte is accessed in the segment, instead, it is set whenever it loads the segment selector for the segment into a segment register. The OS must explicitly clear this bit.
; @note A data segment cannot be accessed if CPL > DPL. But, for a data segment, if CPL < DPL, it can be accessed, "without using a special access gate".
;
; * Our code segment uses Type[bit 11:8] == 1010B
; @note @doc [Intel 64 & IA-32 SDM, Vol.3, Table 3-1. Code- and Data-Segment Types]
; |                 11|              10|                9|            8| Description                                               |
; |-------------------|----------------|-----------------|-------------|-----------------------------------------------------------|
; | Data (0)/Code (1) | conforming (C) | read-enable (R) | accessed (A)|                                                           |
; |                 1 |              0 |               1 |           0 | Code segment, Nonconforming, Execute/Read, , Not Accessed |
; |--------------------------------------------------------------------------------------------------------------------------------|
; * @note "An execute/read segment might be used when constants or other static data have been placed with instruction code in a ROM."
; * @note "data can be read from the code segment either by using an instruction with a CS override prefix or by loading a segment selector for the code segment in a data-segment register (the DS, ES, FS, or GS registers)."
; * @note In protected mode, a code segment is never writable.
; * @note "Nonconforming" means that a transfer of execution to this segment is only allowed if CPL == DPL otherwise a #GP is generated. "Utilities that need to be protected from less privileged programs and procedures should be placed in nonconforming code segments." For details on conforming and nonconforming code segments see @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter.5.8.1 Direct Calls or Jumps to Code Segments].
; * @note Regardless of the conforming flag value, "Execution cannot be transferred by a call or a jump to a less-privileged (numerically higher privilege level, CPL > DPL) code segment, regardless of whether the target segment is a conforming or nonconforming code segment." An attempt to do this generates a #GP.
; * @IMPORTANT "If the segment descriptors in the GDT or an LDT are placed in ROM, the processor can enter an indefinite loop if software or the processor attempts to update (write to) the ROM-based segment descriptors. To prevent this problem, set the accessed bits for all segment descriptors placed in a ROM. Also, remove operating-system or executive code that attempts to modify segment descriptors located in ROM."
; segment_limit | F_FFFFH
;   segment_limit[bit 19:16] | FH
;   segment_limit[bit 15:0]  | FFFFH
; base_address | 0000_0000H
;   base_address[bit 31:24] | 00H
;   base_address[bit 23:16] | 00H
;   base_address[bit 15:00] | 0000H

; * @note
;   * @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter.3.5 System Descriptor Types]
;   * In the above we have discussed the entries in the GDT/LDT which are called "Segment Descriptors". They are 8-byte entries that describe code segments of data segments in the linear address space.
;   * The is one more kind of "descriptor" other than "Segment Descriptors". They are called "System Descriptors".
;   * System descriptors have the same general format as segment descriptors. To indicate a system-descriptor type, the S flag should be clear (S == 0). @doc [Intel 64 & IA-32 SDM, Vol.3, Figure 3.8 Segment Descriptor].
;   * System descriptors fall into 2 categories:
;     * 1. System-**segment** descriptors - these descriptors are used to point to "system segments" (in contrast to code segments and data segments). There are 2 kinds of system segments: a local descriptor-table segment descriptor (LDT segment), and a task-state segment descriptor (TSS segment).
;     * 2. **Gate** descriptors - there are 4 kinds. call-gates, interrupt-gates, and trap-gates - these descriptors hold pointers to procedure entry points in **code segments**. A task-gate descriptor "which hold segment selectors for TSS's".
;   * To specify a system-segment, set the Type[bit 11:8] field according to @doc [Intel 64 & IA-32 SDM, Vol.3, Table 3-2. System-Segment and Gate-Descriptor Types].
;     * @remark From having configured the IDT, I recognize:
;       * Type[bit 11:8] == 0101B ==  5 == Task gate.
;       * Type[bit 11:8] == 1110B == 14 == 32-bit interrupt gate.
;       * Type[bit 11:8] == 1111B == 15 == 32-bit trap gate.
;   * @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter.7.2.2 TSS Descriptor]
;   * @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter.5.8.3 Call Gates]
;   * @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter.7.2.5 IDT Descriptors]

; * @note
;   * @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter.3.5.1 Segment Descriptor Tables]
;   * A segment descriptor table is an array of segment descriptors.
;   * A "descriptor table" can contain up to 8192 = 2^13, 8-byte descriptors. @remark Why 2^13? Because an offset into the table is specified by the 16-bit logical_address.segment_selector, in which the low order 3-bits have a special meaning: TI, RPL. @doc [Intel 64 & IA-32 SDM, Vol.3, Figure 3-6 Segment Selector].
;   * There are only 2 kinds of descriptor tables:
;     * 1. The global descriptor table (GDT).
;     * 2. The local descriptor table (LDT).
;     * @doc [Intel 64 & IA-32 SDM, Vol.3, Figure 3-10 Global and Local Descriptor Tables].
;   * The programmer is required to define one GDT, it is used for all programs and tasks in the system.
;   * LDT's are optional, they are used by tasks. You can defined one LDT per task being run. Or, you can have a subset or all tasks share a single LDT.
;   * The GDT is a data structure that resides in the linear address space. You inform the CPU of its location and size by loading the GDTR. @doc [Intel 64 & IA-32 SDM, Vol.3, Figure 2-6 Memory Management Registers].
;   * @IMPORTANT The base address of the GDT should be 8-byte aligned.
;   * The limit value of the GDTR has 1 byte units. The limit should be set to the offset of the last valid byte, as with segment descriptor limits. A limit value of 0 means that the size of the GDT is 1 byte. Since each GDT entry is 8 bytes, the limit should  always be a multiple of 8-bytes minus 1.
;   * The first descriptor in the GDT is not used. It should be set to all 0's and is called the null descriptor.
;     * It is allowed to the load the 4 data segment registers DS, ES, FS, GS with a segment selector pointing the the null descriptor i.e. loading those registers will not generate an exception. Notice that the CS and SS registers were not mentioned here, you should never load them with the null descriptor.
;     * However, if a memory access is attempted with the null descriptor a #GP will be generate.
;     * "accidental reference to **unused segment registers** can be guaranteed to generate a exception."
;   * An LDT is defining by an LDT segment descriptor in the GDT. Each LDT requires its own LDT segment descriptor in the GDT. LDT segment descriptors may be located anywhere in the GDT.
;   * "An LDT is accessed with its segment selector. To eliminate address translations when accessing the LDT, the segment selector, base linear address, limit, and access rights of the LDT are stored in the LDTR register". @doc [Intel 64 & IA-32 SDM, Vol.3, Chapter.2.4 Memory-Management Registers].
;   * The GDTR can be stored using the SGDT instruction. When the 48-bit GDTR is stored in memory it is called a "pseudo-descriptor". @remark Why is it called that?
;     * @IMPORTANT
;       * A strange quirk about using the SGDT instruction is that, to prevent alignment check faults (#AC) when executing at privilege level 3, "the pseudo-descriptor should be located at an odd word address (i.e. address MOD 4 == 2).
;       * "This causes the CPU to store an aligned word, followed by an aligned doubleword."
;       * "Privilege level 3 programs normally do not store pseudo-descriptors, but the possibility of generating an #AC can be avoided by aligning pseudo-descriptors in this way."
;     * @IMPORTANT
;      * "The same alignment should be used when storing the IDTR register when using the SIDT instruction."
;     * @IMPORTANT
;       * "When storing the LDTR or task register using the SLDT or STR instruction, respectively), the pseudo-descriptor should be located at a doubleword address (i.e. address MOD 4 == 0)."
;   * @IMPORTANT The lesson here is that, when storing any special purpose registers like the GDTR, make sure you check the Intel SDM for any alignment requirements.

;TODO @remark What is meant by "located at an odd word address"?
;       * ANS: Notice below word address 1 (which is odd), has byte address == 2, and 2 MOD 4 == 2. Similarly, word address 3 == byte address 6, 6 MOD 4 == 2.
;TODO @remark What is meant by "store an aligned word, followed by an aligned double word"?
;       * ANS: Since a pseudo descriptor is 48-bits (== 6 bytes == 16-bits + 32-bits == word + doubleword). Notice that if we store the pseudo-descriptor at byte address 2, its first word is word aligned since 2 MOD 2 == 0, and, its remaining doubleword is also doubleword aligned since it is stored at byte address 4 and 4 MOD 4 = 0.
; |  |  | 6 <<               3 << // 6
; |  |  | 4 <<               2 << // 4 = 100B
; |  |  | 2 <<               1 << // 2 = 010B
; |  |  | 0 << byte address, 0 << word address
;TODO @remark What is meant by "should be located at a doubleword address (i.e. address MOD 4 == 0)"?
;       * ANS: A doubleword == 32-bits == 4 bytes. Therefore, any byte address such that (address MOD 4 == 0) is doubleword aligned.


;;;;;;;;;;;;;;;;;;;;;;;;;;END OF VOL.3 CHAPTER.3 NOTES ;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This configures a basic flat model of memory with 2 overlapping segments: a
; code segment and a data segment, after the standard null descriptor.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
gdt_start:

gdt_null: ; the mandatory null descriptor
dd 0x0 ; dd = declare double word = 32 bit values
dd 0x0

gdt_code: ; the code segment descriptor
; base = 0x0000_0000, limit = 0xf_ffff
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