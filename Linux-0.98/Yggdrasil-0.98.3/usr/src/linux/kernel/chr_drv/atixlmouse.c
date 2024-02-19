/*
 * ATI XL Bus Mouse Driver for Linux
 * by Bob Harris (rth@sparta.com)
 *
 * Uses VFS interface for linux 0.98 (01OCT92)
 *
 * version 0.3
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/signal.h>
#include <linux/errno.h>

#include <asm/io.h>
#include <asm/segment.h>
#include <asm/system.h>
#include <asm/irq.h>

#define ATIXL_MOUSE_IRQ		5 /* H/W interrupt # set up on ATIXL board */
#define ATIXL_BUSMOUSE		3 /* Minor device # (mknod c 10 3 /dev/bm) */

/* ATI XL Inport Busmouse Definitions */

#define	ATIXL_MSE_DATA_PORT		0x23d
#define	ATIXL_MSE_SIGNATURE_PORT	0x23e
#define	ATIXL_MSE_CONTROL_PORT		0x23c

#define	ATIXL_MSE_READ_BUTTONS		0x00
#define	ATIXL_MSE_READ_X		0x01
#define	ATIXL_MSE_READ_Y		0x02

/* Some nice ATI XL macros */

/* Select IR7, HOLD UPDATES (INT ENABLED), save X,Y */
#define ATIXL_MSE_DISABLE_UPDATE() { outb( 0x07, ATIXL_MSE_CONTROL_PORT ); \
	outb( (0x20 | inb( ATIXL_MSE_DATA_PORT )), ATIXL_MSE_DATA_PORT ); }

/* Select IR7, Enable updates (INT ENABLED) */
#define ATIXL_MSE_ENABLE_UPDATE() { outb( 0x07, ATIXL_MSE_CONTROL_PORT ); \
	 outb( (0xdf & inb( ATIXL_MSE_DATA_PORT )), ATIXL_MSE_DATA_PORT ); }

/* Select IR7 - Mode Register, NO INTERRUPTS */
#define ATIXL_MSE_INT_OFF() { outb( 0x07, ATIXL_MSE_CONTROL_PORT ); \
	outb( (0xe7 & inb( ATIXL_MSE_DATA_PORT )), ATIXL_MSE_DATA_PORT ); }

/* Select IR7 - Mode Register, DATA INTERRUPTS ENABLED */
#define ATIXL_MSE_INT_ON() { outb( 0x07, ATIXL_MSE_CONTROL_PORT ); \
	outb( (0x08 | inb( ATIXL_MSE_DATA_PORT )), ATIXL_MSE_DATA_PORT ); }

/* Same general mouse structure */

static struct mouse_status {
	char buttons;
	char latch_buttons;
	int dx;
	int dy;
	int present;
	int ready;
	int active;
	struct wait_queue *wait;
} mouse;

void mouse_interrupt(int unused)
{
	ATIXL_MSE_DISABLE_UPDATE(); /* Note that interrupts are still enabled */
	outb(ATIXL_MSE_READ_X, ATIXL_MSE_CONTROL_PORT); /* Select IR1 - X movement */
	mouse.dx += inb( ATIXL_MSE_DATA_PORT);
	outb(ATIXL_MSE_READ_Y, ATIXL_MSE_CONTROL_PORT); /* Select IR2 - Y movement */
	mouse.dy += inb( ATIXL_MSE_DATA_PORT);
	outb(ATIXL_MSE_READ_BUTTONS, ATIXL_MSE_CONTROL_PORT); /* Select IR0 - Button Status */
	mouse.latch_buttons |= inb( ATIXL_MSE_DATA_PORT);
	ATIXL_MSE_ENABLE_UPDATE();
	mouse.ready = 1;
	wake_up(&mouse.wait);
}

static void release_mouse(struct inode * inode, struct file * file)
{
	ATIXL_MSE_INT_OFF(); /* Interrupts are really shut down here */
	mouse.active = 0;
	mouse.ready = 0;
	free_irq(ATIXL_MOUSE_IRQ);
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
	mouse.buttons = mouse.latch_buttons = 0;
	if (request_irq(ATIXL_MOUSE_IRQ, mouse_interrupt)) {
		mouse.active = 0;
		return -EBUSY;
	}
	ATIXL_MSE_INT_ON(); /* Interrupts are really enabled here */
	return 0;
}


static int write_mouse(struct inode * inode, struct file * file, char * buffer, int count)
{
	return -EINVAL;
}

static int read_mouse(struct inode * inode, struct file * file, char * buffer, int count)
{
	if (count < 3)
		return -EINVAL;
	if (!mouse.ready)
		return -EAGAIN;
	ATIXL_MSE_DISABLE_UPDATE();
	/* Allowed interrupts to occur during data gathering - shouldn't hurt */
	put_fs_byte((char)(~mouse.latch_buttons&7) | 0x80 , buffer);
	put_fs_byte((char)mouse.dx, buffer + 1);
	put_fs_byte((char)-mouse.dy, buffer + 2);
	mouse.dx = 0;
	mouse.dy = 0;
	mouse.latch_buttons = mouse.buttons;
	mouse.ready = 0;
	ATIXL_MSE_ENABLE_UPDATE();
	return 3; /* 3 data bytes returned */
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

struct file_operations atixl_busmouse_fops = {
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

unsigned long atixl_busmouse_init(unsigned long kmem_start)
{
	unsigned char a,b,c;

	a = inb( ATIXL_MSE_SIGNATURE_PORT );	/* Get signature */
	b = inb( ATIXL_MSE_SIGNATURE_PORT );
	c = inb( ATIXL_MSE_SIGNATURE_PORT );
	if (( a != b ) && ( a == c ))
		printk("\nATI Inport ");
	else{
		mouse.present = 0;
		return kmem_start;
	}
	outb(0x80, ATIXL_MSE_CONTROL_PORT);	/* Reset the Inport device */
	outb(0x07, ATIXL_MSE_CONTROL_PORT);	/* Select Internal Register 7 */
	outb(0x0a, ATIXL_MSE_DATA_PORT);	/* Data Interrupts 8+, 1=30hz, 2=50hz, 3=100hz, 4=200hz rate */
	mouse.present = 1;
	mouse.active = 0;
	mouse.ready = 0;
	mouse.buttons = mouse.latch_buttons = 0;
	mouse.dx = mouse.dy = 0;
	mouse.wait = NULL;
	printk("Bus mouse detected and installed.\n");
	return kmem_start;
}
