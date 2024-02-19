/*
 * linux/kernel/itimer.c
 *
 * Copyright (C) 1992 Darren Senn
 */

/* These are all the functions necessary to implement itimers */

#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/time.h>

#include <asm/segment.h>

static unsigned long tvtojiffies(struct timeval *value)
{
	return((unsigned long )value->tv_sec * HZ +
		(unsigned long )(value->tv_usec + (1000000 / HZ - 1)) /
		(1000000 / HZ));
}

static void jiffiestotv(unsigned long jiffies, struct timeval *value)
{
	value->tv_usec = (jiffies % HZ) * (1000000 / HZ);
	value->tv_sec = jiffies / HZ;
	return;
}

int _getitimer(int which, struct itimerval *value)
{
	register unsigned long val, interval;

	switch (which) {
	case ITIMER_REAL:
		val = current->it_real_value;
		interval = current->it_real_incr;
		break;
	case ITIMER_VIRTUAL:
		val = current->it_virt_value;
		interval = current->it_virt_incr;
		break;
	case ITIMER_PROF:
		val = current->it_prof_value;
		interval = current->it_prof_incr;
		break;
	default:
		return(-EINVAL);
	}
	jiffiestotv(val, &value->it_value);
	jiffiestotv(interval, &value->it_interval);
	return(0);
}

int sys_getitimer(int which, struct itimerval *value)
{
	struct itimerval get_buffer;
	int k;

	if (!value)
		return -EFAULT;
	k = _getitimer(which, &get_buffer);
	if (k < 0)
		return k;
	verify_area(value, sizeof(struct itimerval));
	memcpy_tofs(value, &get_buffer, sizeof(get_buffer));
	return 0;
}

int _setitimer(int which, struct itimerval *value, struct itimerval *ovalue)
{
	register unsigned long i, j;
	int k;

	i = tvtojiffies(&value->it_interval);
	j = tvtojiffies(&value->it_value);
	if (ovalue && (k = _getitimer(which, ovalue)) < 0)
		return k;
	switch (which) {
		case ITIMER_REAL:
			current->it_real_value = j;
			current->it_real_incr = i;
			break;
		case ITIMER_VIRTUAL:
			current->it_virt_value = j;
			current->it_virt_incr = i;
			break;
		case ITIMER_PROF:
			current->it_prof_value = j;
			current->it_prof_incr = i;
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

int sys_setitimer(int which, struct itimerval *value, struct itimerval *ovalue)
{
	struct itimerval set_buffer, get_buffer;
	int k;

	if (!value)
		memset((char *) &set_buffer, 0, sizeof(set_buffer));
	else
		memcpy_fromfs(&set_buffer, value, sizeof(set_buffer));
	k = _setitimer(which, &set_buffer, ovalue ? &get_buffer : 0);
	if (k < 0 || !ovalue)
		return k;
	verify_area(ovalue, sizeof(struct itimerval));
	memcpy_tofs(ovalue, &get_buffer, sizeof(get_buffer));
	return 0;
}
