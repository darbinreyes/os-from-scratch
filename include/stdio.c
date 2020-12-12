/*!
    @header Standard C Header
*/

#include "stdio.h"
#include "assert.h"
#include "limits.h"

/*!
    @function    rstr

    @discussion Reverses the characters in the given null terminated string.

    @param    s    The string to be reversed.

    @result Return s.

*/
static inline char *rstr(char *s) {
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
    @function    _utoa

    @discussion This function converts an unsigned int to an ASCII string in
    decimal. It is not part of the standard C library. Example output:
    "18446744073709551615".

    @param    d    The integer to convert.
    @param    s    Pointer to a character array at least STDIO_STR_SIZE_MAX
                   chars in size.

    @result    0    If successful.
               1    s is NULL.
*/
int _utoa(unsigned long long d, char *s) {
    char *t;

    assert(sizeof(d) == 8);

    if (s == NULL) {
        assert(0);
        return 1;
    }

    *s = '\0';

    t = s; // Keep pointer to first char.

    if (d == 0) {
        *s = '0';
        s++;
        *s = '\0';
        return 0;
    }

    while (d > 0) {
        *s = d % 10 + '0';
        d /= 10;
        s++;
    }

    *s = '\0';

    rstr(t);

    return 0;
}

/*!
    @function    _dtoa

    @discussion This function converts an int to an ASCII string in decimal. It
    is not part of the standard C library. Example output: "-2147483648".

    @param    d    The integer to convert.
    @param    s    Pointer to a character array at least STDIO_STR_SIZE_MAX
                   chars in size.

    @result    0    If successful.
               1    s is NULL.
*/
int _dtoa(long long d, char *s) {
    int sign;
    char *t;

    assert(sizeof(d) == 8);

    if (s == NULL) {
        assert(0);
        return 1;
    }

    *s = '\0';

    t = s; // Keep pointer to first char.

    if (d == 0) {
        *s = '0';
        s++;
        *s = '\0';
        return 0;
    }

    sign = 1;

    if (d < 0) {
        sign = -1;
        d = -d;
    }

    while (d > 0) {
        *s = d % 10 + '0';
        d /= 10;
        s++;
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
    @function    dectoa

    @discussion Converts integer d to its ASCII equivalent digit in the set
    [0-9]. If d is out of range (> 9) returns -1.

    @param    d    The integer to convert.

    @result The ASCII equivalent or -1.
*/
static inline char dectoa(uint8_t d) {
    if (d > 9)
        return -1;

    return d + '0';
}

/*!
    @function nibtoa

    @discussion Converts a nibble to an ASCII hexadecimal digit. Upper nibble is
    masked out, lower nibble is converted to an ASCII character in the set
    [0-9|A-F|a-f].

    @param    b    The nibble to convert to ASCII.

    @param    cf    Case flag. [a-f] or [A-F]. 0 = lower. 1 upper.

    @result The ASCII encoded hexadecimal digit.
*/
static inline char nibtoa (uint8_t b, int cf) {
    char c = 'A';

    b = 0x0F & b; // Take the lower nibble only.

    if(cf == 0)
        c = 'a';

    if (b < 10)
        return dectoa(b);

    return b - 10 + c;
}

/*!
    @function    _xtoa

    @discussion This function converts an int to an ASCII string in hexadecimal
    format. It is not part of the standard C library. Example output:
    "deadbeef".

    @param    x    The integer to convert.
    @param    s    Pointer to a character array at least STDIO_STR_SIZE_MAX
                   chars in size.
    @param    nbits The number of bits in x. One of 8, 16, 32, 64.
    @param    cf    Case flag. Case for letters. 0 = lower. 1 upper.

    @result    0    If successful.
               1    s is NULL.
               2    nbits is invalid.
*/
int _xtoa(unsigned long long x, int nbits, char *s, int cf) {
    uint8_t b;
    int sh;

    assert(sizeof(x) == 8);

    if(s == NULL) {
        assert(0);
        return 1;
    }

    *s = '\0';

    if (nbits <= 0 || nbits % 8 != 0 || nbits > 64) {
        assert(0);
        return 2;
    }

    sh = nbits - 8;

    while (sh >= 0) {
        b = (x >> sh) & 0xFFU;
        *s = nibtoa(b >> 4, cf);
        s++;
        *s = nibtoa(b, cf);
        s++;
        sh -= 8;
    }

    *s = '\0';

    return 0;
}

/*!
    @function    _otoa

    @discussion This function converts an int to an ASCII string in octal
    format. It is not part of the standard C library. Example output:
    "7654".

    @param    o    The integer to convert.
    @param    s    Pointer to a character array at least STDIO_STR_SIZE_MAX
                   chars in size.
    @param    nbits The number of bits in o. One of 8, 16, 32, 64.

    @result    0    If successful.
               1    s is NULL.
               2    nbits is invalid.
*/
int _otoa(unsigned long long o, int nbits, char *s) {
    int sh;

    assert(sizeof(o) == 8);

    if(s == NULL) {
        assert(0);
        return 1;
    }

    *s = '\0';

    if (nbits <= 0 || nbits % 8 != 0 || nbits > 64) {
        assert(0);
        return 2;
    }

    sh = nbits - nbits % 3;

    while (sh >= 0) {
        *s =  dectoa((o >> sh) & 0x07);
        s++;
        sh -= 3;
    }

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