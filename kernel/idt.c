/*!
    @header
    @discussion
*/

#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../include/stdint.h"
#include "../include/assert.h"
#include "idt_asm.h"
#include "i8259a_pic.h"
#include "low_level.h"


/*******************************************************************************
@doc [Figure 6-2. IDT Gate Descriptors]
     (Intel 64 & IA-32 Arch. SDM Vol.3 Ch.6.11)

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

@remark
For the relationship between GDT entries and IDT entries see
@doc [Table 3-2. System-Segment and Gate-Descriptor Types]
     (Intel 64 & IA-32 Arch. SDM Vol.3 Ch.3.5)
For more info. on the meaning of fields inside descriptors in general, e.g. DPL
and P, see
@doc [Segment Descriptors](Intel 64 & IA-32 Arch. SDM Vol.3 Ch.3.4.5)
*******************************************************************************/

/*!
    @struct    idt_gate_d_t

    @discussion A structure representing IDT gate descriptors. See Figure 6-2
    above. By setting the fields appropriately, this structure can represent any
    of the three gate descriptors. sizeof(struct idt_gate_d_t) == 8.

    @field    offset_l    Offset[bit 15:0]
    @field    seg_sel     Segment Selector for destination code segment
    @field    rsvd        Reserved
    @field    const0      Constant 000B
    @field    const1      Gate Descriptor [byte 7:4][bit 10:8].
                          One of: Task = 101B; Interrupt = 110B; Trap = 111B;
    @field    d           Size of gate: 1 = 32 bits; 0 = 16 bits
    @field    const2      Constant 0B
    @field    dpl         Descriptor Privilege Level
    @field    p           Segment Present Flag
    @field    offset_h    Offset[bit 31:16]
*/
struct idt_gate_d_t {
    uint16_t offset_l;
    uint16_t seg_sel;
    uint8_t rsvd:5;   // 00000B
    uint8_t const0:3; // 000B
    uint8_t const1:3; // Gate Descriptor [byte 7:4][bit 10:8].
    uint8_t d:1;
    uint8_t const2:1; // 0B
    uint8_t dpl:2;
    uint8_t p:1;
    uint16_t offset_h;
} __attribute__((packed));

/*!
    @defined    SEG_PRESENT
    @discussion Value of the P field in an IDT gate descriptor. Indicates the
    segment is present in memory.
*/
#define SEG_PRESENT (1)

/*!
    @defined    SEG_NOT_PRESENT
    @discussion Value of the P field in an IDT gate descriptor. Indicates the
    segment is **not** present in memory.
*/
#define SEG_NOT_PRESENT (0)

/*!
    @defined    DPL_0
    @discussion Value of the DPL field in a gate descriptor. Most privileged.
*/
#define DPL_0 (0)

/*!
    @defined    DPL_KERNEL
    @discussion Alias for DPL_0.
*/
#define DPL_KERNEL DPL_0

/*!
    @defined    DPL_3
    @discussion Value of the DPL field in a gate descriptor. Least privileged.
*/
#define DPL_3 (3)

/*!
    @defined    DPL_USER
    @discussion Alias for DPL_3.
*/
#define DPL_USER DPL_3

/*!
    @defined    GATE_SIZE_32
    @discussion Value of the D field in interrupt and trap gate descriptors.
    Size of gate 32-bits.
*/
#define GATE_SIZE_32 (1)

/*!
    @defined    GATE_SIZE_16
    @discussion Value of the D field in interrupt and trap gate descriptors.
    Size of gate 16-bits.
*/
#define GATE_SIZE_16 (0)

/*!
    @defined    TASK_GATE_CONST1
    @discussion Value of Task Gate Descriptor [byte 7:4][bit 10:8].
*/
#define TASK_GATE_CONST1 (5U)

/*!
    @defined    INTR_GATE_CONST1
    @discussion Value of Interrupt Gate Descriptor [byte 7:4][bit 10:8].
*/
#define INTR_GATE_CONST1 (6U)

/*!
    @defined    TRAP_GATE_CONST1
    @discussion Value of Trap Gate Descriptor [byte 7:4][bit 10:8].
*/
#define TRAP_GATE_CONST1 (7U)

/*! @TODO Import from gdt.asm.
    @defined    CODE_SEG
    @discussion Value of the segment selector field in an interrupt of trap gate
    descriptor. Equal to the byte offset of the code segment in the GDT.
*/
#define CODE_SEG (0x0008)

/*!
    @function    intr_gate_d

    @discussion Returns an interrupt gate descriptor.

    @param    offset     Offset to procedure entry point
    @param    p          Segment Present Flag
    @param    dpl        Descriptor Privilege Level
    @param    d          Size of gate: 1 = 32 bits; 0 = 16 bits
    @param    seg_sel    Segment Selector for destination code segment
*/
static inline uint64_t intr_gate_d(uint32_t offset, uint32_t p, uint32_t dpl,
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

/*!
    @defined    IDT_LEN
    @discussion The length of the IDT array.
*/
#define IDT_LEN (34)

/*
    @IMPORTANT The base addresses of the IDT should be aligned on an 8-byte
    boundary to maximize performance of cache line fills.

    @doc [INTERRUPT DESCRIPTOR TABLE (IDT)]
         (Intel 64 & IA-32 Arch. SDM Vol.3 Ch.6.10)
*/
uint64_t idt[IDT_LEN] __attribute__((aligned (8) ));

void vn_not_handled(uint32_t vn, uint32_t err_code) {
    if (vn || err_code) { // Suppress warning.
        ;
    }
    print("This interrupt is not handled.\n");
    assert(0);
}

void v33_handler(uint32_t vn, uint32_t err_code) {
    uint8_t sc;
    char c;

    if (vn || err_code || c) { // Suppress warning.
        ;
    }

    sc = inb (0x0060); // Read keyboard output buffer.
    pic_eoi(vn);
    print_x32(sc);
    print("\n");
    if((sc & 0x80) == 0) { // if its not a released scan code.
        //c = scan_code_to_ascii (sc);
        //print_ch_at(c, 0, -1, -1);
    }
    //print("again, THE KEYBOARD SAYS DIJKSTRA.\n");
}

/*!
    @const    idt_handlers
    @discussion Array of interrupt/exception procedure entry points and vector
    specific handlers. The first member is used as value for the offset field in
    interrupt and trap gate descriptors while the second member is the function
    that is called by the common interrupt/exception handler.
*/
const idt_handler_t idt_handlers[IDT_LEN] = {
    {INTR_VN_HANDLER(0), vn_not_handled},
    {INTR_VN_HANDLER(1), vn_not_handled},
    {INTR_VN_HANDLER(2), vn_not_handled},
    {INTR_VN_HANDLER(3), vn_not_handled},
    {INTR_VN_HANDLER(4), vn_not_handled},
    {INTR_VN_HANDLER(5), vn_not_handled},
    {INTR_VN_HANDLER(6), vn_not_handled},
    {INTR_VN_HANDLER(7), vn_not_handled},
    {INTR_VN_HANDLER(8), vn_not_handled},
    {INTR_VN_HANDLER(9), vn_not_handled},
    {INTR_VN_HANDLER(10), vn_not_handled},
    {INTR_VN_HANDLER(11), vn_not_handled},
    {INTR_VN_HANDLER(12), vn_not_handled},
    {INTR_VN_HANDLER(13), vn_not_handled},
    {INTR_VN_HANDLER(14), vn_not_handled},
    {0, 0}, // 15 - Intel Reserved.
    {INTR_VN_HANDLER(16), vn_not_handled},
    {INTR_VN_HANDLER(17), vn_not_handled},
    {INTR_VN_HANDLER(18), vn_not_handled},
    {INTR_VN_HANDLER(19), vn_not_handled},
    {INTR_VN_HANDLER(20), vn_not_handled},
    {INTR_VN_HANDLER(21), vn_not_handled},
    {0, 0}, // 22 - Intel Reserved.
    {0, 0}, // 23
    {0, 0}, // 24
    {0, 0}, // 25
    {0, 0}, // 26
    {0, 0}, // 27
    {0, 0}, // 28
    {0, 0}, // 29
    {0, 0}, // 30
    {0, 0}, // 31
    {INTR_VN_HANDLER(32), vn_not_handled}, // 32-255 - User Defined Interrupts
    {INTR_VN_HANDLER(33), v33_handler}
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

/*!
    @defined IDT_RSVD_VECT(v)
    @discussion Returns true if v is an Intel reserved vector number.
    @doc [Table 6-1. Protected-Mode Exceptions and Interrupts]
         (Intel 64 & IA-32 Arch. SDM Vol.3 Ch.6.2)
*/
#define IDT_RSVD_VECT(v) ((v) == 15 || ((v) >= 22 && (v) <= 31))

/*!
    @struct    intr_err_code_t
    @discussion When an exception condition is related to a specific segment
    selector or IDT vector, the processor pushes an error code onto the stack of
    the exception handler (whether it is a procedure or task).
    @doc [Figure 6-7. Error Code](Intel 64 & IA-32 Arch. SDM Vol.3 Ch.6.13)

    @field    ext            When set, indicates that the exception occurred
                             during delivery of an event external to the
                             program, such as an interrupt or an earlier
                             exception.
    @field    idt            When set, indicates that the index portion of the
                             error code refers to a gate descriptor in the IDT;
                             when clear, indicates that the index refers to a
                             descriptor in the GDT or the current LDT.
    @field    ti             Only used when the IDT flag is clear. When set, the
                             TI flag indicates that the index portion of the
                             error code refers to a segment or gate descriptor
                             in the LDT; when clear, it indicates that the index
                             refers to a descriptor in the current GDT.
    @field    seg_sel_idx    The segment selector index field provides an index
                             into the IDT, GDT, or current LDT to the segment or
                             gate selector being referenced by the error code.
    @field    rsvd           Reserved.
*/
struct intr_err_code_t {
    uint16_t ext:1;
    uint16_t idt:1;
    uint16_t ti:1;
    uint16_t seg_sel_idx:13; // @IMPORTANT Pretty sure this should be shifted left by 3-bits to produce a byte offset from a entry index into the IDT/GDT/LDT.
    uint16_t rsvd;
} __attribute__((packed));

/*!
    @function    intr_handler
    @discussion Every interrupt handler procedure in the IDT calls this
    function. The interrupt handler procedures are defined in the .s file. The
    interrupt handler procedures all call this function with its vector number,
    which identifies the source of the interrupt, and an error code if
    applicable. Error codes provide additional information about the source of
    the interrupt. Interrupt vectors that do not provide an error code set the
    error code to 0.
    @doc [Table 6-1. Protected-Mode Exceptions and Interrupts]
         (Intel 64 & IA-32 Arch. SDM Vol.3 Ch.6.2)
    @param    vn          Interrupt vector number. Identifies the source of the
                          interrupt.
    @param    err_code    Error code value for the interrupt. If no error code
                          applies, it is set to 0.
*/
void intr_handler(uint32_t vn, uint32_t err_code) {
#if 0
    struct intr_err_code_t *errc;

    print("Vector Number = ");
    print_d(vn);
    print("\n");

    errc = (struct intr_err_code_t *) &err_code;
    print("errc.ext = ");
    print_x32(errc->ext);
    print("\n");

    print("errc.idt = ");
    print_x32(errc->idt);
    print("\n");

    print("errc.ti = ");
    print_x32(errc->ti);
    print("\n");

    print("errc.seg_sel_idx = ");
    print_d(errc->seg_sel_idx);
    print("\n");
#endif

    // Call the specific interrupt/exception handler.
    idt_handlers[vn].vn_handler(vn, err_code);
}

/*!
    @function    init_interrupts
    @discussion Performs the work necessary to enable interrupts.
*/
void init_interrupts(void) {
    struct idt_reg_t idtr;

    // Fill IDT.
    for (int v = 0; v < IDT_LEN; v++) {
        if (!IDT_RSVD_VECT(v))
            idt[v] = intr_gate_d((uint32_t) idt_handlers[v].idt_proc,
                                 SEG_PRESENT, DPL_0, GATE_SIZE_32,
                                 CODE_SEG);
    }

    // Init. IDT register
    idtr.idt_limit = sizeof(idt) - 1;
    idtr.idt_base_addr = (uint32_t)idt;

    // Initialize programmable interrupt controllers (PIC).
    init_pics(); // @IMPORTANT This should be called before lidt_and_sti().

    // Load the IDT register and enable interrupts.
    lidt_and_sti((void *) &idtr);
}