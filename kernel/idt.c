/*!
    @header
    @discussion
*/

#include "../drivers/screen.h"
#include "../include/stdint.h"
#include "test.h"

/*******************************************************************************
@doc [Figure 6-2. IDT Gate Descriptors](Intel 64 & IA-32 Arch. SDM Vol.3 Ch.6.11)

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
 | Seg. Select.     | Offset 15..0                     | Byte 0
 |-----------------------------------------------------|

 |------------------Trap Gate--------------------------|
 | 31            16 | 15 | 14 13 | 12  8 | 7 5 | 4   0 | Bit
 |------------------|----|-------|-------|-----|-------|
 | Offset 31..16    | P  | DPL   | 0D111 | 000 | Rsvd  | Byte 4
 |-----------------------------------------------------|
 |-----------------------------------------------------|
 | 31            16 | 15                             0 | Bit
 |------------------|----------------------------------|
 | Seg. Select.     | Offset 15..0                     | Byte 0
 |-----------------------------------------------------|


 DPL          := Descriptor Privilege Level
 Offset       := Offset to procedure entry point
 P            := Segment Present Flag
 Seg. Select. := Segment Selector for destination code segment.
                 TSS := Task-State Segment.
 D            := Size of gate: 1 = 32 bits; 0 = 16 bits
 Rsvd         := Reserved


@IMPORTANT
@doc [Flag Usage By Exception- or Interrupt-Handler Procedure]
     (Intel 64 & IA-32 Arch. SDM Vol.3 Ch.6.12.1.3)
The only difference between an interrupt gate and a trap gate is the way the
processor handles the IF flag in the EFLAGS register. When accessing an
exception- or interrupt-handling procedure through an interrupt gate, the
processor clears the IF flag to prevent other interrupts from interfering with
the current interrupt handler. A subsequent IRET instruction restores the IF
flag to its value in the saved contents of the EFLAGS register on the stack.
Accessing a handler procedure through a trap gate does not affect the IF flag.

*******************************************************************************/

/*!
    @struct    idt_gate_d_t

    @discussion A structure representing IDT gate descriptors. See Figure 6-2
    above. By setting the fields appropriately, this structure can represent any
    of the three gate descriptors. sizeof(struct idt_gate_d_t) == 8.

    @field    offset_l    Offset[bit 15:0]

    @field    seg_sel    Segment Selector for destination code segment

    @field    rsvd    Reserved

    @field    const0    Constant 000B

    @field    const1    Gate Descriptor [byte 7:4][bit 10:8].
                        One of: Task = 101B; Interrupt = 110B; Trap = 111B;

    @field    d    Size of gate: 1 = 32 bits; 0 = 16 bits

    @field    const2    Constant 0B

    @field    dpl    Descriptor Privilege Level

    @field    p    Segment Present Flag

    @field    offset_h    Offset[bit 31:16]
*/
struct idt_gate_d_t {

    uint16_t offset_l;
    uint16_t seg_sel;

    uint8_t rsvd:5;   // 00000B
    uint8_t const0:3; // 000B

/*!
    @defined TASK_GATE_CONST1
    @discussion Task Gate Descriptor [byte 7:4][bit 10:8].
*/
#define TASK_GATE_CONST1 5U
/*!
    @defined INTR_GATE_CONST1
    @discussion Interrupt Gate Descriptor [byte 7:4][bit 10:8].
*/
#define INTR_GATE_CONST1 6U
/*!
    @defined TRAP_GATE_CONST1
    @discussion Trap Gate Descriptor [byte 7:4][bit 10:8].
*/
#define TRAP_GATE_CONST1 7U
    uint8_t const1:3; // Gate Descriptor [byte 7:4][bit 10:8].
    uint8_t d:1;
    uint8_t const2:1; // 0B
    uint8_t dpl:2;
    uint8_t p:1;

    uint16_t offset_h;

} __attribute__((packed));

/*!
    @function    intr_gate_d

    @discussion Returns an interrupt gate descriptor.

    @param    offset    Offset to procedure entry point

    @param    p    Segment Present Flag

    @param    dpl    Descriptor Privilege Level

    @param    seg_sel    Segment Selector for destination code segment
*/
static inline uint64_t intr_gate_d (uint32_t offset, uint32_t p, uint32_t dpl,
                                    uint32_t d, uint16_t seg_sel) {
    struct idt_gate_d_t dt;

    dt.offset_h = offset >> 16;
    dt.p = p & 1U;
    dt.dpl = dpl & 3U;
    dt.const2 = 0;
    dt.d = d & 1U;
    dt.const1 = INTR_GATE_CONST1;
    dt.const0 = 0;
    dt.rsvd = 0;
    dt.seg_sel = seg_sel;
    dt.offset_l = offset;

    return *((uint64_t *) &dt);
}


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
/******************************************************************************/

/*!
    @defined IDT_LEN
    @discussion The length of the IDT array.
*/
#define IDT_LEN (34)

#define GDT_CODE_SEG (0x0008) // @TODO Import from gdt.asm.

#define GATE_SIZE_32_BITS 1
#define GATE_SIZE_16_BITS 0

#define DPL_0 0
#define DPL_KERNEL 0

#define DPL_3 3
#define DPL_USER 3

// @spec Intel SDM Vol.3.Chapter.3.4.5.
#define SEGMENT_PRESENT 1
#define SEGMENT_NOT_PRESENT 0

uint64_t idt[IDT_LEN]; // @IMPORTANT @TODO [ ] ".align 8? iSDM.Vol.3.Ch.6.11."

typedef void (*idt_proc_t)(void);

idt_proc_t idt_proc_entry_p[IDT_LEN] = {
    INTR_V_N_HANDLER_FUNC_NAME(0),
    INTR_V_N_HANDLER_FUNC_NAME(1),
    INTR_V_N_HANDLER_FUNC_NAME(2),
    INTR_V_N_HANDLER_FUNC_NAME(3),
    INTR_V_N_HANDLER_FUNC_NAME(4),
    INTR_V_N_HANDLER_FUNC_NAME(5),
    INTR_V_N_HANDLER_FUNC_NAME(6),
    INTR_V_N_HANDLER_FUNC_NAME(7),
    INTR_V_N_HANDLER_FUNC_NAME(8),
    INTR_V_N_HANDLER_FUNC_NAME(9),
    INTR_V_N_HANDLER_FUNC_NAME(10),
    INTR_V_N_HANDLER_FUNC_NAME(11),
    INTR_V_N_HANDLER_FUNC_NAME(12),
    INTR_V_N_HANDLER_FUNC_NAME(13),
    INTR_V_N_HANDLER_FUNC_NAME(14),
    0, // 15 - Intel Reserved.
    INTR_V_N_HANDLER_FUNC_NAME(16),
    INTR_V_N_HANDLER_FUNC_NAME(17),
    INTR_V_N_HANDLER_FUNC_NAME(18),
    INTR_V_N_HANDLER_FUNC_NAME(19),
    INTR_V_N_HANDLER_FUNC_NAME(20),
    INTR_V_N_HANDLER_FUNC_NAME(21),
    0, // 22 - Intel Reserved.
    0, // 23
    0, // 24
    0, // 25
    0, // 26
    0, // 27
    0, // 28
    0, // 29
    0, // 30
    0, // 31
    INTR_V_N_HANDLER_FUNC_NAME(32), // 32-255 - User Defined Interrupts
    INTR_V_N_HANDLER_FUNC_NAME(33)
};

/*!
    @struct idt_reg_t

    @discussion Struct representing value loaded into the IDT register (IDTR).

    @doc [Figure 6-1. Relationship of the IDTR and IDT]
         (Intel 64 & IA-32 Arch. SDM Vol.3 Ch.6.10)
*/
struct idt_reg_t {
    uint16_t idt_limit;
    uint32_t idt_base_addr;
} __attribute__((packed));

struct idt_reg_t idtr;

/*!
    @defined IDT_RSVD_VECT(v)
    @discussion Returns true if v is an Intel reserved vector number.
    @doc [Table 6-1. Protected-Mode Exceptions and Interrupts]
         (Intel 64 & IA-32 Arch. SDM Vol.3 Ch.6.2)
*/
#define IDT_RSVD_VECT(v) ((v) == 15 || ((v) >= 22 && (v) <= 31))

void init_idt(void) {


    // Fill IDT.
    for (int v = 0; v < IDT_LEN; v++) {
        if (!IDT_RSVD_VECT(v))
            idt[v] = intr_gate_d ((uint32_t)idt_proc_entry_p[v],
                                  SEGMENT_PRESENT, DPL_0, GATE_SIZE_32_BITS,
                                  GDT_CODE_SEG);
    }

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