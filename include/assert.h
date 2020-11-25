/*!
    @header Standard C Header
*/

#ifndef __ASSERT_H__
#define __ASSERT_H__ // @TODO Local assert.h does not use include guard.

#include "../drivers/screen.h" // print()

/* #define NDEADLOOP */ /* If defined, the assert message is printed and the
                           dead loop is compiled out, execution continues as
                           usual. */

/* #define NDEBUG */ /* The assert() macro may be removed at compile time with
                        the gcc option -DNDEBUG. */

#ifdef NDEBUG
#define assert(e)    ((void)0) // Implements disabling asserts. See man.
#else

/*!
    @defined assert(e)

    @discussion An approximation to the assert macro found in the standard C
    library header @doc [assert.h](<assert.h>) @doc [man assert](man assert).
    The assert() macro tests the given `expression` and if it is false, a dead
    loop is entered. A diagnostic message is written to the screen

    If `expression` is true, the assert() macro does nothing.

    The assert() macro may be removed at compile time with the gcc option
    -DNDEBUG.

    @param    e    Expression to evaluate.
*/
#define assert(e) \
    ( (void) ( (e) ? ((void) 0) : __assert(#e, __FILE__, __LINE__) ) )

/*!
    @defined __assert(e, file, line)

    @discussion Helper macro for assert macro.
*/
#define __assert(e, file, line) \
   ( (void) (print_assert(e, file, line), dead_loop() ) )

/*! See .c */
void dead_loop(void);

/*! See .c */
void print_assert(char *e, char *f, int l);

#endif /* NDEBUG */

#endif /* __ASSERT_H__ */
/*******************************************************************************
ASSERT(3)                BSD Library Functions Manual                ASSERT(3)

NAME
     assert -- expression verification macro

SYNOPSIS
     #include <assert.h>

     assert(`expression`);

DESCRIPTION
     The assert() macro tests the given `expression` and if it is false, the
     calling process is terminated.  A diagnostic message is written to `stderr`
     and the abort(3) function is called, effectively terminating the program.

     If `expression` is true, the assert() macro does nothing.

     The assert() macro may be removed at compile time with the cc(1) option
     -DNDEBUG.

DIAGNOSTICS
     The following diagnostic message is written to `stderr` if `expression` is
     false:

           "assertion \"%s\" failed: file \"%s\", line %d\n", \
                               "expression", __FILE__, __LINE__);

SEE ALSO
     cc(1), abort(3)

STANDARDS
     The assert() macro conforms to ANSI X3.159-1989 (``ANSI C89'').

HISTORY
     A assert macro appeared in Version 6 AT&T UNIX.

BSD                              June 9, 1993                              BSD
*******************************************************************************/