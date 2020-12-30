/*! See .s */
#ifndef __IDT_ASM_H__
#define __IDT_ASM_H__

/*! See .s */
void *lidt_and_sti(void *idtr);

/*!
    @defined    INTR_VN_HANDLER(vn)

    @discussion Helper macro for `INTR_VN_HANDLER_DECL(vn)`. For example,
    `INTR_VN_HANDLER(0)` declares the token `intr_v0_handler`.
*/
#define INTR_VN_HANDLER(vn) intr_v##vn##_handler

/*!
    @defined    INTR_VN_HANDLER_DECL(vn)

    @discussion Helper macro for declaring the exception/interrupt handlers
    that are defined and exported in the .s file. For example,
    `INTR_VN_HANDLER_DECL(0)` declares the function
    `void intr_v0_handler(void);`.
*/
#define INTR_VN_HANDLER_DECL(vn) void INTR_VN_HANDLER(vn)(void)

/*!
    @typedef    idt_proc_t
    @discussion Pointer to function/procedure entry point of an
                interrupt/exception handler. Used to fill IDT entries.
*/
typedef void (*idt_proc_t)(void);

/*!
    @typedef vn_handler
    @discussion Pointer to function that handles a specific interrupt/exception
                taking into account the vector number, i.e. taking into account
                the source of the interrupt.

*/
typedef void (*vn_handler_t)(uint32_t vn, uint32_t err_code);

/*!
    @typedef idt_handler_t
    @discussion Typedef for organizing IDT procedure entry points and their
    vector specific handlers into a single table.
*/
typedef struct _idt_handler_t {
    idt_proc_t   idt_proc;
    vn_handler_t vn_handler;
} idt_handler_t;

/*!
    @function intr_v0_handler . . . intr_v32_handler

    @discussion Declaration via macro of the pre-defined exception/interrupt
    handlers that are defined and exported in the .s file. The declarations
    make the handler entry points visible in the .c file so they can be used
    to fill the IDT.
*/
INTR_VN_HANDLER_DECL(0);
INTR_VN_HANDLER_DECL(1);
INTR_VN_HANDLER_DECL(2);
INTR_VN_HANDLER_DECL(3);
INTR_VN_HANDLER_DECL(4);
INTR_VN_HANDLER_DECL(5);
INTR_VN_HANDLER_DECL(6);
INTR_VN_HANDLER_DECL(7);
INTR_VN_HANDLER_DECL(8);
INTR_VN_HANDLER_DECL(9);
INTR_VN_HANDLER_DECL(10);
INTR_VN_HANDLER_DECL(11);
INTR_VN_HANDLER_DECL(12);
INTR_VN_HANDLER_DECL(13);
INTR_VN_HANDLER_DECL(14);
// 15 - RESERVED
INTR_VN_HANDLER_DECL(16);
INTR_VN_HANDLER_DECL(17);
INTR_VN_HANDLER_DECL(18);
INTR_VN_HANDLER_DECL(19);
INTR_VN_HANDLER_DECL(20);
INTR_VN_HANDLER_DECL(21);
// 22 - 31 - RESERVED
INTR_VN_HANDLER_DECL(32); // 32-255 - User Defined Interrupts
INTR_VN_HANDLER_DECL(33);
/******************************************************************************/

#endif