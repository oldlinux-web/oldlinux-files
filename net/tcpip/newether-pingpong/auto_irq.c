/* auto_irq.c: Auto-configure IRQ lines for linux. */
/*
    Written 1993 by Donald Becker. This is alpha test code.

    The Author may be reached as becker@super.org or
    C/O Supercomputing Research Ctr., 17100 Science Dr., Bowie MD 20715

    This code is a general-purpose IRQ line detector for devices with
    jumpered IRQ lines.  If you can make the device raise an IRQ (and
    that IRQ line isn't already being used), these routines will tell
    you what IRQ line it's using -- perfect for those oh-so-cool boot-time
    device probes!

    To use this, first call autoirq_setup(timeout). TIMEOUT is how many
    'jiffies' (1/18 sec.) to detect other devices that have active IRQ lines,
    and can usually be zero at boot.  'autoirq_setup()' returns the bit
    vector of nominally-available IRQ lines (lines may be physically in-use,
    but not yet registered to a device).
    Next, set up your device to trigger an interrupt.
    Finally call autoirq_report(TIMEOUT) to find out which IRQ line was
    most recently active.  The TIMEOUT should usually be zero, but may
    be set to the number of jiffies to wait for a slow device to raise an IRQ.

    The idea of using the setup timeout to filter out bogus IRQs came from
    the serial driver.
*/


#ifdef version
static char *version="auto_irq.c:v0.01 1993 Donald Becker (becker@super.org)";
#endif

/*#include <linux/config.h>*/
/*#include <linux/kernel.h>*/
#include <linux/sched.h>
#include <asm/bitops.h>
#include <asm/io.h>
/*#include <asm/system.h>*/

int irqs_busy = 0x01;		/* The set of fixed IRQs always enabled */
int irqs_used = 0x01;		/* The set of fixed IRQs sometimes enabled. */
int irqs_reserved = 0x00;	/* An advisory "reserved" table. */
int irqs_shared = 0x00;		/* IRQ lines "shared" among conforming cards.*/

static volatile int irq_number;	/* The latest irq number we actually found. */
static volatile int irq_bitmap; /* The irqs we actually found. */
static int irq_handled;		/* The irq lines we have a handler on. */

static void autoirq_probe(int irq)
{
	irq_number = irq;
	set_bit(irq, (void *)&irq_bitmap);	/* irq_bitmap |= 1 << irq; */
	return;
}
struct sigaction autoirq_sigaction = { autoirq_probe, 0, SA_INTERRUPT, NULL};

int autoirq_setup(int waittime)
{
    int i, mask;
    int timeout = jiffies+waittime;

    irq_number = 0;
    irq_bitmap = 0;
    irq_handled = 0;
    for (i = 0; i < 16; i++) {
	if (!irqaction(i, &autoirq_sigaction))
	    set_bit(i, (void *)&irq_handled);	/* irq_handled |= 1 << i;*/
    }
    /* Update our USED lists. */
    irqs_used |= ~irq_handled;

    /* Hang out at least <waittime> jiffies waiting for bogus IRQ hits. */
    while (timeout >= jiffies)
	;

    for (i = 0, mask = 0x01; i < 16; i++, mask <<= 1) {
	if (irq_bitmap & irq_handled & mask) {
	    irq_handled &= ~mask;
	    printk(" Spurious interrupt on IRQ %d\n", i);
	    free_irq(i);
	}
    }
    return irq_handled;
}

int autoirq_report(int waittime)
{
    int i;
    int timeout = jiffies+waittime;

    /* Hang out at least <waittime> jiffies waiting for the IRQ. */
    while (timeout >= jiffies)
	if (irq_number)
	    break;

    /* Retract the irq handlers that we installed. */
    for (i = 0; i < 16; i++) {
	if (test_bit(i, (void *)&irq_handled))
	    free_irq(i);
    }
    return irq_number;
}

/*
 * Local variables:
 *  compile-command: "gcc -DKERNEL -Wall -O6 -fomit-frame-pointer -I/usr/src/linux/net/tcp -c auto_irq.c"
 *  version-control: t
 *  kept-new-versions: 5
 * End:
 */
