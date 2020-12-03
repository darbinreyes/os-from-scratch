/*!
    @header Integer min/max limits.
    The header <limits.h> defines constants for the sizes of integral types. The
    values below are acceptable minimum magnitudes.
*/

/* @TODO Match <limits.h> */

#define INT8_MIN -128 // - 2**7
#define INT8_MAX  127 // 2**7 - 1

#define INT16_MIN -32768 // - 2**15
#define INT16_MAX  32767 // 2**15 - 1

#define INT32_MIN -2147483648 // -2,147,483,648 = - 2**31
#define INT32_MAX  2147483647 // 2,147,483,647 = 2**31 - 1

/* @remark The compiler rejects `-2^63 = -9223372036854775808` so we must use
   `-2^63 + 1 = 9223372036854775807`. It doesn't complain about this for the
   smaller signed integer types. */
#define INT64_MIN -9223372036854775807 // - 2**63 + 1
#define INT64_MAX  9223372036854775807 // 2**63 - 1

#define UINT8_MAX 255 // 2**8 - 1

#define UIN16_MAX 65535 // 2**16 - 1

#define UINT32_MAX 4294967295 // 2**32 - 1

#define UINT64_MAX 18446744073709551615 // 2**64 - 1