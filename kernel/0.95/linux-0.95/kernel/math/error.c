/*
 * linux/kernel/math/error.c
 *
 * (C) 1991 Linus Torvalds
 */

#include <signal.h>

#include <linux/sched.h>

void math_error(void)
{
	if (last_task_used_math)
		last_task_used_math->signal |= 1<<(SIGFPE-1);
	__asm__("fnclex");
}
