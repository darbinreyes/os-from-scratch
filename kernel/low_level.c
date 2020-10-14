/*

    C wrapper functions for in-line assembly.

    In general, we must use in-line assembly when we wish to use a CPU
    instruction explicitly. The most common reason to do this is to access some
    CPU feature that is not available to us from the standard C programming
    syntax.




    Remarks:
    [Documentation for in-line assembly in C](https://gcc.gnu.org/onlinedocs/gcc/Using-Assembly-Language-with-C.html#Using-Assembly-Language-with-C)
    Use `gcc -masm` to set the in-line assembly syntax e.g. between GAS and NASM.

*/

unsigned char port_byte_in (unsigned short port) {
    /*

        Read a byte from the specified I/O port
        "=a" (result) means: put AL register in value result when finished.
        "d" (port) means: load EDX register with port.

    */

    unsigned char result;

    /*

        Read from I/O port specified in DX into AL.

    */
    __asm__("in al, dx" : "=a" (result) : "d" (port) );

    /* __asm__("in %%dx, %%al" : "=a" (result) : "d" (port) ); */ // Author's given assembly, I change the assembly syntax to NASM to be consistent with our other assembly code like that in the boot sector.

    return result;
}

void port_byte_out (unsigned short port, unsigned char data) {
    /*

        "a" (data) means: load EAX with data.
        "d" (port) means: load EDX with port.

    */

    /*

        Write to I/O port DX the value specified in AL

    */

    __asm__("out dx, al" : :"a" (data), "d" (port) );

    /*__asm__("out %% al, %%dx" : :"a" (data), "d" (port) ); See note above. */
}

// TODO: port_WORD_in()
// TODO: port_WORD_out()
