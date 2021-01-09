/*
    Can I execute a binary from i386-elf-gcc on my Mac?
    ANS: No.
********************************************************************************
    Compile with:
    i386-elf-gcc -c c0.c
    i386-elf-ld c0.o --oformat elf32-i386
    i386-elf-ld c0.o --oformat coff-i386
    Compilation is successful but I cannot execute it on my mac. Error is:
    -bash: ./a.out: cannot execute binary file
*/
int main(void) {
    return 0;
}