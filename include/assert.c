/*!
    @function dead_loop

    @discussion Infinite loop.

*/
void dead_loop(void) {
    while (1)
        ;
}

/*!
    @function print_assert

    @discussion Prints a message associated with the assert macro. @doc
    [assert.h].

*/
void print_assert(char *e, char *f, int l) {
    print(f);
    print(":");
    print_uint32h(l);
    print(": failed assertion `");
    print(e);
    print("'\n");
}


