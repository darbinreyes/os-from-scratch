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
void print_x32(uint32_t x);
/*! See .c */
void print_d(int d);
#endif