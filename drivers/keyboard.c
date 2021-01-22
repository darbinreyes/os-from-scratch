/*!
    @header PS/2 keyboard driver.
    Provides basic functions for getting input from a PS/2 keyboard.

    @discussion
    * FYI:
    * Notational conventions.
      * Some of the scan codes in "scan code set 1" correspond to keys that do
        not exist on my Apple keyboard, however, in some cases is possible to
        generate the scan code by pressing a key or combination of keys that is
        not entirely obvious. For example, the "print screen pressed" key
        has the 4 byte scan code "0xE0, 0x2A, 0xE0, 0x37". To generate this
        scan code on my Apple keyboard, press and hold the key labeled "fn",
        and then the press the key labeled "F13". This is indicated in comments
        adjacent to the corresponding scan code as "Apple Keyboard=<fn>+<F13>".
     * Some keys on my physical Apple keyboard do not generate any scan code at
       all. This should not be surprising since my physical keyboard is not
       a PS/2 keyboard.
     * FYI: "Apple Keyboard=<fn>+<F14>" does not generate a scan code but puts
       BOCHS into a full screen mode which does not seem to be accessible via
       the GUI.

    * @TODO
      * [] Test all scan codes.
      * [] short get_scn_code(void);
      * [] char getch(void);
      * [] char *prompt_user_for_str(char *prompt);
      * [] With interrupts.
      * [] Implement key state table so shift key works. 0 = released 1 =
        pressed.
      * [] @doc [See "driver model"]
        (https://wiki.osdev.org/Keyboard#Driver_Model). Also see @doc
        [My keyboard driver notes](./docs/keyboard/keyboard.md)
*/
#include "ps_2_ctlr.h"
#include "keyboard.h"
#include "screen.h"
#include "../kernel/i8259a_pic.h"
#include "../kernel/low_level.h"

/*!
    @defined    KEY_CODE_TO_ASCII_ROWS

    @discussion The maximum number of rows in the kc_rc_to_ascii and
    shift_kc_rc_to_ascii table.
*/
#define KEY_CODE_TO_ASCII_ROWS 6

/*!
    @defined    KEY_CODE_TO_ASCII_COLS

    @discussion The maximum number of columns in the kc_rc_to_ascii and
    shift_kc_rc_to_ascii table. The actual number of columns in the tables
    varies. The number of columns in each of the 6 rows is currently:
    21, 21, 21, 17, 16, 13, respectively.
*/
#define KEY_CODE_TO_ASCII_COLS 21

/*!
    @const    kc_rc_to_ascii

    @discussion Table used to convert a key code (kc) row and column (rc) to
    an ASCII character code.
*/
const char kc_rc_to_ascii[KEY_CODE_TO_ASCII_ROWS][KEY_CODE_TO_ASCII_COLS] = {
/* Apple USB Keyboard Model A1243 */
 {  '?'/*<ESC>*/,      'X'/*<F1>*/,   'X'/*<F2>*/,       'X'/*<F3>*/,   'X'/*<F4>*/,                    'X'/*<F5>*/,              'X'/*<F6>*/,                 'X'/*<F7>*/,                   'X'/*<F8>*/,                 'X'/*<F9>*/,       'X'/*<F10>*/,  'X'/*<F11>*/,                   'X'/*<F12>*/,   '?'/*<EJECT>*/,        '?'/*<F13>*/,        '?'/*<F14>*/,     '?'/*<F15>*/,         '?'/*<F16>*/,                 '?'/*<F17>*/,        '?'/*<F18>*/,    '?'/*<F19>*/         },
 {  '`',               '1',           '2',               '3',           '4',                            '5',                      '6',                         '7',                           '8',                         '9',               '0',           '-',                            '=',            '?'/*<BACKSPACE>*/,    '?'/*<Fn>*/,         '?'/*<HOME>*/,    '?'/*<PG-UP>*/,       '?'/*<NUMPAD-CLEAR/NUMLOCK>*/,'='/*NUMPAD"="NoSc*/, '/'/*NUMPAD-/*/,'*'/*NUMPAD"*"*/     },
 {  '?'/*<TAB>*/,      'q',           'w',               'e',           'r',                            't',                      'y',                         'u',                           'i',                         'o',               'p',           '[',                            ']',            '\\',                  '?'/*<DEL/NUMLOCK>*/,'?'/*<END>*/,     '?'/*<PG-DOWN>*/,     '7',                          '8',                 '9',             '-'                  },
 {  '?'/*<CAPSLOCK>*/, 'a',           's',               'd',           'f',                            'g',                      'h',                         'j',                           'k',                         'l',               ';',           '\'',                           '?'/*<RETURN>*/,                                                                                    '4',                          '5',                 '6',             '+'                  },
 {  '?'/*<L-SHIFT>*/,  'z',           'x',               'c',           'v',                            'b',                      'n',                         'm',                           ',',                         '.',               '/',                                           '?'/*<R-SHIFT>*/,                                           '?'/*<CUR-UP>*/,                        '1',                          '2',                 '3'                                   },
 {  '?'/*<L-CTRL>*/,   '?'/*<L-ALT>*/,'?'/*<L-CMD>NoSc*/,' '/*<SPACE>*/,                                                                                                                                                   '?'/*<R-CMD>NoSc*/,'?'/*<R-ALT>*/,                                '?'/*<R-CTRL>*/,                       '?'/*<CUR-LEFT>*/,   '?'/*<CUR-DOWN>*/,'?'/*<CUR-RIGHT>*/,   '0',                                               '.',             '?'/*<NUMPAD-ENTER>*/}
};

/************************************************************************************************************************************************************************************************************************************************************************************************************
Physical Key Layout by Row/Column - Apple USB Keyboard Model A1243
************************************************************************************************************************************************************************************************************************************************************************************************************
   | 0          | 1                     | 2                     | 3                 | 4               | 5    | 6    | 7               | 8               | 9              | 10               | 11                | 12                | 13          | 14    | 15    | 16    | 17    | 18    | 19    | 20    |
---|------------|-----------------------|-----------------------|-------------------|- ---------------|------|------|-----------------|-----------------|----------------|------------------|-------------------|-------------------|-------------|-------|-------|-------|-------|-------|-------|-------|
 0 | <ESC>      | <Apl-Bright-Inc>/<F1> | <Apl-Bright-Dec>/<F2> | <Apl-M-Ctrl>/<F3> | <Apl-Apps>/<F4> | <F5> | <F6> | <Apl-Prev>/<F7> | <Apl-Play>/<F8> | <Apl-Fwd>/<F9> | <Apl-Mute>/<F10> | <Apl-Vol-D>/<F11> | <Apl-Vol-U>/<F12> | <Apl-Eject> | <F13> | <F14> | <F15> | <F16> | <F17> | <F18> | <F19> |
************************************************************************************************************************************************************************************************************************************************************************************************************
   | 0          | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10        | 11   | 12    | 13    | 14       | 15     | 16        | 17          | 18      | 19      | 20        |
---|------------|---|---|---|---|---|---|---|---|---|-----------|------|-------|-------|----------|--------|-----------|-------------|---------|---------|-----------|
 1 | `          | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0         | -    | =     | <DEL> | <Apl-Fn> | <home> | <pg-up>   | <num-clear> | <num-=> | <num-/> | <num-*>   |
 2 | <TAB>      | q | w | e | r | t | y | u | i | o | p         | [    | ]     | "|"   | <delete> | <end>  | <pg-down> | 7           | 8       | 9       | <num- - > |
 3 | <CAPS>     | a | s | d | f | g | h | j | k | l | ;         | '    | <ret> | 4     | 5        | 6      | <num-+>   |
 4 | <L-SHIFT>  | z | x | c | v | b | n | m | , | / | <R-SHIFT> | <Up> | 1     | 2     | 3        |
************************************************************************************************************************************************************************************************************************************************************************************************************
  | 0        | 1                | 2         | 3       | 4         | 5                | 6        | 7      | 8      | 9       | 10  | 11 | 12      |
--|----------|------------------|-----------|---------|-----------|------------------|----------|--------|--------|---------|-----|----|---------|
5 | <L-CTRL> | L-<Option>/<alt> | <Apl-cmd> | <space> | <Apl-cmd> | R-<Option>/<alt> | <R-CTRL> | <Left> | <Down> | <Right> | 0   | .  | <enter> |
*************************************************************************************************************************************************************************************************************************************************************************************************************/

/*!
    @const    shift_kc_rc_to_ascii

    @discussion Table used to convert a key code (kc) row and column (rc) to
    an ASCII character code when the shift key is held down.
*/
const char shift_kc_rc_to_ascii[KEY_CODE_TO_ASCII_ROWS][KEY_CODE_TO_ASCII_COLS] = {
/* Apple USB Keyboard Model A1243 */
 {  '?'/*<ESC>*/,      'X'/*<F1>*/,   'X'/*<F2>*/,       'X'/*<F3>*/,   'X'/*<F4>*/,                    'X'/*<F5>*/,              'X'/*<F6>*/,                 'X'/*<F7>*/,                   'X'/*<F8>*/,                 'X'/*<F9>*/,       'X'/*<F10>*/,  'X'/*<F11>*/,                   'X'/*<F12>*/,   '?'/*<EJECT>*/,        '?'/*<F13>*/,        '?'/*<F14>*/,     '?'/*<F15>*/,         '?'/*<F16>*/,                 '?'/*<F17>*/,        '?'/*<F18>*/,    '?'/*<F19>*/         }, // 21
 {  '~',               '!',           '@',               '#',           '$',                            '%',                      '^',                         '&',                           '*',                         '(',               ')',           '_',                            '+',            '?'/*<BACKSPACE>*/,    '?'/*<Fn>*/,         '?'/*<HOME>*/,    '?'/*<PG-UP>*/,       '?'/*<NUMPAD-CLEAR/NUMLOCK>*/,'='/*NUMPAD"="NoSc*/, '/'/*NUMPAD-/*/,'*'/*NUMPAD"*"*/     },
 {  '?'/*<TAB>*/,      'Q',           'W',               'E',           'R',                            'T',                      'Y',                         'U',                           'I',                         'O',               'P',           '{',                            '}',            '|',                   '?'/*<DEL/NUMLOCK>*/,'?'/*<END>*/,     '?'/*<PG-DOWN>*/,     '7',                          '8',                 '9',             '-'                  },
 {  '?'/*<CAPSLOCK>*/, 'A',           'S',               'D',           'F',                            'G',                      'H',                         'J',                           'K',                         'L',               ':',           '\"',                           '?'/*<RETURN>*/,                                                                                    '4',                          '5',                 '6',             '+'                  },
 {  '?'/*<L-SHIFT>*/,  'Z',           'X',               'C',           'V',                            'B',                      'N',                         'M',                           '<',                         '>',               '?',                                           '?'/*<R-SHIFT>*/,                                           '?'/*<CUR-UP>*/,                        '1',                          '2',                 '3'                                   },
 {  '?'/*<L-CTRL>*/,   '?'/*<L-ALT>*/,'?'/*<L-CMD>NoSc*/,' '/*<SPACE>*/,                                                                                                                                                   '?'/*<R-CMD>NoSc*/,'?'/*<R-ALT>*/,                                '?'/*<R-CTRL>*/,                       '?'/*<CUR-LEFT>*/,   '?'/*<CUR-DOWN>*/,'?'/*<CUR-RIGHT>*/,   '0',                                               '.',             '?'/*<NUMPAD-ENTER>*/}
};

/*!
    @defined    KEY_CODE_FROM_ROW_COL(r, c)

    @discussion Macro used to create an 8-bit key code from a given row and
    column number. The row number is placed into the high order 3-bits and the
    column number is placed in the low order 5-bits.
*/
#define KEY_CODE_FROM_ROW_COL(r, c)  ( ( ( (r) & 0x07) << 5 ) | ( (c) & 0x1F) )

/*!
    @defined    NOT_A_SCAN_CODE

    @discussion Used in the scan code (sc) to key code (kc) tables to indicate
    that the scan code value normally corresponding to this entry is not defined
    in scan code set 1. Its value, 0xFF, does not correspond to a valid scan
    code in any scan code set (1, 2, or 3).
*/
#define NOT_A_SCAN_CODE 0xFF

/*!
    @defined    SCAN_CODE_TODO

    @discussion Used in the scan code (sc) to key code (kc) tables to indicate
    that the scan code value normally corresponding to this entry is assigned a
    key in scan code set 1 but it does not exist on my physical (Apple)
    keyboard. Its value, 0xFE, does not correspond to a valid scan code in any
    scan code set (1, 2, or 3).
*/
#define SCAN_CODE_TODO  0xFE

/*!
    @defined    KEY_CODE_TO_ROW(kc)

    @discussion Macro for obtaining the row part of a key code.
*/
#define KEY_CODE_TO_ROW(kc) ( ( (kc) & 0xE0 ) >> 5 )

/*!
    @defined    KEY_CODE_TO_COL(kc)

    @discussion Macro for obtaining the column part of a key code.
*/
#define KEY_CODE_TO_COL(kc) ( (kc) & 0x1F )

/*!
    @const  sc_to_kc_tbl1

    @discussion Table used for converting a **single** byte scan code (scan code
    set 1) into a key code. A scan code is used as an index into the table and
    the entry obtained is the key code. Entries that do not contain a valid key
    code are set to either SCAN_CODE_TODO or NOT_A_SCAN_CODE. The scan codes
    used to index into this table must be those that correspond to "pressed" and
    not "released". A "key released" scan code will be out of this table's
    bounds. Since the key code for a key is the same regardless of whether it
    was "pressed" or "released", this table can be used for obtaining the key
    code for a key "released" by clearing the high order bit of the scan code
    before indexing into this table. This is possible because the "released"
    scan code is identical to the "pressed" scan code, except for that the high
    order bit is set for a "released" scan code. For example, the "pressed" scan
    code for the ESCAPE key is 0x01, while the "released" scan code is 0x81.
*/
const uint8_t sc_to_kc_tbl1[] = {
                                 /* Scan Code | Key        | Tested           */
                                 /* ----------|------------|------------------*/
    NOT_A_SCAN_CODE,             /* 0x00      |            | Not a scan code. */
    KEY_CODE_FROM_ROW_COL(0, 0), /* 0x01      | <ESC>      |                  */
    KEY_CODE_FROM_ROW_COL(1, 1), /* 0x02      | 1          |*/
    KEY_CODE_FROM_ROW_COL(1, 2), /* 0x03      | 2          |*/
    KEY_CODE_FROM_ROW_COL(1, 3), /* 0x04      | 3          |*/
    KEY_CODE_FROM_ROW_COL(1, 4), /* 0x05      | 4          |*/
    KEY_CODE_FROM_ROW_COL(1, 5), /* 0x06      | 5          |*/
    KEY_CODE_FROM_ROW_COL(1, 6), /* 0x07      | 6          |*/
    KEY_CODE_FROM_ROW_COL(1, 7), /* 0x08      | 7          |*/
    KEY_CODE_FROM_ROW_COL(1, 8), /* 0x09      | 8          |*/
    KEY_CODE_FROM_ROW_COL(1, 9), /* 0x0A      | 9          |*/
    KEY_CODE_FROM_ROW_COL(1, 10),/* 0x0B      | 0          |*/
    KEY_CODE_FROM_ROW_COL(1, 11),/* 0x0C      | -          |*/
    KEY_CODE_FROM_ROW_COL(1, 12),/* 0x0D      | =          |*/
    KEY_CODE_FROM_ROW_COL(1, 13),/* 0x0E      | <BACKSPACE>|*/

    KEY_CODE_FROM_ROW_COL(2, 0),/*0x0F <TAB> p.|r,c=2,0*/
    KEY_CODE_FROM_ROW_COL(2, 1),/*0x10 q p.|r,c=2,1*/
    KEY_CODE_FROM_ROW_COL(2, 2),/*0x11 w p.|r,c=2,2*/
    KEY_CODE_FROM_ROW_COL(2, 3),/*0x12 e p.|r,c=2,3*/
    KEY_CODE_FROM_ROW_COL(2, 4),/*0x13 r p.|r,c=2,4*/
    KEY_CODE_FROM_ROW_COL(2, 5),/*0x14 t p.|r,c=2,5*/
    KEY_CODE_FROM_ROW_COL(2, 6),/*0x15 y p.|r,c=2,6*/
    KEY_CODE_FROM_ROW_COL(2, 7),/*0x16 u p.|r,c=2,7*/
    KEY_CODE_FROM_ROW_COL(2, 8),/*0x17 i p.|r,c=2,8*/
    KEY_CODE_FROM_ROW_COL(2, 9),/*0x18 o p.|r,c=2,9*/
    KEY_CODE_FROM_ROW_COL(2, 10),/*0x19 p p.|r,c=2,10*/
    KEY_CODE_FROM_ROW_COL(2, 11),/*0x1A [ p.|r,c=2,11*/
    KEY_CODE_FROM_ROW_COL(2, 12),/*0x1B ] p.|r,c=2,12*/
    KEY_CODE_FROM_ROW_COL(2, 13),/*0x1C <ENTER> p.|r,c=2,13*/

    KEY_CODE_FROM_ROW_COL(5, 0),/*0x1D <L-CTLR> p.|r,c=5,0*/

    KEY_CODE_FROM_ROW_COL(3, 1),/*0x1E a p.|r,c=3,1*/
    KEY_CODE_FROM_ROW_COL(3, 2),/*0x1F s p.|r,c=3,2*/
    KEY_CODE_FROM_ROW_COL(3, 3),/*0x20 d p.|r,c=3,3*/
    KEY_CODE_FROM_ROW_COL(3, 4),/*0x21 f p.|r,c=3,4*/
    KEY_CODE_FROM_ROW_COL(3, 5),/*0x22 g p.|r,c=3,5*/
    KEY_CODE_FROM_ROW_COL(3, 6),/*0x23 h p.|r,c=3,6*/
    KEY_CODE_FROM_ROW_COL(3, 7),/*0x24 j p.|r,c=3,7*/
    KEY_CODE_FROM_ROW_COL(3, 8),/*0x25 k p.|r,c=3,8*/
    KEY_CODE_FROM_ROW_COL(3, 9),/*0x26 l p.|r,c=3,9*/
    KEY_CODE_FROM_ROW_COL(3, 10),/*0x27 ; p.|r,c=3,10*/
    KEY_CODE_FROM_ROW_COL(3, 11),/*0x28 ' p.|r,c=3,11*/

    KEY_CODE_FROM_ROW_COL(1, 0),/*0x29 ` p.|r,c=1,0*/


    KEY_CODE_FROM_ROW_COL(4, 0),/*0x2A <L-SHIFT> p.|r,c=4,0*/

    KEY_CODE_FROM_ROW_COL(2, 13),/*0x2B \ p.|r,c=2,13*/

    KEY_CODE_FROM_ROW_COL(4, 1),/*0x2C z p.|r,c=4,1*/
    KEY_CODE_FROM_ROW_COL(4, 2),/*0x2D x p.|r,c=4,2*/
    KEY_CODE_FROM_ROW_COL(4, 3),/*0x2E c p.|r,c=4,3*/
    KEY_CODE_FROM_ROW_COL(4, 4),/*0x2F v p.|r,c=4,4*/
    KEY_CODE_FROM_ROW_COL(4, 5),/*0x30 b p.|r,c=4,5*/
    KEY_CODE_FROM_ROW_COL(4, 6),/*0x31 n p.|r,c=4,6*/
    KEY_CODE_FROM_ROW_COL(4, 7),/*0x32 m p.|r,c=4,7*/
    KEY_CODE_FROM_ROW_COL(4, 8),/*0x33 , p.|r,c=4,8*/
    KEY_CODE_FROM_ROW_COL(4, 9),/*0x34 . p.|r,c=4,9*/
    KEY_CODE_FROM_ROW_COL(4, 10),/*0x35 / p.|r,c=4,10*/
    KEY_CODE_FROM_ROW_COL(4, 11),/*0x36 <R-SHIFT> p.|r,c=4,11*/

    KEY_CODE_FROM_ROW_COL(1, 20),/*0x37 NUMPAD-* p.|r,c=1,20*/

    KEY_CODE_FROM_ROW_COL(5, 1),/*0x38 <L-ALT> p.|r,c=5,1*/

    KEY_CODE_FROM_ROW_COL(5, 3),/*0x39 <SPACE> p.|r,c=5,3*/

    KEY_CODE_FROM_ROW_COL(3, 0),/*0x3A <CAPSLOCK> p.|r,c=3,0*/

    KEY_CODE_FROM_ROW_COL(0, 1),/*0x3B <F1> p.|r,c=0,1*/
    KEY_CODE_FROM_ROW_COL(0, 2),/*0x3C <F2> p.|r,c=0,2*/
    KEY_CODE_FROM_ROW_COL(0, 3),/*0x3D <F3> p.|r,c=0,3*/
    KEY_CODE_FROM_ROW_COL(0, 4),/*0x3E <F4> p.|r,c=0,4*/
    KEY_CODE_FROM_ROW_COL(0, 5),/*0x3F <F5> p.|r,c=0,5*/
    KEY_CODE_FROM_ROW_COL(0, 6),/*0x40 <F6> p.|r,c=0,6*/
    KEY_CODE_FROM_ROW_COL(0, 7),/*0x41 <F7> p.|r,c=0,7*/
    KEY_CODE_FROM_ROW_COL(0, 8),/*0x42 <F8> p.|r,c=0,8*/
    KEY_CODE_FROM_ROW_COL(0, 9),/*0x43 <F9> p.|r,c=0,9*/
    KEY_CODE_FROM_ROW_COL(0, 10),/*0x44 <F10> p.|r,c=0,10*/

    KEY_CODE_FROM_ROW_COL(1, 17),/*0x45 <NUMLOCK> p.|r,c=1,17|Not a key on my
                                   keyboard.|Try <CLEAR>*/

    KEY_CODE_FROM_ROW_COL(0, 15),/*0x46 <SCROLL-LOCK> p.|r,c=0,15|Not a key on
                                   my keyboard.|Try <F14>*/

    KEY_CODE_FROM_ROW_COL(2, 17),/*0x47 NUMPAD-7 p.|r,c=2,17*/
    KEY_CODE_FROM_ROW_COL(2, 18),/*0x48 NUMPAD-8 p.|r,c=2,18*/
    KEY_CODE_FROM_ROW_COL(2, 19),/*0x49 NUMPAD-9 p.|r,c=2,19*/
    KEY_CODE_FROM_ROW_COL(2, 20),/*0x4A NUMPAD-"-" p.|r,c=2,20*/

    KEY_CODE_FROM_ROW_COL(3, 14),/*0x4B NUMPAD-4 p.|r,c=3,14*/
    KEY_CODE_FROM_ROW_COL(3, 15),/*0x4C NUMPAD-5 p.|r,c=3,15*/
    KEY_CODE_FROM_ROW_COL(3, 16),/*0x4D NUMPAD-6 p.|r,c=3,16*/
    KEY_CODE_FROM_ROW_COL(3, 17),/*0x4E NUMPAD-+ p.|r,c=3,17*/

    KEY_CODE_FROM_ROW_COL(4, 13),/*0x4F NUMPAD-1 p.|r,c=4,13*/
    KEY_CODE_FROM_ROW_COL(4, 14),/*0x50 NUMPAD-2 p.|r,c=4,14*/
    KEY_CODE_FROM_ROW_COL(4, 15),/*0x51 NUMPAD-3 p.|r,c=4,15*/

    KEY_CODE_FROM_ROW_COL(5, 10),/*0x52 NUMPAD-0 p.|r,c=5,10*/
    KEY_CODE_FROM_ROW_COL(5, 11),/*0x53 NUMPAD-"." p.|r,c=5,11*/

    NOT_A_SCAN_CODE, /*0x54 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x55 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x56 Not a scan code .*/

    KEY_CODE_FROM_ROW_COL(0, 11),/*0x57 <F11> p.|r,c=0,11*/
    KEY_CODE_FROM_ROW_COL(0, 12)/*0x58 <F12> p.|r,c=0,12*/
};

/*!
    @const  sc_to_kc_tbl2

    @discussion Table used for converting a **two** byte scan code (scan code
    set 1) into a key code. This table is used similarly to the table for
    converting single byte scan codes to key codes. All two byte scan codes have
    0xE0 as the first byte, the second byte of the scan code, minus 0x10, is
    used as an index into this table to obtain its key code. To obtain the key
    code of a key "released", clear the high order bit of the second byte, and
    subtract 0x10, before indexing into this table. For example, the
    NUMPAD-ENTER "pressed" scan code is 0xE0, 0x1C, to obtain its key code use
    index 0x1C - 0x10 = 0x0C into this table. Similarly, the NUMPAD-ENTER
    "released" scan code is 0xE0, 0x9C, the key code is obtained at the same
    index (0x9C & 0x7F) - 0x10 = 0x0C. Entries that do not contain a valid key
    code are set to either SCAN_CODE_TODO or NOT_A_SCAN_CODE. Note that most
    entries in this table do not contain valid key codes.
*/
const uint8_t sc_to_kc_tbl2[] = {
                                 /* Scan Code | Key        | Tested           */
                                 /* ----------|------------|------------------*/
    SCAN_CODE_TODO,/*0x10 <(Media)PREV-TRACK> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x11 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x12 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x13 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x14 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x15 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x16 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x17 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x18 Not a scan code .*/
    SCAN_CODE_TODO,/*0x19 <(Media)NEXT-TRACK> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x1A Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x1B Not a scan code .*/
    KEY_CODE_FROM_ROW_COL(5, 12),/*0x1C <NUMPAD-ENTER> p.|r,c=5,12*/
    KEY_CODE_FROM_ROW_COL(5, 6),/*0x1D <R-CTLR> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x1E Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x1F Not a scan code .*/
    SCAN_CODE_TODO,/*0x20 <(Media)MUTE> p.|r,c=,*/
    SCAN_CODE_TODO,/*0x21 <(Media)Calculator> p.|r,c=,*/
    SCAN_CODE_TODO,/*0x22 <(Media)PLAY> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x23 Not a scan code .*/
    SCAN_CODE_TODO,/*0x24 <(Media)STOP> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x25 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x26 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x27 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x28 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x29 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x2A Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x2B Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x2C Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x2D Not a scan code .*/
    SCAN_CODE_TODO,/*0x2E <(Media)VOL-DOWN> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x2F Not a scan code .*/
    SCAN_CODE_TODO,/*0x30 <(Media)VOL-UP> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x31 Not a scan code .*/
    SCAN_CODE_TODO,/*0x32 <(Media)WWW-HOME> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x33 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x34 Not a scan code .*/
    KEY_CODE_FROM_ROW_COL(1, 19),/*0x35 <NUMPAD-"/"> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x36 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x37 Not a scan code .*/
    KEY_CODE_FROM_ROW_COL(5, 5),/*0x38 <R-ALT> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x39 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x3A Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x3B Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x3C Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x3D Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x3E Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x3F Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x40 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x41 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x42 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x43 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x44 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x45 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x46 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x47 Not a scan code .*/
    KEY_CODE_FROM_ROW_COL(4, 12),/*0x48 <CUR-UP> p.|r,c=,*/
    KEY_CODE_FROM_ROW_COL(1, 16),/*0x49 <PG-UP> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x4A Not a scan code .*/
    KEY_CODE_FROM_ROW_COL(5, 7),/*0x4B <CUR-LEFT> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x4C Not a scan code .*/
    KEY_CODE_FROM_ROW_COL(5, 9),/*0x4D <CUR-RIGHT> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x4E Not a scan code .*/
    KEY_CODE_FROM_ROW_COL(2, 15),/*0x4F <END> p.|r,c=,*/
    KEY_CODE_FROM_ROW_COL(5, 8),/*0x50 <CUR-DOWN> p.|r,c=,*/
    KEY_CODE_FROM_ROW_COL(2, 16),/*0x51 <PG-DOWN> p.|r,c=,*/
    KEY_CODE_FROM_ROW_COL(0, 16),/*0x52 <INSERT> p.|r,c=,| Try <F15>*/
    KEY_CODE_FROM_ROW_COL(2, 14),/*0x53 <DEL(Not backspace)> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x54 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x55 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x56 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x57 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x58 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x59 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x5A Not a scan code .*/
    SCAN_CODE_TODO,/*0x5B <"left-GUI"> p.|r,c=,. Apple Keyboard=<fn>+<L-CMD>.*/
    SCAN_CODE_TODO,/*0x5C <"right-GUI"> p.|r,c=,. Apple Keyboard=<fn>+<R-CMD>.*/
    SCAN_CODE_TODO,/*0x5D <"apps"> p.|r,c=,*/
    SCAN_CODE_TODO,/*0x5E <"(ACPI)Power"> p.|r,c=,*/
    SCAN_CODE_TODO,/*0x5F <"(ACPI)Sleep"> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x60 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x61 Not a scan code .*/
    NOT_A_SCAN_CODE, /*0x62 Not a scan code .*/
    SCAN_CODE_TODO,/*0x63 <"(ACPI)Wake"> p.|r,c=,*/
    NOT_A_SCAN_CODE, /*0x64 Not a scan code .*/
    SCAN_CODE_TODO,/*0x65 <(Media)WWW-SEARCH> p.|r,c=,*/
    SCAN_CODE_TODO,/*0x66 <(Media)WWW-FAVS> p.|r,c=,*/
    SCAN_CODE_TODO,/*0x67 <(Media)WWW-REFRESH> p.|r,c=,*/
    SCAN_CODE_TODO,/*0x68 <(Media)WWW-STOP> p.|r,c=,*/
    SCAN_CODE_TODO,/*0x69 <(Media)WWW-FORWARD> p.|r,c=,*/
    SCAN_CODE_TODO,/*0x6A <(Media)WWW-BACK> p.|r,c=,*/
    SCAN_CODE_TODO,/*0x6B <(Media)WWW-My-Computer> p.|r,c=,*/
    SCAN_CODE_TODO,/*0x6C <(Media)WWW-Email> p.|r,c=,*/
    SCAN_CODE_TODO/*0x6D <(Media)WWW-Media-Select> p.|r,c=,*/
};

/*******************************************************************************
The only remaining scan codes are the following 4-byte and 6-byte scan codes
are not handled by this driver.

Scan code                          | Meaning               | Remarks
-----------------------------------|-----------------------|--------------------
0xE0, 0x2A, 0xE0, 0x37             | print screen pressed  | 0xE0_0x2A is not a valid 2 byte scan code. Apple Keyboard=<fn>+<F13>.
0xE0, 0xB7, 0xE0, 0xAA             | print screen released | 0xE0_0xB7 is not a valid 2 byte scan code.
0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5 | pause pressed         | Only scan code starting with 0xE1. Apple Keyboard=<fn>+<F15>.

@remark The "print screen" pressed vs. released scan codes don't follow a simple
1-bit flip pattern. There does seem to be a pattern, which I worked out in
Evernote. But since this is the only 4 byte scan code, and the first pair
of bytes does not correspond to a valid 2 byte scan code, there is no downside
to special casing this scan code.

@remark There is no scan code for "pause key released" (it behaves as
if it is released as soon as it's pressed)

@IMPORTANT @TODO Make sure the function that gets scan codes works correctly
in case more than 2 byte scan codes are received. Note the prefix bytes.
*******************************************************************************/

/*!
    @function   get_key_code

    @discussion Converts a scan code into a key code.

    @param  sc  The scan code.

    @result The key code.
*/
uint8_t get_key_code(uint8_t sc) {
    // @TODO Handle multi-byte scan codes correctly.

    if (sc == 0xE0 || sc == 0xE1) // Multi-byte scan codes.
        return 0xFF; // For now.

    if (sc > 0x00 && sc < 0x59) { // A valid single byte scan code.
        return sc_to_kc_tbl1[sc];
    }

    return 0xFF;
}

/*!
    @function   scan_code_to_ascii

    @discussion Converts a scan code to an ASCII code.

    @param  sc  The scan code.

    @result An ASCII character.
*/
char scan_code_to_ascii (uint8_t sc) { // @TODO
    uint8_t kc, r, c;

    kc = get_key_code(sc);

    if(kc != 0xFF) {
        r = KEY_CODE_TO_ROW(kc);
        c = KEY_CODE_TO_COL(kc);
        // [] Improve this. e.g. use assert.
        if (r >= KEY_CODE_TO_ASCII_ROWS || c >= KEY_CODE_TO_ASCII_COLS)
            return '!';

        return kc_rc_to_ascii[r][c];
    }

    return '^';
}

/*!
    @function   get_scan_code

    @discussion Polling based implementation. Returns a 1 byte scan code from
    the keyboard.

    @param  sc  Pointer in which to return the single scan code.

    @result 0 on success. Non-0 on error.
            1 error, failed to get PS/2 controller status register.
            2 error, failed to receive data byte from PS/2 keyboard.
*/
int get_scan_code(uint8_t *sc) { // @TODO
    ps_2_ctrl_stat_t stat;
    int r;
    uint8_t b;

    r = get_ctlr_stat(&stat);

    if (r != 0)
        return 1;

    while (stat.obuf_full == PS2_BUF_EMPTY) { /* Loop until a scan code is
                                                 received. */
        r = get_ctlr_stat(&stat);

        if (r != 0)
            return 1;
    }

    r = rcv_byte (&b);

    if (r != 0)
        return 2;

    *sc = b;

    return 0;
}

/*!
    @function get_scan_code2

    @discussion Returns a 2 byte scan code from the keyboard. Caller must ensure
    that `sc` is 2 bytes in size.

    @param  sc  Pointer in which to return the 2-byte scan code.

    @result 0 on success. Non-0 on error.
*/
int get_scan_code2(uint8_t *sc) {
    ps_2_ctrl_stat_t stat;
    int r;
    uint8_t b;

    r = get_ctlr_stat(&stat);

    if (r != 0)
        return 1;

    while (stat.obuf_full == PS2_BUF_EMPTY) { /* Loop until a scan code is
                                                 received. */
        r = get_ctlr_stat(&stat);

        if (r != 0)
            return 2;
    }

    r = rcv_byte (&b);

    if (r != 0)
        return 3;

    *sc = b;

    if (b != 0xE0) {
        // This is a single byte scan code.
        *(sc+1) = 0x00;
        return 0;
    }

    /* Get the second scan code byte. */
    while (stat.obuf_full == PS2_BUF_EMPTY) { /* Loop until a scan code is
                                                 received. */
        r = get_ctlr_stat(&stat);

        if (r != 0)
            return 4;
    }

    r = rcv_byte (&b);

    if (r != 0)
        return 5;

    *(sc+1) = b;

    return 0;
}

/*!
    @typedef sc_state_t

    @discussion States of the state machine used to detect the receipt of a
    complete scan code.

    @constant SSCS    State, Scan Code, Start.
    @constant S1B0P_F    State, 1-Byte scan code, byte # 0, Pressed scan code, Final state.
    @constant S1B0R_F    State, 1-Byte scan code, byte # 0, Released scan code, Final state.
    @constant S2B0_S4B0  State, 2-Byte scan code, byte # 0, OR, State, 4-Byte scan code, byte # 0.

    ...
    @TODO
    ...
    @TODO Draw the state machine.
*/
typedef enum _sc_state_t {
    SSCS = 0,
    S1B0P_F,
    S1B0R_F,
    S2B0_S4B0,
    S2B1P_F,
    S2B1R_F,
    S4B1P,
    S4B2P,
    S4B3P_F,
    S4B1R,
    S4B2R,
    S4B3R_F,
    S6B0,
    S6B1,
    S6B2,
    S6B3,
    S6B4,
    S6B5P_F,
    SSC_ERR // Error state
} sc_state_t;

static sc_state_t sc_sm_next_state(sc_state_t cs, uint8_t in) {
    /*!
        @typedef sc_transition_tbl_t

        @discussion Scan code transition table entry. Used define the state machine
        that detects the receipt of a complete scan code.

        @field cs    Current state.
        @field il    Input lower bound.
        @field ih    Input higher bound.
        @field ns    Next state.
    */
    typedef struct _sc_transition_tbl_t {
        sc_state_t cs;
        uint8_t il;
        uint8_t ih;
        sc_state_t ns;
    } sc_transition_tbl_t;
    static const sc_transition_tbl_t sc_t_tbl[] = { // Scan code transition table. Used to implement the state machine transition function.
        {SSCS, 0x00, 0x58, S1B0P_F},
        {SSCS, 0x80, 0xD8, S1B0R_F},

        {SSCS, 0xE0, 0xE0, S2B0_S4B0},
        {S2B0_S4B0, 0x2A, 0x2A, S4B1P}, // @TODO Is this a hacky fix? 0x2A falls inside [0x10, 0x6D]. The transition table now depends on the order of the entries. But isn't an NFA always reducible to a DFA?
        {S2B0_S4B0, 0x10, 0x6D, S2B1P_F},
        {S2B0_S4B0, 0xB7, 0xB7, S4B1R}, // @TODO Is this a hacky fix? 0xB7 falls inside [0x10, 0x6D].
        {S2B0_S4B0, 0x90, 0xED, S2B1R_F},

        //{S2B0_S4B0, 0x2A, 0x2A, S4B1P},
        {S4B1P, 0xE0, 0xE0, S4B2P},
        {S4B2P, 0x37, 0x37, S4B3P_F},

        //{S2B0_S4B0, 0xB7, 0xB7, S4B1R},
        {S4B1R, 0xE0, 0xE0, S4B2R},
        {S4B2R, 0xAA, 0xAA, S4B3R_F},

        {SSCS, 0xE1, 0xE1, S6B0},
        {S6B0, 0x1D, 0x1D, S6B1},
        {S6B1, 0x45, 0x45, S6B2},
        {S6B2, 0xE1, 0xE1, S6B3},
        {S6B3, 0x9D, 0x9D, S6B4},
        {S6B4, 0xC5, 0xC5, S6B5P_F} // @IMPORTANT 6-Byte scan code does not have a released state.
    };

    static const int sc_t_tbl_len = sizeof(sc_t_tbl)/sizeof(sc_t_tbl[0]);

    int i;

    for (i = 0; i < sc_t_tbl_len; i++) {
        if(sc_t_tbl[i].cs == cs && in >= sc_t_tbl[i].il && in <= sc_t_tbl[i].ih) {
            return sc_t_tbl[i].ns;
        }
    }

    return SSC_ERR;
}

static int sc_sm_is_final_state(sc_state_t s) {
    switch(s) {
    case S1B0P_F:
        print("1-Byte Scan Code Pressed\n");
        return 1;
        break;
    case S1B0R_F:
        print("1-Byte Scan Code Released\n");
        return 1;
        break;
    case S2B1P_F:
        print("2-Byte Scan Code Pressed\n");
        return 1;
        break;
    case S2B1R_F:
        print("2-Byte Scan Code Released\n");
        return 1;
        break;
    case S4B3P_F:
        print("4-Byte Scan Code Pressed\n");
        return 1;
        break;
    case S4B3R_F:
        print("4-Byte Scan Code Released\n");
        return 1;
        break;
    case S6B5P_F:
        print("6-Byte Scan Code Pressed\n");
        return 1;
        break;
    case SSC_ERR:
        print("Scan Code Error State!\n");
        return 1;
        break;
    default:
        break;
    }

    return 0; // Not final state.
}

void sc_sm_update(uint8_t in) {
    static sc_state_t sc_sm_cs = SSCS; // cs = Current State

    sc_sm_cs = sc_sm_next_state(sc_sm_cs, in);

    if(sc_sm_is_final_state(sc_sm_cs))
        sc_sm_cs = SSCS; // Scan code state machine reset to start state.
}

void v33_handler(uint32_t vn, uint32_t err_code) {
    uint8_t sc;
    char c;

    if (vn || err_code || c) { // Suppress warning.
        ;
    }

    sc = inb (0x0060); // Read keyboard output buffer.
    pic_eoi(vn);
    print_x32(sc);
    print("\n");

    sc_sm_update(sc);
    //if((sc & 0x80) == 0) { // if its not a released scan code.
        //c = scan_code_to_ascii (sc);
        //print_ch_at(c, 0, -1, -1);
    //}
    //print("again, THE KEYBOARD SAYS DIJKSTRA.\n");
}

#if 0
// TODO: Thoughts on how to implement <stdarg.h>
void foo(int v, ...) {
    // &v + 4 // call stack
    if(v) {
        ;
    }
}
#endif