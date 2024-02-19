/*
 *  linux/fs/fifo.c
 *
 *  written by Paul H. Hargrove
 */

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fcntl.h>

extern struct file_operations read_pipe_fops;
extern struct file_operations write_pipe_fops;
extern struct file_operations rdwr_pipe_fops;

static int fifo_open(struct inode * inode,struct file * filp)
{
	int retval = 0;
	unsigned long page;

	switch( filp->f_mode ) {

	case 1:
	/*
	 *  O_RDONLY
	 *  POSIX.1 says that O_NONBLOCK means return with the FIFO
	 *  opened, even when there is no process writing the FIFO.
	 */
		filp->f_op = &read_pipe_fops;
		PIPE_READERS(*inode)++;
		if (!(filp->f_flags & O_NONBLOCK))
			while (!PIPE_WRITERS(*inode)) {
				if (PIPE_HEAD(*inode) != PIPE_TAIL(*inode))
					break;
				if (current->signal & ~current->blocked) {
					retval = -ERESTARTSYS;
					break;
				}
				interruptible_sleep_on(&PIPE_READ_WAIT(*inode));
			}
		if (retval)
			PIPE_READERS(*inode)--;
		break;
	
	case 2:
	/*
	 *  O_WRONLY
	 *  POSIX.1 says that O_NONBLOCK means return -1 with
	 *  errno=ENXIO when there is no process reading the FIFO.
	 */
		if ((filp->f_flags & O_NONBLOCK) && !PIPE_READERS(*inode)) {
			retval = -ENXIO;
			break;
		}
		filp->f_op = &write_pipe_fops;
		PIPE_WRITERS(*inode)++;
		while (!PIPE_READERS(*inode)) {
			if (current->signal & ~current->blocked) {
				retval = -ERESTARTSYS;
				break;
			}
			interruptible_sleep_on(&PIPE_WRITE_WAIT(*inode));
		}
		if (retval)
			PIPE_WRITERS(*inode)--;
		break;
	
	case 3:
	/*
	 *  O_RDWR
	 *  POSIX.1 leaves this case "undefined" when O_NONBLOCK is set.
	 *  This implementation will NEVER block on a O_RDWR open, since
	 *  the process can at least talk to itself.
	 */
		filp->f_op = &rdwr_pipe_fops;
		PIPE_WRITERS(*inode) += 1;
		PIPE_READERS(*inode) += 1;
		break;

	default:
		retval = -EINVAL;
	}
	if (PIPE_WRITERS(*inode))
		wake_up(&PIPE_READ_WAIT(*inode));
	if (PIPE_READERS(*inode))
		wake_up(&PIPE_WRITE_WAIT(*inode));
	if (retval || PIPE_BASE(*inode))
		return retval;
	page = get_free_page(GFP_KERNEL);
	if (PIPE_BASE(*inode)) {
		free_page(page);
		return 0;
	}
	if (!page)
		return -ENOMEM;
	PIPE_BASE(*inode) = (char *) page;
	return 0;
}

/*
 * Dummy default file-operations: the only thing this does
 * is contain the open that then fills in the correct operations
 * depending on the access mode of the file...
 */
struct file_operations def_fifo_fops = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	fifo_open,		/* will set read or write pipe_fops */
	NULL
};
