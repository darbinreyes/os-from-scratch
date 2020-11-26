/*******************************************************************************
    This C program contains code compiled on my host machine using clang, its
    purpose is to compare the output to equivalent C programs compiled with
    i386-elf-gcc, e.g. to compile the size of (void*).
*******************************************************************************/
#include <stdio.h>

int a[2] = {2,5};

void foo(void) {
    printf("Dijkstra!\n");
}

/*

i386gcc_vs_cc.c:12:1: warning: incompatible pointer to integer conversion initializing
      'unsigned long' with an expression of type 'void (*)(void)' [-Wint-conversion]
func_ptr
^~~~~~~~
i386gcc_vs_cc.c:12:1: error: initializer element is not a compile-time constant
func_ptr
^~~~~~~~

*/
void (*func_ptr)(void) = foo;
#define ENTRY_COUNT 34
unsigned long idt[ENTRY_COUNT] = {
//func_ptr
(unsigned long)foo
};


int main(void) {
    //void (*func_ptr)(void) = foo;
    /*

    i386gcc_vs_cc.c:9:28: warning: array index 2 is past the end of the array
      (which contains 2 elements) [-Warray-bounds]

    */

    printf("a[x] = %d.\n", a[1]); //  warning: array index 3 is past the end of the array
    printf("sizeof(int) = %lu.\n", sizeof(int));
    printf("sizeof(size_t) = %lu.\n", sizeof(size_t));
    printf("sizeof(void*) = %lu.\n", sizeof(void*));
    printf("sizeof(unsigned long) = %lu.\n", sizeof(unsigned long));
    printf("sizeof(foo) = %lu.\n", sizeof(foo));
    foo();
    func_ptr();
    printf("sizeof(func_ptr) = %lu.\n", sizeof(func_ptr));
    printf("%%p func_ptr = %p.\n", func_ptr);
    printf("%%lx func_ptr = %lx.\n", (unsigned long)func_ptr);
    printf("%%lx idt[0] = %lx.\n", idt[0]);
    printf("%%lx sizeof(idt) = %lx.\n", sizeof(idt));
    return 0;
}