#ifndef __MY_LIB_C_H__
#define __MY_LIB_C_H__

#define NULL (0)

#define BITN(b) (1U << (b))
#define BIT0    ( BITN(0) )
#define BIT1    ( BITN(1) )
#define BIT2    ( BITN(2) )
#define BIT3    ( BITN(3) )
#define BIT4    ( BITN(4) )
#define BIT5    ( BITN(5) )
#define BIT6    ( BITN(6) )
#define BIT7    ( BITN(7) )

/*

sizeof(unsigned char) == 1


sizeof(unsigned short) == 2


sizeof(unsigned int) == 4


sizeof(unsigned long) == 4


sizeof(unsigned long long) == 8


sizeof(void*) == 4

sizeof(func_ptr) == 4 // void (*func_ptr)(unsigned char b, int pf);

*/

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
#endif