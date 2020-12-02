/*!
    @header A VGA screen driver.
    Implements functions for printing text to the screen. The underlying screen
    device is IBM VGA. @doc [IBM VGA docs](./docs/screen/screen.md).
*/

#include "../include/mylibc.h"
#include "../include/stdio.h" // NULL
#include "screen.h"
#include "../kernel/low_level.h"


/*!
    @defined VIDEO_ADDRESS

    @discussion Start of video memory address in text mode, 2 bytes each, first
    byte is the ASCII code of the character, second byte is the character
    display attributes e.g. text color.

*/
#define VIDEO_ADDRESS 0xB8000

/*!
    @defined MAX_ROWS

    @discussion In text mode, the total number of character rows.
*/
#define MAX_ROWS 25

/*!
    @defined MAX_COLS

    @discussion In text mode, the total number of character columns.
*/
#define MAX_COLS 80

/*!
    @defined CHAR_ATTR_WHITE_ON_BLACK

    @discussion Character attribute byte for white/black foreground/background.
*/
#define CHAR_ATTR_WHITE_ON_BLACK 0x0F

/*!
    @defined REG_SCREEN_CTRL_IO_PORT

    @discussion Screen device control I/O port.
*/
#define REG_SCREEN_CTRL_IO_PORT 0x3D4

/*!
    @defined REG_SCREEN_DATA_IO_PORT

    @discussion Screen device data I/O port.
*/
#define REG_SCREEN_DATA_IO_PORT 0x3D5

/*!
    @defined CURSOR_LOCATION_HIGH_BYTE

    @discussion CRTC Registers - Cursor position control. Cursor Location High
    0xE.

*/
#define CURSOR_LOCATION_HIGH_BYTE 0x0E

/*!
    @defined CURSOR_LOCATION_LOW_BYTE

    @discussion CRTC Registers - Cursor position control. Cursor Location Low
    0xF.
*/
#define CURSOR_LOCATION_LOW_BYTE 0x0F

/*!
    @function row_col_to_screen_video_mem_offset

    @discussion Converts a row and column number to an offset into video memory.

    @param    row    The row number.
    @param    col    The column number.

    @result The offset into video memory corresponding to the given row and
    column.
*/
static inline int row_col_to_screen_video_mem_offset(int row, int col) {
    return ((row * MAX_COLS) + col) * 2;
}

/*!
    @function vid_mem_offset_to_row

    @discussion Returns the row number corresponding to the given offset into
    video memory.

    @param    vid_mem_offset    An offset into video.

    @result The row number that corresponds to the given video memory offset.
*/
static inline int vid_mem_offset_to_row (int vid_mem_offset) {
    /* The algebra is: `(row * MAX_COLS + col) * 2` / `(MAX_COLS* 2)` == row. */
    return vid_mem_offset / (MAX_COLS * 2);
}

/*!
    @function get_cursor

    @discussion Returns the video memory offset corresponding to the current
    position of the cursor.

    @result The video memory offset corresponding to the current cursor
    position.
*/
static inline int get_cursor(void) {
    int vid_mem_offset;

    /* The cursor position is stored in the VGA's internal registers in the
    form of a character cell offset, as opposed to a video memory offset. A
    character cell offset is = `row * MAX_COLS + col` while the video memory
    offset is = `(row * MAX_COLS + col) * 2` since in video memory space
    each character cell gets 2 bytes: viz. <ASCII CODE> and <ATTRIBUTES>. */
    port_byte_out(REG_SCREEN_CTRL_IO_PORT, CURSOR_LOCATION_HIGH_BYTE);
    vid_mem_offset = port_byte_in(REG_SCREEN_DATA_IO_PORT) << 8; // High byte.
    port_byte_out(REG_SCREEN_CTRL_IO_PORT, CURSOR_LOCATION_LOW_BYTE);
    vid_mem_offset += port_byte_in(REG_SCREEN_DATA_IO_PORT); // Low byte.

    return vid_mem_offset * 2;
}

/*!
    @function set_cursor

    @discussion Takes a cursor position in the form of a video memory offset,
    converts that into a character offset, and finally writes that character
    offset into the appropriate VGA internal registers.

    @param    vid_mem_offset    The desired position of the cursor in the form
                                of a video memory offset.
*/
static inline void set_cursor(int vid_mem_offset) {

    /* The algebra is: character offset = `(row * MAX_COLS + col) * 2` / `2` ==
    `(row * MAX_COLS + col)`. */
    vid_mem_offset /= 2;

    port_byte_out(REG_SCREEN_CTRL_IO_PORT, CURSOR_LOCATION_HIGH_BYTE);
    port_byte_out(REG_SCREEN_DATA_IO_PORT, (uint8_t) (vid_mem_offset >> 8) );
    port_byte_out(REG_SCREEN_CTRL_IO_PORT, CURSOR_LOCATION_LOW_BYTE);
    port_byte_out(REG_SCREEN_DATA_IO_PORT, (vid_mem_offset & 0x00FF));

}

/*!
    @TODO Replace with to string.h->memcpy().
    FYI: size_t is defined in stddef.h. On my mac is is a typedef of unsigned
    long.
*/
static void memory_copy (void *dst, void *src, int n) {
    uint8_t *d, *s;

    if (dst == NULL || src == NULL || n == 0)
        return;

    d = dst;
    s = src;

    for (int i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

/*!
    @TODO Replace with to string.h->memset().
*/
static void zero_memory (void *dst, int n) {
    uint8_t *d;

    if (dst == NULL || n == 0)
        return;

    d = dst;

    for (int i = 0; i < n; i++)
        d[i] = 0;
}

/*!
    @function handle_scrolling

    @discussion Performs a scrolling operation if the given video memory offset
    indicates that the cursor has fallen off the bottom of the
    screen. Scrolling means copying every row upwards and clearing
    the last row.

    @param    vid_mem_offset    The video memory offset of the current cursor
                                position.

    @result The video memory offset of the cursor position after scrolling is
    performed.
*/
static int handle_scrolling(int vid_mem_offset) {
    int trow;
    uint8_t *vid_mem;

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

    vid_mem = (uint8_t *) VIDEO_ADDRESS;

    /*!
        @defined SCROLL_MEM_COPY_SIZE

        @discussion The number of bytes that must be copied to perform a scroll
        operation. Scrolling requires copying 24 out of the 25 rows.
    */
    #define SCROLL_MEM_COPY_SIZE (((MAX_ROWS - 1) * MAX_COLS) * 2)

    /* Copy rows. */
    memory_copy(vid_mem, vid_mem + MAX_COLS * 2, SCROLL_MEM_COPY_SIZE);

    /* Clear last row. */
    zero_memory (vid_mem + SCROLL_MEM_COPY_SIZE, MAX_COLS * 2);

    vid_mem_offset = row_col_to_screen_video_mem_offset (24, 0);

    return vid_mem_offset;
}


/*!
    @function print_ch_at
    @abstraction Prints a single character to the screen at the specified
    position and specified background/foreground color.

    @param    c    The ASCII code of the character to print

    @param    cattr    Sets background/foreground color.

    @param    row    Row position of the character.
    @param    col    Column position of the character.

    @discussion Print a single character to the screen at the specified
    position. The meaning of "position": the screen is treated as a 25x80 grid
    of characters i.e. 25 rows, 80 columns. The special position
    <row, col> == <-1, -1> instructs the function to print the character at the
    current cursor position. The cursor position is automatically advanced after
    any print, regardless of the arguments.

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
void print_ch_at(char c, uint8_t cattr, int row, int col) {
    uint8_t *vid_mem;
    int vid_mem_offset;
    int trow;

    vid_mem = (uint8_t *) VIDEO_ADDRESS;

    if (cattr == 0)
        cattr = CHAR_ATTR_WHITE_ON_BLACK;

    /* Determine where the character will be printed. */
    if (row >= 0 && col >= 0) {
        vid_mem_offset = row_col_to_screen_video_mem_offset(row, col);
    } else {
        vid_mem_offset = get_cursor();
    }

    if (c == '\n') {
        /* Move cursor to a new line. */

        trow = vid_mem_offset_to_row (vid_mem_offset);

        /* Set the video memory offset to the last column of the current row.
        Then the code below will increment the video memory offset, which
        has the net effect of leaving the cursor at the first column of the
        next row. */
        vid_mem_offset = row_col_to_screen_video_mem_offset(trow, 79);
    } else {
        /* Print the given character. */
        vid_mem[vid_mem_offset] = c;
        vid_mem[vid_mem_offset + 1] = cattr;
    }



    /* Advance the cursor position. */
    vid_mem_offset += 2;

    /* Auto scroll. */
    vid_mem_offset = handle_scrolling(vid_mem_offset);

    set_cursor(vid_mem_offset);
}

/*!
    @function clear_screen

    @discussion Sets every character cell to the background color.
*/
void clear_screen(void) {
    uint8_t *vid_mem;

    vid_mem = (uint8_t *) VIDEO_ADDRESS;

    /*!
        @defined CLEAR_SIZE

        @discussion The number of bytes that must be zeroed to perform a clear
        screen operation.
    */
    #define CLEAR_SIZE (MAX_ROWS * MAX_COLS * 2)

    zero_memory (vid_mem, CLEAR_SIZE);
}

/*!
    @function print_at

    @discussion Prints a string starting at the specified position.

    @param    s    The string to print.
    @param    row    Row number of the position.
    @param    col    Column number of the position.
*/
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

/*!
    @function print

    @discussion Prints a string at the current cursor position.

    @param    s    The string to print.
*/
void print(const char *s) {

    while (*s != '\0') {
        print_ch_at(*s, 0, -1, -1);
        s++;
    }
}

/*!
    @function print_byteh

    @discussion Prints a given byte value in hexadecimal format.

    @param    b    The byte value to print.
*/
void print_byteh (uint8_t b) {
    char s[21];
    xtoa(b, s, 0);
    print(s + 6);
}

/*!
    @function print_uint32h

    @discussion Prints an unsigned 32-bit value in hexadecimal format.

    @param    i    The 32-bit value to print.
*/
void print_uint32h(uint32_t i) {
    char s[21];
    xtoa(i, s, 0);
    print(s);
}

/*!
    @function print_d

    @discussion Prints an int value in decimal format.

    @param    d    The value to print.
*/
void print_d(int d) {
    char s[21];
    dtoa(d, s);
    print(s);
}

void print_llx(long long x) {
    char s[21];
    llxtoa(x, s, 0);
    print(s);
}
