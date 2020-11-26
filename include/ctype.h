/*!
    @header Standard C Header
*/

#ifndef __CTYPE_H__
#define __CTYPE_H__

/*!
    @function isspace

    @discussion The isspace() function tests for the white-space characters.
    @doc [man isspace / ctype.h].

    @param    c    The character to test.

    @result The isspace() function returns zero if the character tests false
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
    @function isdigit

    @discussion The isdigit() function tests for a decimal digit character.

    @result The isdigit() function returns zero if the character tests false
    and returns non-zero if the character tests true.
*/
static inline int isdigit(int c) {
    if (c >= '0' && c <= '9')
        return 1;

    return 0;
}

/*!
    @function isxdigit

    @discussion The isxdigit() function tests for any hexadecimal-digit
    character.

    @result The isxdigit() function returns zero if the character tests false
    and returns non-zero if the character tests true.
*/
static inline int isxdigit(int c) {
    if ((c >= '0' && c <= '9') ||  (c >= 'A' && c <= 'F') ||  (c >= 'a' && c <= 'f'))
        return 1;

    return 0;
}

/*!
    @function digittoint

    @discussion The digittoint() function converts a numeric character to its
    corresponding integer value.  The character can be any decimal digit or
    hexadecimal digit. With hexadecimal characters, the case of the values does
    not matter. @doc [man digittoint / ctype.h].

    @result The digittoint() function always returns an integer from the
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
#endif