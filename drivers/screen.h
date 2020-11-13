#ifndef __SCREEN_H__ // Add a standard include guard.
#define __SCREEN_H__


#define VIDEO_ADDRESS 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80

/* Character attribute byte for color scheme. */
#define CHAR_ATTR_WHITE_ON_BLACK 0x0F

/* screen device I/O ports */
#define REG_SCREEN_CTRL_IO_PORT 0x3D4
#define REG_SCREEN_DATA_IO_PORT 0x3D5

/*

CRTC Registers - Cursor position control.


Cursor Location High 0xE = 0d14.
Cursor Location Low 0xF = 0d15.

*/

#define CURSOR_LOCATION_HIGH_BYTE 0x0E
#define CURSOR_LOCATION_LOW_BYTE 0x0F

void print_ch_at(char c, char cattr, int row, int col);
void print_at(const char *s, int row, int col);
void print(const char *s);
void clear_screen(void);
void print_byteb(unsigned char b);
void print_byteh(unsigned char b, int pf);
void print_uint32h(unsigned int i);
int atoi(const char *str);
void dead_loop(void);
void print_assert(char *e, char *f, int l);

/*!
    @defined    assert
    @discussion An approximation to the assert macro found in the standard C
                library header <assert.h>.
*/
#define assert(e) \
    ( (void) ( (e) ? ((void) 0) : __assert(#e, __FILE__, __LINE__) ) )

/*!
    @defined    __assert
    @discussion helper macro for assert macro.
*/
#define __assert(e, file, line) \
   ( (void) (print_assert(e, file, line), dead_loop() ) )

#endif