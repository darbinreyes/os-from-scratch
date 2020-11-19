/*!
    @header Driver for a PS/2 controller.
    Used to interface with a PS/2 device, typically a PS/2 keyboard and PS/2
    mouse.

    @TODO Status register bit 3 definition is ambiguous. Is there a test I can
    perform to determine its meaning?
*/
#include "../mylibc/mylibc.h"
#include "../kernel/low_level.h"
#include "ps_2_ctlr.h"

/*!
    @defined IO_PS2_CTLR_DATA

    @discussion PS/2 controller data I/O port.
    I/O port addresses used for communication with the PS/2 controller.
*/
#define IO_PS2_CTLR_DATA          (0x60) // CTLR = ConTroLleR.

/*!
    @defined IO_PS2_CTLR_STAT_REGISTER

    @discussion PS/2 controller status register I/O port.
    I/O port addresses used for communication with the PS/2 controller.
*/
#define IO_PS2_CTLR_STAT_REGISTER (0x64)

/*!
    @defined IO_PS2_CTLR_CMD_REGISTER

    @discussion PS/2 controller command register I/O port.
    I/O port addresses used for communication with the PS/2 controller.
*/
#define IO_PS2_CTLR_CMD_REGISTER  (0x64)

/*!
    @defined POLL_COUNT

    @discussion The number of times to poll the state of a buffer before
    considering the operation as timed-out.
*/
#define POLL_COUNT (0x01 << 20)

/*!
    @function get_ctlr_stat

    @discussion Returns the value of the PS/2 controller's status register.

    @param    stat    Pointer in which to return that status register value.

    @result Nonzero on error. Zero if successful. The status has been returned
    in stat.
*/
int get_ctlr_stat(ps_2_ctrl_stat_t *stat) {
    unsigned char b;
    unsigned char *pb;

    if (stat == NULL)
        return 1;

    b = port_byte_in (IO_PS2_CTLR_STAT_REGISTER);
    pb = (unsigned char *) stat;
    *pb = b;

    /* FYI:

        Type conversion not allowed.
            struct <- int

        Allowed
            int* <- struct*
            *struct = int

    */

    return 0;
}

/*
    @function send_byte

    @discussion Polling based implementation. Sends a byte to the PS/2
    controller's data port.

    @param    b    The byte value to send.

    @result Zero if successful. Nonzero on error.
*/
int send_byte (unsigned char b) {
    ps_2_ctrl_stat_t stat;
    int timeout_counter = POLL_COUNT;
    int r;

    r = get_ctlr_stat(&stat);

    if (r != 0)
        return 4;

    while (timeout_counter > 0 && stat.ibuf_full == PS2_BUF_FULL) {
        r = get_ctlr_stat(&stat);

        if (r != 0)
            return 4;

        timeout_counter--;
    }

    if (timeout_counter == 0) {
        return 1;
    }

    if (stat.ibuf_full == PS2_BUF_FULL) {
        return 2;
    }

    port_byte_out (IO_PS2_CTLR_DATA, b);

    return 0;
}

/*
    @function rcv_byte

    @discussion Polling based implementation receive a byte from the PS/2
    controller's data port.

    @param    b    Pointer in which to return the received byte.


    @result Zero if successful. Nonzero on error.
*/
int rcv_byte (unsigned char *b) {
    ps_2_ctrl_stat_t stat;
    int timeout_counter = POLL_COUNT;
    int r;

    if (b == NULL)
        return 3;

    r = get_ctlr_stat(&stat);

    if (r != 0)
        return 4;

    while (timeout_counter > 0 && stat.obuf_full == PS2_BUF_EMPTY) {
        r = get_ctlr_stat(&stat);

        if (r != 0)
            return 4;

        timeout_counter--;
    }

    if (timeout_counter == 0) {
        return 1;
    }

    if (stat.obuf_full == PS2_BUF_EMPTY) {
        return 2;
    }

    *b = port_byte_in (IO_PS2_CTLR_DATA);

    return 0;
}

/*
    @function send_byte_ctlr

    @discussion Sends a byte to the PS/2 controller's command registers. This
    simply writes to the appropriate I/O port.

    @param    b    The byte value to send.
*/
void send_byte_ctlr (unsigned char b) {
    port_byte_out (IO_PS2_CTLR_CMD_REGISTER, b);
}