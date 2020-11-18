/*!
    @function    port_byte_in
    @discussion C wrapper for the `in` instruction.

    @param    port    The 16-bit  I/O port number to read from.

    @result The 8-bit value read from the I/O port.
*/
unsigned char port_byte_in (unsigned short port) {

    unsigned char result;

    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port) );

    return result;
}

/*!
    @function    port_byte_out
    @discussion C wrapper for the `out` instruction.

    @param    port    The 16-bit I/O port number to write to.
    @param    data    The 8-bit value to write.
*/
void port_byte_out (unsigned short port, unsigned char data) {
    __asm__("out %% al, %%dx" : :"a" (data), "d" (port) );
}
