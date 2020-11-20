# Make file for i386gcc_vs_cc.c
# $^ = all target dependency files.
# $< = first target dependency file.
# $@ = target file.

MAINFILE := i386_elf_gcc_vs_x64_macho_cc

all: $(MAINFILE).bin

run:
	./$(MAINFILE).bin

$(MAINFILE).bin: $(MAINFILE).c
	cc $< -o $@

clean:
	rm -Rf *.bin