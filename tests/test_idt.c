#include "../kernel/idt.h"

void test_int_0(void) {
    /* @IMPORTANT `INT n` instructions always skip pushing error codes onto the
       stack. */
    __asm__("int $0"); // Generates a "fake" #DE.
}

void test_int_1(void) {
    __asm__("int $1"); // Generates a "fake" #DB.
}

void test_v0_intr(void) {
    // Generate "real" Divide Error Interrupt (#DE)
    __asm__("mov $0, %eax");
    __asm__("div %eax");
}



void test_v13_intr(void) {
    /* This generates the #GP/13 interrupt since the IDT currently has only 33
       entries. If this test fails, check if the number of IDT entries has
       changed. Notice that err_code->seg_sel_idx == 34, which equals the INT
       instruction immediate operand below. */
    __asm__("int $34");
}

void test_all_idt(void) {
    init_interrupts();

    /* @remark These test cases other than those using the `INT n` instruction
    must remain commented because most interrupts push a return address (EIP)
    value on the stack that points to the instruction that generated the
    exception/interrupt. Thus simply returning from the interrupt handler effectively
    calls the interrupt handler forever. For example, returning from the divide
    by 0 interrupt (#DE), returns to the DIV instruction that caused the error,
    which divides by 0 again, generates #DE again, and so on, forever.
    @TODO Right now, the interrupt handlers do nothing, in some cases, a
    corrective action can be taken which allows the offending instruction to
    execute without generating an interrupt, for instance, with a page fault. */

    //test_int_0();
    //test_int_1();
    //test_v0_intr();
    //test_v13_intr();
}