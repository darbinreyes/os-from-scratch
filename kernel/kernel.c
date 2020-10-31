/*

Remarks:
On moving the IDT into C.

gcc is not reporting out of bounds access on arrays but cc does. e.g. int a[3]; int t = a[4]; does not generate an error.

external variables do not appear to be working correctly e.g. unsigned int idt[1] = {0xDEADBEEF}; // printing idt[0] prints all 0's. Works fine as local variable.

*/

// []IMPORTANT: Add param checks in all C functions. []Avoid/remove unnecessary/dangerous macros. - Elements of Programming Style - Brian Kernighan [] fully specify function pre and post conditions.

/* [Index of Directives](https://gcc.gnu.org/onlinedocs/cpp/Index-of-Directives.html#Index-of-Directives)
    [] How to ensure my integers are of the size I expect?
       How can I implement my own version the "C99 <inttypes.h> <stdint.h>"?

    // uint16_t tw; // Generates compiler error "unknown type".
    #if sizeof int != 4 // Does not work. "The preprocessor does not know anything about types in the language. Therefore, sizeof operators are not recognized in ‘#if’"

    A preprecessor NO-OP = // "‘#’ followed by a newline, with only whitespace (including comments) in between. A null directive is understood as a preprocessing directive but has no effect on the preprocessor output. "

    Standard pre-defined macros e.g.:
        __FILE__
        __LINE__

    In conclusion, the int sizes should be tested at run time using size of. The target architecture determines the integer sizes, so as long as you are only targeting a single architecture you only need to check the sizes once.

    //#if 0 == 1
        // #error "caca" // "The directive ‘#error’ causes the preprocessor to report a fatal error."
        // #warning "pipi" // Just a warning, not fatal.
    //#endif
*/

#include "../drivers/screen.h"
#include "../mylibc/mylibc.h"
#include "idt.h"

extern uint64_t idt[];

int main(void) {

    clear_screen();
    print_at("Edsger Dijkstra!\n", 0, 0);
    init_idt();
    unsigned char *t = (unsigned char *) &idt[0];

    for (int i = 7; i >= 0; i--) {
        print_byteh(t[i], 0);
    }

    return 0;
}
