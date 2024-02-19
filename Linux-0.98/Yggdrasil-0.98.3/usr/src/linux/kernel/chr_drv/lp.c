/*
 * Copyright (C) 1992 by Jim Weigand, Linus Torvalds, and Michael K. Johnson
 */

#include <linux/lp.h>
/* sched.h is included from lp.h */

static int lp_reset(int minor)
{
	int testvalue;

	/* reset value */
	outb(0, LP_C(minor));
	for (testvalue = 0 ; testvalue < LP_DELAY ; testvalue++)
		;
	outb(LP_PSELECP | LP_PINITP, LP_C(minor));
	return LP_S(minor);
}

static int lp_char(char lpchar, int minor)
{
	int retval = 0;
	unsigned long count  = 0; 

	outb(lpchar, LP_B(minor));
	do {
		retval = LP_S(minor);
		count ++;
		if (need_resched)
			schedule();
	} while(!(retval & LP_PBUSY) && count < LP_TIME_CHAR);

	if (count == LP_TIME_CHAR) {
		return 0;
		/* we timed out, and the character was /not/ printed */
	}
        /* control port takes strobe high */
	outb(( LP_PSELECP | LP_PINITP | LP_PSTROBE ), ( LP_C( minor )));
        /* take strobe low */
	outb(( LP_PSELECP | LP_PINITP ), ( LP_C( minor )));
       /* get something meaningful for return value */
	return LP_S(minor);
}

static int lp_write(struct inode * inode, struct file * file, char * buf, int count)
{
	int  retval;
	unsigned int minor = MINOR(inode->i_rdev);
	unsigned int each_cnt = 0, old_cnt = 0;
	char c, *temp = buf;

	temp = buf;
	while (count > 0) {
		c = get_fs_byte(temp);
		retval = lp_char(c, minor);
		/* only update counting vars if character was printed */
		if (retval) {
			count--;
			temp++;
		}

		if (!retval) { /* if printer timed out */
			each_cnt = count - old_cnt;
			old_cnt = count;

			/* here we do calculations based on old count
			   and change the time that we will sleep.
			   For now this will be hard coded... */

			/* check for signals before going to sleep */
			if (current->signal & ~current->blocked) {
				return temp-buf?temp-buf:-ERESTARTSYS;
			}
			current->state = TASK_INTERRUPTIBLE;
			current->timeout = jiffies + LP_TIME(minor);
			schedule();
			LP_COUNT(minor) = each_cnt;

			/* the following is ugly, but should alert me if
			   something dreadful is going on. It will disappear
			   in the final versions of the driver. */
			if (!(LP_S(minor) & LP_BUSY)) {
				current->state = TASK_INTERRUPTIBLE;
				current->timeout = jiffies + LP_TIMEOUT;
				schedule();
				if (!(LP_S(minor) & LP_BUSY))
					printk("lp%d timeout\n\r", minor);
			}
		} else {
			if (retval & LP_POUTPA) {
				LP_F(minor) |= LP_NOPA;
				printk("lp%d out of paper\n\r", minor);
				return temp-buf?temp-buf:-ENOSPC;
			} else
				LP_F(minor) &= ~LP_NOPA;

			if (!(retval & LP_PSELECD)) {
				LP_F(minor) |= LP_SELEC;
				printk("lp%d off-line\n\r", minor);
				return temp-buf?temp-buf:-EFAULT;
			} else
				LP_F(minor) &= ~LP_SELEC;

	                /* not offline or out of paper. on fire? */
			if (!(retval & LP_PERRORP)) {
				LP_F(minor) |= LP_ERR;
				printk("lp%d on fire\n\r", minor);
				return temp-buf?temp-buf:-EIO;
			} else
				LP_F(minor) &= ~LP_SELEC;
		}
	}
	return temp-buf;
}

static int lp_read(struct inode * inode, struct file * file, char * buf, int count)
{
	return -EINVAL;
}

static int lp_lseek(struct inode * inode, struct file * file, off_t offset, int origin)
{
	return -EINVAL;
}

static int lp_open(struct inode * inode, struct file * file)
{
	unsigned int minor = MINOR(inode->i_rdev);

	if (minor >= LP_NO)
		return -ENODEV;
	if ((LP_F(minor) & LP_EXIST) == 0)
		return -ENODEV;
	if (LP_F(minor) & LP_BUSY)
		return -EBUSY;
	LP_F(minor) |= LP_BUSY;
	return 0;
}

static void lp_release(struct inode * inode, struct file * file)
{
	unsigned int minor = MINOR(inode->i_rdev);

	LP_F(minor) &= ~LP_BUSY;
}

static struct file_operations lp_fops = {
	lp_lseek,	/* why not null? */
	lp_read,	/* why not null? */
	lp_write,
	NULL,		/* lp_readdir */
	NULL,		/* lp_select */
	NULL,		/* lp_ioctl */
	NULL,		/* mmap */
	lp_open,
	lp_release
};

long lp_init(long kmem_start)
{
	int offset = 0;
	unsigned int testvalue = 0;
	int count = 0;

	chrdev_fops[6] = &lp_fops;
	/* take on all known port values */
	for (offset = 0; offset < LP_NO; offset++) {
		/* write to port & read back to check */
		outb( LP_DUMMY, LP_B(offset));
		for (testvalue = 0 ; testvalue < LP_DELAY ; testvalue++)
			;
		testvalue = inb(LP_B(offset));
		if (testvalue != 255) {
			LP_F(offset) |= LP_EXIST;
			lp_reset(offset);
			printk("lp_init: lp%d exists (%d)\n", offset, testvalue);
			count++;
		}
	}
	if (count == 0)
		printk("lp_init: no lp devices found\n");
	return kmem_start;
}
