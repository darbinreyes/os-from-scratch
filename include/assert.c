/*!
    @header Standard C Header
*/

#include "assert.h"

/*!
    @function dead_loop

    @discussion Infinite loop. @TODO The standard assert macro calls abort()
    instead of looping.

*/
void dead_loop(void) {
#ifndef NDEADLOOP
    while (1)
        ;
#endif
}

/*!
    @function print_assert

    @discussion Prints a message associated with the assert macro. @doc
    [assert.h on your local machine](include/assert.h)
    @doc [man page for assert](man assert). @TODO This is a temporary
    approximation to what the standard assert macro prints. Print should be
    replaced by printf().

    @param    e    A string, the expression that was evaluated as false by the
                   assert macro.

    @param    f    A string, the name of the file in which the assert macro was
                   called.

    @param    l    The line number at which the assert macro was called.

*/
void print_assert(char *e, char *f, int l) {
    print(f);
    print(":");
    print_uint32h(l); // @TODO Print in decimal.
    print(": failed assertion `");
    print(e);
    print("'\n");
}