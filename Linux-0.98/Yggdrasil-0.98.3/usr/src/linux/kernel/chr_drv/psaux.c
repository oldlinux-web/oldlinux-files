/*
 * linux/kernel/chr_drv/psaux.c
 *
 * Driver for PS/2 type mouse by Johan Myreen.
 *
 * Supports pointing devices attached to a PS/2 type
 * Keyboard and Auxiliary Device Controller.
 *
 * Modified by Dean Troyer (troyer@saifr00.cfsat.Honeywell.COM) 03Oct92
 *   to perform (some of) the hardware initialization formerly done in
 *   setup.S by the BIOS
 *
 * Modified by Dean Troyer (troyer@saifr00.cfsat.Honeywell.COM) 09Oct92
 *   to perform the hardware initialization formerly done in setup.S by
 *   the BIOS.  Mouse characteristic setup is now included.
 *
 */

#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fcntl.h>
#include <linux/errno.h>

#include <asm/io.h>
#include <asm/segment.h>
#include <asm/system.h>

/* aux controller ports */
#define AUX_INPUT_PORT	0x60		/* Aux device output buffer */
#define AUX_OUTPUT_PORT	0x60		/* Aux device input buffer */
#define AUX_COMMAND	0x64		/* Aux device command buffer */
#define AUX_STATUS	0x64		/* Aux device status reg */

/* aux controller status bits */
#define AUX_OBUF_FULL	0x01		/* output buffer (from device) full */
#define AUX_IBUF_FULL	0x02		/* input buffer (to device) full */

/* aux controller commands */
#define AUX_CMD_WRITE	0x60		/* value to write to controller */
#define AUX_MAGIC_WRITE	0xd4		/* value to send aux device data */

#define AUX_INTS_ON	0x47		/* enable controller interrupts */
#define AUX_INTS_OFF	0x65		/* disable controller interrupts */

#define AUX_DISABLE	0xa7		/* disable aux */
#define AUX_ENABLE	0xa8		/* enable aux */

/* aux device commands */
#define AUX_SET_RES	0xe8		/* set resolution */
#define AUX_SET_SCALE	0xe9		/* set scaling factor */
#define AUX_SET_STREAM	0xea		/* set stream mode */
#define AUX_SET_SAMPLE	0xf3		/* set sample rate */
#define AUX_ENABLE_DEV	0xf4		/* enable aux device */
#define AUX_DISABLE_DEV	0xf5		/* disable aux device */
#define AUX_RESET	0xff		/* reset aux device */

#define MAX_RETRIES	3
#define AUX_IRQ		12
#define AUX_BUF_SIZE	2048

extern unsigned char aux_device_present;

struct aux_queue {
	unsigned long head;
	unsigned long tail;
	struct wait_queue *proc_list;
	unsigned char buf[AUX_BUF_SIZE];
};

static struct aux_queue *queue;
static int aux_ready = 0;
static int aux_busy = 0;
static int aux_present = 0;

static int poll_status(void);


/*
 * Write to aux device
 */

static void aux_write_dev(int val)
{
	poll_status();
	outb_p(AUX_MAGIC_WRITE,AUX_COMMAND);	/* write magic cookie */
	poll_status();
	outb_p(val,AUX_OUTPUT_PORT);		/* write data */
	
}


/*
 * Write to device & handle returned ack
 */
 
static int aux_write_ack(int val)
{
	aux_write_dev(val);		/* write the value to the device */
	while ((inb(AUX_STATUS) & AUX_OBUF_FULL) == 0);  /* wait for ack */
	if ((inb(AUX_STATUS) & 0x20) == 0x20)
	{
		return (inb(AUX_INPUT_PORT));
	}
	return 0;
}


/*
 * Write aux device command
 */

static void aux_write_cmd(int val)
{
	poll_status();
	outb_p(AUX_CMD_WRITE,AUX_COMMAND);
	poll_status();
	outb_p(val,AUX_OUTPUT_PORT);
}


static unsigned int get_from_queue()
{
	unsigned int result;
	unsigned long flags;

	__asm__ __volatile__ ("pushfl ; popl %0; cli":"=r" (flags));
	result = queue->buf[queue->tail];
	queue->tail = (queue->tail + 1) & (AUX_BUF_SIZE-1);
	__asm__ __volatile__ ("pushl %0 ; popfl"::"r" (flags));
	return result;
}


static inline int queue_empty()
{
	return queue->head == queue->tail;
}


/*
 * Interrupt from the auxiliary device: a character
 * is waiting in the keyboard/aux controller.
 */

static void aux_interrupt(int cpl)
{
	int head = queue->head;
	int maxhead = (queue->tail-1) & (AUX_BUF_SIZE-1);

	queue->buf[head] = inb(AUX_INPUT_PORT);
	if (head != maxhead) {
		head++;
		head &= AUX_BUF_SIZE-1;
	}
	queue->head = head;
	aux_ready = 1;
	wake_up(&queue->proc_list);
}


static void release_aux(struct inode * inode, struct file * file)
{
	poll_status();
	outb_p(AUX_DISABLE,AUX_COMMAND);      	/* Disable Aux device */
	aux_write_dev(AUX_DISABLE_DEV);		/* disable aux device */
	aux_write_cmd(AUX_INTS_OFF);		/* disable controller ints */
	free_irq(AUX_IRQ);
	aux_busy = 0;
}


/*
 * Install interrupt handler.
 * Enable auxiliary device.
 */

static int open_aux(struct inode * inode, struct file * file)
{
	if (!aux_present)
		return -EINVAL;
	if (aux_busy)
		return -EBUSY;
	if (!poll_status())
		return -EBUSY;
	aux_busy = 1;
	queue->head = queue->tail = 0;  /* Flush input queue */
	if (request_irq(AUX_IRQ, aux_interrupt))
		return -EBUSY;
	aux_write_dev(AUX_ENABLE_DEV);		/* enable aux device */
	aux_write_cmd(AUX_INTS_ON);		/* enable controller ints */
	poll_status();
	outb_p(AUX_ENABLE,AUX_COMMAND);		/* Enable Aux */
	return 0;
}


/*
 * Write to the aux device.
 */

static int write_aux(struct inode * inode, struct file * file, char * buffer, int count)
{
	int i = count;

	while (i--) {
		if (!poll_status())
			return -EIO;
		outb_p(AUX_MAGIC_WRITE,AUX_COMMAND);
		if (!poll_status())
			return -EIO;
		outb_p(get_fs_byte(buffer++),AUX_OUTPUT_PORT);
	}
	inode->i_mtime = CURRENT_TIME;
	return count;
}


/*
 * Put bytes from input queue to buffer.
 */

static int read_aux(struct inode * inode, struct file * file, char * buffer, int count)
{
	int i = count;
	unsigned char c;

	if (queue_empty()) {
		if (file->f_flags & O_NONBLOCK)
			return -EWOULDBLOCK;
		cli();
		interruptible_sleep_on(&queue->proc_list);
		sti();
	}		
	while (i > 0 && !queue_empty()) {
		c = get_from_queue();
		put_fs_byte(c, buffer++);
		i--;
	}
	aux_ready = !queue_empty();
	if (count-i) {
		inode->i_atime = CURRENT_TIME;
		return count-i;
	}
	if (current->signal & ~current->blocked)
		return -ERESTARTSYS;
	return 0;
}


static int aux_select(struct inode *inode, struct file *file, int sel_type, select_table * wait)
{
	if (sel_type != SEL_IN)
		return 0;
	if (aux_ready)
		return 1;
	select_wait(&queue->proc_list, wait);
	return 0;
}


struct file_operations psaux_fops = {
	NULL,		/* seek */
	read_aux,
	write_aux,
	NULL, 		/* readdir */
	aux_select,
	NULL, 		/* ioctl */
	NULL,		/* mmap */
	open_aux,
	release_aux,
};


unsigned long psaux_init(unsigned long kmem_start)
{
	if (aux_device_present != 0xaa) {
		return kmem_start;
	}
	aux_write_ack(AUX_SET_RES);
	aux_write_ack(0x03);		/* set resultion to 8 counts/mm */
	aux_write_ack(AUX_SET_SCALE);
	aux_write_ack(0x02);		/* set scaling to 2:1 */
	aux_write_ack(AUX_SET_SAMPLE);
	aux_write_ack(0x64);		/* set sampling rate to 100/sec */
	aux_write_ack(AUX_SET_STREAM);	/* set stream mode */
	printk("PS/2 type pointing device detected and installed.\n");
	queue = (struct aux_queue *) kmem_start;
	kmem_start += sizeof (struct aux_queue);
	queue->head = queue->tail = 0;
	queue->proc_list = NULL;
	aux_present = 1;
	return kmem_start;
}


static int poll_status(void)
{
	int retries=0;

	while ((inb(AUX_STATUS)&0x03) && retries++ < MAX_RETRIES) {
		if (inb_p(AUX_STATUS)&0x01)
			inb_p(AUX_INPUT_PORT);
		current->state = TASK_INTERRUPTIBLE;
		current->timeout = jiffies + 5;
		schedule();
	}
	return !(retries==MAX_RETRIES);
}
