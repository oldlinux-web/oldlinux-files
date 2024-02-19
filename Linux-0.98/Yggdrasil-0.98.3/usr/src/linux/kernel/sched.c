/*
 *  linux/kernel/sched.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * 'sched.c' is the main kernel file. It contains scheduling primitives
 * (sleep_on, wakeup, schedule etc) as well as a number of simple system
 * call functions (type getpid(), which just extracts a field from
 * current-task
 */

#define TIMER_IRQ 0

#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/sys.h>
#include <linux/fdreg.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/ptrace.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/segment.h>

int need_resched = 0;
int hard_math = 0;		/* set by boot/head.S */

unsigned long * prof_buffer = NULL;
unsigned long prof_len = 0;

#define _S(nr) (1<<((nr)-1))
#define _BLOCKABLE (~(_S(SIGKILL) | _S(SIGSTOP)))

static void show_task(int nr,struct task_struct * p)
{
	int i,j = 4096-sizeof(struct task_struct);

	printk("%d: pid=%d, state=%d, father=%d, child=%d, ",(p == current)?-nr:nr,p->pid,
		p->state, p->p_pptr->pid, p->p_cptr ? p->p_cptr->pid : -1);
	i=0;
	while (i<j && !((char *)(p+1))[i])
		i++;
	printk("%d/%d chars free in kstack\n\r",i,j);
	printk("   PC=%08X.", *(1019 + (unsigned long *) p));
	if (p->p_ysptr || p->p_osptr) 
		printk("   Younger sib=%d, older sib=%d\n\r", 
			p->p_ysptr ? p->p_ysptr->pid : -1,
			p->p_osptr ? p->p_osptr->pid : -1);
	else
		printk("\n\r");
}

void show_state(void)
{
	int i;

	printk("\rTask-info:\n\r");
	for (i=0 ; i<NR_TASKS ; i++)
		if (task[i])
			show_task(i,task[i]);
}

#define LATCH (1193180/HZ)

extern void mem_use(void);

extern int timer_interrupt(void);
extern int system_call(void);

union task_union {
	struct task_struct task;
	char stack[PAGE_SIZE];
};

static union task_union init_task = {INIT_TASK, };

unsigned long volatile jiffies=0;
unsigned long startup_time=0;
int jiffies_offset = 0;		/* # clock ticks to add to get "true
				   time".  Should always be less than
				   1 second's worth.  For time fanatics
				   who like to syncronize their machines
				   to WWV :-) */

struct task_struct *current = &(init_task.task);
struct task_struct *last_task_used_math = NULL;

struct task_struct * task[NR_TASKS] = {&(init_task.task), };

long user_stack [ PAGE_SIZE>>2 ] ;

struct {
	long * a;
	short b;
	} stack_start = { & user_stack [PAGE_SIZE>>2] , 0x10 };
/*
 *  'math_state_restore()' saves the current math information in the
 * old math state array, and gets the new ones from the current task
 */
void math_state_restore()
{
	if (last_task_used_math == current)
		return;
	__asm__("fwait");
	if (last_task_used_math) {
		__asm__("fnsave %0"::"m" (last_task_used_math->tss.i387));
	}
	last_task_used_math=current;
	if (current->used_math) {
		__asm__("frstor %0"::"m" (current->tss.i387));
	} else {
		__asm__("fninit"::);
		current->used_math=1;
	}
}

/*
 *  'schedule()' is the scheduler function. It's a very simple and nice
 * scheduler: it's not perfect, but certainly works for most things.
 * The one thing you might take a look at is the signal-handler code here.
 *
 *   NOTE!!  Task 0 is the 'idle' task, which gets called when no other
 * tasks can run. It can not be killed, and it cannot sleep. The 'state'
 * information in task[0] is never used.
 */
void schedule(void)
{
	int i,next,c;
	struct task_struct ** p;

/* check alarm, wake up any interruptible tasks that have got a signal */

	need_resched = 0;
	for(p = &LAST_TASK ; p > &FIRST_TASK ; --p)
		if (*p) {
			if ((*p)->timeout && (*p)->timeout < jiffies)
				if ((*p)->state == TASK_INTERRUPTIBLE) {
					(*p)->timeout = 0;
					wake_one_task(*p);
				}
			if (((*p)->signal & ~(*p)->blocked) &&
			    (*p)->state==TASK_INTERRUPTIBLE)
				wake_one_task(*p);
		}

/* this is the scheduler proper: */

	while (1) {
		c = -1;
		next = 0;
		i = NR_TASKS;
		p = &task[NR_TASKS];
		while (--i) {
			if (!*--p)
				continue;
			if ((*p)->state == TASK_RUNNING && (*p)->counter > c)
				c = (*p)->counter, next = i;
		}
		if (c)
			break;
		for(p = &LAST_TASK ; p > &FIRST_TASK ; --p)
			if (*p)
				(*p)->counter = ((*p)->counter >> 1) +
						(*p)->priority;
	}
	sti();
	switch_to(next);
}

int sys_pause(void)
{
	unsigned long old_blocked;
	unsigned long mask;
	struct sigaction * sa = current->sigaction;

	old_blocked = current->blocked;
	for (mask=1 ; mask ; sa++,mask += mask)
		if (sa->sa_handler == SIG_IGN)
			current->blocked |= mask;
	current->state = TASK_INTERRUPTIBLE;
	schedule();
	current->blocked = old_blocked;
	return -EINTR;
}

void wake_one_task(struct task_struct * p)
{
	p->state = TASK_RUNNING;
	if (p->counter > current->counter)
		need_resched = 1;
}

/*
 * wake_up doesn't wake up stopped processes - they have to be awakened
 * with signals or similar.
 *
 * Note that this doesn't need cli-sti pairs: interrupts may not change
 * the wait-queue structures directly, but only call wake_up() to wake
 * a process. The process itself must remove the queue once it has woken.
 */
void wake_up(struct wait_queue **q)
{
	struct wait_queue *tmp;
	struct task_struct * p;

	if (!q || !(tmp = *q))
		return;
	do {
		if (p = tmp->task) {
			if (p->state == TASK_ZOMBIE)
				printk("wake_up: TASK_ZOMBIE\n");
			else if (p->state != TASK_STOPPED) {
				p->state = TASK_RUNNING;
				if (p->counter > current->counter)
					need_resched = 1;
			}
		}
#ifdef DEBUG
		if (!tmp->next) {
			printk("wait_queue is bad\n");
			printk("        q = %08x\n",q);
			printk("       *q = %08x\n",*q);
			printk("      tmp = %08x\n",tmp);
			break;
		}
#endif
		tmp = tmp->next;
	} while (tmp != *q);
}

static inline void __sleep_on(struct wait_queue **p, int state)
{
	unsigned long flags;

	if (!p)
		return;
	if (current == task[0])
		panic("task[0] trying to sleep");
	current->state = state;
	add_wait_queue(p,&current->wait);
	save_flags(flags);
	sti();
	schedule();
	remove_wait_queue(p,&current->wait);
	restore_flags(flags);
}

void interruptible_sleep_on(struct wait_queue **p)
{
	__sleep_on(p,TASK_INTERRUPTIBLE);
}

void sleep_on(struct wait_queue **p)
{
	__sleep_on(p,TASK_UNINTERRUPTIBLE);
}

/*
 * OK, here are some floppy things that shouldn't be in the kernel
 * proper. They are here because the floppy needs a timer, and this
 * was the easiest way of doing it.
 */
static struct wait_queue * wait_motor[4] = {NULL,NULL,NULL,NULL};
static int  mon_timer[4]={0,0,0,0};
static int moff_timer[4]={0,0,0,0};
unsigned char current_DOR = 0x0C;

int ticks_to_floppy_on(unsigned int nr)
{
	extern unsigned char selected;
	unsigned char mask = 0x10 << nr;

	if (nr>3)
		panic("floppy_on: nr>3");
	moff_timer[nr]=10000;		/* 100 s = very big :-) */
	cli();				/* use floppy_off to turn it off */
	mask |= current_DOR;
	if (!selected) {
		mask &= 0xFC;
		mask |= nr;
	}
	if (mask != current_DOR) {
		outb(mask,FD_DOR);
		if ((mask ^ current_DOR) & 0xf0)
			mon_timer[nr] = HZ/2;
		else if (mon_timer[nr] < 2)
			mon_timer[nr] = 2;
		current_DOR = mask;
	}
	sti();
	return mon_timer[nr];
}

void floppy_off(unsigned int nr)
{
	moff_timer[nr]=3*HZ;
}

void do_floppy_timer(void)
{
	int i;
	unsigned char mask = 0x10;

	for (i=0 ; i<4 ; i++,mask <<= 1) {
		if (!(mask & current_DOR))
			continue;
		if (mon_timer[i]) {
			if (!--mon_timer[i])
				wake_up(i+wait_motor);
		} else if (!moff_timer[i]) {
			current_DOR &= ~mask;
			outb(current_DOR,FD_DOR);
		} else
			moff_timer[i]--;
	}
}

#define TIME_REQUESTS 64

static struct timer_list {
	long jiffies;
	void (*fn)();
	struct timer_list * next;
} timer_list[TIME_REQUESTS] = { { 0, NULL, NULL }, };

static struct timer_list * next_timer = NULL;

void add_timer(long jiffies, void (*fn)(void))
{
	struct timer_list * p;

	if (!fn)
		return;
	cli();
	if (jiffies <= 0)
		(fn)();
	else {
		for (p = timer_list ; p < timer_list + TIME_REQUESTS ; p++)
			if (!p->fn)
				break;
		if (p >= timer_list + TIME_REQUESTS)
			panic("No more time requests free");
		p->fn = fn;
		p->jiffies = jiffies;
		p->next = next_timer;
		next_timer = p;
		while (p->next && p->next->jiffies < p->jiffies) {
			p->jiffies -= p->next->jiffies;
			fn = p->fn;
			p->fn = p->next->fn;
			p->next->fn = fn;
			jiffies = p->jiffies;
			p->jiffies = p->next->jiffies;
			p->next->jiffies = jiffies;
			p = p->next;
		}
	}
	sti();
}

unsigned long timer_active = 0;
struct timer_struct timer_table[32];

/*
 * Hmm.. Changed this, as the GNU make sources (load.c) seems to
 * imply that avenrun[] is the standard name for this kind of thing.
 * Nothing else seems to be standardized: the fractional size etc
 * all seem to differ on different machines.
 */
unsigned long avenrun[3] = { 0,0,0 };

/*
 * Nr of active tasks - counted in fixed-point numbers
 */
static unsigned long count_active_tasks(void)
{
	struct task_struct **p;
	unsigned long nr = 0;

	for(p = &LAST_TASK; p > &FIRST_TASK; --p)
		if (*p && (*p)->state == TASK_RUNNING)
			nr += FIXED_1;
	return nr;
}

static inline void calc_load(void)
{
	unsigned long active_tasks; /* fixed-point */
	static int count = LOAD_FREQ;

	if (count-- > 0)
		return;
	count = LOAD_FREQ;
	active_tasks = count_active_tasks();
	CALC_LOAD(avenrun[0], EXP_1, active_tasks);
	CALC_LOAD(avenrun[1], EXP_5, active_tasks);
	CALC_LOAD(avenrun[2], EXP_15, active_tasks);
}

/*
 * The int argument is really a (struct pt_regs *), in case the
 * interrupt wants to know from where it was called. The timer
 * irq uses this to decide if it should update the user or system
 * times.
 */
static void do_timer(struct pt_regs * regs)
{
	unsigned long mask;
	struct timer_struct *tp = timer_table+0;
	struct task_struct ** task_p;

	jiffies++;
	calc_load();
	if ((VM_MASK & regs->eflags) || (3 & regs->cs)) {
		current->utime++;
		/* Update ITIMER_VIRT for current task if not in a system call */
		if (current->it_virt_value && !(--current->it_virt_value)) {
			current->it_virt_value = current->it_virt_incr;
			send_sig(SIGVTALRM,current,1);
		}
	} else {
		current->stime++;
#ifdef PROFILE_SHIFT
		if (prof_buffer && current != task[0]) {
			unsigned long eip = regs->eip;
			eip >>= PROFILE_SHIFT;
			if (eip < prof_len)
				prof_buffer[eip]++;
		}
#endif
	}
	if (current == task[0] || (--current->counter)<=0) {
		current->counter=0;
		need_resched = 1;
	}
	/* Update ITIMER_REAL for every task */
	for (task_p = &LAST_TASK; task_p >= &FIRST_TASK; task_p--)
		if (*task_p && (*task_p)->it_real_value
			&& !(--(*task_p)->it_real_value)) {
			send_sig(SIGALRM,*task_p,1);
			(*task_p)->it_real_value = (*task_p)->it_real_incr;
			need_resched = 1;
		}
	/* Update ITIMER_PROF for the current task */
	if (current->it_prof_value && !(--current->it_prof_value)) {
		current->it_prof_value = current->it_prof_incr;
		send_sig(SIGPROF,current,1);
	}
	for (mask = 1 ; mask ; tp++,mask += mask) {
		if (mask > timer_active)
			break;
		if (!(mask & timer_active))
			continue;
		if (tp->expires > jiffies)
			continue;
		timer_active &= ~mask;
		tp->fn();
		sti();
	}
	if (next_timer) {
		next_timer->jiffies--;
		while (next_timer && next_timer->jiffies <= 0) {
			void (*fn)(void);
			
			fn = next_timer->fn;
			next_timer->fn = NULL;
			next_timer = next_timer->next;
			(fn)();
		}
	}
	if (current_DOR & 0xf0)
		do_floppy_timer();
}

int sys_alarm(long seconds)
{
	extern int _setitimer(int, struct itimerval *, struct itimerval *);
	struct itimerval new, old;

	new.it_interval.tv_sec = new.it_interval.tv_usec = 0;
	new.it_value.tv_sec = seconds;
	new.it_value.tv_usec = 0;
	_setitimer(ITIMER_REAL, &new, &old);
	return(old.it_value.tv_sec + (old.it_value.tv_usec / 1000000));
}

int sys_getpid(void)
{
	return current->pid;
}

int sys_getppid(void)
{
	return current->p_pptr->pid;
}

int sys_getuid(void)
{
	return current->uid;
}

int sys_geteuid(void)
{
	return current->euid;
}

int sys_getgid(void)
{
	return current->gid;
}

int sys_getegid(void)
{
	return current->egid;
}

int sys_nice(long increment)
{
	if (increment < 0 && !suser())
		return -EPERM;
	if (increment >= current->priority)
		increment = current->priority-1;
	current->priority -= increment;
	return 0;
}

void sched_init(void)
{
	int i;
	struct desc_struct * p;

	if (sizeof(struct sigaction) != 16)
		panic("Struct sigaction MUST be 16 bytes");
	set_tss_desc(gdt+FIRST_TSS_ENTRY,&(init_task.task.tss));
	set_ldt_desc(gdt+FIRST_LDT_ENTRY,&(init_task.task.ldt));
	set_system_gate(0x80,&system_call);
	p = gdt+2+FIRST_TSS_ENTRY;
	for(i=1 ; i<NR_TASKS ; i++) {
		task[i] = NULL;
		p->a=p->b=0;
		p++;
		p->a=p->b=0;
		p++;
	}
/* Clear NT, so that we won't have troubles with that later on */
	__asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl");
	ltr(0);
	lldt(0);
	outb_p(0x36,0x43);		/* binary, mode 3, LSB/MSB, ch 0 */
	outb_p(LATCH & 0xff , 0x40);	/* LSB */
	outb(LATCH >> 8 , 0x40);	/* MSB */
	request_irq(TIMER_IRQ,(void (*)(int)) do_timer);
}
