
/*! @remark This function not part of the standard C library. But it is
    necessary to implement printf().
    @function itoa

    @discussion Converts an integer to a string. @remark This function not part
    of the standard C library.

    @param    i    The integer to convert to a string.

    @result String containing the result of the conversion.
*/
// char *itoa(int32_t i, char *s) {
//     //static char str[1 + 10 + 1]; // Max int32_t value = 2**31 - 1 = 2,147,483,647. Min. 2**31 = -2147483648. 1 negative sign, 10 digits, 1 null terminator.
//     int64_t pwr;

//     /*
// i | pwr | i / pwr
// 57| 10  | 5
// 57 | 100| 0
//     */
//     pwr = 10;
//     while (i / pwr > 0) {
//         pwr *= 10;
//     }

//     //str[0] = '\0';

//     // @TODO
//     return s;
// }