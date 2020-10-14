
```
# To compile C.

i386-elf-gcc -O0 -ffreestanding -c basic.c -o basic.o

# To dump the disassembled text section. -M changes the disassembly syntax.

i386-elf-objdump -d basic.o
i386-elf-objdump -M intel -d basic.o

# To link an object file output by gcc.

i386-elf-ld -O0 -o basic.bin -Ttext 0x0 --oformat binary basic.o


# To disassemble with ndisasm.

ndisasm -b 32 basic.bin

```