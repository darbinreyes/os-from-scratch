/*!
    @header Temporary place for commonly used definitions.

*/
#ifndef __MY_LIB_C_H__
#define __MY_LIB_C_H__

/*!
    @defined    NULL
    @discussion An alias for 0. Typically used with pointers.
*/
#define NULL (0)

/*!
    @defined    BITN(b)
    @discussion Macro for creating an integer with bit `b` set and all other
    bits cleared.
*/
#define BITN(b) (1U << (b))

/*!
    @defined    BIT0
    @discussion Alias for BITN(0).
*/
#define BIT0    ( BITN(0) )

/*!
    @defined    BITb
    @discussion Alias for BITN(b).
*/
#define BIT1    ( BITN(1) )

/*!
    @defined    BITb
    @discussion Alias for BITN(b).
*/
#define BIT2    ( BITN(2) )

/*!
    @defined    BITb
    @discussion Alias for BITN(b).
*/
#define BIT3    ( BITN(3) )

/*!
    @defined    BITb
    @discussion Alias for BITN(b).
*/
#define BIT4    ( BITN(4) )

/*!
    @defined    BITb
    @discussion Alias for BITN(b).
*/
#define BIT5    ( BITN(5) )

/*!
    @defined    BITb
    @discussion Alias for BITN(b).
*/
#define BIT6    ( BITN(6) )

/*!
    @defined    BITb
    @discussion Alias for BITN(b).
*/
#define BIT7    ( BITN(7) )

/*!
    @typedef    uint8_t
    @discussion Alias for an 8-bit unsigned integer.
    The size of each type was tested at runtime, as follows:
    sizeof(unsigned char) == 1
    sizeof(unsigned short) == 2
    sizeof(unsigned int) == 4
    sizeof(unsigned long) == 4
    sizeof(unsigned long long) == 8
    sizeof(void*) == 4
*/
typedef unsigned char uint8_t;

/*!
    @typedef    uint16_t
    @discussion Alias for an 16-bit unsigned integer.
*/
typedef unsigned short uint16_t;

/*!
    @typedef    uint32_t
    @discussion Alias for an 32-bit unsigned integer.
*/
typedef unsigned int uint32_t;

/*!
    @typedef    uint64_t
    @discussion Alias for an 64-bit unsigned integer.
*/
typedef unsigned long long uint64_t;

/*!
    @typedef    int8_t
    @discussion Alias for an 8-bit signed integer.
*/
typedef signed char int8_t;

/*!
    @typedef    int16_t
    @discussion Alias for an 16-bit signed integer.
*/
typedef signed short int16_t;

/*!
    @typedef    int32_t
    @discussion Alias for an 32-bit signed integer.
*/
typedef signed int int32_t;

/*!
    @typedef    int64_t
    @discussion Alias for an 64-bit signed integer.
*/
typedef signed long long int64_t;
#endif