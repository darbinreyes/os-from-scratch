/*

    test.s: Functions implemented in x86 assembly.
    test.h: C function declarations corresponding to the functions implemented in test.s.
    Make these functions, which are implemented in test.s, in x86 assembly,
    callable from any *.c C code file that includes it.

    Functions test.s assembly to *.c

*/
#ifndef __TEST_H__
#define __TEST_H__

#include "../mylibc/mylibc.h"

/* load_idt_reg:
   Load the IDT register. Defined in test.s.

   @param addr Address of the value to be loaded into the IDT register.
*/
int load_idt_reg(uint32_t addr);

void intr_v0_handler(void);
#endif