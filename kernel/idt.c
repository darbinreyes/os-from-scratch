/*

    Defining the IDT in C.

```
@spec Intel SDM Vol.3. Chapter 1.3.4.
@myconventions

1.3.4 Hexadecimal and Binary Numbers

    Base 16 (hexadecimal) numbers are represented by a string of hexadecimal digits followed by the character H (for example, F82EH). A hexadecimal digit is a character from the following set: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, and F.

    Base 2 (binary) numbers are represented by a string of 1s and 0s, sometimes followed by the character B (for example, 1010B). The “B” designation is only used in situations where confusion as to the type of number might arise.
```

*/
#include "../mylibc/mylibc.h"
#include "../drivers/screen.h"
#include "test.h"

// @spec Intel SDM Vol.3.Chapter.6.11.
#define IDT_TASK_TYPE (0x00000500)
#define IDT_INTR_TYPE (0x00000600)
#define IDT_TRAP_TYPE (0x00000700)

#define IDT_DESCRIPTOR_H(offset, p, dpl, d, type) ( (offset & 0xFFFF0000U) | (type) | ( (p & 0x00000001U) << 15 ) | ( (dpl & 0x00000003U) << 13 ) | ( (d & 0x00000001U) << 11 ) )

#define IDT_DESCRIPTOR_L(segment_sel, offset)                         ( ((segment_sel & 0x0000FFFFU) << 16) | (offset & 0x0000FFFFU) )

#define IDT_X_GATE_DESCRIPTOR(p, dpl, d, type, segment_sel, offset)   ( ( ( 0x00000000FFFFFFFFULL & IDT_DESCRIPTOR_H(offset, p, dpl, d, type) ) << 32) | ( 0x00000000FFFFFFFFULL & IDT_DESCRIPTOR_L(segment_sel, offset) ) )

#define IDT_TASK_GATE_DESCRIPTOR(p, dpl, segment_sel)                 IDT_X_GATE_DESCRIPTOR(p, dpl, 0U, IDT_TASK_TYPE, segment_sel, 0x00000000U)

#define IDT_INTR_GATE_DESCRIPTOR(p, dpl, d, segment_sel, offset)      IDT_X_GATE_DESCRIPTOR(p, dpl, d, IDT_INTR_TYPE, segment_sel, offset)

#define IDT_TRAP_GATE_DESCRIPTOR(p, dpl, d, segment_sel, offset)      IDT_X_GATE_DESCRIPTOR(p, dpl, d, IDT_TRAP_TYPE, segment_sel, offset)

#define IDT_ENTRY_COUNT 34

/*
idt_start:
;- - - - - - - vector 0 - - - - - - -;
dw v_0_handler_procedure
dw 0x0008
; 4-byte boundary.
db 00000000b
db 10001110b
dw 0x0000
*/
#define PROCEDURE_ENTRY_POINT_NOT_PRESENT 0x00000000U
#define GDT_CODE_SEGMENT 0x0008

#define GATE_SIZE_32_BITS 1
#define GATE_SIZE_16_BITS 0

#define DPL_0 0
#define DPL_KERNEL 0

#define DPL_3 3
#define DPL_USER 3

// @spec Intel SDM Vol.3.Chapter.3.4.5.
#define SEGMENT_PRESENT 1
#define SEGMENT_NOT_PRESENT 0

#define V_N_HANDLER_FUNC(vn)          \
void v_##vn##_handler(void) {         \
    print(__func__);                  \
    print(" in Dijkstra I trust.\n"); \
}                                     \

V_N_HANDLER_FUNC(0)
V_N_HANDLER_FUNC(1)

#define V_N_HANDLER_FUNC_NAME(vn) v_##vn##_handler

const uint32_t handler_entry [] = { // hndlr
    (unsigned int) V_N_HANDLER_FUNC_NAME(0),
    (unsigned int) V_N_HANDLER_FUNC_NAME(1)
};

// void (*func_ptr)(unsigned char b, int pf);

uint64_t idt[IDT_ENTRY_COUNT];

struct intr_gate_d_t { // interrupt gate descriptor.
    // @spec Intel SDM Vol.3.Chapter.6.11.
    uint16_t offset_l;
    uint16_t seg_sel;

    uint8_t rsvd:5;
    uint8_t const0:3; // 000B

    uint8_t const1:3; // 110B
    uint8_t d:1;
    uint8_t const2:1; // 0B
    uint8_t dpl:2;
    uint8_t p:1;

    uint16_t offset_h;

} __attribute__((packed)); // sizeof(struct intr_gate_d_t) == 8.

struct idt_reg_t {
    // @spec Intel SDM Vol.3.Chapter.6.10.
    uint16_t idt_limit; // sizeof(idt) - 1
    uint32_t idt_base_addr;
} __attribute__((packed));

struct idt_reg_t idtr;

#define TOKEN_TO_STR(tkn) #tkn

#define PRINT_BYTEH_STRUCT_MEMBER(sm) \
  do {                                \
    print(TOKEN_TO_STR(sm) " = ");    \
    print_uint32h(sm);                \
    print("\n");                      \
  } while (0)                         \

void init_idt(void) {
    struct intr_gate_d_t *igd_p;
    void (*func_ptr)(void);
    unsigned int t;

    idt[0] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, handler_entry[0]);
    idt[1] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, handler_entry[1]);

    igd_p = (struct intr_gate_d_t *) &idt[0];
    v_0_handler();
    print_uint32h(handler_entry[0]);
    print("\n");
    PRINT_BYTEH_STRUCT_MEMBER(igd_p->offset_l);
    PRINT_BYTEH_STRUCT_MEMBER(igd_p->seg_sel);
    PRINT_BYTEH_STRUCT_MEMBER(igd_p->rsvd);
    PRINT_BYTEH_STRUCT_MEMBER(igd_p->const0);
    PRINT_BYTEH_STRUCT_MEMBER(igd_p->const1);
    PRINT_BYTEH_STRUCT_MEMBER(igd_p->d);
    PRINT_BYTEH_STRUCT_MEMBER(igd_p->const2);
    PRINT_BYTEH_STRUCT_MEMBER(igd_p->dpl);
    PRINT_BYTEH_STRUCT_MEMBER(igd_p->p);
    PRINT_BYTEH_STRUCT_MEMBER(igd_p->offset_h);
    func_ptr = (void (*)(void)) handler_entry[0];
    func_ptr();

    print_uint32h( 8 * 34);
    print("\n");

    print_uint32h(  sizeof(idt[0]) * IDT_ENTRY_COUNT); // Why is sizeof(idt) == 16 and not  sizeof(idt[0])*IDT_ENTRY_COUNT? !!!! sizeof(idt[0])*IDT_ENTRY_COUNT == 0x10?!!!
    print("\n");

    print_uint32h((uint32_t)idt);
    print("\n");

    idtr.idt_limit = sizeof(idt) - 1;
    idtr.idt_base_addr = (uint32_t)idt;

    print_uint32h(idtr.idt_limit);
    print("\n");

    print_uint32h(idtr.idt_base_addr);
    print("\n");

    print_uint32h((uint32_t)&idtr);
    print("\n");

    t = load_idt_reg((uint32_t)&idtr);

    print_uint32h(t);
    print("\n");

    //__asm__("int $0");
    //__asm__("int $1");

}

/*

; Issue special instruction to load the IDT register.
load_idtr:
lidt [idt_register]
sti
;int 0
;int 1
;int 20
;int 21
ret

*/


