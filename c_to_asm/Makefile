MAINFILE := c0

all: $(MAINFILE).bin

#%.bin: %.o
#	i386-elf-ld -O0 -o $@ -Ttext 0x0 --oformat binary $^

%.bin: %.o
	i386-elf-ld -O0 -o $@ -Ttext 0x0 --oformat elf32-i386 $^

$(MAINFILE).o: $(MAINFILE).c
	i386-elf-gcc -O0 -Wall -Wextra -ffreestanding -c $< -o $@

ndis: $(MAINFILE).bin
	ndisasm -b 32 $<

objd: $(MAINFILE).o
	i386-elf-objdump -d $< -M intel

clean:
	rm -Rf *.bin *.o