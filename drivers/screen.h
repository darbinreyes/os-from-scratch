#ifndef __SCREEN_H__
#define __SCREEN_H__

/*! See .c */
void print_ch_at(char c, char cattr, int row, int col);
/*! See .c */
void print_at(const char *s, int row, int col);
/*! See .c */
void print(const char *s);
/*! See .c */
void clear_screen(void);
/*! See .c */
void print_byteb(unsigned char b);
/*! See .c */
void print_byteh(unsigned char b, int pf);
/*! See .c */
void print_uint32h(unsigned int i);
/*! See .c */
int atoi(const char *str);

#endif