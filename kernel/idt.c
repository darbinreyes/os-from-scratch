/*!
    @header
    @discussion
*/

#include "../drivers/screen.h"
#include "../include/stdint.h"
#include "test.h"

/*******************************************************************************
@doc [Figure 6-2. IDT Gate Descriptors](Intel64 & IA-32 Arch. SDM Vol.3 Ch.6.11)

|------------------Task Gate--------------------------|
| 31            16 | 15 | 14 13 | 12  8 | 7         0 | Bit
|------------------|----|-------|-------|-------------|
| Rsvd             | P  | DPL   | 00101 | Rsvd        | Byte 4
|-----------------------------------------------------|
|-----------------------------------------------------|
| 31           16  | 15                             0 | Bit
|------------------|----------------------------------|
| TSS Seg. Select. | Rsvd                             | Byte 0
|-----------------------------------------------------|

|------------------Interrupt Gate---------------------|
| 31            16 | 15 | 14 13 | 12  8 | 7 5 | 4   0 | Bit
|------------------|----|-------|-------|-----|-------|
| Offset 31..16    | P  | DPL   | 0D110 | 000 | Rsvd  | Byte 4
|-----------------------------------------------------|
|-----------------------------------------------------|
| 31            16 | 15                             0 | Bit
|------------------|----------------------------------|
| Segment Selector | Offset 15..0                     | Byte 0
|-----------------------------------------------------|


|------------------Trap Gate--------------------------|
| 31            16 | 15 | 14 13 | 12  8 | 7 5 | 4   0 | Bit
|------------------|----|-------|-------|-----|-------|
| Offset 31..16    | P  | DPL   | 0D111 | 000 | Rsvd  | Byte 4
|-----------------------------------------------------|
|-----------------------------------------------------|
| 31            16 | 15                             0 | Bit
|------------------|----------------------------------|
| Segment Selector | Offset 15..0                     | Byte 0
|-----------------------------------------------------|

DPL      = Descriptor Privilege Level
Offset   = Offset to procedure entry point
P        = Segment Present Flag
Selector = Segment Selector for destination code segment
D        = Size of gate: 1 = 32 bits; 0 = 14 bits
Rsvd     = Reserved
*******************************************************************************/

struct idt_gate_d_t { // interrupt gate descriptor.
    // @spec Intel SDM Vol.3.Chapter.6.11.
    uint16_t offset_l;
    uint16_t seg_sel;

    uint8_t rsvd:5;   // 00000B
    uint8_t const0:3; // 000B

    uint8_t const1:3; // Bits 10:8. Task = 101B; Interrupt = 110B; Trap = 111B;
    uint8_t d:1;
    uint8_t const2:1; // 0B
    uint8_t dpl:2;
    uint8_t p:1;

    uint16_t offset_h;

} __attribute__((packed)); // sizeof(struct idt_gate_d_t) == 8.

#define TASK_GATE_BITS_10_8 5U
#define INTR_GATE_BITS_10_8 6U
#define TRAP_GATE_BITS_10_8 7U

static inline uint64_t intr_gate_d (uint32_t offset, uint32_t p, uint32_t dpl,
                                    uint32_t d, uint16_t seg_sel) {
    struct idt_gate_d_t dt;

    dt.offset_h = offset >> 16;
    dt.p = p & 1U;
    dt.dpl = dpl & 3U;
    dt.const2 = 0;
    dt.d = d & 1U;
    dt.const1 = INTR_GATE_BITS_10_8;
    dt.const0 = 0;
    dt.rsvd = 0;
    dt.seg_sel = seg_sel;
    dt.offset_l = offset;

    return *((uint64_t *) &dt);
}

// @spec Intel SDM Vol.3.Chapter.6.11.
// @doc [Intel 64 & IA-32 SDM, Vol.3, Table 3-2. System-Segment and Gate-Descriptor Types].
#define IDT_TASK_TYPE (0x00000500) // Type[bit 11:8] == 0101B ==  5 == Task gate.
#define IDT_INTR_TYPE (0x00000600) // Type[bit 11:8] == 1110B == 14 == 32-bit interrupt gate.
#define IDT_TRAP_TYPE (0x00000700) // Type[bit 11:8] == 1111B == 15 == 32-bit trap gate.
/******************************************************************************/
#define IDT_DESCRIPTOR_H(offset, p, dpl, d, type) \
   ( (offset & 0xFFFF0000U) | (type) | ( (p & 0x00000001U) << 15 ) \
 | ( (dpl & 0x00000003U) << 13 ) | ( (d & 0x00000001U) << 11 ) )

#define x_IDT_DESCRIPTOR_H(offset, p, dpl, d, bits_10_8) \
((((uint32_t)offset) & 0xFFFF0000U) | \
((((uint32_t)p) & 0x1U) << 15) | ((((uint32_t)dpl) & 0x3U) << 13) | \
((((uint32_t)d) & 0x1U) << 11) | ( (((uint32_t)bits_10_8) & 0x7U) << 8))
/******************************************************************************/
#define IDT_DESCRIPTOR_L(segment_sel, offset) \
( ((segment_sel & 0x0000FFFFU) << 16) | (offset & 0x0000FFFFU) )

#define x_IDT_DESCRIPTOR_L(segment_sel, offset) \
(((((uint32_t)segment_sel) & 0xFFFFU) << 16) | ((((uint32_t)offset) & 0xFFFFU)))
/******************************************************************************/
#define IDT_X_GATE_DESCRIPTOR(p, dpl, d, type, segment_sel, offset) \
(((0x00000000FFFFFFFFULL & IDT_DESCRIPTOR_H(offset, p, dpl, d, type)) << 32) | \
( 0x00000000FFFFFFFFULL & IDT_DESCRIPTOR_L(segment_sel, offset) ) )

#define x_IDT_X_GATE_DESCRIPTOR(offset, p, dpl, d, bits_10_8, segment_sel) \
(((uint64_t)x_IDT_DESCRIPTOR_H(offset, p, dpl, d, bits_10_8)) << 32) | \
((uint64_t)x_IDT_DESCRIPTOR_L(segment_sel, offset))
/******************************************************************************/
#define IDT_TASK_GATE_DESCRIPTOR(p, dpl, segment_sel) \
IDT_X_GATE_DESCRIPTOR(p, dpl, 0U, IDT_TASK_TYPE, segment_sel, 0x00000000U)


#define IDT_INTR_GATE_DESCRIPTOR(p, dpl, d, segment_sel, offset)      IDT_X_GATE_DESCRIPTOR(p, dpl, d, IDT_INTR_TYPE, segment_sel, offset)

#define IDT_TRAP_GATE_DESCRIPTOR(p, dpl, d, segment_sel, offset)      IDT_X_GATE_DESCRIPTOR(p, dpl, d, IDT_TRAP_TYPE, segment_sel, offset)

#define IDT_ENTRY_COUNT 34

#define INTR_VECTOR_RESERVED 0x00000000U
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

struct idt_reg_t {
    // @spec Intel SDM Vol.3.Chapter.6.10.
    uint16_t idt_limit; // sizeof(idt) - 1
    uint32_t idt_base_addr;
} __attribute__((packed));


#define V_N_HANDLER_FUNC(vn)          \
void v_##vn##_handler(void) {         \
    print(__func__);                  \
    print(" in Dijkstra I trust.\n"); \
}                                     \

V_N_HANDLER_FUNC(0)
V_N_HANDLER_FUNC(1)

#define V_N_HANDLER_FUNC_NAME(vn) v_##vn##_handler

#define TOKEN_TO_STR(tkn) #tkn

#define PRINT_BYTEH_STRUCT_MEMBER(sm) \
  do {                                \
    print(TOKEN_TO_STR(sm) " = ");    \
    print_uint32h(sm);                \
    print("\n");                      \
  } while (0)                         \

// void (*func_ptr)(unsigned char b, int pf);

uint64_t idt[IDT_ENTRY_COUNT]; // @TODO [ ] ".align 8? iSDM.Vol.3.Ch.6.11."

struct idt_reg_t idtr;

void init_idt(void) {


    // Fill IDT.
    //idt[0] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(0)) );
    idt[0] = intr_gate_d (((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(0)), SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT);
    //idt[1] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(1)) );
    idt[1] = intr_gate_d (((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(1)), SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT);

    idt[2] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(2)) );
    idt[3] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(3)) );
    idt[4] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(4)) );
    idt[5] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(5)) );
    idt[6] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(6)) );
    idt[7] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(7)) );
    idt[8] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(8)) );
    idt[9] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(9)) );
    idt[10] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(10)) );
    idt[11] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(11)) );
    idt[12] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(12)) );
    idt[13] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(13)) );
    idt[14] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(14)) );

    // RESERVED
    idt[15] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_VECTOR_RESERVED) );

    idt[16] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(16)) );
    idt[17] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(17)) );
    idt[18] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(18)) );
    idt[19] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(19)) );
    idt[20] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(20)) );
    idt[21] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(21)) );

    // RESERVED
    idt[22] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_VECTOR_RESERVED) );
    idt[23] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_VECTOR_RESERVED) );
    idt[24] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_VECTOR_RESERVED) );
    idt[25] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_VECTOR_RESERVED) );
    idt[26] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_VECTOR_RESERVED) );
    idt[27] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_VECTOR_RESERVED) );
    idt[28] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_VECTOR_RESERVED) );
    idt[29] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_VECTOR_RESERVED) );
    idt[30] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_VECTOR_RESERVED) );
    idt[31] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_VECTOR_RESERVED) );

    idt[32] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(32)) );
    idt[33] = IDT_INTR_GATE_DESCRIPTOR(SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS, GDT_CODE_SEGMENT, ((uint32_t)INTR_V_N_HANDLER_FUNC_NAME(33)) );

    // Load IDT register
    idtr.idt_limit = sizeof(idt) - 1;
    idtr.idt_base_addr = (uint32_t)idt;

    init_pics();
    lidt_and_sti((void *) &idtr);

    __asm__("int $0"); // Test IDT vector 0.
    __asm__("int $1");
}

void intr_handler(uint32_t vn, uint32_t err_code) {
    print("Vector Number = ");
    print_uint32h(vn);
    print("\n");
    print_uint32h(err_code);
    print("\n");
    print("Programming Notation. Not programming language.\n");
    if(vn == 33)
        print("THE KEYBOARD SAYS DIJKSTRA.\n");

}


