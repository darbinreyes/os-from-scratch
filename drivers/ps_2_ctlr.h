/*

===.md

    * Remark:
        * I find it surprising that the SMCS controller datasheet refers
    specifically to I/O port numbers, e.g. "I/O address hex 60", I am accustomed to
    chip data sheets which are described independent of what the engineer intended
    to use them for. From this I must infer that this device is designed to be used
    very specifically, viz. wired up with an x86 CPU and connected in some manner to
    I/O port 0x60 and 0x64.

    * I/O buffer (= register) naming conventions:
        * Output means "output of the controller".
        * Input means "input of the controller".
        * Status register. A single, 8-bit, read-only register, "at I/O address" 0x64. Remark: The use of the phrase "at I/O address" implies that each register is conceived as existing within the x86 CPU's I/O address rather than being electrically connected to I/O pins on the CPU chip. Read restrictions: None.
        * Output buffer. A single, 8-bit, read-only register, "at I/O address" 0x60. Two things arrive here, for the CPU. 1. Scan codes that the controller has received from the PS/2 device (e.g. keyboard). 2. Response data from the controller, for commands that the CPU has sent to the controller, if expected by the CPU. Read restrictions: Only when the output buffer is full (see status register bit 0).
        * Input buffer. A single, 8-bit, write-only register, "at I/O address" 0x60 **OR** 0x64. Two things arrive here, for the controller. 1. Data for the device (e.g. keyboard). 2. A command for the controller. The controller distinguishes between #1, #2, according to whether the CPU write was "at I/O address" 0x60, 0x64, respectively. A write "at I/O address" 0x60 has one additional use, data from the CPU to the controller, for a command that the CPU sent to the controller, if expected by the controller.

    * [] Status register bit 3 def.? Ambiguous. [TEst if bit 3  is set after sending the disable scanning command.]
      * [?If 0x60, status register bit 3 is 0. If 0x64, status register bit 3 is 1.]. Write restrictions: Only when the input buffer is empty (see status register bit 1).]
      * vs.
      * [Status register bit 3 can be used to determine if the next write will be sent to the device (e.g. keyboard) or to the controller.]
      * vs.
      * Both definitions are equivalent.

===

*/

#ifndef __PS_2_CTLR_H__
#define __PS_2_CTLR_H__

/*

    I/O port addresses used for communication with the PS/2 controller.

*/
#define IO_PS2_CTLR_DATA          (0x60) // CTLR = ConTroLleR.
#define IO_PS2_CTLR_STAT_REGISTER (0x64)
#define IO_PS2_CTLR_CMD_REGISTER  (0x64)



typedef
enum _ctlr_cmd_t {
    R_CMD_BYTE             = 0x20, // [] Test.                                    // Reads "controller config. byte." See osdev/8042 def.
    W_CMD_BYTE             = 060,  // [] Test.                                    //
    SELF_TEST              = 0xAA, // Response 0x55 = passed. 0xFC = test failed. //
    INTERFACE_TEST         = 0xAB, // Response 0x00 = test passed.                // osdev/8042 def. = "test **1st** PS/2 port". // Will not test response 0x01, 0x02, 0x04, 0x04.
  //DIAG_DUMP              = 0xAC, // Will not use.                               //
    DISABLE_DEV            = 0xAD, // [] Test. disables KDB.                      // osdev/8042 def. = "disable **1st** PS/2 port".
    ENABLE_DEV             = 0xAE, // [] Test. enables KDB.                       // osdev/8042 def.  = "enable  **1st** PS/2 port".
  //R_INPUT_PORT           = 0xC0, // Will not use.                               //
    R_OUTPUT_PORT          = 0xD0, // [] Test.                                    // Writes "controller output port." See osdev/8042 def.
  //W_OUTPUT_PORT          = 0xD1, // Will not use.                               //
  //R_TEST_INPUTS          = 0xE0, // Will not use.                               //
    PULSE_OUTPUT_PORT_BIT0 = 0xFE, // 0=pulse, 1=don't pulse.                     // Note: Pulses **LOW** for 6 microseconds (us), triggers "system reset".
  //PULSE_OUTPUT_PORT_BIT1 = 0xFD  // Will not use.                               // "Gate A20".
} ctlr_cmd_t;

/*

    PS/2 controller status register bit definitions.

===.md
    * Remark:

        * This structure works fine as is. If necessary, I can byte pack it
        later. For now, I want to avoid any potential performance implications since the
        \#pragma changes memory alignment.

        * [From the gcc docs](https://gcc.gnu.org/onlinedocs/gcc-4.4.4/gcc/Structure_002dPacking-Pragmas.html)
            * "... change the maximum alignment of members of structures ... "

        * The \#pragma usage is:

            * \#pragma pack(push,1)
            * ... struct ...
            * \#pragma pack(pop)
===

*/
#define PS2_BUF_EMPTY (0)
#define PS2_BUF_FULL  (1)
typedef
struct _ps_2_ctrl_stat_t {
    unsigned char obuf_full:1;     // bit 0 //                  // Value on power on = 0. // 0 = Buffer empty. 1 = buffer full.
    unsigned char ibuf_full:1;     //                           // Value on power on = 0. // 0 = Buffer empty. 1 = buffer full.
    unsigned char sys_flg:1;       //          [] How to test?  // Value on power on = 0. // 0=,1=
    unsigned char data_or_cmd:1;   //          [] Test.         // Value on power on = 0. // 0=,1=
    unsigned char dev_inhibited:1; //          [] Test.         // Value on power on = 1. // 0=,1=
    unsigned char tx_timeout:1;    //          [] How to test?  // Value on power on = 0. // 0=,1=
    unsigned char rcv_timeout:1;   //          [] How to test?  // Value on power on = 0. // 0=,1=
    unsigned char par_err:1;       // bit 7 // [] How to test?  // Value on power on = 0. // 0=,1=
} ps_2_ctrl_stat_t;



int get_ctlr_stat(ps_2_ctrl_stat_t *stat);
int send_byte (unsigned char b);
int rcv_byte (unsigned char *b);
void send_byte_ctlr (unsigned char b);

#endif