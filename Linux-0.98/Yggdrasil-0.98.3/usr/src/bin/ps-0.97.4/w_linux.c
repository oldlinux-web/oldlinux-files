
#include <linux/sched.h>

struct task_struct *
kvm_nextproc()
{

	struct task_struct *task, *taskp;
	static unsigned long _task;
	static int n = NR_TASKS;
	
	if (_task == 0)
		_task = k_addr("_task") + 4;


	task = (struct task_struct *) xmalloc(sizeof(struct task_struct));

repeat:
	if (--n == 0)
		return(NULL);

	kmemread(&taskp, _task, 4);
	_task += 4;

	if (taskp == 0)
		goto repeat;

	kmemread(task, taskp, sizeof(struct task_struct));

		    /* check if valid, proc may have exited */
	if ((unsigned) task->state > 4 || task->pid <= 0)
		goto repeat;

	return(task);
}

proc_compare(p1, p2)
struct task_struct *p1, *p2;
{
	if (p1 == NULL)
		return 1;
	return(p1->counter > p2->counter);
}


#define	FSHIFT	11
#define	FSCALE	(1<<FSHIFT)

getloadavg(avenrun, n)
double *avenrun;
int n;
{
    	unsigned long averunnable[3];
	int i;

	kmemread(averunnable, k_addr("_avenrun"), sizeof averunnable);
	for (i = 0; i < n; ++i)
		avenrun[i] = (double) averunnable[i] / FSCALE;
}

char *
strdup(s)
char *s;
{
    char *p;

    p = (char *) malloc(strlen(s) + 1);
    if (p)
	strcpy(p,s);
    return(p);
}
