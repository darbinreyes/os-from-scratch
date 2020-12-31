/*!
    @header Standard C Header
*/

#ifndef __STDDEF_H__
#define __STDDEF_H__

/*!
    @defined    NULL

    @discussion An alias for 0. Typically used with pointers. "The symbolic
    constant NULL is often used in place of zero, as a mnemonic to indicate more
    clearly that this is a special value for a pointer."
*/
#ifndef NULL
#define NULL ((void *)0)
#endif

/*!
    @typedef size_t

    @discussion The type returned by sizeof.

    @remark Verified at runtime: sizeof(sizeof(char)) == 4.
*/
typedef unsigned int size_t;

#endif