; @header Global Descriptor Table (GDT).
; This file defines the GDT. Defining the GDT is a necessary prerequisite step
; to switching the CPU mode from 16-bit to 32-bit mode. The definition of the
; GDT in this file consists of two segment descriptors. A code segment
; descriptor and a data segment descriptor.
; @doc [Writing a Simple Operating System - from Scratch, by Nick Blundell,
; Chapter 4.2 Understanding the Global Descriptor Table]
;
; @abstract Definition of the global descriptor table (GDT).
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
; @remark For brevity, where the Intel SDM uses "processor" I use "CPU".
; @remark I have adopted the Intel SDM notational conventions. @doc [Intel 64 &
; IA-32 SDM, Vol.3, Chapter.1 About This Manual]


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

; @TODO
; * [x] What is bochs physical memory setting?
;   * ANS: The default is 32MB.
; * [x] What happens if we try to read above the physical memory limit?
;   * ANS: Since we are using segment limit == 4GB, the CPU will not generate an
;     exception.
; * [ ] How do I tell NASM to align things?
;   * @IMPORTANT The base address of the GDT should be 8-byte aligned.
;   * @IMPORTANT Segment bases addresses **should** aligned to 16-byte
;     boundaries. This is equivalent to having the lowest order 4 bits of the
;     base address == 0000B = 0H.