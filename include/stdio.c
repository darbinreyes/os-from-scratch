/*!
    @header Standard C Header
*/

#include "stdio.h"
#include "assert.h"

/*!
    @function    rstr

    @discussion Reverses the characters in the given null terminated string.

    @param    s    The string to be reversed.

    @result Return s.

*/
char *rstr(char *s) {
    char t, *ct;

    if (s == NULL) {
        assert(0);
        return s;
    }

    if (*s == '\0')
        return s;

    ct = s;

    while (*ct != '\0')
        ct++;

    ct--;

    while (s < ct) {
        t = *s;
        *s = *ct;
        *ct = t;
        s++;
        ct--;
    }

    return s;
}

/*!
    @defined D_STR_SIZE_MAX

    @discussion The max size of a character string necessary to print a 64-bit
                integer in decimal format.
*/
#define D_STR_SIZE_MAX 21

/*!
    @function    dtoa

    @discussion This function converts an int to an ASCII string. It is not part
    of the standard C library.

    @param    d    The integer to convert.
    @param    s    Pointer to a character array at least 21 chars in size.

    @result    0    If successful.
               1    Invalid arg.
*/
int dtoa(int d, char *s) {
    int sign;
    const int pwr = 10;
    char *t;

    assert(sizeof(d) <= 8); // Allow at most printing a 64-bit int.

    if(s == NULL) {
        assert(0);
        return 1;
    }

    sign = 1;

    if (d < 0) {
        sign = -1;
        d *= -1;
    }

    t = s; // Keep pointer to first char.

    while (d > 0) {
        *s = d % pwr + '0';
        s++;
        d /= pwr;
    }

    if (sign == -1) {
        *s = '-';
        s++;
    }

    *s = '\0';

    rstr(t);

    return 0;
}