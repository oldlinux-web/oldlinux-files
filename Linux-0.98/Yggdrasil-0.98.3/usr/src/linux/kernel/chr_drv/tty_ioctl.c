/*
 *  linux/kernel/chr_drv/tty_ioctl.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/types.h>
#include <linux/termios.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <linux/fcntl.h>

#include <asm/io.h>
#include <asm/segment.h>
#include <asm/system.h>

extern int session_of_pgrp(int pgrp);
extern int do_screendump(int arg);
extern int kill_pg(int pgrp, int sig, int priv);

static void flush(struct tty_queue * queue)
{
	if (queue) {
		cli();
		queue->head = queue->tail;
		sti();
		wake_up(&queue->proc_list);
	}
}

void flush_input(struct tty_struct * tty)
{
        tty->status_changed = 1;
	tty->ctrl_status |= TIOCPKT_FLUSHREAD;
	flush(&tty->read_q);
	wake_up(&tty->read_q.proc_list);
	flush(&tty->secondary);
	tty->secondary.data = 0;

	if (tty = tty->link) {
		flush(&tty->write_q);
		wake_up(&tty->write_q.proc_list);
	}
}

void flush_output(struct tty_struct * tty)
{
   	tty->status_changed = 1;
	tty->ctrl_status |= TIOCPKT_FLUSHWRITE;
	flush(&tty->write_q);
	wake_up(&tty->write_q.proc_list);
	if (tty = tty->link) {
		flush(&tty->read_q);
		wake_up(&tty->read_q.proc_list);
		flush(&tty->secondary);
		tty->secondary.data = 0;
	}
}

void wait_until_sent(struct tty_struct * tty)
{
	while (!(current->signal & ~current->blocked) &&
	       !EMPTY(&tty->write_q)) {
		TTY_WRITE_FLUSH(tty);
		current->counter = 0;
		cli();
		if (EMPTY(&tty->write_q))
			break;
		else
			interruptible_sleep_on(&tty->write_q.proc_list);
		sti();
	}
	sti();
}

static int do_get_ps_info(int arg)
{
	struct tstruct {
		int flag;
		int present[NR_TASKS];
		struct task_struct tasks[NR_TASKS];
	};
	struct tstruct *ts = (struct tstruct *)arg;
	struct task_struct **p;
	char *c, *d;
	int i, n = 0;
	
	verify_area((void *)arg, sizeof(struct tstruct));
		
	for (p = &FIRST_TASK ; p <= &LAST_TASK ; p++, n++)
		if (*p)
		{
			c = (char *)(*p);
			d = (char *)(ts->tasks+n);
			for (i=0 ; i<sizeof(struct task_struct) ; i++)
				put_fs_byte(*c++, d++);
			put_fs_long(1, (unsigned long *)(ts->present+n));
		}
		else	
			put_fs_long(0, (unsigned long *)(ts->present+n));
	return(0);			
}

static int get_termios(struct tty_struct * tty, struct termios * termios)
{
	int i;

	verify_area(termios, sizeof (*termios));
	for (i=0 ; i< (sizeof (*termios)) ; i++)
		put_fs_byte( ((char *)tty->termios)[i] , i+(char *)termios );
	return 0;
}

static int set_termios(struct tty_struct * tty, struct termios * termios,
			int channel)
{
	int i;
	unsigned short old_cflag = tty->termios->c_cflag;

	/* If we try to set the state of terminal and we're not in the
	   foreground, send a SIGTTOU.  If the signal is blocked or
	   ignored, go ahead and perform the operation.  POSIX 7.2) */
	if ((current->tty == channel) &&
	     (tty->pgrp != current->pgrp)) {
		if (is_orphaned_pgrp(current->pgrp))
			return -EIO;
		if (!is_ignored(SIGTTOU)) {
			(void) kill_pg(current->pgrp,SIGTTOU,1);
			return -ERESTARTSYS;
		}
	}
	for (i=0 ; i< (sizeof (*termios)) ; i++)
		((char *)tty->termios)[i]=get_fs_byte(i+(char *)termios);
	if (IS_A_SERIAL(channel) && tty->termios->c_cflag != old_cflag)
		change_speed(channel-64);

	/* puting mpty's into echo mode is very bad, and I think under
	   some situations can cause the kernel to do nothing but
	   copy characters back and forth. -RAB */
	if (IS_A_PTY_MASTER(channel)) tty->termios->c_lflag &= ~ECHO;

	return 0;
}

static int get_termio(struct tty_struct * tty, struct termio * termio)
{
	int i;
	struct termio tmp_termio;

	verify_area(termio, sizeof (*termio));
	tmp_termio.c_iflag = tty->termios->c_iflag;
	tmp_termio.c_oflag = tty->termios->c_oflag;
	tmp_termio.c_cflag = tty->termios->c_cflag;
	tmp_termio.c_lflag = tty->termios->c_lflag;
	tmp_termio.c_line = tty->termios->c_line;
	for(i=0 ; i < NCC ; i++)
		tmp_termio.c_cc[i] = tty->termios->c_cc[i];
	for (i=0 ; i< (sizeof (*termio)) ; i++)
		put_fs_byte( ((char *)&tmp_termio)[i] , i+(char *)termio );
	return 0;
}

/*
 * This only works as the 386 is low-byte-first
 */
static int set_termio(struct tty_struct * tty, struct termio * termio,
			int channel)
{
	int i;
	struct termio tmp_termio;
	unsigned short old_cflag = tty->termios->c_cflag;

	if ((current->tty == channel) &&
	    (tty->pgrp > 0) &&
	    (tty->pgrp != current->pgrp)) {
		if (is_orphaned_pgrp(current->pgrp))
			return -EIO;
		if (!is_ignored(SIGTTOU)) {
			(void) kill_pg(current->pgrp,SIGTTOU,1);
			return -ERESTARTSYS;
		}
	}
	for (i=0 ; i< (sizeof (*termio)) ; i++)
		((char *)&tmp_termio)[i]=get_fs_byte(i+(char *)termio);

	/* take care of the packet stuff. */
	if ((tmp_termio.c_iflag & IXON) &&
	    ~(tty->termios->c_iflag & IXON))
	  {
	     tty->status_changed = 1;
	     tty->ctrl_status |= TIOCPKT_DOSTOP;
	  }

	if (~(tmp_termio.c_iflag & IXON) &&
	    (tty->termios->c_iflag & IXON))
	  {
	     tty->status_changed = 1;
	     tty->ctrl_status |= TIOCPKT_NOSTOP;
	  }

	*(unsigned short *)&tty->termios->c_iflag = tmp_termio.c_iflag;
	*(unsigned short *)&tty->termios->c_oflag = tmp_termio.c_oflag;
	*(unsigned short *)&tty->termios->c_cflag = tmp_termio.c_cflag;
	*(unsigned short *)&tty->termios->c_lflag = tmp_termio.c_lflag;
	tty->termios->c_line = tmp_termio.c_line;
	for(i=0 ; i < NCC ; i++)
		tty->termios->c_cc[i] = tmp_termio.c_cc[i];
	if (IS_A_SERIAL(channel) && tty->termios->c_cflag != old_cflag)
		change_speed(channel-64);
	return 0;
}

static int set_window_size(struct tty_struct * tty, struct winsize * ws)
{
	int i,changed;
	char c, * tmp;

	if (!ws)
		return -EINVAL;
	tmp = (char *) &tty->winsize;
	changed = 0;
	for (i = 0; i < sizeof (*ws) ; i++,tmp++) {
		c = get_fs_byte(i + (char *) ws);
		if (c == *tmp)
			continue;
		changed = 1;
		*tmp = c;
	}
	if (changed)
		kill_pg(tty->pgrp, SIGWINCH, 1);
	return 0;
}

static int get_window_size(struct tty_struct * tty, struct winsize * ws)
{
	int i;
	char * tmp;

	if (!ws)
		return -EINVAL;
	verify_area(ws, sizeof (*ws));
	tmp = (char *) ws;
	for (i = 0; i < sizeof (struct winsize) ; i++,tmp++)
		put_fs_byte(((char *) &tty->winsize)[i], tmp);
	return 0;
}

int tty_ioctl(struct inode * inode, struct file * file,
	unsigned int cmd, unsigned int arg)
{
	struct tty_struct * tty;
	struct tty_struct * other_tty;
	int pgrp;
	int dev;

	if (MAJOR(file->f_rdev) != 4) {
		printk("tty_ioctl: tty pseudo-major != 4\n");
		return -EINVAL;
	}
	dev = MINOR(file->f_rdev);
	tty = TTY_TABLE(dev);
	if (!tty)
		return -EINVAL;

	if (IS_A_PTY(dev))
		other_tty = tty_table[PTY_OTHER(dev)];
	else
		other_tty = NULL;
		
	switch (cmd) {
		case TCGETS:
			return get_termios(tty,(struct termios *) arg);
		case TCSETSF:
			flush_input(tty);
		/* fallthrough */
		case TCSETSW:
			wait_until_sent(tty);
		/* fallthrough */
		case TCSETS:
			return set_termios(tty,(struct termios *) arg, dev);
		case TCGETA:
			return get_termio(tty,(struct termio *) arg);
		case TCSETAF:
			flush_input(tty);
		/* fallthrough */
		case TCSETAW:
			wait_until_sent(tty); /* fallthrough */
		case TCSETA:
			return set_termio(tty,(struct termio *) arg, dev);
		case TCXONC:
			switch (arg) {
			case TCOOFF:
				tty->stopped = 1;
				TTY_WRITE_FLUSH(tty);
				return 0;
			case TCOON:
				tty->stopped = 0;
				TTY_WRITE_FLUSH(tty);
				return 0;
			case TCIOFF:
				if (STOP_CHAR(tty))
					put_tty_queue(STOP_CHAR(tty),
						      &tty->write_q);
				return 0;
			case TCION:
				if (START_CHAR(tty))
					put_tty_queue(START_CHAR(tty),
						      &tty->write_q);
				return 0;
			}
			return -EINVAL; /* not implemented */
		case TCFLSH:
			if (arg==0)
				flush_input(tty);
			else if (arg==1)
				flush_output(tty);
			else if (arg==2) {
				flush_input(tty);
				flush_output(tty);
			} else
				return -EINVAL;
			return 0;
		case TIOCEXCL:
			return -EINVAL; /* not implemented */
		case TIOCNXCL:
			return -EINVAL; /* not implemented */
		case TIOCSCTTY:
			return -EINVAL; /* set controlling term NI */
		case TIOCGPGRP:
			verify_area((void *) arg,4);
			put_fs_long(tty->pgrp,(unsigned long *) arg);
			return 0;
		case TIOCSPGRP:
			if ((current->tty < 0) ||
			    (current->tty != dev) ||
			    (tty->session != current->session))
				return -ENOTTY;
			pgrp=get_fs_long((unsigned long *) arg);
			if (pgrp < 0)
				return -EINVAL;
			if (session_of_pgrp(pgrp) != current->session)
				return -EPERM;
			tty->pgrp = pgrp;			
			return 0;
		case TIOCOUTQ:
			verify_area((void *) arg,4);
			put_fs_long(CHARS(&tty->write_q),
				    (unsigned long *) arg);
			return 0;
		case TIOCINQ:
			verify_area((void *) arg,4);
			if (L_CANON(tty) && !tty->secondary.data)
				put_fs_long(0, (unsigned long *) arg);
			else
				put_fs_long(CHARS(&tty->secondary),
					(unsigned long *) arg);
			return 0;
		case TIOCSTI:
			return -EINVAL; /* not implemented */
		case TIOCGWINSZ:
			return get_window_size(tty,(struct winsize *) arg);
		case TIOCSWINSZ:
			if (IS_A_PTY_MASTER(dev))
				set_window_size(other_tty,(struct winsize *) arg);
			return set_window_size(tty,(struct winsize *) arg);
		case TIOCGSOFTCAR:
			return -EINVAL; /* not implemented */
		case TIOCSSOFTCAR:
			return -EINVAL; /* not implemented */
		case TIOCLINUX:
			switch (get_fs_byte((char *)arg))
			{
				case 0: 
					return do_screendump(arg);
				case 1: 
					return do_get_ps_info(arg);
				default: 
					return -EINVAL;
			}
		case TIOCCONS:
			if (!IS_A_PTY(dev))
				return -EINVAL;
			if (redirect)
				return -EBUSY;
			if (!suser())
				return -EPERM;
			if (IS_A_PTY_MASTER(dev))
				redirect = other_tty;
			else
				redirect = tty;
			return 0;
		case FIONBIO:
			arg = get_fs_long((unsigned long *) arg);
			if (arg)
				file->f_flags |= O_NONBLOCK;
			else
				file->f_flags &= ~O_NONBLOCK;
			return 0;
		case TIOCNOTTY:
			if (MINOR(file->f_rdev) != current->tty)
				return -EINVAL;
			current->tty = -1;
			if (current->leader) {
				if (tty->pgrp > 0)
					kill_pg(tty->pgrp, SIGHUP, 0);
				tty->pgrp = -1;
				tty->session = 0;
			}
			return 0;

	       case TIOCPKT:
			{
			   int on;
			   if (!IS_A_PTY_MASTER(dev))
			     return (-EINVAL);
			   verify_area ((unsigned long *)arg, sizeof (int));
			   on=get_fs_long ((unsigned long *)arg);
			   if (on )
			     tty->packet = 1;
			   else
			     tty->packet = 0;
			   return (0);
			}

		default:
			if (tty->ioctl)
				return (tty->ioctl)(tty, file, cmd, arg);
			else
				return -EINVAL;
	}
}
