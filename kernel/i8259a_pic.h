#ifndef __I8259A_PIC_H__
#define __I8259A_PIC_H__

/*! See .c */
void init_pics(void);

/*! See .c */
void m_pic_eoi(void);

/*! See .c */
void s_pic_eoi(void);

#endif