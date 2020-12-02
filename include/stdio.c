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

    assert(sizeof(d) == 4); // Check size of int

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
    @param    cf    Case flag. Case for letters. 0 = lower. 1 upper.

    @result    0    If successful.
               1    Invalid arg.
*/
int xtoa(int x, char *s, int cf) {
    uint8_t b;
    int sh;

    if(s == NULL) {
        assert(0);
        return 1;
    }

    assert(sizeof(x) == 4);

    sh = 24;

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

int lxtoa(long int x, char *s, int cf) {
    assert(sizeof(x) == 4);
    return xtoa(x, s, cf);
}

int llxtoa(long long int x, char *s, int cf) {
    int t;

    assert(sizeof(x) == 8);

    t = (int) (x >> 32);

    xtoa(t, s, cf);

    s += 8;

    t = (int) x;

    return xtoa(t, s, cf);
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