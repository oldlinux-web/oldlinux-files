#ifdef AM_KERNEL
/*
**	The Amoeba Transaction Layer System Call
**
**	Amoeba transactions are implemented as a single system call
**	which implements trans(), getreq() and putrep().  timeout() is
**	implemented in the user library.
**	The job of the transaction layer is to ensure that a user task gets
**	only one kernel task.   The kernel task is the exclusive domain of
**	the user task for the duration of a transaction.
**
**	A note on programming style:
**	  macros names are upper case (all of them except those from andy)
**	  global variable names start with an upper case letter
**	  local variables names are lower case
**
**	The amoeba transactions use message type 2, just like the device
**	drivers.  This is for compatibility with the revive code.
*/

#include "mm.h"
#include "minix/com.h"
#include "amoeba.h"

/*
** There are several external global variables which you need to know about
** but due to the hacks everywhere they are declared in an include file.
**  who   - the task # of the task that sent the request = mm_in.m_source
**  mm_in - the message from that task
**  mm_out - a message buffer normally used by reply()
**  dont_reply - a flag to indicate that no reply should go to sender
**
** do_amoeba returns the error status that will be given to the requesting
** task.  The kernel has already done all the data transfers for the task
** so no return message is required.
*/


PUBLIC int
do_amoeba()
{
    int ktasknr;	/* # of kernel task to perform operation */
    int proc_nr;	/* # of user task requesting the operation */
    int r;		/* return status */
    int cmd;

/* if it is a revive message from kernel then handle it */
    if (mm_in.AM_OP == AM_REVIVE)
	return am_revive(mm_in.AM_PROC_NR, mm_in.AM_STATUS, (int)mm_in.AM_FREE_IT);
/*
** check to see if this task already has a kernel task engaged.
** if so then use that one, else find a free kernel task.
*/
    if ((ktasknr = alloc_ktask()) >= 0)
	return TRYAGAIN;	/* no free kernel task, try again? */
    proc_nr = who;
    mm_in.AM_PROC_NR = proc_nr;
    cmd = mm_in.AM_OP;
    if ((r = sendrec(ktasknr, &mm_in)) != OK)
	panic("do_amoeba: can't send", NO_NUM);
/*
** if necessary suspend.  the kernel task will be freed during the revival.
** otherwise we got an immediate answer.
** in that case it was an error or an instant response.  probably it
** was an error.  if so free the kernel task.  if it was not an error
** then don't free the kernel task if it was a getrequest.
*/
    if (mm_in.AM_STATUS == SUSPEND)
	dont_reply = TRUE;
    else
	if (mm_in.AM_STATUS < 0 || cmd != AM_GETREQ)
	    free_ktask(proc_nr);
    return mm_in.AM_STATUS;
}


/* task number of task in control of kernel task, 0 if task is free */
PRIVATE struct
{
	int proc;
	int signalled;
} In_use[AM_NTASKS];

PRIVATE int
alloc_ktask()
{
    int i;
    int slot = -1;

    for (i = 0; i < AM_NTASKS; i++)	/* look at all kernel tasks!! */
	if (In_use[i].proc == who)
	{			/* already has a kernel task */
	    slot = i;
	    break;
	}
	else
	    if (In_use[i].proc == 0)
		slot = i;	/* slot i is free, but keep looking */
    if (slot < 0)	/* no free slot was found */
	return 0;
    In_use[slot].proc = who;
    In_use[slot].signalled = 0;
    return AMOEBA_CLASS - slot;
}


PRIVATE int
free_ktask(n)
{
    int i;

    for (i = 0; i < AM_NTASKS; i++)
	if (In_use[i].proc == n)
	{
	    In_use[i].proc = 0;
	    return;
	}
}


PRIVATE int
am_revive(task, status, free_it)
int	task;
int	status;
int	free_it;
{
    if (who > 0)
	return EPERM;
    if (free_it)
	free_ktask(task);	/* it was not a getreq! */
    reply(task, status, 0, (char *) 0);	/* revive the task */
    dont_reply = TRUE;
    return OK;
}


PUBLIC int
am_check_sig(proc, type)
int proc;
int type;
{
/* return 0 if the signal was for a transacting task. otherwise return 1 */
    int i;

    for (i = 0; i < AM_NTASKS; i++)
	if (In_use[i].proc == proc && (!In_use[i].signalled || type == 1))
	{
/* mm_out is not being used by anyone right now! */
	    In_use[i].signalled = 1;
	    if (type == 1)
	    {
		In_use[i].proc = 0;
		mm_out.m_type = AM_TASK_DIED;
	    }
	    else
		mm_out.m_type = AM_PUTSIG;
	    mm_out.AM_COUNT = i;
	    send(AMINT_CLASS, &mm_out);
	    return 0;
	}
    return 1;
}
#endif AM_KERNEL
