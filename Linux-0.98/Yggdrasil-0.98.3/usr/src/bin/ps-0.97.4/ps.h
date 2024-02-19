/*
 * ps.h
 *
 * Copyright (c) 1992 Branko Lankester
 *
 */


#include <linux/sched.h>

extern int show_env;	/* -e flag */
extern int kern_comm;	/* -c flag */
extern int maxcmd;

extern char *swappath;
extern char *kmem_path;

#define	MAXCMD	512	/* max # bytes to copy from args/env */

#define	PZERO	15	/* priority of init (in sched.h) */

#define	PAGE_MASK	0xfff

#define	KSTK_EIP(task)	(*((unsigned long *)(task)+1019))
#define	KSTK_ESP(task)	(*((unsigned long *)(task)+1022))

#define	_SSIZE(task)	(TASK_SIZE - KSTK_ESP(task))
#define	SSIZE(task)	(KSTK_ESP(task) ? _SSIZE(task) : 0)
#define	VSIZE(task)	(((task)->brk + 1023 + SSIZE(task)) / 1024)
#define	SIZE(task)	(((task)->brk - (task)->end_code + 1023 + \
			  SSIZE(task)) / 1024)

typedef unsigned reg_t;

union task_union {
    struct task_struct task;
    reg_t stack[PAGE_SIZE/4];
};


char *find_func();
unsigned long k_addr();
unsigned long get_kword();
char *cmd_args();
char *dev_to_tty();
char *wchan();
char *status();
char *xmalloc();
