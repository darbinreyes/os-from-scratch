#if 0
int my_function(void) {
    return 0xbaba;
}
#endif

#if 1
int my_function(void) {
    int my_var = 0xbaba;
    return my_var;
}
#endif

#if 0
int callee_function(int my_arg);

void caller_function(void) {
    callee_function(0xdede);
}

int callee_function(int my_arg) {
    return my_arg;
}
#endif


/*

The disassembly of the following function does not match Nick Blundell's text.
Why?

* Nick Blundell may have made a typo in the commands he specifies for disassembling. He is also working on a Linux system with Linux build tools. Differences are to be expected.

* If you disassemble the object file with `i386-elf-objdump`, linking hasn't happened yet, so addresses will not appear as they would in the final executable.

* The use of `ld --oformat binary` seems to throw off the disassembly. `ld --oformat elf32-i386` seems to produce more reasonable results when the disassembly looks strange.

* The safest option seems to be to use `ndisasm -b 32`. The extra instructions at the end can be ignored because:
"The only problem with disassembling machine code is that some of those bytes
may have been reserved as data but will show up as assembly instructions,
though in our simple C program we didn’t declare any data."

*/

#if 0
void my_function(void) {
    char *my_string = "Hello"; // Nick Blundell wrote this `char *my_string = "Hello";` but he meant `char my_string[] = "Hello";`
}
#endif


