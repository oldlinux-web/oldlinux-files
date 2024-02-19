#ifndef _LINUX_PIPE_FS_I_H
#define _LINUX_PIPE_FS_I_H

struct pipe_inode_info {
	struct wait_queue * read_wait;
	struct wait_queue * write_wait;
	char * base;
	unsigned int head;
	unsigned int tail;
	unsigned int readers;
	unsigned int writers;
};

#define PIPE_READ_WAIT(inode)	((inode).u.pipe_i.read_wait)
#define PIPE_WRITE_WAIT(inode)	((inode).u.pipe_i.write_wait)
#define PIPE_BASE(inode)	((inode).u.pipe_i.base)
#define PIPE_HEAD(inode)	((inode).u.pipe_i.head)
#define PIPE_TAIL(inode)	((inode).u.pipe_i.tail)
#define PIPE_READERS(inode)	((inode).u.pipe_i.readers)
#define PIPE_WRITERS(inode)	((inode).u.pipe_i.writers)
#define PIPE_SIZE(inode)	((PIPE_HEAD(inode)-PIPE_TAIL(inode))&(PAGE_SIZE-1))
#define PIPE_EMPTY(inode)	(PIPE_HEAD(inode)==PIPE_TAIL(inode))
#define PIPE_FULL(inode)	(PIPE_SIZE(inode)==(PAGE_SIZE-1))

#endif
