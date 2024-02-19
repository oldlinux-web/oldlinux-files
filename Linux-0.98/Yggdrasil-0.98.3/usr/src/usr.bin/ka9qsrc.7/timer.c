#include <stdio.h>
#include "global.h"
#include "timer.h"

/* Head of running timer chain */
struct timer *timers;

tick()
{
	register struct timer *t,*tp;
	register struct timer *expired = NULLTIMER;
	char i_state;
	void iostop();

	/* Run through the list of running timers, decrementing each one.
	 * If one has expired, take it off the running list and put it
	 * on a singly linked list of expired timers
	 */
	i_state = disable();
	for(t = timers;t != NULLTIMER; t = tp){
		tp = t->next;
		if(tp == t){
			restore(i_state);
			printf("PANIC: Timer loop at %lx\n",(long)tp);
			iostop();
			exit(1);
                }
		if(t->state == TIMER_RUN && --(t->count) == 0){
			stop_timer(t);
			t->state = TIMER_EXPIRE;
			/* Put on head of expired timer list */
			t->next = expired;
			expired = t;
		}
	}
	restore(i_state);
	/* Now go through the list of expired timers, removing each
	 * one and kicking the notify function, if there is one
	 */
	/* Note: the check for TIMER_EXPIRE below has a specific */
	/* purpose.  It prevents wasted timer calls to the NET/ROM */
	/* transport protocol timeout routine.  This routine does */
	/* not know which timer expired, so it scans and processes */
	/* the whole window.  If multiple timers expired, it handles */
	/* them all and resets their states to something other than */
	/* TIMER_EXPIRE.  So, we oblige here by not re-processing */
	/* them under those circumstances. */
	
	while((t = expired) != NULLTIMER){
		expired = t->next;
		if(t->state == TIMER_EXPIRE && t->func){
			(*t->func)(t->arg);
		}
	}
}
/* Start a timer */
start_timer(t)
register struct timer *t;
{
	char i_state;

	if(t == NULLTIMER || t->start == 0)
		return;
	i_state = disable();
	t->count = t->start;
	if(t->state != TIMER_RUN){
		t->state = TIMER_RUN;
		/* Put on head of active timer list */
		t->prev = NULLTIMER;
		t->next = timers;
		if(t->next != NULLTIMER)
			t->next->prev = t;
		timers = t;
	}
	restore(i_state);
}
/* Stop a timer */
stop_timer(t)
register struct timer *t;
{
	char i_state;

	if(t == NULLTIMER)
		return;
	i_state = disable();
	if(t->state == TIMER_RUN){
		/* Delete from active timer list */
		if(timers == t)
			timers = t->next;
		if(t->next != NULLTIMER)
			t->next->prev = t->prev;
		if(t->prev != NULLTIMER)
			t->prev->next = t->next;
	}
	t->state = TIMER_STOP;
	restore(i_state);
}
