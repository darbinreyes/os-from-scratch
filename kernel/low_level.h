#ifndef __LOW_LEVEL_H__
#define __LOW_LEVEL_H__

#include "../include/stdint.h"

/*! See .c */
uint8_t port_byte_in (uint16_t port);

/*! See .c */
void port_byte_out (uint16_t port, uint8_t data);

#endif