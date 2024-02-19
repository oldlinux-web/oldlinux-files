#include <const.h>
#include <asm/system.h>
#include <cnix/head.h>
#include <cnix/sched.h>
#include <cnix/wait.h>

struct{
	long * a;
	short b;
}stack_start = {(unsigned long)&(init_task.task) + PAGE_SIZE, 0x10};

struct tss_struct tss = {0, };

union task_union init_task;

struct task_struct * current = &(init_task.task);
/* take init_task.task as the head of run_queue */
struct task_struct * run_queue = &(init_task.task);
struct task_struct * task[NR_TASKS] = {&(init_task.task), };

static void __sleep_on(struct wait_queue **p,unsigned long flags);

void sched_init(void)
{
	int i;
	
	current->need_sched = 0;

	current->state = TASK_RUNNING;
	current->pg_dir = (unsigned long)kp_dir;
	current->priority = 10;
	current->counter = 10;
	current->signal = 0;
	current->blocked = 0;
	for(i = 0; i < 32; i++){
		current->sigaction[i].sa_handler = NULL;
		current->sigaction[i].sa_mask = 0;
		/* how to use it ??? */
		current->sigaction[i].sa_flags = 0;
	}
	current->exit_code = 0;
	current->pid = current->ppid = 0;
	current->tty = 0;

	tss.ss0 = 0x10;
	tss.esp0 = (unsigned long)&(init_task.task) + PAGE_SIZE;
	tss.bitmap = (unsigned long)&tss + 104;
	set_tss_desc(gdt + TSS_ENTRY, &tss);
	ltr(TSS_ENTRY);

	run_queue->prev = run_queue->next = run_queue;

	for(i = 1; i < NR_TASKS; i++)
		task[i] = NULL;
}

/* add into tail */
void add_run(struct task_struct * tsk)
{
	if(run_queue->next == run_queue){
		run_queue->prev = run_queue->next = tsk;
		tsk->prev = tsk->next = run_queue;
	}else{
		tsk->prev = run_queue->prev;
		tsk->next = run_queue;
		run_queue->prev->next = tsk;
		run_queue->prev = tsk;
	}
}

/* delete one task from run_queue */
void del_run(struct task_struct * tsk)
{
	tsk->prev->next = tsk->next;
	tsk->next->prev = tsk->prev;
	tsk->prev = tsk->next = NULL;
}

void schedule(void)
{
	int counter;
	struct task_struct * prev, * next, * p, * sel;

	current->need_sched = 0;

	/* if no ok-process but idle */
	if(run_queue->next == run_queue){
		prev = current;
		current = next = run_queue;
		goto go_on;
	}
repeat:
	counter = 0;
	p = run_queue->next;
	sel = NULL;

	while(p != run_queue){
		if(p->counter > counter){
			counter = p->counter;
			sel = p;
		}
		p = p->next;
	}
	
	if(counter == 0){
		if(current->counter == 0){
			p = run_queue->next;
			while(p != run_queue){
				p->counter = p->priority * 10;
				p = p->next;
			}
		}
		goto repeat;
	}
	
	prev = current;
	current = next = sel;
go_on:
	switch_to(prev, next);
}

void __switch_to(struct task_struct * prev, struct task_struct * next) __attribute__((regparm(2)));

void __switch_to(struct task_struct * prev, struct task_struct * next) 
{
	tss.esp0 = (unsigned long)next + PAGE_SIZE;
	__asm__("movl %%eax, %%cr3"::"a"((unsigned long)next->pg_dir));
}

void add_wait_queue(struct wait_queue ** p, struct wait_queue * wait) 
{
	unsigned long flags;

	save_flags(flags);
	cli();
	
	if(!*p){
		wait->next = wait;
		*p = wait;
	}else{
		wait->next = (*p)->next;
		(*p)->next = wait;
	}
	
	restore_flags(flags);
}

/* remove wait from wait_queue whose head is *p */
void remove_wait_queue(struct wait_queue ** p , struct wait_queue *wait)
{
	unsigned long flags;
	struct wait_queue * tmp;
	
	save_flags(flags);
	cli();
	
	if((*p == wait) && ((*p = wait->next) == wait)){
		*p = NULL;
	}else{
		tmp = wait;
		while(tmp->next != wait)
			tmp = tmp->next;
			
		tmp->next = wait->next;
	}
	wait->next = NULL;
	
	restore_flags(flags);
}		



void sleep_on(struct wait_queue ** p)
{
	__sleep_on(p, TASK_UNINTERRUPTIBLE);
}

void interrupt_sleep_on(struct wait_queue ** p)
{
	__sleep_on(p, TASK_INTERRUPTIBLE);

}

static void __sleep_on(struct wait_queue ** p, unsigned long state)
{
	unsigned long flags;
	struct wait_queue wait = {current, NULL};

	if (!p)
		return;

	if (current->pid == 0)
		panic("Task 0 want to sleep, giving up ....\n");

	del_run(current);

	current->state = state;

	add_wait_queue(p, &wait);
	
	save_flags(flags); 
	sti();
	schedule();

	remove_wait_queue(p, &wait);

	restore_flags(flags);
}

void wakeup(struct wait_queue ** p)
{
	struct wait_queue * tmp;
	struct task_struct * tsk;

	if(!p || !(tmp = *p))
		return ;

	do{
		tsk = tmp->task;

		if(tsk->state == TASK_UNINTERRUPTIBLE 
				|| tsk->state == TASK_INTERRUPTIBLE){
			tsk -> state = TASK_RUNNING;

			add_run(tsk);
		}
		tmp = tmp->next;
	}while(tmp != *p);

	current->need_sched = 1;
}
