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

/*! See .c */
int dtoa(int d, char *s);
/*! See .c */
int xtoa(int x, char *s, int cf);
/*! See .c */
int llxtoa(long long int x, char *s, int cf);
#endif