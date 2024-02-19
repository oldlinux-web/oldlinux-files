#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H

#define HZ 100

/*
 * This is the maximum nr of tasks - change it if you need to
 */
#define NR_TASKS	64

/*
 * User space process size: 3GB. This is hardcoded into a few places,
 * so don't change it unless you know what you are doing.
 */
#define TASK_SIZE	0xc0000000

/*
 * Size of io_bitmap in longwords: 32 is ports 0-0x3ff.
 */
#define IO_BITMAP_SIZE	32

/*
 * These are the constant used to fake the fixed-point load-average
 * counting. Some notes:
 *  - 11 bit fractions expand to 22 bits by the multiplies: this gives
 *    a load-average precision of 10 bits integer + 11 bits fractional
 *  - if you want to count load-averages more often, you need more
 *    precision, or rounding will get you. With 2-second counting freq,
 *    the EXP_n values would be 1981, 2034 and 2043 if still using only
 *    11 bit fractions.
 */
#define FSHIFT		11		/* nr of bits of precision */
#define FIXED_1		(1<<FSHIFT)	/* 1.0 as fixed-point */
#define LOAD_FREQ	(5*HZ)		/* 5 sec intervals */
#define EXP_1		1884		/* 1/exp(5sec/1min) as fixed-point */
#define EXP_5		2014		/* 1/exp(5sec/5min) */
#define EXP_15		2037		/* 1/exp(5sec/15min) */

#define CALC_LOAD(load,exp,n) \
	load *= exp; \
	load += n*(FIXED_1-exp); \
	load >>= FSHIFT;

#define CT_TO_SECS(x)	((x) / HZ)
#define CT_TO_USECS(x)	(((x) % HZ) * 1000000/HZ)

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]

#include <linux/head.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/signal.h>
#include <linux/time.h>
#include <linux/param.h>
#include <linux/resource.h>
#include <linux/vm86.h>

#if (NR_OPEN > 32)
#error "Currently the close-on-exec-flags and select masks are in one long, max 32 files/proc"
#endif

#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		3
#define TASK_STOPPED		4

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define MAX_SHARED_LIBS 6

extern void sched_init(void);
extern void show_state(void);
extern void schedule(void);
extern void trap_init(void);
extern void panic(const char * str);

typedef int (*fn_ptr)();

union i387_union {
	struct i387_hard_struct {
		long	cwd;
		long	swd;
		long	twd;
		long	fip;
		long	fcs;
		long	foo;
		long	fos;
		long	st_space[20];	/* 8*10 bytes for each FP-reg = 80 bytes */
	} hard;
	struct i387_soft_struct {
		long	cwd;
		long	swd;
		long	twd;
		long	fip;
		long	fcs;
		long	foo;
		long	fos;
		long    top;
		long	regs_space[32];	/* 8*16 bytes for each FP-reg = 112 bytes */
	} soft;
};

struct tss_struct {
	unsigned long	back_link;	/* 16 high bits zero */
	unsigned long	esp0;
	unsigned long	ss0;		/* 16 high bits zero */
	unsigned long	esp1;
	unsigned long	ss1;		/* 16 high bits zero */
	unsigned long	esp2;
	unsigned long	ss2;		/* 16 high bits zero */
	unsigned long	cr3;
	unsigned long	eip;
	unsigned long	eflags;
	unsigned long	eax,ecx,edx,ebx;
	unsigned long	esp;
	unsigned long	ebp;
	unsigned long	esi;
	unsigned long	edi;
	unsigned long	es;		/* 16 high bits zero */
	unsigned long	cs;		/* 16 high bits zero */
	unsigned long	ss;		/* 16 high bits zero */
	unsigned long	ds;		/* 16 high bits zero */
	unsigned long	fs;		/* 16 high bits zero */
	unsigned long	gs;		/* 16 high bits zero */
	unsigned long	ldt;		/* 16 high bits zero */
	unsigned long	trace_bitmap;	/* bits: trace 0, bitmap 16-31 */
	unsigned long	io_bitmap[IO_BITMAP_SIZE];
	union i387_union i387;
};

struct task_struct {
/* these are hardcoded - don't touch */
	long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
	long counter;
	long priority;
	long signal;
	struct sigaction sigaction[32];
	long blocked;	/* bitmap of masked signals */
	unsigned long saved_kernel_stack;
/* various fields */
	int exit_code;
	int dumpable:1;
	int swappable:1;
	unsigned long start_code,end_code,end_data,brk,start_stack;
	long pid,pgrp,session,leader;
	int	groups[NGROUPS];
	/* 
	 * pointers to (original) parent process, youngest child, younger sibling,
	 * older sibling, respectively.  (p->father can be replaced with 
	 * p->p_pptr->pid)
	 */
	struct task_struct *p_opptr,*p_pptr, *p_cptr, *p_ysptr, *p_osptr;
	/*
	 * For ease of programming... Normal sleeps don't need to
	 * keep track of a wait-queue: every task has an entry of it's own
	 */
	struct wait_queue wait;
	unsigned short uid,euid,suid;
	unsigned short gid,egid,sgid;
	unsigned long timeout;
	unsigned long it_real_value, it_prof_value, it_virt_value;
	unsigned long it_real_incr, it_prof_incr, it_virt_incr;
	long utime,stime,cutime,cstime,start_time;
	unsigned long min_flt, maj_flt;
	unsigned long cmin_flt, cmaj_flt;
	struct rlimit rlim[RLIM_NLIMITS]; 
	unsigned int flags;	/* per process flags, defined below */
	unsigned short used_math;
	unsigned short rss;	/* number of resident pages */
	char comm[8];
	struct vm86_struct * vm86_info;
	unsigned long screen_bitmap;
/* file system info */
	int link_count;
	int tty;		/* -1 if no tty, so it must be signed */
	unsigned short umask;
	struct inode * pwd;
	struct inode * root;
	struct inode * executable;
	struct vm_area_struct * mmap;
	struct {
		struct inode * library;
		unsigned long start;
		unsigned long length;
		unsigned long bss;
	} libraries[MAX_SHARED_LIBS];
	int numlibraries;
	struct file * filp[NR_OPEN];
	unsigned long close_on_exec;
/* ldt for this task 0 - zero 1 - cs 2 - ds&ss */
	struct desc_struct ldt[3];
/* tss for this task */
	struct tss_struct tss;
};

/*
 * Per process flags
 */
#define PF_ALIGNWARN	0x00000001	/* Print alignment warning msgs */
					/* Not implemented yet, only for 486*/
#define PF_PTRACED	0x00000010	/* set if ptrace (0) has been called. */

/*
 *  INIT_TASK is used to set up the first task table, touch at
 * your own risk!. Base=0, limit=0x9ffff (=640kB)
 */
#define INIT_TASK \
/* state etc */	{ 0,15,15, \
/* signals */	0,{{},},0,0, \
/* ec,brk... */	0,0,0,0,0,0,0,0, \
/* pid etc.. */	0,0,0,0, \
/* suppl grps*/ {NOGROUP,}, \
/* proc links*/ &init_task.task,&init_task.task,NULL,NULL,NULL, \
/* wait queue*/ {&init_task.task,NULL}, \
/* uid etc */	0,0,0,0,0,0, \
/* timeout */	0,0,0,0,0,0,0,0,0,0,0,0, \
/* min_flt */	0,0,0,0, \
/* rlimits */   { {0x7fffffff, 0x7fffffff}, {0x7fffffff, 0x7fffffff},  \
		  {0x7fffffff, 0x7fffffff}, {0x7fffffff, 0x7fffffff}, \
		  {0x7fffffff, 0x7fffffff}, {0x7fffffff, 0x7fffffff}}, \
/* flags */	0, \
/* math */	0, \
/* rss */	2, \
/* comm */	"swapper", \
/* vm86_info */	NULL, 0, \
/* fs info */	0,-1,0022,NULL,NULL,NULL,NULL, \
/* libraries */	{ { NULL, 0, 0}, }, 0, \
/* filp */	{NULL,}, 0, \
		{ \
			{0,0}, \
/* ldt */		{0x9f,0xc0c0fa00}, \
			{0x9f,0xc0c0f200} \
		}, \
/*tss*/	{0,PAGE_SIZE+(long)&init_task,0x10,0,0,0,0,(long)&swapper_pg_dir,\
	 0,0,0,0,0,0,0,0, \
	 0,0,0x17,0x17,0x17,0x17,0x17,0x17, \
	 _LDT(0),0x80000000,{0xffffffff}, \
		{ { 0, } } \
	}, \
}

extern struct task_struct *task[NR_TASKS];
extern struct task_struct *last_task_used_math;
extern struct task_struct *current;
extern unsigned long volatile jiffies;
extern unsigned long startup_time;
extern int jiffies_offset;
extern int need_resched;
extern int hard_math;

#define CURRENT_TIME (startup_time+(jiffies+jiffies_offset)/HZ)

extern void add_timer(long jiffies, void (*fn)(void));

extern void sleep_on(struct wait_queue ** p);
extern void interruptible_sleep_on(struct wait_queue ** p);
extern void wake_up(struct wait_queue ** p);
extern void wake_one_task(struct task_struct * p);

extern int send_sig(long sig,struct task_struct * p,int priv);
extern int in_group_p(gid_t grp);

extern int request_irq(unsigned int irq,void (*handler)(int));
extern void free_irq(unsigned int irq);
extern int irqaction(unsigned int irq,struct sigaction * new);

/*
 * Entry into gdt where to find first TSS. 0-nul, 1-cs, 2-ds, 3-syscall
 * 4-TSS0, 5-LDT0, 6-TSS1 etc ...
 */
#define FIRST_TSS_ENTRY 4
#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY+1)
#define _TSS(n) ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))
#define _LDT(n) ((((unsigned long) n)<<4)+(FIRST_LDT_ENTRY<<3))
#define ltr(n) __asm__("ltr %%ax"::"a" (_TSS(n)))
#define lldt(n) __asm__("lldt %%ax"::"a" (_LDT(n)))
#define str(n) \
__asm__("str %%ax\n\t" \
	"subl %2,%%eax\n\t" \
	"shrl $4,%%eax" \
	:"=a" (n) \
	:"0" (0),"i" (FIRST_TSS_ENTRY<<3))
/*
 *	switch_to(n) should switch tasks to task nr n, first
 * checking that n isn't the current task, in which case it does nothing.
 * This also clears the TS-flag if the task we switched to has used
 * tha math co-processor latest.
 */
#define switch_to(n) {\
struct {long a,b;} __tmp; \
__asm__("cmpl %%ecx,_current\n\t" \
	"je 1f\n\t" \
	"movw %%dx,%1\n\t" \
	"cli\n\t" \
	"xchgl %%ecx,_current\n\t" \
	"ljmp %0\n\t" \
	"sti\n\t" \
	"cmpl %%ecx,_last_task_used_math\n\t" \
	"jne 1f\n\t" \
	"clts\n" \
	"1:" \
	::"m" (*&__tmp.a),"m" (*&__tmp.b), \
	"d" (_TSS(n)),"c" ((long) task[n]) \
	:"cx"); \
}

#define PAGE_ALIGN(n) (((n)+0xfff)&0xfffff000)

#define _set_base(addr,base) \
__asm__("movw %%dx,%0\n\t" \
	"rorl $16,%%edx\n\t" \
	"movb %%dl,%1\n\t" \
	"movb %%dh,%2" \
	::"m" (*((addr)+2)), \
	  "m" (*((addr)+4)), \
	  "m" (*((addr)+7)), \
	  "d" (base) \
	:"dx")

#define _set_limit(addr,limit) \
__asm__("movw %%dx,%0\n\t" \
	"rorl $16,%%edx\n\t" \
	"movb %1,%%dh\n\t" \
	"andb $0xf0,%%dh\n\t" \
	"orb %%dh,%%dl\n\t" \
	"movb %%dl,%1" \
	::"m" (*(addr)), \
	  "m" (*((addr)+6)), \
	  "d" (limit) \
	:"dx")

#define set_base(ldt,base) _set_base( ((char *)&(ldt)) , base )
#define set_limit(ldt,limit) _set_limit( ((char *)&(ldt)) , (limit-1)>>12 )

/*
 * The wait-queues are circular lists, and you have to be *very* sure
 * to keep them correct. Use only these two functions to add/remove
 * entries in the queues.
 */
extern inline void add_wait_queue(struct wait_queue ** p, struct wait_queue * wait)
{
	unsigned long flags;

#ifdef DEBUG
	if (wait->next) {
		unsigned long pc;
		__asm__ __volatile__("call 1f\n"
			"1:\tpopl %0":"=r" (pc));
		printk("add_wait_queue (%08x): wait->next = %08x\n",pc,wait->next);
	}
#endif
	__asm__ __volatile__("pushfl ; popl %0 ; cli":"=r" (flags));
	if (!*p) {
		wait->next = wait;
		*p = wait;
	} else {
		wait->next = (*p)->next;
		(*p)->next = wait;
	}
	__asm__ __volatile__("pushl %0 ; popfl"::"r" (flags));
}

extern inline void remove_wait_queue(struct wait_queue ** p, struct wait_queue * wait)
{
	unsigned long flags;
	struct wait_queue * tmp;

	__asm__ __volatile__("pushfl ; popl %0 ; cli":"=r" (flags));
	if ((*p == wait) && ((*p = wait->next) == wait)) {
		*p = NULL;
	} else {
		tmp = wait;
		while (tmp->next != wait)
			tmp = tmp->next;
		tmp->next = wait->next;
	}
	wait->next = NULL;
	__asm__ __volatile__("pushl %0 ; popfl"::"r" (flags));
}

extern inline void select_wait(struct wait_queue ** wait_address, select_table * p)
{
	struct select_table_entry * entry = p->entry + p->nr;

	if (!wait_address)
		return;
	entry->wait_address = wait_address;
	entry->wait.task = current;
	entry->wait.next = NULL;
	add_wait_queue(wait_address,&entry->wait);
	p->nr++;
}

static unsigned long inline _get_base(char * addr)
{
	unsigned long __base;
	__asm__("movb %3,%%dh\n\t"
		"movb %2,%%dl\n\t"
		"shll $16,%%edx\n\t"
		"movw %1,%%dx"
		:"=&d" (__base)
		:"m" (*((addr)+2)),
		 "m" (*((addr)+4)),
		 "m" (*((addr)+7)));
	return __base;
}

#define get_base(ldt) _get_base( ((char *)&(ldt)) )

static unsigned long inline get_limit(unsigned long segment)
{
	unsigned long __limit;
	__asm__("lsll %1,%0"
		:"=r" (__limit):"r" (segment));
	return __limit+1;
}

#define REMOVE_LINKS(p) \
	if ((p)->p_osptr) \
		(p)->p_osptr->p_ysptr = (p)->p_ysptr; \
	if ((p)->p_ysptr) \
		(p)->p_ysptr->p_osptr = (p)->p_osptr; \
	else \
		(p)->p_pptr->p_cptr = (p)->p_osptr

#define SET_LINKS(p) \
	(p)->p_ysptr = NULL; \
	if ((p)->p_osptr = (p)->p_pptr->p_cptr) \
		(p)->p_osptr->p_ysptr = p; \
	(p)->p_pptr->p_cptr = p

#endif
