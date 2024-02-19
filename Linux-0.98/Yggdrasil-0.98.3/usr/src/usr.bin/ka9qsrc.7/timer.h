/* Software timers
 * There is one of these structures for each simulated timer.
 * Whenever the timer is running, it is on a doubly-linked list
 * pointed to by "timers" so that the (hardware) timer interrupt
 * can quickly run through the list and change counts and states.
 * Stopping a timer or letting it expire causes it to be removed
 * from the list; starting a timer puts it on the list.
 */
struct timer {
	struct timer *next;	/* Doubly-linked-list pointers */
	struct timer *prev;
	int32 start;		/* Period of counter (load value) */
	int32 count;		/* Ticks to go until expiration */
	void (*func)();		/* Function to call at expiration */
	char *arg;		/* Arg to pass function */
	char state;		/* Timer state */
#define	TIMER_STOP	0
#define	TIMER_RUN	1
#define	TIMER_EXPIRE	2
};
#define	NULLTIMER	(struct timer *)0
#define	MAX_TIME	(int32)4294967295	/* Max long integer */
#ifndef MSPTICK
#if	ATARI_ST && (!MWC)
#define	MSPTICK		100		/* Milliseconds per tick */
#else
#define	MSPTICK		55		/* Milliseconds per tick */
#endif
#if	defined(NOMAD)
#undef	MSPTICK
#define	MSPTICK		1000		/* one second per tick */
#endif
#endif
/* Useful user macros that hide the timer structure internals */
#define	set_timer(t,x)	(((t)->start) = (x)/MSPTICK)
#define	dur_timer(t)	((t)->start)
#define	read_timer(t)	((t)->count)
#define	run_timer(t)	((t)->state == TIMER_RUN)
