#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "../include/stdint.h"

/*! See .c */
void print_ch_at(char c, uint8_t cattr, int row, int col);
/*! See .c */
void print_at(const char *s, int row, int col);
/*! See .c */
void print(const char *s);
/*! See .c */
void clear_screen(void);
/*! See .c */
void print_byteb(uint8_t b);
/*! See .c */
void print_byteh(uint8_t b, int pf);
/*! See .c */
void print_uint32h(uint32_t i);
/*! See .c */
void print_d(int d);

#endif