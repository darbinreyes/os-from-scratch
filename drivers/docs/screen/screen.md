The screen device we use here is called the "IBM VGA". This is a screen
device from the late 1990's. The low level interface to this device
involves:

1. Over 300 internal registers.
2. MMIO, the device in its default mode responds to host memory access in a
   range starting at address 0xB8000 up to 0xBFFFF.
3. About a dozen I/O ports are used to access registers in #1.

* Links:
* [A concise intro. to the IBM VGA device]
(https://wiki.osdev.org/VGA_Hardware)
* [Deep dive, every possible detail you might want to know. Well written.]
(http://www.osdever.net/FreeVGA/home.htm#vga)
* [Further reading, home page of above link]
(http://www.osdever.net/FreeVGA/home.htm)
* [Manipulating the Text-mode Cursor]
(http://www.osdever.net/FreeVGA/vga/textcur.htm)
* [Cathode Ray Tube Controller (CRTC) Registers, in here we find the registers
that allow us to control text mode cursor position]
(http://www.osdever.net/FreeVGA/vga/crtcreg.htm)

@remark In 1998, engineers had to reference this immense documentation on
physical paper tomes, flipping back and forth to cope with inter-chapter
references. Sounds painful.

@remark In order to control 300+ internal VGA registers with roughly 12 I/O
ports a register indexing scheme is used. For example, an I/O port will be
used to provide an index, which selects a particular register, then a second
I/O port is used to read/write a single byte of data to the index-selected
register. This scheme is used with the 0x3D4 and 0x3D5 I/O ports.

