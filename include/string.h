#ifndef __STRING_H__
#define __STRING_H__

#include "stddef.h" // size_t

/*! See .c */
void *memcpy(void *restrict dst, const void *restrict src, size_t n);

/*! See .c */
void *memset(void *b, int c, size_t len);

/*! See .c */
int strcmp(const char *s1, const char *s2);

#endif