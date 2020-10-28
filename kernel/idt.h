#ifndef __IDT_H__
#define __IDT_H__

#define IDT_TASK_TYPE (0x00000500)
#define IDT_INTR_TYPE (0x00000600)
#define IDT_TRAP_TYPE (0x00000700)

#define IDT_DESCRIPTOR_H(offset, p, dpl, d, type)                     ( (offset & 0xFFFF0000U) | (type) | ( (p & 0x00000001U) << 15 ) | ( (dpl & 0x00000003U) << 13 ) | ( (d & 0x00000001U) << 11 ) )
#define IDT_DESCRIPTOR_L(segment_sel, offset)                         ( ((segment_sel & 0x0000FFFFU) << 16) | (offset & 0x0000FFFFU) )

#define IDT_X_GATE_DESCRIPTOR(p, dpl, d, type, segment_sel, offset)   ( ( ( 0x00000000FFFFFFFFULL & IDT_DESCRIPTOR_H(offset, p, dpl, d, type) ) << 32) | ( 0x00000000FFFFFFFFULL & IDT_DESCRIPTOR_L(segment_sel, offset) ) )

#define IDT_TASK_GATE_DESCRIPTOR(p, dpl, segment_sel)                 IDT_X_GATE_DESCRIPTOR(p, dpl, 0U, IDT_TASK_TYPE, segment_sel, 0x00000000U)

#define IDT_INTR_GATE_DESCRIPTOR(p, dpl, d, segment_sel, offset)      IDT_X_GATE_DESCRIPTOR(p, dpl, d, IDT_INTR_TYPE, segment_sel, offset)

#define IDT_TRAP_GATE_DESCRIPTOR(p, dpl, d, segment_sel, offset)      IDT_X_GATE_DESCRIPTOR(p, dpl, d, IDT_TRAP_TYPE, segment_sel, offset)



#define IDT_ENTRY_COUNT 34


#endif