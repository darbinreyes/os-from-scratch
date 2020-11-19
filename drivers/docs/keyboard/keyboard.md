* I/O buffer (= register) naming conventions:
  * Output means "output of the controller".
  * Input means "input of the controller".
  * Status register. A single, 8-bit, read-only register, "at I/O address"
    0x64. Remark: The use of the phrase "at I/O address" implies that each
    register is conceived as existing within the x86 CPU's I/O address space
    rather than being electrically connected to I/O pins on the CPU chip.
    Read restrictions: None.
  * Output buffer. A single, 8-bit, read-only register, "at I/O address"
    0x60. Two things arrive here, for the CPU.

    1. Scan codes that the controller has received from the PS/2 device
       (e.g. keyboard).
    2. Response data from the controller, for commands that the CPU has sent
       to the controller, if expected by the CPU. Read restrictions: Only
       when the output buffer is full (see status register bit 0).

  * Input buffer. A single, 8-bit, write-only register, "at I/O address"
    0x60 **OR** 0x64. Two things arrive here, for the controller.

    1. Data for the device (e.g. keyboard).
    2. A command for the controller.

  * The controller distinguishes between #1, #2, according to whether the CPU
    write was "at I/O address" 0x60, 0x64, respectively. A write "at I/O
    address" 0x60 has one additional use, data from the CPU to the controller,
    for a command that the CPU sent to the controller, if expected by the
    controller.

@remark I find it surprising that the SMCS controller datasheet refers
specifically to I/O port numbers, e.g. "I/O address hex 60", I am accustomed
to chip data sheets which are described independent of what the engineer
intended to use them for. From this I must infer that this device is
designed to be used very specifically, viz. wired up with an x86 CPU and
connected in some manner to I/O port 0x60 and 0x64.

### Scan codes

A PS/2 keyboard does not return ASCII codes. Instead it returns 1 or more bytes
called scan codes. See @doc [osdev.org]
(https://wiki.osdev.org/Keyboard#Scan_Code_Set_1) documentation for a list of
scan code values.

### Key codes

The recommended way of implementing a keyboard driver involves converting a
scan code into a single byte value called a key code. For reasons not explained
here, the recommended way of assigning key code values is by treating each key
on the keyboard as a located on a 2D grid and defining the key code as the
row and column number of the key packed into 8-bits. Specifically, the high
order 3-bits encode the row number, and the low order 5-bits encode the column
number. For example, the ESCAPE key is usually assigned the key code value 00H
because it is physically located at row 0, column 0 on the keyboard.