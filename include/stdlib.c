#include "ctype.h" // digittoint() etc.
#include "stdio.h" //  NULL
/*!
    @function atoi

    @discussion The atoi() function converts the initial portion of the
    string pointed to by str to int representation. It is equivalent to:
    `(int)strtol(str, (char **)NULL, 10);`. @doc [man atoi / stdlib.h].

    @param    str    The string to convert to an integer.

    @result The result of the conversion to an integer.

    @TODO Test negative numbers.
*/
int atoi(const char *str) {
    int sum;
    int sign;

    if (str == NULL)
        return 0;

    while (isspace(*str))
        str++;

    sign = 1;

    if (*str == '+') {
        str++;
    } else if (*str == '-') {
        sign = -1;
        str++;
    } else {
        ;
    }

    /*
    str | sum | sign
    ----|-----|-----
        |     |
    */
    sum = 0;

    while (isdigit(*str)) {
        sum = 10 * sum + digittoint(*str);
        str++;
    }

    return sum * sign;
}