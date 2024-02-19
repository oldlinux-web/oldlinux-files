#include <const.h>
#include <errno.h>
#include <asm/regs.h>
#include <cnix/sched.h>

extern struct task_struct * current;
extern struct task_struct * task[NR_TASKS];
extern void ret_from_syscall(void); /* only to get the address of it */

int find_empty_process(void)
{
	int i, j;
	static int next_pid = 1;
	
	i = next_pid;
	for(j = 0; j < NR_TASKS; j++, i++){
		if(!task[i]){
			next_pid = ((i + 1) == (NR_TASKS - 1)) ? 1 : i + 1;
			return i;
		}
		if(i == NR_TASKS - 1)
			i = 1;
	}

	return -EAGAIN;
}

int copy_process(int nr, struct regs_t regs)
{
	unsigned long dir_pg;
	struct regs_t * ptr;
	struct task_struct * p;

	p = (struct task_struct *)get_one_page();
	if(!p)
		return -EAGAIN;
	
	*p = *current;

	/* add it into run queue ??? */

	p->pid = nr;
	p->ppid = current->pid;

	dir_pg = (unsigned long)get_one_page();
	if(!dir_pg){
		free_one_page(p);	
		return -EAGAIN;	
	}

	p->pg_dir = dir_pg;
	p->esp = (unsigned long)p + PAGE_SIZE - sizeof(struct regs_t);
	/* ... */
	p->eip = (unsigned long)ret_from_syscall;
	
	/* 32 items in page dir == 128M */
	if(copy_page_tables(current->pg_dir, p->pg_dir, 32) < 0){
		free_page_tables(p->pg_dir, 32);
		free_one_page(dir_pg);
		free_one_page(p);
		return -ENOMEM;
	}

	ptr = (struct regs_t *)(((unsigned long)p) + PAGE_SIZE - 
			sizeof(struct regs_t));
	*ptr = regs;
	ptr->eax = 0;

	task[nr] = p;

	add_run(p);
	current->need_sched = 1;

	return 0;
}
