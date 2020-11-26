#include "low_level.h"

/*!
    @function    port_byte_in
    @discussion C wrapper for the `in` instruction.

    @param    port    The 16-bit  I/O port number to read from.

    @result The 8-bit value read from the I/O port.
*/
uint8_t port_byte_in (uint16_t port) {

    uint8_t result;

    // @TODO Replace with assembly code wrapper per lil OS book.

    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port) );

    return result;
}

/*!
    @function    port_byte_out
    @discussion C wrapper for the `out` instruction.

    @param    port    The 16-bit I/O port number to write to.
    @param    data    The 8-bit value to write.
*/
void port_byte_out (uint16_t port, uint8_t data) {
    __asm__("out %% al, %%dx" : :"a" (data), "d" (port) );
}
