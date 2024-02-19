/*
 * Microsoft busmouse driver based on Logitech driver (see busmouse.c)
 *
 * Microsoft BusMouse support by Teemu Rantanen (tvr@cs.hut.fi) (02AUG92)
 *
 * Microsoft Bus Mouse support modified by Derrick Cole (cole@concert.net)
 *    8/28/92
 *
 * Microsoft Bus Mouse support folded into 0.97pl4 code
 *    by Peter Cervasio (pete%q106fm.uucp@wupost.wustl.edu) (08SEP92)
 * Changes:  Logitech and Microsoft support in the same kernel.
 *           Defined new constants in busmouse.h for MS mice.
 *           Added int mse_busmouse_type to distinguish busmouse types
 *           Added a couple of new functions to handle differences in using
 *             MS vs. Logitech (where the int variable wasn't appropriate).
 *
 * Modified by Peter Cervasio (address above) (26SEP92)
 * Changes:  Included code to (properly?) detect when a Microsoft mouse is
 *           really attached to the machine.  Don't know what this does to
 *           Logitech bus mice, but all it does is read ports.
 *
 * version 0.3
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/busmouse.h>
#include <linux/tty.h>
#include <linux/signal.h>
#include <linux/errno.h>

#include <asm/io.h>
#include <asm/segment.h>
#include <asm/system.h>
#include <asm/irq.h>

static struct mouse_status mouse;

static void ms_mouse_interrupt(int unused)
{
	char dx, dy, buttons;

	outb(MS_MSE_COMMAND_MODE, MS_MSE_CONTROL_PORT);
	outb((inb(MS_MSE_DATA_PORT) | 0x20), MS_MSE_DATA_PORT);

	outb(MS_MSE_READ_X, MS_MSE_CONTROL_PORT);
	dx = inb(MS_MSE_DATA_PORT);

	outb(MS_MSE_READ_Y, MS_MSE_CONTROL_PORT);
	dy = inb(MS_MSE_DATA_PORT);

	outb(MS_MSE_READ_BUTTONS, MS_MSE_CONTROL_PORT);
	buttons = ~(inb(MS_MSE_DATA_PORT)) & 0x07;

	outb(MS_MSE_COMMAND_MODE, MS_MSE_CONTROL_PORT);
	outb((inb(MS_MSE_DATA_PORT) & 0xdf), MS_MSE_DATA_PORT);

	mouse.buttons = buttons;
	mouse.latch_buttons |= buttons;
	mouse.dx += dx;
	mouse.dy += dy;
	mouse.ready = 1;
	wake_up(&mouse.wait);
}

static void release_mouse(struct inode * inode, struct file * file)
{
	MS_MSE_INT_OFF();
	mouse.active = 0;
	mouse.ready = 0; 
	free_irq(MOUSE_IRQ);
}

static int open_mouse(struct inode * inode, struct file * file)
{
	if (!mouse.present)
		return -EINVAL;
	if (mouse.active)
		return -EBUSY;
	mouse.active = 1;
	mouse.ready = 0;
	mouse.dx = 0;
	mouse.dy = 0;	
	mouse.buttons = mouse.latch_buttons = 0x80;
	if (request_irq(MOUSE_IRQ, ms_mouse_interrupt)) {
		mouse.active = 0;
		return -EBUSY;
	}
	outb(MS_MSE_START, MS_MSE_CONTROL_PORT);
	MS_MSE_INT_ON();	
	return 0;
}


static int write_mouse(struct inode * inode, struct file * file, char * buffer, int count)
{
	return -EINVAL;
}

static int read_mouse(struct inode * inode, struct file * file, char * buffer, int count)
{
	int i;

	if (count < 3)
		return -EINVAL;
	if (!mouse.ready)
		return -EAGAIN;
	put_fs_byte(mouse.latch_buttons | 0x80, buffer);
	if (mouse.dx < -127)
		mouse.dx = -127;
	if (mouse.dx > 127)
		mouse.dx =  127;
	put_fs_byte((char)mouse.dx, buffer + 1);
	if (mouse.dy < -127)
		mouse.dy = -127;
	if (mouse.dy > 127)
		mouse.dy =  127;
	put_fs_byte((char) -mouse.dy, buffer + 2);
	for (i = 3; i < count; i++)
		put_fs_byte(0x00, buffer + i);
	mouse.dx = 0;
	mouse.dy = 0;
	mouse.latch_buttons = mouse.buttons;
	mouse.ready = 0;
	return i;	
}

static int mouse_select(struct inode *inode, struct file *file, int sel_type, select_table * wait)
{
	if (sel_type != SEL_IN)
		return 0;
	if (mouse.ready) 
		return 1;
	select_wait(&mouse.wait,wait);
	return 0;
}

struct file_operations ms_bus_mouse_fops = {
	NULL,		/* mouse_seek */
	read_mouse,
	write_mouse,
	NULL, 		/* mouse_readdir */
	mouse_select, 	/* mouse_select */
	NULL, 		/* mouse_ioctl */
	NULL,		/* mouse_mmap */
	open_mouse,
	release_mouse,
};

#define MS_DELAY 100000

unsigned long ms_bus_mouse_init(unsigned long kmem_start)
{
	register int mse_byte;
	int i, delay_val, msfound = 1;

	mouse.present = 0;
	mouse.active = mouse.ready = 0;
	mouse.buttons = mouse.latch_buttons = 0x80;
	mouse.dx = mouse.dy = 0;
	mouse.wait = NULL;
	if (inb(MS_MSE_SIGNATURE_PORT) == 0xde) {
		for (delay_val=0; delay_val<MS_DELAY;)
			delay_val++;

		mse_byte = inb(MS_MSE_SIGNATURE_PORT);
		for (delay_val=0; delay_val<MS_DELAY; )
			delay_val++;

		for (i = 0; i < 4; i++) {
			for (delay_val=0; delay_val<MS_DELAY;)
				delay_val++;
			if (inb(MS_MSE_SIGNATURE_PORT) == 0xde) {
				for (delay_val=0; delay_val<MS_DELAY; )
					delay_val++;
				if (inb(MS_MSE_SIGNATURE_PORT) == mse_byte)
					msfound = 0;
				else
					msfound = 1;
			} else
				msfound = 1;
		}
	}
	if (msfound == 1) {
		return kmem_start;
	}
	MS_MSE_INT_OFF();
	mouse.present = 1;
	printk("Microsoft Bus mouse detected and installed.\n");
	return kmem_start;
}
