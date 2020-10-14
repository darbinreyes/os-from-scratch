/*

INCREMENTAL GOAL:
[] short get_scn_code(void);
[] char getch(void);
[] char *prompt_user_for_str(char *prompt);
[] With interrupts.

*/

#include "../mylibc/mylibc.h"
#include "ps_2_ctlr.h"
#include "keyboard.h"

// TODO: driver start()
// TODO: driver stop()


/*

From my notes:

* As an OS dev. you are free to set the key codes as you wish. There are
* commonly used schemes however: 8-bits total per key code, upper 3 bits = KB
* row, lower 5 bits = KB col, KB treated as a grid. Once you define your key
* codes you must ensure that all KB drivers use the same key codes. FYI: your
* key codes can also be used by non-KB devices, like a mouse, in which a right
* click might be reported as “key code 0xF1”. Thus, when your state machine
* receives a complete, possibly multi-byte, scan code, you convert it to your
* single byte key code.

*/

//int sc_to_kc_tbl_row_len[] = {21, 21, 21, 17, 16, 13};
#define KEY_CODE_TO_ASCII_ROWS 6
#define KEY_CODE_TO_ASCII_COLS 21

char kc_rc_to_ascii[KEY_CODE_TO_ASCII_ROWS][KEY_CODE_TO_ASCII_COLS] = {
/* Apple USB Keyboard Model A1243 */
 {  '?'/*<ESC>*/,      'X'/*<F1>*/,   'X'/*<F2>*/,       'X'/*<F3>*/,   'X'/*<F4>*/,                    'X'/*<F5>*/,              'X'/*<F6>*/,                 'X'/*<F7>*/,                   'X'/*<F8>*/,                 'X'/*<F9>*/,       'X'/*<F10>*/,  'X'/*<F11>*/,                   'X'/*<F12>*/,   '?'/*<EJECT>*/,        '?'/*<F13>*/,        '?'/*<F14>*/,     '?'/*<F15>*/,         '?'/*<F16>*/,                 '?'/*<F17>*/,        '?'/*<F18>*/,    '?'/*<F19>*/         },
 {  '`',               '1',           '2',               '3',           '4',                            '5',                      '6',                         '7',                           '8',                         '9',               '0',           '-',                            '=',            '?'/*<BACKSPACE>*/,    '?'/*<Fn>*/,         '?'/*<HOME>*/,    '?'/*<PG-UP>*/,       '?'/*<NUMPAD-CLEAR/NUMLOCK>*/,'='/*NUMPAD"="NoSc*/, '/'/*NUMPAD-/*/,'*'/*NUMPAD"*"*/     },
 {  '?'/*<TAB>*/,      'q',           'w',               'e',           'r',                            't',                      'y',                         'u',                           'i',                         'o',               'p',           '[',                            ']',            '\\',                  '?'/*<DEL/NUMLOCK>*/,'?'/*<END>*/,     '?'/*<PG-DOWN>*/,     '7',                          '8',                 '9',             '-'                  },
 {  '?'/*<CAPSLOCK>*/, 'a',           's',               'd',           'f',                            'g',                      'h',                         'j',                           'k',                         'l',               ';',           '\'',                           '?'/*<RETURN>*/,                                                                                    '4',                          '5',                 '6',             '+'                  },
 {  '?'/*<L-SHIFT>*/,  'z',           'x',               'c',           'v',                            'b',                      'n',                         'm',                           ',',                         '.',               '/',                                           '?'/*<R-SHIFT>*/,                                           '?'/*<CUR-UP>*/,                        '1',                          '2',                 '3'                                   },
 {  '?'/*<L-CTRL>*/,   '?'/*<L-ALT>*/,'?'/*<L-CMD>NoSc*/,' '/*<SPACE>*/,                                                                                                                                                   '?'/*<R-CMD>NoSc*/,'?'/*<R-ALT>*/,                                '?'/*<R-CTRL>*/,                       '?'/*<CUR-LEFT>*/,   '?'/*<CUR-DOWN>*/,'?'/*<CUR-RIGHT>*/,   '0',                                               '.',             '?'/*<NUMPAD-ENTER>*/}
};

char shift_kc_rc_to_ascii[KEY_CODE_TO_ASCII_ROWS][KEY_CODE_TO_ASCII_COLS] = {
/* Apple USB Keyboard Model A1243 */
 {  '?'/*<ESC>*/,      'X'/*<F1>*/,   'X'/*<F2>*/,       'X'/*<F3>*/,   'X'/*<F4>*/,                    'X'/*<F5>*/,              'X'/*<F6>*/,                 'X'/*<F7>*/,                   'X'/*<F8>*/,                 'X'/*<F9>*/,       'X'/*<F10>*/,  'X'/*<F11>*/,                   'X'/*<F12>*/,   '?'/*<EJECT>*/,        '?'/*<F13>*/,        '?'/*<F14>*/,     '?'/*<F15>*/,         '?'/*<F16>*/,                 '?'/*<F17>*/,        '?'/*<F18>*/,    '?'/*<F19>*/         }, // 21
 {  '~',               '!',           '@',               '#',           '$',                            '%',                      '^',                         '&',                           '*',                         '(',               ')',           '_',                            '+',            '?'/*<BACKSPACE>*/,    '?'/*<Fn>*/,         '?'/*<HOME>*/,    '?'/*<PG-UP>*/,       '?'/*<NUMPAD-CLEAR/NUMLOCK>*/,'='/*NUMPAD"="NoSc*/, '/'/*NUMPAD-/*/,'*'/*NUMPAD"*"*/     },
 {  '?'/*<TAB>*/,      'Q',           'W',               'E',           'R',                            'T',                      'Y',                         'U',                           'I',                         'O',               'P',           '{',                            '}',            '|',                   '?'/*<DEL/NUMLOCK>*/,'?'/*<END>*/,     '?'/*<PG-DOWN>*/,     '7',                          '8',                 '9',             '-'                  },
 {  '?'/*<CAPSLOCK>*/, 'A',           'S',               'D',           'F',                            'G',                      'H',                         'J',                           'K',                         'L',               ':',           '\"',                           '?'/*<RETURN>*/,                                                                                    '4',                          '5',                 '6',             '+'                  },
 {  '?'/*<L-SHIFT>*/,  'Z',           'X',               'C',           'V',                            'B',                      'N',                         'M',                           '<',                         '>',               '?',                                           '?'/*<R-SHIFT>*/,                                           '?'/*<CUR-UP>*/,                        '1',                          '2',                 '3'                                   },
 {  '?'/*<L-CTRL>*/,   '?'/*<L-ALT>*/,'?'/*<L-CMD>NoSc*/,' '/*<SPACE>*/,                                                                                                                                                   '?'/*<R-CMD>NoSc*/,'?'/*<R-ALT>*/,                                '?'/*<R-CTRL>*/,                       '?'/*<CUR-LEFT>*/,   '?'/*<CUR-DOWN>*/,'?'/*<CUR-RIGHT>*/,   '0',                                               '.',             '?'/*<NUMPAD-ENTER>*/}
};


#define KEY_CODE_FROM_ROW_COL(r, c)  ( ( ( (r) & 0x07) << 5 ) | ( (c) & 0x1F) ) // Row = upper 3 bits. Col. = lower 5 bits.
#define NOT_A_SCAN_CODE 0xFF
#define SCAN_CODE_TODO  0xFE
#define KEY_CODE_TO_ROW(kc) ( ( (kc) & 0xE0 ) >> 5 )
#define KEY_CODE_TO_COL(kc) ( (kc) & 0x1F )

// 1 byte scan codes. // State == pressed.
unsigned char sc_to_kc_tbl2[] = {
    NOT_A_SCAN_CODE, /*0x00 Not a scan code .*/
    KEY_CODE_FROM_ROW_COL(0, 0),/*0x01 <ESC> p.|r,c=0,0*/
    KEY_CODE_FROM_ROW_COL(1, 1),/*0x02 1 p.|r,c=1,1*/
    KEY_CODE_FROM_ROW_COL(1, 2),/*0x03 2 p.|r,c=1,2*/
    KEY_CODE_FROM_ROW_COL(1, 3),/*0x04 3 p.|r,c=1,3*/
    KEY_CODE_FROM_ROW_COL(1, 4),/*0x05 4 p.|r,c=1,4*/
    KEY_CODE_FROM_ROW_COL(1, 5),/*0x06 5 p.|r,c=1,5*/
    KEY_CODE_FROM_ROW_COL(1, 6),/*0x07 6 p.|r,c=1,6*/
    KEY_CODE_FROM_ROW_COL(1, 7),/*0x08 7 p.|r,c=1,7*/
    KEY_CODE_FROM_ROW_COL(1, 8),/*0x09 8 p.|r,c=1,8*/
    KEY_CODE_FROM_ROW_COL(1, 9),/*0x0A 9 p.|r,c=1,9*/
    KEY_CODE_FROM_ROW_COL(1, 10),/*0x0B 0 p.|r,c=1,10*/
    KEY_CODE_FROM_ROW_COL(1, 11),/*0x0C - p.|r,c=1,11*/
    KEY_CODE_FROM_ROW_COL(1, 12),/*0x0D = p.|r,c=1,12*/
    KEY_CODE_FROM_ROW_COL(1, 13),/*0x0E <BACKSPACE> p.|r,c=1,13*/

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

    KEY_CODE_FROM_ROW_COL(1, 17),/*0x45 <NUMLOCK> p.|r,c=1,17|Not a key on my keyboard.|Try <CLEAR>*/

    KEY_CODE_FROM_ROW_COL(0, 15),/*0x46 <SCROLL-LOCK> p.|r,c=0,15|Not a key on my keyboard.|Try <F14>*/

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

// 2 byte scan codes. With 0xE0 for first byte. // State == pressed.
unsigned char sc_to_kc_tbl3[] = {

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
    SCAN_CODE_TODO,/*0x5B <"left-GUI"> p.|r,c=,*/
    SCAN_CODE_TODO,/*0x5C <"right-GUI"> p.|r,c=,*/
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

/* ********************** TODO:START ********************** */



/*0xE0, 0x2A, 0xE0, 0x37| print screen pressed. */
/*0xE0, 0xB7, 0xE0, 0xAA| print screen released. */

/*0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5| pause pressed.*/ /*Note: There is no scan code for "pause key released" (it behaves as if it is released as soon as it's pressed)*/

/* ********************** TODO:END   ********************** */

/* Returns the key code corresponding to the given key code. 0xFF indicates the
given scan code does not exist. 0xFE indicates the given scan code is a "TODO",
i.e. is valid but not currently assigned a key code. */
unsigned char get_key_code(unsigned char sc) {

    if (sc == 0xE0 || sc == 0xE1) // Multi-byte scan codes.
        return 0xFF; // For now.

    if (sc > 0x00 && sc < 0x59) { // A valid single byte scan code.
        return sc_to_kc_tbl2[sc];
    }

    return 0xFF;
}

char scan_code_to_ascii (unsigned char sc) {
    unsigned char kc, r, c;

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

// Returns a 1 byte scan code from the keyboard.
int get_scan_code(unsigned char *sc) {
    ps_2_ctrl_stat_t stat;
    int r;
    unsigned char b;

    r = get_ctlr_stat(&stat);

    if (r != 0)
        return 1;

    while (stat.obuf_full == PS2_BUF_EMPTY) { // Loop until a scan code is received.
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

// Returns a 2 byte scan code from the keyboard. Caller must ensure that `sc` is 2 bytes in size.
int get_scan_code2(unsigned char *sc) {
    ps_2_ctrl_stat_t stat;
    int r;
    unsigned char b;

    r = get_ctlr_stat(&stat);

    if (r != 0)
        return 1;

    while (stat.obuf_full == PS2_BUF_EMPTY) { // Loop until a scan code is received.
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

    // Get the second scan code byte.
    while (stat.obuf_full == PS2_BUF_EMPTY) { // Loop until a scan code is received.
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