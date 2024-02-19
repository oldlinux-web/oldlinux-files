/*
 *	linux/kernel/irq.c
 *
 *	Copyright (C) 1992 Linus Torvalds
 *
 * This file contains the code used by various IRQ handling routines:
 * asking for different IRQ's should be done through these routines
 * instead of just grabbing them. Thus setups with different IRQ numbers
 * shouldn't result in any weird surprises, and installing new handlers
 * should be easier.
 */

/*
 * IRQ's are in fact implemented a bit like signal handlers for the kernel.
 * The same sigaction struct is used, and with similar semantics (ie there
 * is a SA_INTERRUPT flag etc). Naturally it's not a 1:1 relation, but there
 * are similarities.
 *
 * sa_handler(int irq_NR) is the default function called.
 * sa_mask is 0 if nothing uses this IRQ
 * sa_flags contains various info: SA_INTERRUPT etc
 * sa_restorer is the unused
 */

#include <linux/ptrace.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>

void irq13(void);

/*
 * This builds up the IRQ handler stubs using some ugly macros in irq.h
 *
 * These macros create the low-level assembly IRQ routines that do all
 * the operations that are needed to keep the AT interrupt-controller
 * happy. They are also written to be fast - and to disable interrupts
 * as little as humanly possible.
 *
 * NOTE! These macros expand to three different handlers for each line: one
 * complete handler that does all the fancy stuff (including signal handling),
 * and one fast handler that is meant for simple IRQ's that want to be
 * atomic. The specific handler is chosen depending on the SA_INTERRUPT
 * flag when installing a handler. Finally, one "bad interrupt" handler, that
 * is used when no handler is present.
 */
BUILD_IRQ(FIRST,0,0x01)
BUILD_IRQ(FIRST,1,0x02)
BUILD_IRQ(FIRST,2,0x04)
BUILD_IRQ(FIRST,3,0x08)
BUILD_IRQ(FIRST,4,0x10)
BUILD_IRQ(FIRST,5,0x20)
BUILD_IRQ(FIRST,6,0x40)
BUILD_IRQ(FIRST,7,0x80)
BUILD_IRQ(SECOND,8,0x01)
BUILD_IRQ(SECOND,9,0x02)
BUILD_IRQ(SECOND,10,0x04)
BUILD_IRQ(SECOND,11,0x08)
BUILD_IRQ(SECOND,12,0x10)
BUILD_IRQ(SECOND,13,0x20)
BUILD_IRQ(SECOND,14,0x40)
BUILD_IRQ(SECOND,15,0x80)

/*
 * Pointers to the low-level handlers: first the general ones, then the
 * fast ones, then the bad ones.
 */
static void (*interrupt[16])(void) = {
	IRQ0_interrupt, IRQ1_interrupt, IRQ2_interrupt, IRQ3_interrupt,
	IRQ4_interrupt, IRQ5_interrupt, IRQ6_interrupt, IRQ7_interrupt,
	IRQ8_interrupt, IRQ9_interrupt, IRQ10_interrupt, IRQ11_interrupt,
	IRQ12_interrupt, IRQ13_interrupt, IRQ14_interrupt, IRQ15_interrupt
};

static void (*fast_interrupt[16])(void) = {
	fast_IRQ0_interrupt, fast_IRQ1_interrupt,
	fast_IRQ2_interrupt, fast_IRQ3_interrupt,
	fast_IRQ4_interrupt, fast_IRQ5_interrupt,
	fast_IRQ6_interrupt, fast_IRQ7_interrupt,
	fast_IRQ8_interrupt, fast_IRQ9_interrupt,
	fast_IRQ10_interrupt, fast_IRQ11_interrupt,
	fast_IRQ12_interrupt, fast_IRQ13_interrupt,
	fast_IRQ14_interrupt, fast_IRQ15_interrupt
};

static void (*bad_interrupt[16])(void) = {
	bad_IRQ0_interrupt, bad_IRQ1_interrupt,
	bad_IRQ2_interrupt, bad_IRQ3_interrupt,
	bad_IRQ4_interrupt, bad_IRQ5_interrupt,
	bad_IRQ6_interrupt, bad_IRQ7_interrupt,
	bad_IRQ8_interrupt, bad_IRQ9_interrupt,
	bad_IRQ10_interrupt, bad_IRQ11_interrupt,
	bad_IRQ12_interrupt, bad_IRQ13_interrupt,
	bad_IRQ14_interrupt, bad_IRQ15_interrupt
};

/*
 * Initial irq handlers.
 */
static struct sigaction irq_sigaction[16] = {
	{ NULL, 0, 0, NULL }, { NULL, 0, 0, NULL },
	{ NULL, 0, 0, NULL }, { NULL, 0, 0, NULL },
	{ NULL, 0, 0, NULL }, { NULL, 0, 0, NULL },
	{ NULL, 0, 0, NULL }, { NULL, 0, 0, NULL },
	{ NULL, 0, 0, NULL }, { NULL, 0, 0, NULL },
	{ NULL, 0, 0, NULL }, { NULL, 0, 0, NULL },
	{ NULL, 0, 0, NULL }, { NULL, 0, 0, NULL },
	{ NULL, 0, 0, NULL }, { NULL, 0, 0, NULL }
};

/*
 * do_IRQ handles IRQ's that have been installed without the
 * SA_INTERRUPT flag: it uses the full signal-handling return
 * and runs with other interrupts enabled. All relatively slow
 * IRQ's should use this format: notably the keyboard/timer
 * routines.
 */
int do_IRQ(int irq, struct pt_regs * regs)
{
	struct sigaction * sa = irq + irq_sigaction;

	sa->sa_handler((int) regs);
	return 0;		/* re-enable the irq when returning */
}

/*
 * do_fast_IRQ handles IRQ's that don't need the fancy interrupt return
 * stuff - the handler is also running with interrupts disabled unless
 * it explicitly enables them later.
 */
int do_fast_IRQ(int irq)
{
	struct sigaction * sa = irq + irq_sigaction;

	sa->sa_handler(irq);
	return 0;		/* re-enable the irq when returning */
}

int irqaction(unsigned int irq, struct sigaction * new)
{
	struct sigaction * sa;
	unsigned long flags;

	if (irq > 15)
		return -EINVAL;
	sa = irq + irq_sigaction;
	if (sa->sa_mask)
		return -EBUSY;
	if (!new->sa_handler)
		return -EINVAL;
	save_flags(flags);
	cli();
	*sa = *new;
	sa->sa_mask = 1;
	if (sa->sa_flags & SA_INTERRUPT)
		set_intr_gate(0x20+irq,fast_interrupt[irq]);
	else
		set_intr_gate(0x20+irq,interrupt[irq]);
	if (irq < 8)
		outb(inb_p(0x21) & ~(1<<irq),0x21);
	else {
		outb(inb_p(0x21) & ~(1<<2),0x21);
		outb(inb_p(0xA1) & ~(1<<(irq-8)),0xA1);
	}
	restore_flags(flags);
	return 0;
}
		
int request_irq(unsigned int irq, void (*handler)(int))
{
	struct sigaction sa;

	sa.sa_handler = handler;
	sa.sa_flags = 0;
	sa.sa_mask = 0;
	sa.sa_restorer = NULL;
	return irqaction(irq,&sa);
}

void free_irq(unsigned int irq)
{
	struct sigaction * sa = irq + irq_sigaction;
	unsigned long flags;

	if (irq > 15) {
		printk("Trying to free IRQ%d\n",irq);
		return;
	}
	if (!sa->sa_mask) {
		printk("Trying to free free IRQ%d\n",irq);
		return;
	}
	save_flags(flags);
	cli();
	if (irq < 8)
		outb(inb_p(0x21) | (1<<irq),0x21);
	else
		outb(inb_p(0xA1) | (1<<(irq-8)),0xA1);
	set_intr_gate(0x20+irq,bad_interrupt[irq]);
	sa->sa_handler = NULL;
	sa->sa_flags = 0;
	sa->sa_mask = 0;
	sa->sa_restorer = NULL;
	restore_flags(flags);
}

extern void do_coprocessor_error(long,long);

static void math_error_irq(int cpl)
{
	outb(0,0xF0);
	do_coprocessor_error(0,0);
}

static void no_action(int cpl) { }

static struct sigaction ignore_IRQ = {
	no_action,
	0,
	SA_INTERRUPT,
	NULL
};

void init_IRQ(void)
{
	int i;

	for (i = 0; i < 16 ; i++)
		set_intr_gate(0x20+i,bad_interrupt[i]);
	if (irqaction(2,&ignore_IRQ))
		printk("Unable to get IRQ2 for cascade\n");
	if (request_irq(13,math_error_irq))
		printk("Unable to get IRQ13 for math-error handler\n");
}
