#ifndef FS_H
#define FS_H

#include <cnix/sched.h>

#define NR_HASH 50
#define NR_BUF  ((160*0x1000 - 10*0x1000)/0x400)
#define BLOCKSIZ  0x400
#define NULL '\0'
#define NODEV 0
#define WRITE 0
#define READ 1

struct buf_head{
	int b_dev;        
	int b_blocknr;
	short int b_flags;     /* buf state */
	struct task_struct *b_wait;
	unsigned char *b_data;  /* point to the disk data in memory */
	struct buf_head *b_next;
	struct buf_head *b_prev;
	struct buf_head *b_next_free;
	struct buf_head *b_prev_free;
};

#define B_WRITE 0
#define B_READ  01
/* The buf is not in free list , only one proc can  use it */
#define B_BUSY  02    
/* delay write , The buf is different from disk ,so if release it ,you must write to disk */ 
#define B_DELWRI 04  

/* disk io complete , the buf contains validated data */ 
#define B_DONE   010 

#define B_WANTED  020  /* another proc want this buf , but it is busy , so the proc must sleep */

#define B_ERROR  040  /* the disk i/o has some errors , data can not be retrieve */

#define B_ASY   0100     /* asynchronize write or read ,not wait the io complicaton */

struct dinode {
	int d_dev;
	int d_inum;
	int d_imode;  /* which type of this file */
	unsigned char d_ilock;  /* d_ilock == 1 locked , d_ilock == 0  free */
};


extern struct buf_head * bread(int ,int);
extern void bwrite(struct buf_head *);
extern void iodone(struct buf_head *);

#endif
