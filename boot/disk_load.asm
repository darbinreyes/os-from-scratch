;
; @function.label    disk_load    Procedure to read DH number of sectors from a drive DL into memory at address ES:BX.
;
; @param.register    DL           The drive number identifying the drive from which sectors will be read.
;
; @param.register    DH           The requested number of sectors to read from the drive.
;
; @param.register    ES           The segment base address value to use when reading sectors into memory at ES:BX.
;
; @param.register    BX           The segment offset value to use when reading sectors into memory at ES:BX.
;
; @discussion
;
; @doc Writing a Simple Operating System - from Scratch by Nick Blundell, section 3.6.4.
;

disk_load:
    push dx                  ; Push DX on the stack so we can use DH for error checking after int 0x13
                             ; returns.

    mov ah, 0x02             ; BIOS ISR usage convention. AH := 2, means we want to use the BIOS read sector
                             ; function.

    mov al, dh               ; BIOS ISR usage convention. AL specifies the number of sectors to read.

    ;
    ; BIOS ISR usage convention. Specifying the starting cylinder-head-sector
    ; address for the read operation.
    ; CH := cylinder (a.k.a. track) number (0 based index).
    ; DH := head number (0 based index).
    ; CL := sector number (1 based index).
    ;

    mov ch, 0x00             ; Select cylinder 0.
    mov dh, 0x00             ; Select head 0.
    mov cl, 0x02             ; Select sector 2, since sector 1 contains this boot program. (This is a 1 based
                             ; index).

    int 0x13                 ; BIOS ISR for disk device access.

    jc disk_error_1          ; If the carry flag is set it means a general fault (a.k.a. error) occurred.

    pop dx                   ; Restore DX register value.

    cmp dh, al               ; BIOS ISR usage convention. AL = number of sectors actually read. If DH != AL,
                             ; an has error occurred.
    jne disk_error_2

    ret                      ; DONE!

disk_error_1:
    mov bx, STR_DISK_ERROR_1
    call print_string
    jmp $                    ; Infinite loop.

disk_error_2:
    mov bx, STR_DISK_ERROR_2
    call print_string
    jmp $                    ; Infinite loop.

;
; Global variables - strings.
;
; Remark: Is the colon needed here? ANS: No. Works the same. Assembler doesn't
; report an error and the runtime behavior is the same.
;
STR_DISK_ERROR_1: db "Carry flag (CF) set. Disk read error!", 0xa, 0x0d, 0

STR_DISK_ERROR_2: db "Sector read count mismatch. Requested != Actual. DH != AL. Disk read error!", 0xa, 0x0d, 0


