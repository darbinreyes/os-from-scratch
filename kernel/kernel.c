/*!
    @header Entry point for the kernel.

    @discussion

    @remark i386-elf-gcc is not reporting out of bounds access on arrays but cc
    does. e.g. int a[3]; int t = a[4]; does not generate an error.
*/


#include "../drivers/screen.h"
#include "../include/stdint.h" // uint64_t
#include "idt.h"

/************************** Testing *******************************************/
#include "../include/test_assert.h"
#include "../include/test_stdlib.h"
/******************************************************************************/

extern uint64_t idt[]; // @IMPORTANT Remember the kernel.bin size limit!

/*!
    @function    main

    @discussion Entry point for the kernel.

    @result 0
*/
int main(void) {

    clear_screen();
    print_at("Edsger Dijkstra!\n", 0, 0);
    test_atoi();
    test_assert();
    init_idt();
    unsigned char *t = (unsigned char *) &idt[0];

    for (int i = 7; i >= 0; i--) {
        print_byteh(t[i], 0);
    }

    return 0;
}
