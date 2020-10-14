// []IMPORTANT: Add param checks in all C functions. []Avoid/remove unnecessary/dangerous macros. - Elements of Programming Style - Brian Kernighan [] fully specify function pre and post conditions.


// Print a single character to via direct write to video memory.

#include "../drivers/screen.h"
#include "../drivers/ps_2_ctlr.h"
#include "../drivers/keyboard.h"
#include "../mylibc/mylibc.h"

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
*/


#if 0 == 1
    #error "caca" // "The directive ‘#error’ causes the preprocessor to report a fatal error."
    // #warning "pipi" // Just a warning, not fatal.
#endif

int main(void) {
    int r;
    // unsigned char b;
    unsigned char sc[2];
    // char c;
    // ps_2_ctrl_stat_t stat;


    clear_screen();
    print_at("Edsger Dijkstra!\n", 0, 0);

    // while(1) { // Test getting both 1 byte and 2 byte scan codes using a single function.
    //     r = get_scan_code2(sc);

    //     if (r != 0) {
    //         print("Error 2.\n");
    //         return 1;
    //     }

    //     if (sc[1] != 0x00) {
    //         print(" 2 byte sc.");
    //         print("[");
    //         print_byteh(sc[0]);
    //         print("]");
    //         print("[");
    //         print_byteh(sc[1]);
    //         print("]\n");
    //     } else {
    //         print(" 1 byte sc.");
    //         print("[");
    //         print_byteh(sc[0]);
    //         print("]\n");
    //     }
    // }
    return 0;
}