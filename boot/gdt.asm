; @header Definition of the global descriptor table (GDT).
; This file defines the GDT. Defining the GDT is a necessary prerequisite step
; to switching the CPU mode from 16-bit to 32-bit mode. The definition of the
; GDT in this file consists of two segment descriptors. A code segment
; descriptor and a data segment descriptor. The original GDT definition was
; based on @doc [Writing a Simple Operating System - from Scratch, by Nick
; Blundell, Chapter 4.2 Understanding the Global Descriptor Table]. For
; important notes about the GDT from the Intel SDM see @doc [doc/gdt.md].
;
; @discussion
; The CPU supports a view of memory such that different regions of memory can
; be assigned various attributes and features depending on they way in which
; that memory used. For example, these attributes may include file-like access
; flags, a region of memory can be marked as read-only, read/write, or
; execute/read. These regions of memory consists of contiguous arrays of bytes
; defined by a base address and limit value. For brevity, we call these regions
; of memory "segments". The GDT is Intel's mechanism for the programmer to
; describe various segments. Each entry in the GDT is an 8-byte value called a
; "segment descriptor".

;
; The GDT definition below configures a basic flat model of memory with 2
; overlapping segments: a code segment and a data segment, after the required
; null descriptor. For both the code segment and data segment we have set the
; following: segment-limit = F_FFFFH; base-address  = 0000_0000H;
;
gdt_start:

;
; The first descriptor in the GDT is not used by the processor, it is called the
; "null descriptor".
;
gdt_null:


; Base 31:24 | G | D | L | A | Seg.  | P | D | S | Type | Base 23:16 | Byte
;            |   | / |   | V | Limit |   | P |   |      |            |
;            |   | B |   | L | 19:16 |   | L |   |      |            |
; 00H        | 0B| 0B| 0B| 0B| 0000B | 0B|00B| 0B| 0000B| 00H        | 4
;--------------------------------------------------------------------|
; Base Address 15:00 | Segment Limit 15:00                           | Byte
; 0000H              | 0000H                                         | 0
dd 0x00000000
dd 0x00000000

;
; The code segment descriptor.
;
gdt_code:
;
; Base 31:24 | G | D | L | A | Seg.  | P | D | S | Type | Base 23:16 | Byte
;            |   | / |   | V | Limit |   | P |   |      |            |
;            |   | B |   | L | 19:16 |   | L |   |      |            |
; 00H        | 1B| 1B| 0B| 0B| 1111B | 1B|00B| 1B| 1010B| 00H        | 4
;--------------------------------------------------------------------|
; Base Address 15:00 | Segment Limit 15:00                           | Byte
; 0000H              | FFFFH                                         | 0
;
;------------------------Type-----------------------------|
;           1B|            0B|           1B|            0B|
; Code segment| Nonconforming| Execute/Read| Not Accessed |
;
dw 0xffff    ; Limit[bit 15:0]
dw 0x0000    ; Base[bit 15:0]
db 0x00      ; Base[bit 23:16]
db 10011010b ; P, DPL, S, Type
db 11001111b ; G, D/B, L, AVL, Limit[19:16]
db 0x00      ; Base[bit 31:24]

;
; The data segment descriptor.
;
gdt_data:
;
; Base 31:24 | G | D | L | A | Seg.  | P | D | S | Type | Base 23:16 | Byte
;            |   | / |   | V | Limit |   | P |   |      |            |
;            |   | B |   | L | 19:16 |   | L |   |      |            |
; 00H        | 1B| 1B| 0B| 0B| 1111B | 1B|00B| 1B| 0010B| 00H        | 4
;--------------------------------------------------------------------|
; Base Address 15:00 | Segment Limit 15:00                           | Byte
; 0000H              | FFFFH                                         | 0
;
;-----------------------------Type--------------------------------------|
;           0B|                            0B|         1B|           0B |
; Data segment| Expand-Up (limit is constant)| Read/Write| Not Accessed |
;
dw 0xffff    ; Limit[bit 15:0]
dw 0x0000    ; Base[bit 15:0]
db 0x00      ; Base[bit 23:16]
db 10010010b ; P, DPL, S, Type
db 11001111b ; G, D/B, L, AVL, Limit[19:16]
db 0x00      ; Base[bit 31:24]

gdt_end:

;
; GDT Register
;
gdt_descriptor:

dw gdt_end - gdt_start - 1 ; Number if bytes in GDT minus 1.
dd gdt_start               ; The linear address of byte 0 of the GDT.

;
; Constants for the GDT segment descriptor OFFSETS a.k.a. "segment selectors".
; We load these values into the segment registers (CS, SS, DS, ES, FS, GS)in
; order to access a particular byte with that segment.
;
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; @TODO
; * [x] What is bochs physical memory setting?
;   * ANS: The default is 32MB.
; * [x] What happens if we try to read above the physical memory limit?
;   * ANS: Since we are using segment limit == 4GB, the CPU will not generate an
;     exception.
; * [ ] How do I tell NASM to align things?
;   * @doc [NASM manual Chapter.5.10 Alignment Control]
;   * @IMPORTANT The base address of the GDT should be 8-byte aligned.
;   * @IMPORTANT Segment bases addresses **should** aligned to 16-byte
;     boundaries. This is equivalent to having the lowest order 4 bits of the
;     base address == 0000B = 0H.
; * [ ] Define some simple changes to the GDT to verify expected Intel SDM
;       behavior. e.g. if access DS with null descriptor, generates #GP.