#include <asm/io.h>
#include <asm/regs.h>
#include <asm/system.h>
#include <cnix/head.h>

#define IRQ0		0x20

#define INT1_CTL	0x20
#define INT1_MASK	0x21
#define INT2_CTL	0xa0
#define INT2_MASK	0xa1
#define ENABLE_INT	0x20

#define IRQ_NUM		16

#define cli() __asm__("cli"::)
#define sti() __asm__("sti"::)

extern void hw_int0(void);
extern void hw_int1(void);
extern void hw_int2(void);
extern void hw_int3(void);
extern void hw_int4(void);
extern void hw_int5(void);
extern void hw_int6(void);
extern void hw_int7(void);
extern void hw_int8(void);
extern void hw_int9(void);
extern void hw_int10(void);
extern void hw_int11(void);
extern void hw_int12(void);
extern void hw_int13(void);
extern void hw_int14(void);
extern void hw_int15(void);

extern void printk(char * fmt, ...);

typedef void (*fn_t)(void);

static fn_t irq_table[IRQ_NUM];

static fn_t intr_gate[IRQ_NUM] = {
	hw_int0, hw_int1, hw_int2, hw_int3, hw_int4, hw_int5, hw_int6, hw_int7,
	hw_int8, hw_int9, hw_int10, hw_int11, hw_int12, hw_int13, hw_int14, 
	hw_int15
};

static void disable_irq(int irq)
{
	unsigned char value;
	unsigned int port;
	
	port = irq < 8 ? INT1_MASK : INT2_MASK;

	value = inb(port) | (1 << irq);
	outb(value, port);
}

static void enable_irq(int irq)
{
	unsigned char value;
	unsigned int port;

	port = irq < 8 ? INT1_MASK : INT2_MASK;

	value = inb(port) & (~(1 << irq));
	outb(value, port);
}

static void ack_irq(int irq)
{
	outb(ENABLE_INT, INT1_CTL);
	/* if will delay. */
	if(irq > 8)
		outb(ENABLE_INT, INT2_CTL);
}

static void default_handler(int irq)
{
	printk("unexpected irq: %d\n", irq);
}

void intr_init(void)
{
	int i;

	for(i = 0; i < IRQ_NUM; i++)
		irq_table[i] = (fn_t)default_handler;

	for(i = 0; i < IRQ_NUM; i++)
		set_intr_gate(IRQ0 + i, intr_gate[i]);	
}

int put_irq_handler(int irq, fn_t fn)
{
	if(irq < 0 || irq >= IRQ_NUM)
		panic("invalid irq %d\n", irq);
	
	if(irq_table[irq] == fn)
		return 0;

	if(irq_table[irq] != (fn_t)default_handler)
		panic("have set one handler on irq %d\n", irq);
	
	disable_irq(irq);
	irq_table[irq] = fn;
	enable_irq(irq);

	return 0;
}

void do_with_irq(struct regs_t regs)
{
	disable_irq(regs.index);
	ack_irq(regs.index);
	sti();
	irq_table[regs.index]();
	cli();
	enable_irq(regs.index);
}	
