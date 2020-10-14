;
; A simple boot sector program that loops forever
;
loop: ; Define a label, "loop", so we can jump to it.

jmp loop ; Jumps to the address of the current instruction

times 510-($-$$) db 0 ; tell our assembly compiler to pad out our program with
                      ; with enough 0 bytes to bring us to the 510th byte.
                      ; 510 = boot sector size - 2 for magic number.
                      ; ($-$$) = number of bytes consumed by the previous
                      ; machine instructions.

dw 0xaa55 ; the magic number, so the BIOS knows we are a boot sector.