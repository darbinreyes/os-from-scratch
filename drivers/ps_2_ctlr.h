#ifndef __PS_2_CTLR_H__
#define __PS_2_CTLR_H__

#include "../include/stdint.h"

/*!
    @typedef ctlr_cmd_t

    @discussion Commands that can be sent to the PS/2 controller.

    @constant   R_CMD_BYTE  @TODO
    @constant   W_CMD_BYTE
    @constant   SELF_TEST
    @constant   INTERFACE_TEST
    @constant   DISABLE_DEV
    @constant   ENABLE_DEV
    @constant   R_OUTPUT_PORT
    @constant   PULSE_OUTPUT_PORT_BIT0
*/
typedef
enum _ctlr_cmd_t {
    R_CMD_BYTE             = 0x20, // [] Test.                                    // Reads "controller config. byte." See osdev/8042 def.
    W_CMD_BYTE             = 0x60,  // [] Test.                                    //
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

/*!
    @defined PS2_BUF_EMPTY

    @discussion Indicates that the buffer is empty.
*/
#define PS2_BUF_EMPTY (0)

/*!
    @defined PS2_BUF_FULL

    @discussion Indicates that the buffer is full.
*/
#define PS2_BUF_FULL  (1)

/*!
    @typedef ps_2_ctrl_stat_t

    @discussion PS/2 controller status register bit definitions.

    @field  obuf_full
    @field  ibuf_full
    @field  sys_flg
    @field  data_or_cmd
    @field  dev_inhibited
    @field  tx_timeout
    @field  rcv_timeout
    @field  par_err

    @remark This structure works fine as is. If necessary, I can byte pack it
    later. For now, I want to avoid any potential performance implications
    since the #pragma changes memory alignment.

    @remark The #pragma usage is:
    #pragma pack(push,1)... struct ...#pragma pack(pop)
*/
typedef
struct _ps_2_ctrl_stat_t {
    uint8_t obuf_full:1;     // bit 0 //                  // Value on power on = 0. // 0 = Buffer empty. 1 = buffer full.
    uint8_t ibuf_full:1;     //                           // Value on power on = 0. // 0 = Buffer empty. 1 = buffer full.
    uint8_t sys_flg:1;       //          [] How to test?  // Value on power on = 0. // 0=,1=
    uint8_t data_or_cmd:1;   //          [] Test.         // Value on power on = 0. // 0=,1=
    uint8_t dev_inhibited:1; //          [] Test.         // Value on power on = 1. // 0=,1=
    uint8_t tx_timeout:1;    //          [] How to test?  // Value on power on = 0. // 0=,1=
    uint8_t rcv_timeout:1;   //          [] How to test?  // Value on power on = 0. // 0=,1=
    uint8_t par_err:1;       // bit 7 // [] How to test?  // Value on power on = 0. // 0=,1=
} ps_2_ctrl_stat_t;

/*! See .c */
int get_ctlr_stat(ps_2_ctrl_stat_t *stat);

/*! See .c */
int send_byte (uint8_t b);

/*! See .c */
int rcv_byte (uint8_t *b);

/*! See .c */
void send_byte_ctlr (uint8_t b);

#endif