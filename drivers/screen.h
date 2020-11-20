#ifndef __SCREEN_H__
#define __SCREEN_H__

/*! See .c */
void print_ch_at(char c, char cattr, int row, int col);
/*! See .c */
void print_at(const char *s, int row, int col);
/*! See .c */
void print(const char *s);
/*! See .c */
void clear_screen(void);
/*! See .c */
void print_byteb(unsigned char b);
/*! See .c */
void print_byteh(unsigned char b, int pf);
/*! See .c */
void print_uint32h(unsigned int i);
/*! See .c */
int atoi(const char *str);
/*! See .c */
void dead_loop(void);
/*! See .c */
void print_assert(char *e, char *f, int l);

/*!
    @defined assert(e)

    @discussion An approximation to the assert macro found in the standard C
    library header <assert.h>.
*/
#define assert(e) \
    ( (void) ( (e) ? ((void) 0) : __assert(#e, __FILE__, __LINE__) ) )

/*!
    @defined __assert(e, file, line)

    @discussion Helper macro for assert macro.
*/
#define __assert(e, file, line) \
   ( (void) (print_assert(e, file, line), dead_loop() ) )

#endif