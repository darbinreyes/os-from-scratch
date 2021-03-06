/*!
    @header Entry point for the kernel.

    @discussion

    @remark i386-elf-gcc is not reporting out of bounds access on arrays but cc
    does. e.g. int a[3]; int t = a[4]; does not generate an error.
*/


#include "../drivers/screen.h"
#include "../include/stdint.h"
#include "../include/stdio.h"
#include "idt.h"

extern uint64_t idt[]; // @IMPORTANT Remember the kernel.bin size limit!

/************************** Testing *******************************************/
#ifdef TEST_MODE
#include "../tests/test_all.h"
int main(void) {
    clear_screen();
    print_at("Edsger Dijkstra!\n", 0, 0);
    test_all();
    return 0;
}
#else
/******************************************************************************/

/*!
    @function    main

    @discussion Entry point for the kernel.

    @result 0
*/
int main(void) {
    clear_screen();
    print_at("Edsger Dijkstra!\n", 0, 0);
    init_interrupts();

    while(1)
        ;


    return 0;
}
#endif /* TEST_MODE */