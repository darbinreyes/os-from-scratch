This repo. is a continuation of: https://github.com/darbinreyes/subparprogrammer/tree/master/c_projects/project1
===

This directory contains things related to my study of Writing a
**[Simple Operating System - from Scratch by Nick Blundell](https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf)**

I wanted to know how to go about implementing bare bones multiprogramming,
inspired by my study of Operating System Concepts, Galvin. This text provides a
path toward that end.


The text eventually suggests that our source code be well organized, as follows:

* We will copy the directory structure of the Linux kernel as follows:
    * [Besides keeping our code organized as it grows, we want our makefile to facilitate the addition of new source files and easy incremental testing with our emulator (bochs/qemu)]
    * boot/ , anything related to booting, including the boot sector and its dependencies, basically all 16-bit real mode code. e.g.:
        * boot_sect.asm
        * print_string.asm
        * gdt.asm
        * switch_to_pm.asm
    * kernel/
        * The kernel’s main file, and any other parts of the kernel code. But no device driver specific code here. Right now, just:
        * kernel.c
    * drivers/
        * any hardware specific driver code.
