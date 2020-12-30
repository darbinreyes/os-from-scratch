/*!
    @header Initialization and configuration of the Intel 8259A PIC.
    The Intel 8259A Programmable Interrupt Controller (PIC) is one of the chips
    that can be used for delivering interrupts to an Intel CPU.

    @IMPORTANT
    * There is no use of "io_wait()"" here.
    * "Spurious IRQs" are not handled.
    See @doc [PIC](https://wiki.osdev.org/PIC).
*/

#include "low_level.h"
#include "../include/mylibc.h"
#include "../include/assert.h"

/*!
    @defined IO_MASTER_PIC_PORT_A
    @discussion Port A of the master PIC. A.K.A. The command port.
*/
#define IO_MASTER_PIC_PORT_A (0x20)

/*!
    @defined IO_MASTER_PIC_PORT_B
    @discussion Port B of the master PIC. A.K.A. The data port.
*/
#define IO_MASTER_PIC_PORT_B (0x21)

/*!
    @defined IO_SLAVE_PIC_PORT_A
    @discussion Port A of the slave PIC.  A.K.A. The command port.
*/
#define IO_SLAVE_PIC_PORT_A (0xA0)

/*!
    @defined IO_SLAVE_PIC_PORT_B
    @discussion Port B of the slave PIC. A.K.A. The data port.
*/
#define IO_SLAVE_PIC_PORT_B (0xA1)

/*!
    @struct pic_icw1_t
    @discussion Structure for the ICW1.

    @field icw4      0 = No ICW4, 1 = ICW4 will be sent. (ICW4 is optional).
    @field ms        0 = master/slave configuration, 1 = master only.
    @field const0    Constant value set to 0B.
    @field trig      0 = Edge triggered, 1 = level triggered.
    @field const1    Constant value set to 0001B.

    Recommended value: 0001_0001B = 11H.
*/
struct pic_icw1_t {
    uint8_t icw4:1;
    uint8_t ms:1;
    uint8_t const0:1; // 0B
    uint8_t trig:1;
    uint8_t const1:4; // 0001B
} __attribute__((packed));

/*! See @struct pic_icw1_t */
static inline uint8_t icw1(int icw4, int ms, int trig) {
    struct pic_icw1_t w;
    uint8_t *p;

    w.icw4 = icw4;
    w.ms = ms;
    w.const0 = 0;
    w.trig = trig;
    w.const1 = 1;

    p = (uint8_t *) &w;

    return *p;
}

/*!
    @typedef pic_icw2_t
    @discussion Alias for uint8_t. ICW2 is simply the vector number of the IDT
    entry that should be tied to IRQ 0. The typical setting is 32 = 20H for the
    master and 32 + 8 = 40 = 28H for the slave. The value must be a multiple of
    8.

    Recommended value:
    Master: 32 = 20H
    Slave:  40 = 28H
*/
typedef uint8_t pic_icw2_t;

/*!
    @typedef m_pic_icw3_t

    @discussion Alias for uint8_t. The meaning of ICW3 differs for the master
    vs. the slave PIC. This typedef is for the master PIC.

    Its meaning is to indicate which IRQ pin is connected to the slave PIC. Bits
    7:0 correspond to IRQ 7-0.

    Bit n being set (1) indicates IRQ n is connected to the slave PIC,
    bit n being clear (0) indicates IRQ n is connected to a peripheral device.

    Recommended value: 0000_0100B = 4H.
*/
typedef uint8_t m_pic_icw3_t;

/*!
    @typedef s_pic_icw3_t

    @discussion Alias for uint8_t. The meaning of ICW3 differs for the master
    vs. the slave PIC. This typedef is for the slave PIC.

    Its meaning is to indicate which IRQ # is connected to the master PIC. Its
    value should be 0-7, hence the 5 high order bits should always be 0.

    Recommended value: 0000_0010B = 2H.
*/
typedef uint8_t s_pic_icw3_t;

/*!
    @struct pic_icw4_t
    @discussion Structure for the ICW4. It is optional, this ICW should only be
    sent if it was indicated by ICW1.

    @field mode   0 = MCS-80/85 mode. 1 = 8086/88 mode.
    @field a_eoi  0 = Manual end of interrupt (EOI). 1 = Automatic EOI.
    @field ms     0 = Slave PIC. 1 = Master PIC.
    @field buf    0 = No buffered mode. 1 = Buffered mode.
    @field sfnm   0 = No special fully nested mode. 1 = Special fully nested.
                  mode.
    @field const0 Constant value set to 000B.

    Recommended value:
    Master: 0000_0101B = 5H.
    Slave:  0000_0001B = 1H.
*/
struct pic_icw4_t {
    uint8_t mode:1;
    uint8_t a_eoi:1;
    uint8_t ms:1;
    uint8_t buf:1;
    uint8_t sfnm:1;
    uint8_t const0:3; // 000B
} __attribute__((packed));

/*! See @struct pic_icw4_t */
static inline uint8_t icw4(int mode, int a_eoi, int ms, int buf, int sfnm) {
    struct pic_icw4_t w;
    uint8_t *p;

    w.mode = mode;
    w.a_eoi = a_eoi;
    w.ms = ms;
    w.buf = buf;
    w.sfnm = sfnm;
    w.const0 = 0;

    p = (uint8_t *) &w;

    return *p;
}

/*!
    @function init_pics

    @discussion Configures the master and slave PICs for typical usage
    scenarios.

    Each 8259A is configured using two ports. For convenience, the first PIC
    port we call "port A", the second "port B", see above. Configuration is done
    by writing 4 "interrupt control words" (ICWs) to the ports. In this context
    a "word" is 8-bits. The first ICW is written to port A, and the remaining 3
    ICWs are written to port B. For convenience, we call the ICWs: ICW1, ICW2,
    ICW3, ICW4.

*/
void init_pics(void) {
    pic_icw2_t icw2;
    m_pic_icw3_t m_icw3;
    s_pic_icw3_t s_icw3;

    // ICW1 - General PIC behavior
    outb(IO_MASTER_PIC_PORT_A, icw1(1, 0, 0));
    // @IMPORTANT io_wait() may be necessary on older machines.
    outb(IO_SLAVE_PIC_PORT_A, icw1(1, 0, 0));
    // ICW2 - User defined interrupt offset into IDT.
    icw2 = 32;
    outb(IO_MASTER_PIC_PORT_B, icw2);
    icw2 = 40;
    outb(IO_SLAVE_PIC_PORT_B, icw2);
    // ICW3 - How master and slave are connected.
    m_icw3 = BIT2;
    outb(IO_MASTER_PIC_PORT_B, m_icw3);
    s_icw3 = 2;
    outb(IO_SLAVE_PIC_PORT_B, s_icw3);
    // ICW4 - Other PIC behavior
    outb(IO_MASTER_PIC_PORT_B, icw4(1, 0, 1, 0, 0));
    outb(IO_SLAVE_PIC_PORT_B, icw4(1, 0, 0, 0, 0));

    // Interrupt mask - 0 means listen, 1 means ignore.
    /*!
        @discussion When the PIC isn't expecting any other configuration bytes,
        it is in "normal operating mode". In this mode, we can tell the PIC to
        ignore specified IRQs, tell it to "mask off" specified interrupts. This
        is achieved by writing a single byte to port B of the corresponding PIC.
        In this byte 0 = listen, 1 = ignore. Reading from port B returns the
        current mask value.
    */
    outb(IO_MASTER_PIC_PORT_B, (uint8_t)(~(BIT1)));
    outb(IO_SLAVE_PIC_PORT_B, (uint8_t)(~(0U)));
}

/*!
    @defined PIC_EOI

    @discussion When the 8259A is in "manual end of interrupt (EOI) mode"
    (ICW4[bit 1] = 0), it will wait for an EOI acknowledgment to be written to
    it before it will raise further interrupts. The EOI is indicated by writing
    the special value 0x20 to port A of the PIC.
*/
#define PIC_EOI 0x20

/*!
    @function pic_eoi
    @discussion Write EOI byte to the PIC that raised the interrupt.

    @IMPORTANT If the IRQ came from the Master PIC, it is sufficient to issue
    EOI only to the Master PIC; however if the IRQ came from the Slave PIC, it
    is necessary to issue EOI to **both** PIC chips.
*/
void pic_eoi(uint32_t vn) {
    vn -= 32; // IRQ # from vector #.
    if (vn < 8) {
        outb(IO_MASTER_PIC_PORT_A, PIC_EOI);
    } else if (vn < 16) {
        outb(IO_SLAVE_PIC_PORT_A, PIC_EOI);
        outb(IO_MASTER_PIC_PORT_A, PIC_EOI);
    } else {
        assert(0);
    }
}