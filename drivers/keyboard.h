#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "../mylibc/mylibc.h"

/*!
    @typedef    ps2_kbd_cmd_t

    @discussion Commands that can be sent to a PS/2 keyboard device.

    @constant   SET_LEDS
    @constant   ECHO
    @constant   GET_SET_SCAN_CODE_SET
    @constant   IDENTIFY_KBD
    @constant   ENABLE_SCANNING
    @constant   DISABLE_SCANNING
    @constant   SET_DEFAULTS
    @constant   RESET_AND_SELF_TEST
*/
typedef
enum _ps2_kbd_cmd_t {
    /***************************************************************************
    * @IMPORTANT
      * If a command expects data, send it immediately after the command.
      * You may have to insert a wait time before sending expected data. And
        before checking for the response.
      * "Response: default" means "Response: 0xFA=ACK/0xFE=RESEND."
    ***************************************************************************/
    // @TODO Cleanup ugly comments.
    SET_LEDS = 0xED, // [x] Tested. Expects Data: Yes. Response: default.
    ECHO = 0xEE, /* [x] Tested. Expects Data: No. Response:
                    0xEE=echo/0xFE=RESEND. */
    /***************************************************************************
    * GET_SET_SCAN_CODE_SET
      * [x] Tested GET returns 0x02=SCAN_CODE_SET_2. Expects Data: Yes.
        0x00=Get scan code set number. Response:  ACK, then the scan code set
        number. See spec. for other responses.
      * FYI: If translation is enabled, and by **default** it is, the
        controller will translate "scan code set 2" into "scan code set 1".
    ***************************************************************************/
    GET_SET_SCAN_CODE_SET = 0xF0, // See Above.
    IDENTIFY_KBD = 0xF2, /* [x] Tested. Expects Data: No. Response:  ACK, then
                            0, 1, or 2 bytes identifying the keyboard.
                            // [x] Tested, received (0xAB, 0x41) = "MF2 keyboard
                            with translation enabled in the PS/2 controller".
                            // See spec. for other responses, e.g.
                            0x00="standard PS/2 mouse". */
    /***************************************************************************
      @remark "typematic rate and delay", "make/release", "autorepeat" related
      commands will not be used for know.
    ***************************************************************************/
    ENABLE_SCANNING = 0xF4, // [x] Tested. Expects Data: No. Response: default.
    /***************************************************************************
    * DISABLE_SCANNING
      * [x] Tested. Expects Data: No. Response: default.
      * Does this clear the inhibit bit in the status register? No.
      * Does this stop the kbd from sending scan codes? Yes.
    ***************************************************************************/
    DISABLE_SCANNING = 0xF5, // See above.
    SET_DEFAULTS = 0xF6, // [x] Tested. Expects Data: No. Response: default.
    /* RESEND_CMD = 0xFE, [x] Tested. Causes BOCHS PANIC. Expects Data: No.
                          Response: 0xXX="Previously sent byte"/0xFE=RESEND. */
    RESET_AND_SELF_TEST = 0xFF /* [x] Tested. but returns ACK instead instead
                                  off 0xAA. Expects Data: No. Response:
                                  0xAA=self-test passed,
                                  0xFC||0xFD=self-test failed, 0xFE=RESEND. */
} ps2_kbd_cmd_t;

/*!
    @typedef    ps2_kbd_cmd_data_t

    @discussion PS/2 keyboard command arguments/data. To be sent subsequent to
    the command.

    @constant   GET_SCAN_CODE_SET
    @constant   SET_SCAN_CODE_SET_1
    @constant   SET_SCAN_CODE_SET_2
    @constant   SET_SCAN_CODE_SET_3
    @constant   SCROLL_LOCK_LED_ON
    @constant   NUM_LOCK_LED_ON
    @constant   CAPS_LOCK_LED_ON
    @constant   KBD_LEDS_OFF
*/
typedef
enum _ps2_kbd_cmd_data_t {
    GET_SCAN_CODE_SET = 0x00,
    SET_SCAN_CODE_SET_1 = 0x01, // Not tested.
    SET_SCAN_CODE_SET_2 = 0x02, // Not tested.
    SET_SCAN_CODE_SET_3 = 0x03, // Not tested.
    SCROLL_LOCK_LED_ON = 0x01,
    NUM_LOCK_LED_ON = 0x02,
    CAPS_LOCK_LED_ON = 0x04,
    KBD_LEDS_OFF = 0x00
}  ps2_kbd_cmd_data_t;

/*!
    @typedef    ps2_kbd_rsp_t

    @discussion Responses from a PS/2 keyboard.

    @constant   ECHO_R
    @constant   ACK
    @constant   RESEND
*/
typedef
enum _ps2_kbd_rsp_t {
    /* KEY_DETECT_OR_BUF_ERR_0 = 0x00, // Not tested. Not sure how to simulate
       this error in BOCHS so can't test. */
    /* SELF_TEST_PASSED = 0xAA, // Not returned by BOCHS. BOCHS returns ACK for
       self test. */
    ECHO_R = 0xEE, // [x] Tested.
    ACK = 0xFA, // [x] Tested.
    // SELF_TEST_FAILED = 0xFC, // Not returned by BOCHS.
    RESEND = 0xFE, // [] TODO: If used, limit retries to 3.
    /* KEY_DETECT_OR_BUF_ERR_1 = 0xFF // Not tested. Not sure how to simulate
       this error in BOCHS so can't test. */
} ps2_kbd_rsp_t;

/*! See .c */
int get_scan_code(unsigned char *sc);
/*! See .c */
char scan_code_to_ascii (unsigned char sc);
/*! See .c */
int get_scan_code2(unsigned char *sc);
#endif