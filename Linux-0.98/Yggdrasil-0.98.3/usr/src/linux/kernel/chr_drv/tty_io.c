/*
 *  linux/kernel/tty_io.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * 'tty_io.c' gives an orthogonal feeling to tty's, be they consoles
 * or rs-channels. It also implements echoing, cooked mode etc.
 *
 * Kill-line thanks to John T Kohl, who also corrected VMIN = VTIME = 0.
 *
 * Modified by Theodore Ts'o, 9/14/92, to dynamically allocate the
 * tty_struct and tty_queue structures.  Previously there was a array
 * of 256 tty_struct's which was statically allocated, and the
 * tty_queue structures were allocated at boot time.  Both are now
 * dynamically allocated only when the tty is open.
 *
 * Also restructured routines so that there is more of a separation
 * between the high-level tty routines (tty_io.c and tty_ioctl.c) and
 * the low-level tty routines (serial.c, pty.c, console.c).  This
 * makes for cleaner and more compact code.  -TYT, 9/17/92 
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/fcntl.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/ctype.h>
#include <linux/kd.h>
#include <linux/mm.h>
#include <linux/string.h>

#include <asm/segment.h>
#include <asm/system.h>
#include <asm/bitops.h>

#include "vt_kern.h"

struct tty_struct *tty_table[256];
struct termios *tty_termios[256]; /* We need to keep the termios state */
				  /* around, even when a tty is closed */

/*
 * fg_console is the current virtual console,
 * redirect is the pseudo-tty that console output
 * is redirected to if asked by TIOCCONS.
 */
int fg_console = 0;
struct tty_struct * redirect = NULL;
struct wait_queue * keypress_wait = NULL;

int initialize_tty_struct(struct tty_struct *tty, int line);

void put_tty_queue(char c, struct tty_queue * queue)
{
	int head;
	unsigned long flags;

	__asm__ __volatile__("pushfl ; popl %0 ; cli":"=r" (flags));
	head = (queue->head + 1) & (TTY_BUF_SIZE-1);
	if (head != queue->tail) {
		queue->buf[queue->head] = c;
		queue->head = head;
	}
	__asm__ __volatile__("pushl %0 ; popfl"::"r" (flags));
}

int get_tty_queue(struct tty_queue * queue)
{
	int result = -1;
	unsigned long flags;

	__asm__ __volatile__("pushfl ; popl %0 ; cli":"=r" (flags));
	if (queue->tail != queue->head) {
		result = 0xff & queue->buf[queue->tail];
		queue->tail = (queue->tail + 1) & (TTY_BUF_SIZE-1);
	}
	__asm__ __volatile__("pushl %0 ; popfl"::"r" (flags));
	return result;
}

void tty_write_flush(struct tty_struct * tty)
{
	if (!tty->write || EMPTY(&tty->write_q))
		return;
	if (set_bit(TTY_WRITE_BUSY,&tty->flags))
		return;
	tty->write(tty);
	if (clear_bit(TTY_WRITE_BUSY,&tty->flags))
		printk("tty_write_flush: bit already cleared\n");
}

void tty_read_flush(struct tty_struct * tty)
{
	if (!tty || EMPTY(&tty->read_q))
		return;
	if (set_bit(TTY_READ_BUSY, &tty->flags))
		return;
	copy_to_cooked(tty);
	if (clear_bit(TTY_READ_BUSY, &tty->flags))
		printk("tty_read_flush: bit already cleared\n");
}

void change_console(unsigned int new_console)
{
	if (vt_cons[fg_console].vt_mode == KD_GRAPHICS)
		return;
	if (new_console == fg_console || new_console >= NR_CONSOLES)
		return;
	update_screen(new_console);
}

void wait_for_keypress(void)
{
	interruptible_sleep_on(&keypress_wait);
}

void copy_to_cooked(struct tty_struct * tty)
{
	int c;

	if (!tty) {
		printk("copy_to_cooked: called with NULL tty\n");
		return;
	}
	if (!tty->write) {
		printk("copy_to_cooked: tty %d has null write routine\n",
		       tty->line);
	}
	while (1) {
		/*
		 * Check to see how much room we have left in the
		 * secondary queue.  Send a throttle command or abort
		 * if necessary.
		 */
		c = LEFT(&tty->secondary);
		if (tty->throttle && (c < SQ_THRESHOLD_LW)
		    && !set_bit(TTY_SQ_THROTTLED, &tty->flags))
			tty->throttle(tty, TTY_THROTTLE_SQ_FULL);
		if (c == 0)
			break;
		c = get_tty_queue(&tty->read_q);
		if (c < 0)
			break;
		if (I_STRP(tty))
			c &= 0x7f;
		if (c==13) {
			if (I_CRNL(tty))
				c=10;
			else if (I_NOCR(tty))
				continue;
		} else if (c==10 && I_NLCR(tty))
			c=13;
		if (I_UCLC(tty))
			c=tolower(c);
		if (L_CANON(tty)) {
			if ((KILL_CHAR(tty) != __DISABLED_CHAR) &&
			    (c==KILL_CHAR(tty))) {
				/* deal with killing the input line */
				while(!(EMPTY(&tty->secondary) ||
					(c=LAST(&tty->secondary))==10 ||
					((EOF_CHAR(tty) != __DISABLED_CHAR) &&
					 (c==EOF_CHAR(tty))))) {
					if (L_ECHO(tty)) {
						if (c<32) {
							put_tty_queue(8, &tty->write_q);
							put_tty_queue(' ', &tty->write_q);
							put_tty_queue(8,&tty->write_q);
						}
						put_tty_queue(8,&tty->write_q);
						put_tty_queue(' ',&tty->write_q);
						put_tty_queue(8,&tty->write_q);
					}
					DEC(tty->secondary.head);
				}
				continue;
			}
			if ((ERASE_CHAR(tty) != __DISABLED_CHAR) &&
			    (c==ERASE_CHAR(tty))) {
				if (EMPTY(&tty->secondary) ||
				   (c=LAST(&tty->secondary))==10 ||
				   ((EOF_CHAR(tty) != __DISABLED_CHAR) &&
				    (c==EOF_CHAR(tty))))
					continue;
				if (L_ECHO(tty)) {
					if (c<32) {
						put_tty_queue(8,&tty->write_q);
						put_tty_queue(' ',&tty->write_q);
						put_tty_queue(8,&tty->write_q);
					}
					put_tty_queue(8,&tty->write_q);
					put_tty_queue(32,&tty->write_q);
					put_tty_queue(8,&tty->write_q);
				}
				DEC(tty->secondary.head);
				continue;
			}
		}
		if (I_IXON(tty)) {
			if ((STOP_CHAR(tty) != __DISABLED_CHAR) &&
			    (c==STOP_CHAR(tty))) {
			        tty->status_changed = 1;
				tty->ctrl_status |= TIOCPKT_STOP;
				tty->stopped=1;
				continue;
			}
			if (((I_IXANY(tty)) && tty->stopped) ||
			    ((START_CHAR(tty) != __DISABLED_CHAR) &&
			     (c==START_CHAR(tty)))) {
			        tty->status_changed = 1;
				tty->ctrl_status |= TIOCPKT_START;
				tty->stopped=0;
				continue;
			}
		}
		if (L_ISIG(tty)) {
			if ((INTR_CHAR(tty) != __DISABLED_CHAR) &&
			    (c==INTR_CHAR(tty))) {
				kill_pg(tty->pgrp, SIGINT, 1);
				flush_input(tty);
				continue;
			}
			if ((QUIT_CHAR(tty) != __DISABLED_CHAR) &&
			    (c==QUIT_CHAR(tty))) {
				kill_pg(tty->pgrp, SIGQUIT, 1);
				flush_input(tty);
				continue;
			}
			if ((SUSPEND_CHAR(tty) != __DISABLED_CHAR) &&
			    (c==SUSPEND_CHAR(tty))) {
				if (!is_orphaned_pgrp(tty->pgrp))
					kill_pg(tty->pgrp, SIGTSTP, 1);
				continue;
			}
		}
		if (c==10 || (EOF_CHAR(tty) != __DISABLED_CHAR &&
		    c==EOF_CHAR(tty)))
			tty->secondary.data++;
		if ((c==10) && (L_ECHO(tty) || (L_CANON(tty) && L_ECHONL(tty)))) {
			put_tty_queue(10,&tty->write_q);
			put_tty_queue(13,&tty->write_q);
		} else if (L_ECHO(tty)) {
			if (c<32 && L_ECHOCTL(tty)) {
				put_tty_queue('^',&tty->write_q);
				put_tty_queue(c+64, &tty->write_q);
			} else
				put_tty_queue(c, &tty->write_q);
		}
		put_tty_queue(c, &tty->secondary);
	}
	TTY_WRITE_FLUSH(tty);
	if (!EMPTY(&tty->secondary))
		wake_up(&tty->secondary.proc_list);
	if (tty->write_q.proc_list && LEFT(&tty->write_q) > TTY_BUF_SIZE/2)
		wake_up(&tty->write_q.proc_list);
	if (tty->throttle && (LEFT(&tty->read_q) >= RQ_THRESHOLD_HW)
	    && !clear_bit(TTY_RQ_THROTTLED, &tty->flags))
		tty->throttle(tty, TTY_THROTTLE_RQ_AVAIL);
	if (tty->throttle && (LEFT(&tty->secondary) >= SQ_THRESHOLD_HW)
	    && !clear_bit(TTY_SQ_THROTTLED, &tty->flags))
		tty->throttle(tty, TTY_THROTTLE_SQ_AVAIL);
}

int is_ignored(int sig)
{
	return ((current->blocked & (1<<(sig-1))) ||
	        (current->sigaction[sig-1].sa_handler == SIG_IGN));
}

static void wait_for_canon_input(struct tty_struct * tty)
{
	while (1) {
		TTY_READ_FLUSH(tty);
		if (tty->link)
			if (tty->link->count)
				TTY_WRITE_FLUSH(tty->link);
			else
				return;
		if (current->signal & ~current->blocked)
			return;
		if (FULL(&tty->read_q))
			return;
		if (tty->secondary.data)
			return;
		cli();
		if (!tty->secondary.data)
			interruptible_sleep_on(&tty->secondary.proc_list);
		sti();
	}
}

static int read_chan(unsigned int channel, struct file * file, char * buf, int nr)
{
	struct tty_struct * tty;
	int c;
	char * b=buf;
	int minimum,time;

	if (channel > 255)
		return -EIO;
	tty = TTY_TABLE(channel);
	if (!tty)
		return -EIO;
	if ((tty->pgrp > 0) &&
	    (current->tty == channel) &&
	    (tty->pgrp != current->pgrp))
		if (is_ignored(SIGTTIN) || is_orphaned_pgrp(current->pgrp))
			return -EIO;
		else {
			(void) kill_pg(current->pgrp, SIGTTIN, 1);
			return -ERESTARTSYS;
		}
	if (L_CANON(tty))
		minimum = time = current->timeout = 0;
	else {
		time = 10L*tty->termios->c_cc[VTIME];
		minimum = tty->termios->c_cc[VMIN];
		if (minimum)
			current->timeout = 0xffffffff;
		else {
			if (time)
				current->timeout = time + jiffies;
			else
				current->timeout = 0;
			time = 0;
			minimum = 1;
		}
	}
	if (file->f_flags & O_NONBLOCK)
		time = current->timeout = 0;
	else if (L_CANON(tty)) {
		wait_for_canon_input(tty);
		if (current->signal & ~current->blocked)
			return -ERESTARTSYS;
	}
	if (minimum>nr)
		minimum = nr;

	/* deal with packet mode:  First test for status change */
	if (tty->packet && tty->link && tty->link->status_changed) {
		put_fs_byte (tty->link->ctrl_status, b);
		tty->link->status_changed = 0;
		return 1;
	}
	  
	/* now bump the buffer up one. */
	if (tty->packet) {
		put_fs_byte (0,b++);
		nr--;
		/* this really shouldn't happen, but we need to 
		put it here. */
		if (nr == 0)
			return 1;
	}

	while (nr>0) {
		TTY_READ_FLUSH(tty);
		if (tty->link)
			TTY_WRITE_FLUSH(tty->link);
		while (nr > 0 && ((c = get_tty_queue(&tty->secondary)) >= 0)) {
			if ((EOF_CHAR(tty) != __DISABLED_CHAR &&
			     c==EOF_CHAR(tty)) || c==10)
				tty->secondary.data--;
			if ((EOF_CHAR(tty) != __DISABLED_CHAR &&
			     c==EOF_CHAR(tty)) && L_CANON(tty))
				break;
			put_fs_byte(c,b++);
			nr--;
			if (time)
				current->timeout = time+jiffies;
			if (c==10 && L_CANON(tty))
				break;
		};
		wake_up(&tty->read_q.proc_list);
		/*
		 * If there is enough space in the secondary queue
		 * now, let the low-level driver know.
		 */
		if (tty->throttle && (LEFT(&tty->secondary) >= SQ_THRESHOLD_HW)
		    && !clear_bit(TTY_SQ_THROTTLED, &tty->flags))
			tty->throttle(tty, TTY_THROTTLE_SQ_AVAIL);
		if (b-buf >= minimum || !current->timeout)
			break;
		if (current->signal & ~current->blocked) 
			break;
		if (tty->link && !tty->link->count)
			break;
		TTY_READ_FLUSH(tty);
		if (tty->link)
			TTY_WRITE_FLUSH(tty->link);
		cli();
		if (EMPTY(&tty->secondary))
			interruptible_sleep_on(&tty->secondary.proc_list);
		sti();
	}
	TTY_READ_FLUSH(tty);
	if (tty->link && tty->link->write)
		TTY_WRITE_FLUSH(tty->link);
	current->timeout = 0;

	/* packet mode sticks in an extra 0.  If that's all we've got,
	   we should count it a zero bytes. */
	if (tty->packet) {
		if ((b-buf) > 1)
			return b-buf;
	} else {
		if (b-buf)
			return b-buf;
	}

	if (current->signal & ~current->blocked)
		return -ERESTARTSYS;
	if (file->f_flags & O_NONBLOCK)
		return -EAGAIN;
	return 0;
}

static int write_chan(unsigned int channel, struct file * file, char * buf, int nr)
{
	struct tty_struct * tty;
	char c, *b=buf;

	if (channel > 255)
		return -EIO;
	if (redirect && ((channel == 0) || (channel+1 == fg_console)))
		tty = redirect;
	else
		tty = TTY_TABLE(channel);
	if (!tty || !tty->write)
		return -EIO;
	if (L_TOSTOP(tty) && (tty->pgrp > 0) &&
	    (current->tty == channel) && (tty->pgrp != current->pgrp)) {
		if (is_orphaned_pgrp(tty->pgrp))
			return -EIO;
		if (!is_ignored(SIGTTOU)) {
			(void) kill_pg(current->pgrp, SIGTTOU, 1);
			return -ERESTARTSYS;
		}
	}
	if (nr < 0)
		return -EINVAL;
	if (!nr)
		return 0;
	while (nr>0) {
		if (current->signal & ~current->blocked)
			break;
		if (tty->link && !tty->link->count) {
			send_sig(SIGPIPE,current,0);
			break;
		}
		if (FULL(&tty->write_q)) {
			TTY_WRITE_FLUSH(tty);
			cli();
			if (FULL(&tty->write_q))
				interruptible_sleep_on(&tty->write_q.proc_list);
			sti();
			continue;
		}
		while (nr>0 && !FULL(&tty->write_q)) {
			c=get_fs_byte(b);
			if (O_POST(tty)) {
				if (c=='\r' && O_CRNL(tty))
					c='\n';
				else if (c=='\n' && O_NLRET(tty))
					c='\r';
				if (c=='\n' && O_NLCR(tty) &&
				    !set_bit(TTY_CR_PENDING,&tty->flags)) {
					put_tty_queue(13,&tty->write_q);
					continue;
				}
				if (O_LCUC(tty))
					c=toupper(c);
			}
			b++; nr--;
			clear_bit(TTY_CR_PENDING,&tty->flags);
			put_tty_queue(c,&tty->write_q);
		}
		if (nr>0)
			schedule();
	}
	TTY_WRITE_FLUSH(tty);
	if (b-buf)
		return b-buf;
	if (tty->link && !tty->link->count)
		return -EPIPE;
	if (current->signal & ~current->blocked)
		return -ERESTARTSYS;
	return 0;
}

static int tty_read(struct inode * inode, struct file * file, char * buf, int count)
{
	int i;

	if (MAJOR(file->f_rdev) != 4) {
		printk("tty_read: pseudo-major != 4\n");
		return -EINVAL;
	}
	i = read_chan(MINOR(file->f_rdev),file,buf,count);
	if (i > 0)
		inode->i_atime = CURRENT_TIME;
	return i;
}

static int tty_write(struct inode * inode, struct file * file, char * buf, int count)
{
	int i;
	
	if (MAJOR(file->f_rdev) != 4) {
		printk("tty_write: pseudo-major != 4\n");
		return -EINVAL;
	}
	i = write_chan(MINOR(file->f_rdev),file,buf,count);
	if (i > 0)
		inode->i_mtime = CURRENT_TIME;
	return i;
}

static int tty_lseek(struct inode * inode, struct file * file, off_t offset, int orig)
{
	return -EBADF;
}

/*
 * tty_open and tty_release keep up the tty count that contains the
 * number of opens done on a tty. We cannot use the inode-count, as
 * different inodes might point to the same tty.
 *
 * Open-counting is needed for pty masters, as well as for keeping
 * track of serial lines: DTR is dropped when the last close happens.
 */
static int tty_open(struct inode * inode, struct file * filp)
{
	struct tty_struct *tty, *o_tty;
	int dev, retval;

	dev = inode->i_rdev;
	if (MAJOR(dev) == 5)
		dev = current->tty;
	else
		dev = MINOR(dev);
	if (dev < 0)
		return -ENODEV;
	filp->f_rdev = 0x0400 | dev;
/*
 * There be race-conditions here... Lots of them. Careful now.
 */
	tty = o_tty = NULL;
	if (!TTY_TABLE(dev)) {
		tty = (struct tty_struct *) get_free_page(GFP_KERNEL);
		if (tty) {
			retval = initialize_tty_struct(tty, TTY_TABLE_IDX(dev));
			if (retval) {
				free_page((unsigned long)tty);
				return retval;
			}
		}
	}
	if (IS_A_PTY(dev)) {
		if (!tty_table[PTY_OTHER(dev)]) {
			o_tty = (struct tty_struct *) get_free_page(GFP_KERNEL);
			if (o_tty) {
				retval = initialize_tty_struct(o_tty, PTY_OTHER(dev));
				if (retval) {
					free_page((unsigned long) tty);
					free_page((unsigned long) o_tty);
					return retval;
				}
			}
		}
		if (!o_tty && !tty_table[PTY_OTHER(dev)]) {
			free_page((unsigned long) tty);
			return -ENOMEM;
		}
	}
	if (TTY_TABLE(dev)) {
		free_page((unsigned long) tty);
		tty = TTY_TABLE(dev);
	} else if (tty)
		TTY_TABLE(dev) = tty;
	else {
		free_page((unsigned long) o_tty);
		return -ENOMEM;
	}
	if (IS_A_PTY(dev)) {
		if (tty_table[PTY_OTHER(dev)]) {
			free_page((unsigned long) o_tty);
			o_tty = tty_table[PTY_OTHER(dev)];
		} else
			tty_table[PTY_OTHER(dev)] = o_tty;
		tty->link = o_tty;
		o_tty->link = tty;
	}
	if (IS_A_PTY_MASTER(dev)) {
		if (tty->count)
			return -EAGAIN;
		if (tty->link)
			tty->link->count++;

		/* perhaps user applications that don't take care of
		   this deserve what the get, but I think my system
		   has hung do to this, esp. in X. -RAB */
		tty->termios->c_lflag &= ~ECHO;
	}
	tty->count++;
	retval = 0;

	/* clean up the packet stuff. */
	tty->status_changed = 0;
	tty->ctrl_status = 0;
	tty->packet = 0;

	if (!(filp->f_flags & O_NOCTTY) &&
	    current->leader &&
	    current->tty<0 &&
	    tty->session==0) {
		current->tty = dev;
		tty->session = current->session;
		tty->pgrp = current->pgrp;
	}
	if (tty->open)
		retval = tty->open(tty, filp);
	else
		retval = -ENODEV;
	if (retval) {
		tty->count--;
		if (IS_A_PTY_MASTER(dev) && tty->link)
			tty->link->count--;
	}
	return retval;
}

/*
 * Note that releasing a pty master also releases the child, so
 * we have to make the redirection checks after that and on both
 * sides of a pty.
 */
static void tty_release(struct inode * inode, struct file * filp)
{
	int dev;
	struct tty_struct * tty;

	dev = filp->f_rdev;
	if (MAJOR(dev) != 4) {
		printk("tty_release: tty pseudo-major != 4\n");
		return;
	}
	dev = MINOR(filp->f_rdev);
	tty = TTY_TABLE(dev);
	if (!tty) {
		printk("tty_release: TTY_TABLE(%d) was NULL\n", dev);
		return;
	}
	if (IS_A_PTY_MASTER(dev) && tty->link)  {
		if (--tty->link->count < 0) {
			printk("tty_release: bad tty slave count (dev = %d): %d\n",
			       dev, tty->count);	
			tty->link->count = 0;
		}
	}
	if (--tty->count < 0) {
		printk("tty_release: bad TTY_TABLE(%d)->count: %d\n",
		       dev, tty->count);
		tty->count = 0;
	}
	if (tty->count)
		return;
	if (tty->close)
		tty->close(tty, filp);
	if (tty == redirect)
		redirect = NULL;
	if (tty->link && !tty->link->count && (tty->link == redirect))
		redirect = NULL;
	if (tty->link) {
		if (tty->link->count)
			return;
		free_page((unsigned long) TTY_TABLE(PTY_OTHER(dev)));
		TTY_TABLE(PTY_OTHER(dev)) = 0;
	}
	free_page((unsigned long) TTY_TABLE(dev));
	TTY_TABLE(dev) = 0;
}

static int tty_select(struct inode * inode, struct file * filp, int sel_type, select_table * wait)
{
	int dev;
	struct tty_struct * tty;

	dev = filp->f_rdev;
	if (MAJOR(dev) != 4) {
		printk("tty_select: tty pseudo-major != 4\n");
		return 0;
	}
	dev = MINOR(filp->f_rdev);
	tty = TTY_TABLE(dev);
	if (!tty) {
		printk("tty_select: tty struct for dev %d was NULL\n", dev);
		return 0;
	}
	switch (sel_type) {
		case SEL_IN:
			if (!EMPTY(&tty->secondary))
				return 1;
			if (tty->link && !tty->link->count)
				return 1;

			/* see if the status byte can be read. */
			if (tty->packet && tty->link &&
			    tty->link->status_changed)
			  return 1;

			select_wait(&tty->secondary.proc_list, wait);
			return 0;
		case SEL_OUT:
			if (!FULL(&tty->write_q))
				return 1;
			select_wait(&tty->write_q.proc_list, wait);
			return 0;
		case SEL_EX:
			if (tty->link && !tty->link->count)
				return 1;
			return 0;
	}
	return 0;
}

static struct file_operations tty_fops = {
	tty_lseek,
	tty_read,
	tty_write,
	NULL,		/* tty_readdir */
	tty_select,
	tty_ioctl,
	NULL,		/* tty_mmap */
	tty_open,
	tty_release
};

/*
 * This implements the "Secure Attention Key" ---  the idea is to
 * prevent trojan horses by killing all processes associated with this
 * tty when the user hits the "Secure Attention Key".  Required for
 * super-paranoid applications --- see the Orange Book for more details.
 * 
 * This code could be nicer; ideally it should send a HUP, wait a few
 * seconds, then send a INT, and then a KILL signal.  But you then
 * have to coordinate with the init process, since all processes associated
 * with the current tty must be dead before the new getty is allowed
 * to spawn.
 */
void do_SAK( struct tty_struct *tty)
{
	struct task_struct **p;
	int line = tty->line;
	int session = tty->session;
	int		i;
	struct file	*filp;
	
	flush_input(tty);
	flush_output(tty);
 	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if (!(*p))
			continue;
		if (((*p)->tty == line) ||
		    ((session > 0) && ((*p)->session == session)))
			send_sig(SIGKILL, *p, 1);
		else {
			for (i=0; i < NR_FILE; i++) {
				filp = (*p)->filp[i];
				if (filp && (filp->f_op == &tty_fops) &&
				    (MINOR(filp->f_rdev) == line)) {
					send_sig(SIGKILL, *p, 1);
					break;
				}
			}
		}
	}
}

/*
 * This subroutine initializes a tty structure.  We have to set up
 * things correctly for each different type of tty.
 */
int initialize_tty_struct(struct tty_struct *tty, int line)
{
	struct termios *tp;
	
	memset(tty, 0, sizeof(struct tty_struct));
	tty->line = line;
	tty->pgrp = -1;
	tty->winsize.ws_row = 24;
	tty->winsize.ws_col = 80;
	if (!tty_termios[line]) {
		tp = tty_termios[line] = malloc(sizeof(struct termios));
		if (!tp)
			return -ENOMEM;
		memset(tp, 0, sizeof(struct termios));
		memcpy(tp->c_cc, INIT_C_CC, NCCS);
		if (IS_A_CONSOLE(line)) {
			tp->c_iflag = ICRNL | IXON;
			tp->c_oflag = OPOST | ONLCR;
			tp->c_cflag = B38400 | CS8 | CREAD;
			tp->c_lflag = ISIG | ICANON | ECHO | ECHOCTL | ECHOKE;
		} else if (IS_A_SERIAL(line)) {
			tp->c_cflag = B2400 | CS8 | CREAD | HUPCL;
		} else if (IS_A_PTY_MASTER(line)) {
			tp->c_cflag = B9600 | CS8 | CREAD;
		} else if (IS_A_PTY_SLAVE(line)) {
			tp->c_cflag = B9600 | CS8 | CREAD;
			tp->c_lflag = ISIG | ICANON;
		}
	}
	tty->termios = tty_termios[line];
	
	if (IS_A_CONSOLE(line)) {
		tty->open = con_open;
		tty->winsize.ws_row = video_num_lines;
		tty->winsize.ws_col = video_num_columns;
	} else if IS_A_SERIAL(line) {
		tty->open = rs_open;
	} else if IS_A_PTY(line) {
		tty->open = pty_open;
	}
	return 0;
}

long tty_init(long kmem_start)
{
	int i;

	chrdev_fops[4] = &tty_fops;
	chrdev_fops[5] = &tty_fops;
	for (i=0 ; i<256 ; i++) {
		tty_table[i] =  0;
		tty_termios[i] = 0;
	}
	kmem_start = con_init(kmem_start);
	kmem_start = rs_init(kmem_start);
	printk("%d virtual consoles\n\r",NR_CONSOLES);
	return kmem_start;
}
