/*!
    @header Temporary and non-standard definitions.
    Temporary place for standard C library definition definitions. Also,
    definitions which are not part of the standard C library.

*/

#ifndef __MY_LIB_C_H__
#define __MY_LIB_C_H__

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

#endif