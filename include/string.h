/*!
    @header Standard C Header
*/
#ifndef __STRING_H__
#define __STRING_H__

//void *memcpy(void *restrict dst, const void *restrict src, size_t n); // @TODO what is restrict?

/*! See .c */
void *memcpy(void * dst, const void * src, size_t n);

/*! See .c */
void *memset(void *b, int c, size_t len);

#endif