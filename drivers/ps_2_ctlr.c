/*

===.md

* [] driver start().
* [] driver stop().

===

*/
#include "../mylibc/mylibc.h"
#include "../kernel/low_level.h"
#include "ps_2_ctlr.h"

#define POLL_COUNT (0x01 << 20)

/*

    Returns the value of the PS/2 controller's status register.

    @retval 1 Arg. NULL.

    @retval 0 Successful. The status has been returned in stat.

*/
int get_ctlr_stat(ps_2_ctrl_stat_t *stat) {
    unsigned char b;
    unsigned char *pb;

    if (stat == NULL)
        return 1;

    b = port_byte_in (IO_PS2_CTLR_STAT_REGISTER);
    pb = (unsigned char *) stat;
    *pb = b;

    /* TEMP. FYI.

        Type conversion not allowed.
            struct <- int

        Allowed
            int* <- struct*
            *struct = int

    */

    return 0;
}

/*

    Send.

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

    Receive.

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

    Send a byte to the PS/2 controller.

*/
void send_byte_ctlr (unsigned char b) {
    port_byte_out (IO_PS2_CTLR_CMD_REGISTER, b);
}