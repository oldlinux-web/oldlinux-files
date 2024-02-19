/*
 * This file contains the procedures for the handling of select
 *
 * Created for Linux based loosely upon Mathius Lattner's minix
 * patches by Peter MacDonald. Heavily edited by Linus.
 */

#include <linux/types.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/signal.h>
#include <linux/errno.h>

#include <asm/segment.h>
#include <asm/system.h>

/*
 * Ok, Peter made a complicated, but straightforward multiple_wait() function.
 * I have rewritten this, taking some shortcuts: This code may not be easy to
 * follow, but it should be free of race-conditions, and it's practical. If you
 * understand what I'm doing here, then you understand how the linux sleep/wakeup
 * mechanism works.
 *
 * Two very simple procedures, select_wait() and free_wait() make all the work.
 * select_wait() is a inline-function defined in <linux/fs.h>, as all select
 * functions have to call it to add an entry to the select table.
 */

static void free_wait(select_table * p)
{
	struct select_table_entry * entry = p->entry + p->nr;

	while (p->nr > 0) {
		p->nr--;
		entry--;
		remove_wait_queue(entry->wait_address,&entry->wait);
	}
}

/*
 * The check_XX functions check out a file. We know it's either
 * a pipe, a character device or a fifo
 */
static int check_in(select_table * wait, struct inode * inode, struct file * file)
{
	if (file->f_op && file->f_op->select)
		return file->f_op->select(inode,file,SEL_IN,wait);
	if (inode && S_ISREG(inode->i_mode))
		return 1;
	return 0;
}

static int check_out(select_table * wait, struct inode * inode, struct file * file)
{
	if (file->f_op && file->f_op->select)
		return file->f_op->select(inode,file,SEL_OUT,wait);
	if (inode && S_ISREG(inode->i_mode))
		return 1;
	return 0;
}

static int check_ex(select_table * wait, struct inode * inode, struct file * file)
{
	if (file->f_op && file->f_op->select)
		return file->f_op->select(inode,file,SEL_EX,wait);
	if (inode && S_ISREG(inode->i_mode))
		return 1;
	return 0;
}

int do_select(fd_set in, fd_set out, fd_set ex,
	fd_set *inp, fd_set *outp, fd_set *exp)
{
	int count;
	select_table wait_table;
	struct file * file;
	int i;
	fd_set mask;

	mask = in | out | ex;
	for (i = 0 ; i < NR_OPEN ; i++,mask >>= 1) {
		if (!(mask & 1))
			continue;
		if (!current->filp[i])
			return -EBADF;
		if (!current->filp[i]->f_inode)
			return -EBADF;
	}
repeat:
	wait_table.nr = 0;
	*inp = *outp = *exp = 0;
	count = 0;
	current->state = TASK_INTERRUPTIBLE;
	mask = 1;
	for (i = 0 ; i < NR_OPEN ; i++, mask += mask) {
		file = current->filp[i];
		if (mask & in)
			if (check_in(&wait_table,file->f_inode,file)) {
				*inp |= mask;
				count++;
			}
		if (mask & out)
			if (check_out(&wait_table,file->f_inode,file)) {
				*outp |= mask;
				count++;
			}
		if (mask & ex)
			if (check_ex(&wait_table,file->f_inode,file)) {
				*exp |= mask;
				count++;
			}
	}
	if (!(current->signal & ~current->blocked) &&
	    current->timeout && !count) {
		schedule();
		free_wait(&wait_table);
		goto repeat;
	}
	free_wait(&wait_table);
	current->state = TASK_RUNNING;
	return count;
}

/*
 * We can actually return ERESTARTSYS insetad of EINTR, but I'd
 * like to be certain this leads to no problems. So I return
 * EINTR just for safety.
 */
int sys_select( unsigned long *buffer )
{
/* Perform the select(nd, in, out, ex, tv) system call. */
	int i;
	fd_set res_in, in = 0, *inp;
	fd_set res_out, out = 0, *outp;
	fd_set res_ex, ex = 0, *exp;
	fd_set mask;
	struct timeval *tvp;
	unsigned long timeout;

	mask = get_fs_long(buffer++);
	if (mask >= 32)
		mask = ~0;
	else
		mask = ~((~0) << mask);
	inp = (fd_set *) get_fs_long(buffer++);
	outp = (fd_set *) get_fs_long(buffer++);
	exp = (fd_set *) get_fs_long(buffer++);
	tvp = (struct timeval *) get_fs_long(buffer);

	if (inp)
		in = mask & get_fs_long(inp);
	if (outp)
		out = mask & get_fs_long(outp);
	if (exp)
		ex = mask & get_fs_long(exp);
	timeout = 0xffffffff;
	if (tvp) {
		timeout = get_fs_long((unsigned long *)&tvp->tv_usec)/(1000000/HZ);
		timeout += get_fs_long((unsigned long *)&tvp->tv_sec) * HZ;
		timeout += jiffies;
	}
	current->timeout = timeout;
	i = do_select(in, out, ex, &res_in, &res_out, &res_ex);
	if (current->timeout > jiffies)
		timeout = current->timeout - jiffies;
	else
		timeout = 0;
	current->timeout = 0;
	if (tvp) {
		verify_area(tvp, sizeof(*tvp));
		put_fs_long(timeout/HZ, (unsigned long *) &tvp->tv_sec);
		timeout %= HZ;
		timeout *= (1000000/HZ);
		put_fs_long(timeout, (unsigned long *) &tvp->tv_usec);
	}
	if (i < 0)
		return i;
	if (!i && (current->signal & ~current->blocked))
		return -EINTR;
	if (inp) {
		verify_area(inp, 4);
		put_fs_long(res_in,inp);
	}
	if (outp) {
		verify_area(outp,4);
		put_fs_long(res_out,outp);
	}
	if (exp) {
		verify_area(exp,4);
		put_fs_long(res_ex,exp);
	}
	return i;
}
