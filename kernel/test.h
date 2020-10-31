#ifndef __TEST_H__
#define __TEST_H__

/* load_idt_reg:
   Load the IDT register. Defined in test.s.

   @param addr Address of the value to be loaded into the IDT register.
*/
int load_idt_reg(unsigned int addr);
#endif