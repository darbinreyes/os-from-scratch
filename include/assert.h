/*!
    @header Standard C Header
*/
#ifndef __ASSERT_H__
#define __ASSERT_H__

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