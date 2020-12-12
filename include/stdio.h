#ifndef __STDIO_H__
#define __STDIO_H__

/*!
    @defined    NULL

    @discussion An alias for 0. Typically used with pointers. "The symbolic
    constant NULL is often used in place of zero, as a mnemonic to indicate more
    clearly that this is a special value for a pointer."
*/
#define NULL ((void *)0)

/*!
    @defined    EOF

    @discussion    End of file.
*/
#define EOF (-1)

/*!
    @defined STDIO_STR_SIZE_MAX

    @discussion
    A safe buffer size to use when calling the various functions that return
    ASCII strings.
    Example:
    char s[STDIO_STR_SIZE_MAX];
    _utoa(-1, s);

*/
#define STDIO_STR_SIZE_MAX 32

/*! See .c */
int _utoa(unsigned long long d, char *s);
/*! See .c */
int _dtoa(long long d, char *s);
/*! See .c */
int _xtoa(unsigned long long x, int nbits, char *s, int cf);
/*! See .c */
int _otoa(unsigned long long o, int nbits, char *s);
#endif