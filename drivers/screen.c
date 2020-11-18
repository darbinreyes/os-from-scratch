/*!
    @header A screen device driver.
    Implements function for printing text to the screen. The underlying screen
    device is IBM VGA.
*/

#include "screen.h"
#include "../kernel/low_level.h"
#include "../mylibc/mylibc.h"

/*!
    @defined VIDEO_ADDRESS
    @discussion
    Start of video memory address in text mode, 2 bytes each, first byte is
    the ASCII code of the character, second byte is the character display
    attributes e.g. text color.
*/
#define VIDEO_ADDRESS 0xB8000

/*!
    @defined MAX_ROWS
    @discussion
    In text mode, the total number of character rows.
*/
#define MAX_ROWS 25

/*!
    @defined MAX_COLS
    @discussion
    In text mode, the total number of character columns.
*/
#define MAX_COLS 80

/*!
    @defined CHAR_ATTR_WHITE_ON_BLACK
    @discussion
    Character attribute byte for white/black foreground/background.
*/
#define CHAR_ATTR_WHITE_ON_BLACK 0x0F

/*!
    @defined REG_SCREEN_CTRL_IO_PORT
    @discussion
    Screen device control I/O port.
*/
#define REG_SCREEN_CTRL_IO_PORT 0x3D4

/*!
    @defined REG_SCREEN_DATA_IO_PORT
    @discussion
    Screen device data I/O port.
*/
#define REG_SCREEN_DATA_IO_PORT 0x3D5

/*!
    @defined CURSOR_LOCATION_HIGH_BYTE
    CRTC Registers - Cursor position control. Cursor Location High 0xE.
Cursor Location Low 0xF.
*/
#define CURSOR_LOCATION_HIGH_BYTE 0x0E

/*!
    @defined CURSOR_LOCATION_LOW_BYTE
    CRTC Registers - Cursor position control. Cursor Location Low 0xF.
*/
#define CURSOR_LOCATION_LOW_BYTE 0x0F

int row_col_to_screen_video_mem_offset(int row, int col);

int vid_mem_offset_to_row (int vid_mem_offset);

int get_cursor(void);

void set_cursor(int vid_mem_offset);

int handle_scrolling(int vid_mem_offset);


/*!
    @function    print_ch_at
    @abstraction Prints a single character to the screen at the specified
    position and specified background/foreground color.

    @param    c    The ASCII code of the character to print

    @param    cattr    Sets background/foreground color.

    @param    row    Row position of the character.
    @param    col    Column position of the character.

    @discussion
    Print a single character to the screen at the specified position. The
    meaning of "position": the screen is treated as a 25x80 grid of characters
    i.e. 25 rows, 80 columns. The special position <row, col> == <-1, -1>
    instructs the function to print the character at the current cursor
    position. The cursor position is automatically advanced after any print,
    regardless of the arguments.

    The cursor position is automatically advanced after each call to this
    function, so that a subsequent call prints to the next adjacent cursor
    position. The screen position <row, col> == <0,0> corresponds to the
    top-left character cell, the position <row, col> == <24, 79> corresponds to
    the bottom right character cell.

    Rudimentary automatic scrolling is handled, i.e. after printing the
    specified character if <row, col> == <25, 0>, every row is shifted upwards
    by 1 row, thus overwriting row == 0 and clearing row == 24, and the cursor
    position is set to <row, col> == <24, 0>.

    cattr is used to set the character's foreground and background color. If
    cattr == 0, the function uses foreground/background color == WHITE/BLACK,
    otherwise the given cattr value is used.

    c == '\n' is handled specially, it has the natural behavior: it moves the
    cursor position 1 row below the current row.

*/
void print_ch_at(char c, char cattr, int row, int col) {
    unsigned char *vid_mem;
    int vid_mem_offset;
    int trow;

    vid_mem = (unsigned char *) VIDEO_ADDRESS;

    if (cattr == 0)
        cattr = CHAR_ATTR_WHITE_ON_BLACK;

    /* Determine where the character wills be printed. */
    if (row >= 0 && col >= 0) {
        vid_mem_offset = row_col_to_screen_video_mem_offset(row, col);
    } else {
        vid_mem_offset = get_cursor();
    }

    if (c == '\n') {
        /* Move cursor to a new line. */

        /*

            Convert vid_mem_offset into current row position. The algebra is:
            `(row * MAX_COLS + col) * 2` / `(MAX_COLS* 2)` == row.

        */
        trow = vid_mem_offset_to_row (vid_mem_offset);
        /*

            Set the video memory offset to the last column of the current row.
            Then the code below will increment the video memory offset, which
            has the net effect of leaving the cursor at the first column of the
            next row.

        */
        vid_mem_offset = row_col_to_screen_video_mem_offset(trow, 79);
    } else {
        /* Print the character as-is. */
        vid_mem[vid_mem_offset] = c;
        vid_mem[vid_mem_offset + 1] = cattr;
    }

    /* Implement rudimentary auto scrolling. */

    /* Advance the cursor position. */
    vid_mem_offset += 2;

    vid_mem_offset = handle_scrolling(vid_mem_offset);

    set_cursor(vid_mem_offset);
}

/****** Helper functions. *******/

int row_col_to_screen_video_mem_offset(int row, int col) {
    return ((row * MAX_COLS) + col) * 2;
}

int vid_mem_offset_to_row (int vid_mem_offset) {
    /*

        Convert vid_mem_offset into current row position. The algebra is:
        `(row * MAX_COLS + col) * 2` / `(MAX_COLS* 2)` == row.

    */
    return vid_mem_offset / (MAX_COLS * 2);
}

/*

    Returns the video memory offset corresponding to the current position of the
    cursor.

*/
int get_cursor(void) {
    int vid_mem_offset;

    port_byte_out(REG_SCREEN_CTRL_IO_PORT, CURSOR_LOCATION_HIGH_BYTE);
    vid_mem_offset = port_byte_in(REG_SCREEN_DATA_IO_PORT) << 8;
    port_byte_out(REG_SCREEN_CTRL_IO_PORT, CURSOR_LOCATION_LOW_BYTE);
    vid_mem_offset += port_byte_in(REG_SCREEN_DATA_IO_PORT);

    /*

        The cursor position is stored in the VGA's internal registers in the
        form of a character cell offset, as opposed to a video memory offset. A
        character cell offset is = `row * MAX_COLS + col` while the video memory
        offset is = `(row * MAX_COLS + col) * 2` since in video memory space
        each character cell gets 2 bytes: viz. <ASCII CODE> and <ATTRIBUTES>.

    */


    return vid_mem_offset * 2;
}

/*

    Takes a cursor position in the form of a video memory offset, converts that
    into a character offset, and finally writes that character offset into the
    appropriate VGA internal registers.


*/
void set_cursor(int vid_mem_offset) {

    /*

        The algebra is: character offset = `(row * MAX_COLS + col) * 2` / `2` ==
        `(row * MAX_COLS + col)`.

    */
    vid_mem_offset /= 2;

    port_byte_out(REG_SCREEN_CTRL_IO_PORT, CURSOR_LOCATION_HIGH_BYTE);
    port_byte_out(REG_SCREEN_DATA_IO_PORT, (unsigned char) (vid_mem_offset >> 8) );
    port_byte_out(REG_SCREEN_CTRL_IO_PORT, CURSOR_LOCATION_LOW_BYTE);
    port_byte_out(REG_SCREEN_DATA_IO_PORT, (vid_mem_offset & 0x00FF));

}

void memory_copy (void *dst, void *src, int n) {
    unsigned char *d, *s;

    if (dst == NULL || src == NULL || n == 0)
        return;

    d = dst;
    s = src;

    for (int i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

void zero_memory (void *dst, int n) {
    unsigned char *d;

    if (dst == NULL || n == 0)
        return;

    d = dst;

    for (int i = 0; i < n; i++)
        d[i] = 0;
}

int handle_scrolling(int vid_mem_offset) {
    int trow;
    unsigned char *vid_mem;

    /*
        Steps:

        Check if scrolling must be done.
        if no, return vid_mem_offset unchanged.
        if yes,
            Copy row to row-1, starting at row == 1, ending at row == 24.
            Clear row == 24.
            Return video memory offset of row == 24, col == 0.

    */

    trow = vid_mem_offset_to_row (vid_mem_offset);

    if (trow < 25)
        return vid_mem_offset;

    vid_mem = (unsigned char *) VIDEO_ADDRESS;

    /* Scrolling requires copying 24 out of the 25 rows. */
    #define SCROLL_MEM_COPY_SIZE (((MAX_ROWS - 1) * MAX_COLS) * 2)

    memory_copy(vid_mem, vid_mem + MAX_COLS * 2, SCROLL_MEM_COPY_SIZE); // Copy rows.

    zero_memory (vid_mem + SCROLL_MEM_COPY_SIZE, MAX_COLS * 2); // Clear last row.

    vid_mem_offset = row_col_to_screen_video_mem_offset (24, 0);

    return vid_mem_offset;
}

void clear_screen(void) {
    unsigned char *vid_mem;

    vid_mem = (unsigned char *) VIDEO_ADDRESS;

    #define CLEAR_SIZE (MAX_ROWS * MAX_COLS * 2)

    zero_memory (vid_mem, CLEAR_SIZE);
}

/* Note: Empty string is a NO-OP. */
void print_at(const char *s, int row, int col) {

    if (*s == '\0')
        return;

    print_ch_at(*s, 0, row, col);
    s++;

    while (*s != '\0') {
        print_ch_at(*s, 0, -1, -1);
        s++;
    }
}

void print(const char *s) {

    while (*s != '\0') {
        print_ch_at(*s, 0, -1, -1);
        s++;
    }
}

/* Prints a byte in binary format.*/
void print_byteb (unsigned char b) {
    print("0b");
    for (int i = 7; i >= 0; i--)
        if (b & BITN(i))
            print_ch_at('1', 0, -1, -1);
        else
            print_ch_at('0', 0, -1, -1);
}

// Nibble to ASCII hexit. Upper nibble is masked out, lower nibble is converted to an ASCII character in the set [0-9|A-F].
char nibtoa (unsigned char b) {
    b = 0x0F & b; // lower nibble.

    if (b <= 9)
        b += '0';
    else if (b > 9)
        b = b - 10 + 'A';
    return b;
}

void print_byteh (unsigned char b, int pf) {
    char c;

    if (pf)
        print("0x");

    c = nibtoa (b >> 4); // upper nibble

    print_ch_at(c, 0, -1, -1);

    c = nibtoa (b); // lower nibble

    print_ch_at(c, 0, -1, -1);
}

void print_uint32h(uint32_t i) {
    print_byteh ((i >> 24) & 0xFFU, 0);
    print_byteh ((i >> 16) & 0xFFU, 0);
    print_byteh ((i >> 8) & 0xFFU, 0);
    print_byteh ((i >> 0) & 0xFFU, 0);
}

/*!
    @defined    EOF
    @discussion    End of file. @doc [stdio.h].
*/
#define EOF (-1)

/*!
    @function    isspace

    @discussion    The isspace() function tests for the white-space characters.
                   @doc [man isspace / ctype.h].

    @param    c    The character to test.

    @result    The isspace() function returns zero if the character tests false
               and returns non-zero if the character tests true.

*/
static inline int isspace(int c) {
    switch (c) {
    case '\t': // horizontal tab
    case '\n': // newline
    case '\v': // vertical tab
    case '\f': // formfeed
    case '\r': // carriage return
    case ' ':  // space
        return 1;
        break;
    default:
        return 0;
        break;
    }

    return 0;
}

/*!
    @function    isdigit
    @discussion    The isdigit() function tests for a decimal digit character.

    @result    The isdigit() function returns zero if the character tests false
               and returns non-zero if the character tests true.
*/
static inline int isdigit(int c) {
    if (c >= '0' && c <= '9')
        return 1;

    return 0;
}

/*!
    @function    isxdigit
    @discussion    The isxdigit() function tests for any hexadecimal-digit
                   character.

    @result    The isxdigit() function returns zero if the character tests false
               and returns non-zero if the character tests true.
*/
static inline int isxdigit(int c) {
    if ((c >= '0' && c <= '9') ||  (c >= 'A' && c <= 'F') ||  (c >= 'a' && c <= 'f'))
        return 1;

    return 0;
}

/*!
    @function    digittoint
    @discussion    The digittoint() function converts a numeric character to its
                   corresponding integer value.  The character can be any
                   decimal digit or hexadecimal digit.  With hexadecimal
                   characters, the case of the values does not matter.
                   @doc [man digittoint / ctype.h].

    @result     The digittoint() function always returns an integer from the
                range of 0 to 15. If the given character was not a digit as
                defined by isxdigit(3), the function will return 0.

*/
static inline int digittoint(int c) {
    if (!isxdigit(c))
        return 0;

    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'A' && c <= 'F')
        return 10 + (c - 'A');

    if (c >= 'a' && c <= 'f')
        return 10 + (c - 'a');

    return 0;
}

/*!
    @function    atoi
    @discussion    The atoi() function converts the initial portion of the
                   string pointed to by str to int representation. It is
                   equivalent to: `(int)strtol(str, (char **)NULL, 10);`.
                   @doc [man atoi / stdlib.h].

    @param    str    The string to convert to an integer.

    @result    The result of the conversion to an integer.

    @TODO Test negative numbers.
*/
int atoi(const char *str) {
    int sum;
    int sign;

    if (str == NULL)
        return 0;

    while (isspace(*str))
        str++;

    sign = 1;

    if (*str == '+') {
        str++;
    } else if (*str == '-') {
        sign = -1;
        str++;
    } else {
        ;
    }

    /*
    str | sum | sign
    ----|-----|-----
        |     |
    */
    sum = 0;

    while (isdigit(*str)) {
        sum = 10 * sum + digittoint(*str);
        str++;
    }

    return sum * sign;
}

/*!
    @function    itoa
    @discussion Converts an integer to a string. This function not part of the
                standard C library.

    @param    i the integer to convert to a string.

    @result    String containing the result of the conversion.

*/
// char *itoa(int32_t i, char *s) {
//     //static char str[1 + 10 + 1]; // Max int32_t value = 2**31 - 1 = 2,147,483,647. Min. 2**31 = -2147483648. 1 negative sign, 10 digits, 1 null terminator.
//     int64_t pwr;

//     /*
// i | pwr | i / pwr
// 57| 10  | 5
// 57 | 100| 0
//     */
//     pwr = 10;
//     while (i / pwr > 0) {
//         pwr *= 10;
//     }

//     //str[0] = '\0';

//     // @TODO
//     return s;
// }

/*!
    @function    dead_loop
    @discussion Infinite loop.

*/
void dead_loop(void) {
    while (1)
        ;
}
/*!
    @function    print_assert
    @discussion Prints a message associated with the assert macro.
                @doc [assert.h].

*/
void print_assert(char *e, char *f, int l) {
    print(f);
    print(":");
    print_uint32h(l);
    print(": failed assertion `");
    print(e);
    print("'\n");
}


