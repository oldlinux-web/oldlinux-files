#ifndef PROC_H
#define PROC_H

#include <const.h>
#include <signal.h>
#include <cnix/head.h>

#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		3
#define TASK_STOPPED		4

#ifndef NULL
#define NULL	((void *)0)
#endif

#define TSS_ENTRY		5

struct i387_struct{
	long	cwd;
	long	swd;
	long	twd;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long	st_space[20];
};

struct tss_struct{
	long	linkage;
	long	esp0;
	long	ss0;
	long	esp1;
	long	ss1;
	long	esp2;
	long	ss2;
	long	cr3;
	long	eip;
	long	eflags;
	long	eax;
	long	ecx;
	long	edx;
	long	ebx;
	long	esp;
	long	ebp;
	long	esi;
	long	edi;
	long	es;
	long	cs;
	long	ss;
	long	ds;
	long	fs;
	long	gs;
	long	ldt;
	unsigned short trace, bitmap;
	/* Now I still have some questions about it, so not to use it. */
	/* struct i387_struct i387; */
};

extern struct tss_struct tss;	

typedef int (*fn_t)(void);

struct task_struct{
	int	need_sched;

	long	state;
	unsigned long pg_dir;	
	
	unsigned long esp;
	unsigned long eip;

	struct task_struct * prev;
	struct task_struct * next;	/* linked in run queue */

	long	priority;
	long	counter; 

	unsigned long	signal;
	unsigned long	blocked;
	struct sigaction sigaction[32];	/* will be sigaction. */

	int	exit_code;

	unsigned long	end_code, end_data, start_stack;

	long	pid, ppid; /* pgrp, session, leader; */
	int	tty;
	/* unsigned short uid, euid, suid;
	unsigned short gid, egid, sgid;
	long	alarm;
	long	utime, stime, cutime, cstime, start_time;
	unsigned short used_math;
	struct desc_struct ldt[2]; */
};

union task_union{
	struct task_struct task;
	char	stack[PAGE_SIZE];
};

extern union task_union init_task;
extern struct task_struct * current;
extern struct task_struct * run_queue;
extern struct task_struct * task[NR_TASKS];

extern void add_run(struct task_struct * tsk);
extern void del_run(struct task_struct * tsk);
/* extern void sleep_on(struct wait_queue ** p);
   extern void interrupt_sleep_on(struct wait_queue ** p); */

#define ltr(n) __asm__("ltr %%ax"::"a"((unsigned long)n << 3))

#define switch_to(prev, next) do{\
__asm__ volatile("pushl %%ebx\n\t"\
	"pushl %%ecx\n\t"\
	"pushl %%esi\n\t"\
	"pushl %%edi\n\t"\
	"pushl %%ebp\n\t"\
	"pushl %%fs\n\t"\
	"movl %%esp, %0\n\t"\
	"movl %2, %%esp\n\t"\
	"movl $1f, %1\n\t"\
	"pushl %3\n\t"\
	"jmp __switch_to\n\t"\
	"1:\n\t"\
	"popl %%fs\n\t"\
	"popl %%ebp\n\t"\
	"popl %%edi\n\t"\
	"popl %%esi\n\t"\
	"popl %%ecx\n\t"\
	"popl %%ebx\n\t"\
	:"=m"(prev->esp), "=m"(prev->eip)\
	:"m"(next->esp), "m"(next->eip), "a"(prev), "d"(next));\
}while(0)

#endif
