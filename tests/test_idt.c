#include "../kernel/idt.h"

/* @IMPORTANT `INT n` instructions always skip pushing error codes onto the
   stack. */

//__asm__("int $0"); // Test IDT vector 0.
//__asm__("int $1");

void test_v0_intr(void) {
    // Test Divide Error Interrupt
    __asm__("mov $0, %eax");
    __asm__("div %eax");
}



void test_v13_intr(void) {
    /* This generates the #GP/13 interrupt since the IDT currently has only 33
       entries. */
    __asm__("int $34");
}

void test_all_idt(void) {
    init_interrupts();

    /* @remark These test cases remain commented because most interrupts push
    a return address (EIP) value on the stack that points to the instruction
    that generated the exception. Thus simply returning from the interrupt
    handler effectively calls the interrupt handler forever.*/
    //test_v0_intr();
    //test_v13_intr();
}