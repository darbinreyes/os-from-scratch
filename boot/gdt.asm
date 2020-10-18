; Our global descriptor table (GDT)
; this configures a basic flat model of memory with 2 overlapping
; segments: a code segment and a data segment, after the standard null
; descriptor.

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
