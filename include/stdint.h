/*!
    @header Standard C Header
    Typedefs for integers with the number of bits indicated in the type name.
*/
#ifndef __STDINT_H__
#define __STDINT_H__

/*******************************************************************************
@remark The size of each type was tested at runtime, as follows:
sizeof(unsigned char) == 1
sizeof(unsigned short) == 2
sizeof(unsigned int) == 4
sizeof(unsigned long) == 4
sizeof(unsigned long long) == 8
sizeof(void*) == 4
*******************************************************************************/

/*!
    @typedef    uint8_t

    @discussion Alias for an 8-bit unsigned integer.

    @TODO Move these typedefs to stdint.h.
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