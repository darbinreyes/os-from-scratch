A collection of little experiments with C. For example, there is an experiment
to compare C code to the generated x86 assembly. Another experiment compares the
sizeof() various data types on my host machine's compiler, clang, (cc) to the
bochs compiler i386-elf-gcc. There is also an experiment in which I test the
behavior of getchar() to help me implement something similar inside my keyboard
driver.

The convention in this directory is to include a description/purpose of the
experiment in a comment at the top of the file.