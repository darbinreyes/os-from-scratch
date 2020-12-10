/*! See .s */
#ifndef __TEST_H__
#define __TEST_H__

/*! See .s */
void *lidt_and_sti(void *idtr);

/*!
    @defined    INTR_V_N_HANDLER_FUNC_NAME(vn)

    @discussion Helper macro for `INTR_V_N_HANDLER_FUNC(vn)`. For example,
    `INTR_V_N_HANDLER_FUNC_NAME(0)` declares the token `intr_v0_handler`.
*/
#define INTR_V_N_HANDLER_FUNC_NAME(vn) intr_v##vn##_handler

/*!
    @defined    INTR_V_N_HANDLER_FUNC(vn)

    @discussion Helper macro for declaring the exception/interrupt handlers
    that are defined and exported in the .s file. For example,
    `INTR_V_N_HANDLER_FUNC(0)` declares the function
    `void intr_v0_handler(void);`.
*/
#define INTR_V_N_HANDLER_FUNC(vn) void INTR_V_N_HANDLER_FUNC_NAME(vn)(void)

/*!
    @function intr_v0_handler . . . intr_v32_handler

    @discussion Declaration via macro of the pre-defined exception/interrupt
    handlers that are defined and exported in the .s file. The declarations
    make the handler entry points visible in the .c file so they can be used
    to fill the IDT.
*/
INTR_V_N_HANDLER_FUNC(0);
INTR_V_N_HANDLER_FUNC(1);
INTR_V_N_HANDLER_FUNC(2);
INTR_V_N_HANDLER_FUNC(3);
INTR_V_N_HANDLER_FUNC(4);
INTR_V_N_HANDLER_FUNC(5);
INTR_V_N_HANDLER_FUNC(6);
INTR_V_N_HANDLER_FUNC(7);
INTR_V_N_HANDLER_FUNC(8);
INTR_V_N_HANDLER_FUNC(9);
INTR_V_N_HANDLER_FUNC(10);
INTR_V_N_HANDLER_FUNC(11);
INTR_V_N_HANDLER_FUNC(12);
INTR_V_N_HANDLER_FUNC(13);
INTR_V_N_HANDLER_FUNC(14);
// 15 - RESERVED
INTR_V_N_HANDLER_FUNC(16);
INTR_V_N_HANDLER_FUNC(17);
INTR_V_N_HANDLER_FUNC(18);
INTR_V_N_HANDLER_FUNC(19);
INTR_V_N_HANDLER_FUNC(20);
INTR_V_N_HANDLER_FUNC(21);
// 22 - 31 - RESERVED
INTR_V_N_HANDLER_FUNC(32);

/******************************************************************************/

/*! See .s */
void init_pics(void);

/*!
    @function    intr_v33_handler

    @discussion See .s
*/
INTR_V_N_HANDLER_FUNC(33);

#endif