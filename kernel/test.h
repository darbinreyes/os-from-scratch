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

void init_pics(void);

#define INTR_V_N_HANDLER_FUNC_NAME(vn) intr_v##vn##_handler

#define INTR_V_N_HANDLER_FUNC(vn) void INTR_V_N_HANDLER_FUNC_NAME(vn)(void)

//#define INTR_V_N_HANDLER_FUNC(vn) INTR_V_N_HANDLER_FUNC_NAME(0)

//void intr_v0_handler(void);

INTR_V_N_HANDLER_FUNC(0);
INTR_V_N_HANDLER_FUNC(1);
INTR_V_N_HANDLER_FUNC(2);
INTR_V_N_HANDLER_FUNC(3);
INTR_V_N_HANDLER_FUNC(4);
INTR_V_N_HANDLER_FUNC(5);
INTR_V_N_HANDLER_FUNC(6);
INTR_V_N_HANDLER_FUNC(7);
INTR_V_N_HANDLER_FUNC(8);
INTR_V_N_HANDLER_FUNC(9);
INTR_V_N_HANDLER_FUNC(10);
INTR_V_N_HANDLER_FUNC(11);
INTR_V_N_HANDLER_FUNC(12);
INTR_V_N_HANDLER_FUNC(13);
INTR_V_N_HANDLER_FUNC(14);
//INTR_V_N_HANDLER_FUNC(15); // RESERVED
INTR_V_N_HANDLER_FUNC(16);
INTR_V_N_HANDLER_FUNC(17);
INTR_V_N_HANDLER_FUNC(18);
INTR_V_N_HANDLER_FUNC(19);
INTR_V_N_HANDLER_FUNC(20);
INTR_V_N_HANDLER_FUNC(21);

// INTR_V_N_HANDLER_FUNC(22); // RESERVED
// INTR_V_N_HANDLER_FUNC(23);
// INTR_V_N_HANDLER_FUNC(24);
// INTR_V_N_HANDLER_FUNC(25);
// INTR_V_N_HANDLER_FUNC(26);
// INTR_V_N_HANDLER_FUNC(27);
// INTR_V_N_HANDLER_FUNC(28);
// INTR_V_N_HANDLER_FUNC(29);
// INTR_V_N_HANDLER_FUNC(30);
// INTR_V_N_HANDLER_FUNC(31); // RESERVED

INTR_V_N_HANDLER_FUNC(32);
INTR_V_N_HANDLER_FUNC(33);

#endif