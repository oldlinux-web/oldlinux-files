/*
 *  linux/kernel/exit.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#define DEBUG_PROC_TREE

#include <linux/wait.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/resource.h>
#include <linux/mm.h>
#include <linux/tty.h>

#include <asm/segment.h>

int sys_close(int fd);
void getrusage(struct task_struct *, int, struct rusage *);

int send_sig(long sig,struct task_struct * p,int priv)
{
	if (!p || (sig < 0) || (sig > 32))
		return -EINVAL;
	if (!priv && ((sig != SIGCONT) || (current->session != p->session)) &&
	    (current->euid != p->euid) && (current->uid != p->uid) && !suser())
		return -EPERM;
	if (!sig)
		return 0;
	if ((sig == SIGKILL) || (sig == SIGCONT)) {
		if (p->state == TASK_STOPPED)
			p->state = TASK_RUNNING;
		p->exit_code = 0;
		p->signal &= ~( (1<<(SIGSTOP-1)) | (1<<(SIGTSTP-1)) |
				(1<<(SIGTTIN-1)) | (1<<(SIGTTOU-1)) );
	} 
	/* Depends on order SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU */
	if ((sig >= SIGSTOP) && (sig <= SIGTTOU)) 
		p->signal &= ~(1<<(SIGCONT-1));
	/* Actually deliver the signal */
	p->signal |= (1<<(sig-1));
	if (p->flags & PF_PTRACED) {
		/* save the signal number for wait. */
		p->exit_code = sig;

		/* we have to make sure the parent process is awake. */
		if (p->p_pptr != NULL && p->p_pptr->state == TASK_INTERRUPTIBLE)
			p->p_pptr->state = TASK_RUNNING;

		/* we have to make sure that the process stops. */
		if (p->state == TASK_INTERRUPTIBLE || p->state == TASK_RUNNING)
			p->state = TASK_STOPPED;
	}
	return 0;
}

void release(struct task_struct * p)
{
	int i;

	if (!p)
		return;
	if (p == current) {
		printk("task releasing itself\n\r");
		return;
	}
	for (i=1 ; i<NR_TASKS ; i++)
		if (task[i] == p) {
			task[i] = NULL;
			REMOVE_LINKS(p);
			free_page((long) p);
			return;
		}
	panic("trying to release non-existent task");
}

#ifdef DEBUG_PROC_TREE
/*
 * Check to see if a task_struct pointer is present in the task[] array
 * Return 0 if found, and 1 if not found.
 */
int bad_task_ptr(struct task_struct *p)
{
	int 	i;

	if (!p)
		return 0;
	for (i=0 ; i<NR_TASKS ; i++)
		if (task[i] == p)
			return 0;
	return 1;
}
	
/*
 * This routine scans the pid tree and make sure the rep invarient still
 * holds.  Used for debugging only, since it's very slow....
 *
 * It looks a lot scarier than it really is.... we're doing ænothing more
 * than verifying the doubly-linked list foundæin p_ysptr and p_osptr, 
 * and checking it corresponds with the process tree defined by p_cptr and 
 * p_pptr;
 */
void audit_ptree()
{
	int	i;

	for (i=1 ; i<NR_TASKS ; i++) {
		if (!task[i])
			continue;
		if (bad_task_ptr(task[i]->p_pptr))
			printk("Warning, pid %d's parent link is bad\n",
				task[i]->pid);
		if (bad_task_ptr(task[i]->p_cptr))
			printk("Warning, pid %d's child link is bad\n",
				task[i]->pid);
		if (bad_task_ptr(task[i]->p_ysptr))
			printk("Warning, pid %d's ys link is bad\n",
				task[i]->pid);
		if (bad_task_ptr(task[i]->p_osptr))
			printk("Warning, pid %d's os link is bad\n",
				task[i]->pid);
		if (task[i]->p_pptr == task[i])
			printk("Warning, pid %d parent link points to self\n");
		if (task[i]->p_cptr == task[i])
			printk("Warning, pid %d child link points to self\n");
		if (task[i]->p_ysptr == task[i])
			printk("Warning, pid %d ys link points to self\n");
		if (task[i]->p_osptr == task[i])
			printk("Warning, pid %d os link points to self\n");
		if (task[i]->p_osptr) {
			if (task[i]->p_pptr != task[i]->p_osptr->p_pptr)
				printk(
			"Warning, pid %d older sibling %d parent is %d\n",
				task[i]->pid, task[i]->p_osptr->pid,
				task[i]->p_osptr->p_pptr->pid);
			if (task[i]->p_osptr->p_ysptr != task[i])
				printk(
		"Warning, pid %d older sibling %d has mismatched ys link\n",
				task[i]->pid, task[i]->p_osptr->pid);
		}
		if (task[i]->p_ysptr) {
			if (task[i]->p_pptr != task[i]->p_ysptr->p_pptr)
				printk(
			"Warning, pid %d younger sibling %d parent is %d\n",
				task[i]->pid, task[i]->p_osptr->pid,
				task[i]->p_osptr->p_pptr->pid);
			if (task[i]->p_ysptr->p_osptr != task[i])
				printk(
		"Warning, pid %d younger sibling %d has mismatched os link\n",
				task[i]->pid, task[i]->p_ysptr->pid);
		}
		if (task[i]->p_cptr) {
			if (task[i]->p_cptr->p_pptr != task[i])
				printk(
			"Warning, pid %d youngest child %d has mismatched parent link\n",
				task[i]->pid, task[i]->p_cptr->pid);
			if (task[i]->p_cptr->p_ysptr)
				printk(
			"Warning, pid %d youngest child %d has non-NULL ys link\n",
				task[i]->pid, task[i]->p_cptr->pid);
		}
	}
}
#endif /* DEBUG_PROC_TREE */

/*
 * This checks not only the pgrp, but falls back on the pid if no
 * satisfactory prgp is found. I dunno - gdb doesn't work correctly
 * without this...
 */
int session_of_pgrp(int pgrp)
{
	struct task_struct **p;
	int fallback;

	fallback = -1;
 	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
 		if (!*p || (*p)->session <= 0)
 			continue;
		if ((*p)->pgrp == pgrp)
			return (*p)->session;
		if ((*p)->pid == pgrp)
			fallback = (*p)->session;
	}
	return fallback;
}

int kill_pg(int pgrp, int sig, int priv)
{
	struct task_struct **p;
	int err,retval = -ESRCH;
	int found = 0;

	if (sig<0 || sig>32 || pgrp<=0)
		return -EINVAL;
 	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p)
		if (*p && (*p)->pgrp == pgrp) {
			if (err = send_sig(sig,*p,priv))
				retval = err;
			else
				found++;
		}
	return(found ? 0 : retval);
}

int kill_proc(int pid, int sig, int priv)
{
 	struct task_struct **p;

	if (sig<0 || sig>32)
		return -EINVAL;
	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p)
		if (*p && (*p)->pid == pid)
			return send_sig(sig,*p,priv);
	return(-ESRCH);
}

/*
 * POSIX specifies that kill(-1,sig) is unspecified, but what we have
 * is probably wrong.  Should make it like BSD or SYSV.
 */
int sys_kill(int pid,int sig)
{
	struct task_struct **p = NR_TASKS + task;
	int err, retval = 0, count = 0;

	if (!pid)
		return(kill_pg(current->pgrp,sig,0));
	if (pid == -1) {
		while (--p > &FIRST_TASK)
			if (*p && (*p)->pid > 1 && *p != current) {
				++count;
				if ((err = send_sig(sig,*p,0)) != -EPERM)
					retval = err;
			}
		return(count ? retval : -ESRCH);
	}
	if (pid < 0) 
		return(kill_pg(-pid,sig,0));
	/* Normal kill */
	return(kill_proc(pid,sig,0));
}

/*
 * Determine if a process group is "orphaned", according to the POSIX
 * definition in 2.2.2.52.  Orphaned process groups are not to be affected
 * by terminal-generated stop signals.  Newly orphaned process groups are 
 * to receive a SIGHUP and a SIGCONT.
 * 
 * "I ask you, have you ever known what it is to be an orphan?"
 */
int is_orphaned_pgrp(int pgrp)
{
	struct task_struct **p;

	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if (!(*p) ||
		    ((*p)->pgrp != pgrp) || 
		    ((*p)->state == TASK_ZOMBIE) ||
		    ((*p)->p_pptr->pid == 1))
			continue;
		if (((*p)->p_pptr->pgrp != pgrp) &&
		    ((*p)->p_pptr->session == (*p)->session))
			return 0;
	}
	return(1);	/* (sighing) "Often!" */
}

static int has_stopped_jobs(int pgrp)
{
	struct task_struct ** p;

	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if (!*p || (*p)->pgrp != pgrp)
			continue;
		if ((*p)->state == TASK_STOPPED)
			return(1);
	}
	return(0);
}

static void forget_original_parent(struct task_struct * father)
{
	struct task_struct ** p;

	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p)
		if (*p && (*p)->p_opptr == father)
			if (task[1])
				(*p)->p_opptr = task[1];
			else
				(*p)->p_opptr = task[0];
}

volatile void do_exit(long code)
{
	struct task_struct *p;
	int i;

fake_volatile:
	free_page_tables(current);
	for (i=0 ; i<NR_OPEN ; i++)
		if (current->filp[i])
			sys_close(i);
	forget_original_parent(current);
	iput(current->pwd);
	current->pwd = NULL;
	iput(current->root);
	current->root = NULL;
	iput(current->executable);
	current->executable = NULL;
	for (i=0; i < current->numlibraries; i++) {
		iput(current->libraries[i].library);
		current->libraries[i].library = NULL;
	}	
	current->state = TASK_ZOMBIE;
	current->exit_code = code;
	current->rss = 0;
	/* 
	 * Check to see if any process groups have become orphaned
	 * as a result of our exiting, and if they have any stopped
	 * jobs, send them a SIGUP and then a SIGCONT.  (POSIX 3.2.2.2)
	 *
	 * Case i: Our father is in a different pgrp than we are
	 * and we were the only connection outside, so our pgrp
	 * is about to become orphaned.
 	 */
	if ((current->p_pptr->pgrp != current->pgrp) &&
	    (current->p_pptr->session == current->session) &&
	    is_orphaned_pgrp(current->pgrp) &&
	    has_stopped_jobs(current->pgrp)) {
		kill_pg(current->pgrp,SIGHUP,1);
		kill_pg(current->pgrp,SIGCONT,1);
	}
	/* Let father know we died */
	send_sig (SIGCHLD, current->p_pptr, 1);
	
	/*
	 * This loop does two things:
	 * 
  	 * A.  Make init inherit all the child processes
	 * B.  Check to see if any process groups have become orphaned
	 *	as a result of our exiting, and if they have any stopped
	 *	jobs, send them a SIGHUP and then a SIGCONT.  (POSIX 3.2.2.2)
	 */
	while (p = current->p_cptr) {
		current->p_cptr = p->p_osptr;
		p->p_ysptr = NULL;
		p->flags &= ~PF_PTRACED;
		if (task[1])
			p->p_pptr = task[1];
		else
			p->p_pptr = task[0];
		p->p_osptr = p->p_pptr->p_cptr;
		p->p_osptr->p_ysptr = p;
		p->p_pptr->p_cptr = p;
		if (p->state == TASK_ZOMBIE)
			send_sig(SIGCHLD,p->p_pptr,1);
		/*
		 * process group orphan check
		 * Case ii: Our child is in a different pgrp 
		 * than we are, and it was the only connection
		 * outside, so the child pgrp is now orphaned.
		 */
		if ((p->pgrp != current->pgrp) &&
		    (p->session == current->session) &&
		    is_orphaned_pgrp(p->pgrp) &&
		    has_stopped_jobs(p->pgrp)) {
			kill_pg(p->pgrp,SIGHUP,1);
			kill_pg(p->pgrp,SIGCONT,1);
		}
	}
	if (current->leader) {
		struct task_struct **p;
		struct tty_struct *tty;

		if (current->tty >= 0) {
			tty = TTY_TABLE(current->tty);
			if (tty) {
				if (tty->pgrp > 0)
					kill_pg(tty->pgrp, SIGHUP, 1);
				tty->pgrp = -1;
				tty->session = 0;
			}
		}
	 	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p)
			if (*p && (*p)->session == current->session)
				(*p)->tty = -1;
	}
	if (last_task_used_math == current)
		last_task_used_math = NULL;
#ifdef DEBUG_PROC_TREE
	audit_ptree();
#endif
	schedule();
/*
 * In order to get rid of the "volatile function does return" message
 * I did this little loop that confuses gcc to think do_exit really
 * is volatile. In fact it's schedule() that is volatile in some
 * circumstances: when current->state = ZOMBIE, schedule() never
 * returns.
 *
 * In fact the natural way to do all this is to have the label and the
 * goto right after each other, but I put the fake_volatile label at
 * the start of the function just in case something /really/ bad
 * happens, and the schedule returns. This way we can try again. I'm
 * not paranoid: it's just that everybody is out to get me.
 */
	goto fake_volatile;
}

int sys_exit(int error_code)
{
	do_exit((error_code&0xff)<<8);
}

int sys_wait4(pid_t pid,unsigned long * stat_addr, int options, struct rusage * ru)
{
	int flag;
	struct task_struct *p;
	unsigned long oldblocked;

	if (stat_addr)
		verify_area(stat_addr,4);
repeat:
	current->signal &= ~(1<<(SIGCHLD-1));
	flag=0;
 	for (p = current->p_cptr ; p ; p = p->p_osptr) {
		if (pid>0) {
			if (p->pid != pid)
				continue;
		} else if (!pid) {
			if (p->pgrp != current->pgrp)
				continue;
		} else if (pid != -1) {
			if (p->pgrp != -pid)
				continue;
		}
		switch (p->state) {
			case TASK_STOPPED:
				if (!p->exit_code)
					continue;
				if (!(options & WUNTRACED) && !(p->flags & PF_PTRACED))
					continue;
				if (stat_addr)
					put_fs_long((p->exit_code << 8) | 0x7f,
						stat_addr);
				p->exit_code = 0;
				if (ru != NULL)
					getrusage(p, RUSAGE_BOTH, ru);
				return p->pid;
			case TASK_ZOMBIE:
				current->cutime += p->utime + p->cutime;
				current->cstime += p->stime + p->cstime;
				current->cmin_flt += p->min_flt + p->cmin_flt;
				current->cmaj_flt += p->maj_flt + p->cmaj_flt;
				if (ru != NULL)
					getrusage(p, RUSAGE_BOTH, ru);
				flag = p->pid;
				if (stat_addr)
					put_fs_long(p->exit_code, stat_addr);
				if (p->p_opptr != p->p_pptr) {
					REMOVE_LINKS(p);
					p->p_pptr = p->p_opptr;
					SET_LINKS(p);
					send_sig(SIGCHLD,p->p_pptr,1);
				} else
					release(p);
#ifdef DEBUG_PROC_TREE
				audit_ptree();
#endif
				return flag;
			default:
				flag=1;
				continue;
		}
	}
	if (flag) {
		if (options & WNOHANG)
			return 0;
		current->state=TASK_INTERRUPTIBLE;
		oldblocked = current->blocked;
		current->blocked &= ~(1<<(SIGCHLD-1));
		schedule();
		current->blocked = oldblocked;
		if (current->signal & ~(current->blocked | (1<<(SIGCHLD-1))))
			return -ERESTARTSYS;
		else
			goto repeat;
	}
	return -ECHILD;
}

/*
 * sys_waitpid() remains for compatibility. waitpid() should be
 * implemented by calling sys_wait4() from libc.a.
 */
int sys_waitpid(pid_t pid,unsigned long * stat_addr, int options)
{
	return sys_wait4(pid, stat_addr, options, NULL);
}
