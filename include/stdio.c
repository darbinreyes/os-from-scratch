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
static char *rstr(char *s) {
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
                integer in decimal format. See limits.h.
*/
#define D_STR_SIZE_MAX 21

/*!
    @function    dtoa

    @discussion This function converts an int to an ASCII string in decimal. It
    is not part of the standard C library. Example output: "-2147483648".

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

/*!
    @function nibtoa

    @discussion Converts a nibble to an ASCII hexadecimal digit. Upper nibble is
    masked out, lower nibble is converted to an ASCII character in the set
    [0-9|A-F].

    @param    b    The nibble to convert to ASCII.

    @param    cf    Case flag. [a-f] or [A-F]. 0 = lower. 1 upper.

    @result The ASCII encoded hexadecimal digit.
*/
static inline char nibtoa (uint8_t b, int cf) {
    char c;

    if(cf == 0)
        c = 'a';
    else
        c = 'A';

    b = 0x0F & b; // Take the lower nibble.

    if (b <= 9)
        b += '0';
    else // if (b > 9)
        b = b - 10 + c;

    return b;
}

/*!
    @function    xtoa

    @discussion This function converts an int to an ASCII string in hexadecimal
    format. It is not part of the standard C library. Example output:
    "deadbeef".

    @param    x    The integer to convert.
    @param    s    Pointer to a character array at least 21 chars in size.

    @result    0    If successful.
               1    Invalid arg.
*/
int xtoa(int x, char *s) {
    uint8_t b;

    if(s == NULL) {
        assert(0);
        return 1;
    }

    assert(sizeof(x) <= 8); // Allow at most printing a 64-bit int.

    // High order byte // @TODO Use loop.
    b = (x >> 24) & 0xFFU;
    *s = nibtoa (b >> 4, 0);
    s++;
    *s = nibtoa (b, 0);
    s++;
    // Next byte
    b = (x >> 16) & 0xFFU;
    *s = nibtoa (b >> 4, 0);
    s++;
    *s = nibtoa (b, 0);
    s++;
    // Next byte
    b = (x >> 8) & 0xFFU;
    *s = nibtoa (b >> 4, 0);
    s++;
    *s = nibtoa (b, 0);
    s++;
    // Low order byte
    b = (x >> 0) & 0xFFU;
    *s = nibtoa (b >> 4, 0);
    s++;
    *s = nibtoa (b, 0);
    s++;
    *s = '\0';

    return 0;
}

///////////////////////////
/*!
    @function print_byteb

    @discussion Prints a byte in binary format.

    @param    b    The byte value to print.
*/
// void print_byteb (uint8_t b) {
//     print("0b");
//     for (int i = 7; i >= 0; i--)
//         if (b & BITN(i))
//             print_ch_at('1', 0, -1, -1);
//         else
//             print_ch_at('0', 0, -1, -1);
// }
/////////////////////////////