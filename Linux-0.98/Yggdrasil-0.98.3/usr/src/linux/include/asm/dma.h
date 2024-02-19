/* $Header: /sys/linux-0.97/include/asm/RCS/dma.h,v 1.4 1992/09/21 03:15:46 root Exp root $
 * linux/include/asm/dma.h: Defines for using and allocating dma channels.
 * Written by Hennus Bergman, 1992.
 */

#ifndef _ASM_DMA_H
#define _ASM_DMA_H

#include <asm/io.h>		/* need byte IO */
#include <linux/kernel.h>	/* need panic() [FIXME] */


#ifdef HAVE_REALLY_SLOW_DMA_CONTROLLER
#define outb	outb_p
#endif

/* FIXME: better fix this code for dma channels>3!!!!!!! */

/*
 * The routines below should in most cases (with optimizing on) result
 * in equal or better code than similar code using macros.
 *
 * NOTE about DMA transfers: The DMA controller cannot handle transfers
 * that cross a 64k boundary. When the address reaches 0xNffff, it will wrap
 * around to 0xN0000, rather than increment to 0x(N+1)0000 !
 * Make sure you align your buffers properly! Runtime check recommended.
 *
 * NOTE2: DMA1..3 can only use the lower 1MB of physical memory. DMA4..7
 * can access the lower 16MB. There are people with >16MB, so beware!
 */


#define MAX_DMA_CHANNELS	8

/* SOMEBODY should check the following:
 * Channels 0..3 are on the first DMA controller, channels 4..7 are
 * on the second. Channel 0 is for refresh, 4 is for cascading.
 * The first DMA controller uses bytes, the second words.
 *
 * Where are the page regs for the second DMA controller?????
 */


/* 8237 DMA controllers */
#define IO_DMA1_BASE	0x00	/* 8 bit slave DMA, channels 0..3 */
#define IO_DMA2_BASE	0xC0	/* 16 bit master DMA, ch 4(=slave input)..7 */

/* DMA controller registers */
#define DMA1_CMD_REG		0x08	/* DMA command register */
#define DMA1_STAT_REG		0x08	/* DMA status register */
#define DMA1_MASK_REG		0x0A	/* mask individual channels */
#define DMA1_MODE_REG		0x0B	/* set modes for individual channels */
#define DMA1_CLEAR_FF_REG	0x0C	/* Write 0 for LSB, 1 for MSB */
#define DMA1_RESET_REG		0x0D	/* Write here to reset DMA controller */
/* don't have much info on the second DMA controller... */
#define DMA2_MASK_REG		0xD4
#define DMA2_MODE_REG		0xD6
/* #define DMA2_CLEAR_FF_REG 0xD8 -- pure guessing.... */

/************* #error This needs more work!!!!!!!*************/

#define DMA_MODE_READ	0x44	/* I/O to memory, no autoinit, increment, single mode */
#define DMA_MODE_WRITE	0x48	/* memory to I/O, no autoinit, increment, single mode */
#define DMA_MODE_CASCADE	0xC0	/* cascade mode (for DMA2 controller only) */


/* enable/disable a specific DMA channel */
static __inline__ void enable_dma(unsigned int dmanr)
{
	if (dmanr<=3)
		outb(dmanr,  DMA1_MASK_REG);
	else
		outb(dmanr & 3,  DMA2_MASK_REG);
}

static __inline__ void disable_dma(unsigned int dmanr)
{
	if (dmanr<=3)
		outb(dmanr | 4,  DMA1_MASK_REG);
	else
		outb((dmanr & 3) | 4,  DMA2_MASK_REG);
}

/* Clear the 'DMA Pointer Flip Flop'.
 * Write 0 for LSB/MSB, 1 for MSB/LSB access.
 * Use this once to initialize the FF to a know state.
 * After that, keep track of it. :-) In order to do that,
 * dma_set_addr() and dma_set_count() should only be used wile
 * interrupts are disbled.
 */
static __inline__ void clear_dma_ff(unsigned int dmanr)
{
	if (dmanr<=3)
		outb(0,  DMA1_CLEAR_FF_REG);
	else
#ifdef DMA2_CLEAR_FF_REG
		outb(0,  DMA2_CLEAR_FF_REG);
#else
		panic("dma.h: Don't have CLEAR_FF for high dma channels!\n");
#endif
}

/* set mode (above) for a specific DMA channel */
static __inline__ void set_dma_mode(unsigned int dmanr, char mode)
{
	if (dmanr<=3)
		outb(mode | dmanr,  DMA1_MODE_REG);
	else
		outb(DMA_MODE_CASCADE | mode | (dmanr&3),  DMA2_MODE_REG);
}

/* Set only the page register bits of the transfer address.
 * This is used for successive transfers when we know the contents of
 * the lower 16 bits of the DMA current address register, but a 64k boundary
 * may have been crossed.
 */
static __inline__ void set_dma_page(unsigned int dmanr, char pagenr)
{
	switch(dmanr) {
		case 0:
			outb(pagenr, 0x80);
			break;
		case 1:
			outb(pagenr, 0x83);
			break;
		case 2:
			outb(pagenr, 0x81);
			break;
		case 3:
			outb(pagenr, 0x82);
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			panic("dma.h: don't know how to set DMA page regs for channels>3");
			break;
	}
}


/* Set transfer address & page bits for specific DMA channel.
 * Assumes dma flipflop is clear.
 */
static __inline__ void set_dma_addr(unsigned int dmanr, unsigned int a)
{
	unsigned int io_base = (dmanr<=3)? IO_DMA1_BASE : IO_DMA2_BASE;

	set_dma_page(dmanr, a>>16);
	outb(a & 0xff, ((dmanr&3)<<1) + io_base);
	outb((a>>8) & 0xff, ((dmanr&3)<<1) + io_base);
}


/* Set transfer size (max 64k) for a specific DMA channel.
 * You must ensure the parameters are valid.
 * NOTE: from a manual: "the number of transfers is one more 
 * than the initial word count"! This is taken into account.
 * Assumes dma flip-flop is clear.
 */
static __inline__ void set_dma_count(unsigned int dmanr, unsigned int count)
{
	unsigned int dc = count - 1;
	unsigned int io_base = (dmanr<=3)? IO_DMA1_BASE : IO_DMA2_BASE;

	outb(dc & 0xff, ((dmanr&3)<<1) + 1 + io_base);
	outb((dc>>8) & 0xff, ((dmanr&3)<<1) + 1 + io_base);
}


/* Get DMA residue count. After a DMA transfer, this
 * should return zero. Reading this while a DMA transfer is
 * still in progress will return unpredictable results.
 * If called before the channel has been used, it may return 1.
 * Otherwise, it returns the number of bytes left to transfer,
 * minus 1, modulo 64k.
 * Assumes DMA flip-flop is clear.
 */
static __inline__ short int get_dma_residue(unsigned int dmanr)
{
	unsigned int io_base = (dmanr<=3)? IO_DMA1_BASE : IO_DMA2_BASE;

	return 1 + inb( ((dmanr&3)<<1) + 1 + io_base ) +
		( inb( ((dmanr&3)<<1) + 1 + io_base ) << 8 );
}

/* These are in kernel/dma.c: */
extern int request_dma(unsigned int dmanr);	/* reserve a DMA channel */
extern void free_dma(unsigned int dmanr);	/* release it again */


#endif /* _ASM_DMA_H */
