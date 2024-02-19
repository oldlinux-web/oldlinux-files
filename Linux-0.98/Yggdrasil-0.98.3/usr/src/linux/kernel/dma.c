/* $Header: /sys/linux-0.97/kernel/RCS/dma.c,v 1.4 1992/09/18 02:54:14 root Exp $
 * linux/kernel/dma.c: A DMA channel allocator. Inspired by linux/kernel/irq.c.
 * Written by Hennus Bergman, 1992. 
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <asm/dma.h>


/* A note on resource allocation:
 *
 * All drivers needing DMA channels, should allocate and release them
 * through the public routines `request_dma()' and `free_dma()'.
 *
 * In order to avoid problems, all processes should allocate resources in
 * the same sequence and release them in the reverse order.
 * 
 * So, when allocating DMAs and IRQs, first allocate the IRQ, then the DMA.
 * When releasing them, first release the DMA, then release the IRQ.
 * If you don't, you may cause allocation requests to fail unnecessarily.
 * This doesn't really matter now, but it will once we get real semaphores
 * in the kernel.
 */



/* Channel n is busy iff dma_chan_busy[n] != 0.
 * DMA0 is reserved for DRAM refresh, I think.
 * DMA4 is reserved for cascading (?).
 */
static volatile unsigned int dma_chan_busy[MAX_DMA_CHANNELS] = {
	1, 0, 0, 0, 1, 0, 0, 0
};



/* Atomically swap memory location [32 bits] with `newval'.
 * This avoid the cli()/sti() junk and related problems.
 * [And it's faster too :-)]
 * Maybe this should be in include/asm/mutex.h and be used for
 * implementing kernel-semaphores as well.
 */
static unsigned int __inline__ mutex_atomic_swap(volatile unsigned int * p, unsigned int newval)
{
	unsigned int semval = newval;

	/* If one of the operands for the XCHG instructions is a memory ref,
	 * it makes the swap an uninterruptible RMW cycle.
	 *
	 * One operand must be in memory, the other in a register, otherwise
	 * the swap may not be atomic.
	 */

	asm __volatile__ ("xchgl %2, %0\n"
			: /* outputs: semval   */ "=r" (semval)
			: /* inputs: newval, p */ "0" (semval), "m" (*p)
			);	/* p is a var, containing an address */
	return semval;
} /* mutex_atomic_swap */



int request_dma(unsigned int dmanr)
{
	if (dmanr >= MAX_DMA_CHANNELS)
		return -EINVAL;

	if (mutex_atomic_swap(&dma_chan_busy[dmanr], 1) != 0)
		return -EBUSY;
	else
		/* old flag was 0, now contains 1 to indicate busy */
		return 0;
} /* request_dma */


void free_dma(unsigned int dmanr)
{
	if (dmanr >= MAX_DMA_CHANNELS) {
		printk("Trying to free DMA%d\n", dmanr);
		return;
	}

	if (mutex_atomic_swap(&dma_chan_busy[dmanr], 0) == 0)
		printk("Trying to free free DMA%d\n", dmanr);
} /* free_dma */

