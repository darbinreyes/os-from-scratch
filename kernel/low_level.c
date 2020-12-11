#include "low_level.h"

/*!
    @function    inb
    @discussion C wrapper for the `in` instruction.

    @param    port    The 16-bit  I/O port number to read from.

    @result The 8-bit value read from the I/O port.
*/
uint8_t inb (uint16_t port) {

    uint8_t result;

    // @TODO Replace with assembly code wrapper per lil OS book.

    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port) );

    return result;
}

/*!
    @function    outb
    @discussion C wrapper for the `out` instruction.

    @param    port    The 16-bit I/O port number to write to.
    @param    data    The 8-bit value to write.
*/
void outb (uint16_t port, uint8_t data) {
    __asm__("out %% al, %%dx" : :"a" (data), "d" (port) );
}
