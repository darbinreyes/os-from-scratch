; Section 3.6.4 Putting it all Together.
; load DH sectors to ES:BX from drive DL.

disk_load:
    push dx ; Push DX on the stack so we can use DH for error checking after
            ; int 0x13 returns.

    mov ah, 0x02 ; Specify the BIOS read sector function.
    mov al, dh   ; Specify the number of sectors to read.

    ; DL contains the drive number to read from, below, we specify the
    ; starting cylinder-head-sector address.
    mov ch, 0x00 ; Select cylinder 0.
    mov dh, 0x00 ; Select head (= track) 0.
    mov cl, 0x02 ; Select 2nd sector. (This is a 1 based index).
                 ; (If DL is set to the boot disk drive, this sector is the one
                 ;  after our boot sector.)

    int 0x13 ; BIOS interrupt for disk device access.


    jc disk_error ; Carry flag set means general fault (= error) occurred.

    pop dx ; Restore DX register value, DH = requested number of sectors
           ; DL = disk drive number. (DX is set by caller of this function.)

    cmp dh, al ; AL = number of sectors actually read, if these aren't equal an
              ; error occurred.
    jne disk_error_al

    ret ; DONE!

disk_error:
    mov bx, DISK_ERROR_MSG
    call print_string
    jmp $ ; Infinite loop.

disk_error_al:
    mov bx, DISK_ERROR_MSG_AL
    call print_string
    jmp $ ; Infinite loop.

; Variables.
DISK_ERROR_MSG: db "CF set. Disk read error!", 0 ; NEED ":" HERE? ANS: No. Works the same. Assembler doesn't report an error .
DISK_ERROR_MSG_AL: db "DH != AL. Disk read error!", 0


