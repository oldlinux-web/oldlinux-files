#ifndef WAIT_H
#define WAIT_H

#include <asm/system.h>
#include <cnix/sched.h>

struct wait_queue{
	struct task_struct *task;
	struct wait_queue *next;
};

#endif
