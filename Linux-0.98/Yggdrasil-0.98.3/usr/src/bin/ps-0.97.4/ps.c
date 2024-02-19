/*
 * ps.c		- show process status
 *
 * Copyright (c) 1992 Branko Lankester
 *
 */

#include <linux/types.h>
#define _SYS_TYPES_H
#define _SIGNAL_H
#define _TIME_H
#define _SYS_RESOURCE_H
#define _SYS_TIME_H
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include "ps.h"
#include "psdata.h"


#define	PS_D	0	/* default format (short) */
#define	PS_L	1	/* long format */
#define	PS_U	2	/* user format */
#define	PS_J	3	/* jobs format */
#define	PS_S	4	/* signal format */
#define	PS_V	5	/* vm format */
#define	PS_M	6	/* mem. stuff */
#define	PS_X	7	/* regs etc., for testing */

char *hdrs[] = {
"  PID TT STAT  TIME COMMAND",
" F   UID   PID  PPID PRI NI SIZE  RSS WCHAN      STAT TT   TIME COMMAND",
"USER       PID %CPU %MEM SIZE  RSS TT STAT START   TIME COMMAND",
" PPID   PID  PGID   SID TT TPGID  STAT   UID   TIME COMMAND",
"  UID   PID SIGNAL   BLOCKED  IGNORED  CATCHED  STAT TT   TIME COMMAND",
"  PID TT STAT  TIME  PAGEIN TSIZ DSIZ  RSS   LIM %MEM COMMAND",
"  PID TT MAJFLT MINFLT  TRS  DRS SIZE SWAP  RSS SHRD  LIB  DT COMMAND",
"NR   PID    STACK      ESP      EIP TMOUT ALARM STAT TT   TIME COMMAND"
};

extern (*fmt_fnc[])();	/* forward declaration */

unsigned char *memmap;
unsigned pages;
unsigned long low_memory;
unsigned long main_mem;
unsigned long startup_time;
unsigned long jiffies;

/*
 * command line options
 */
int fmt;
int all;
int kern_comm;
int no_ctty;
int run_only;
char *ctty;
pid_t pid = -1;
int show_env;
int num_outp;		/* numeric fields for user or wchan */
int first_task = 1;	/* don't show task 0 */
int pg_shift = 2;	/* default: show k instead of pages */
int Sum;


main(argc, argv)
char **argv;
{
    char *p;
    int no_header = 0;
    int fopt = 0;
    int width = 0;
    int Update = 0;


repeat:
    if (argc > 1) {
	for (p = argv[1]; *p; ++p) {
	    switch (*p) {
		case 'l': fmt = PS_L; ++fopt; break;
		case 'u': fmt = PS_U; ++fopt; break;
		case 'j': fmt = PS_J; ++fopt; break;
		case 's': fmt = PS_S; ++fopt; break;
		case 'v': fmt = PS_V; ++fopt; break;
		case 'm': fmt = PS_M; ++fopt; break;
		case 'X': fmt = PS_X; ++fopt; break; /* regs */
		case 'a': all = 1; break;
		case 'c': kern_comm = 1; break;
		case '0': first_task = 0; /*falltrough*/
		case 'x': no_ctty = 1; break;
		case 't': ctty = p + 1; break;
		case 'r': run_only = 1; break;
		case 'e': show_env = 1; break;
		case 'w': ++width; break;
		case 'h': no_header = 1; break;
		case 'n': num_outp = 1; break;
		case 'S': Sum = 1; break;
		case 'p': pg_shift = 0; break;
		case 'U': Update = 1; break;
#ifdef DEBUG
		case 'd': ++Debug; break;
#endif
		case 'g':	/* old flag, ignore */
		case '-': break;
		default:
		    if (*p >= '0' && *p <= '9') {
			pid = atoi(p);
		    } else
			usage();
	    }
	    if (ctty || pid != -1)
		break;		/* pid and tty always last */
	}
	if (fopt > 1) {
	    fprintf(stderr, "ps: specify only one of j,l,s,u,v\n");
	    exit(1);
	}
    }
    if (argc > 2 && argv[2][0] == '-') {
	++argv;
	--argc;
	goto repeat;
    }

    /*
     * only allow different namelist if already read access to /dev/kmem
     */
    if (argc > 2 && access(kmem_path, 4 /*R_OK*/)) {
	perror(kmem_path);
	exit(1);
    }

    if (argc > 3)
	swappath = argv[3];

    if (open_sys(argc > 2 ? argv[2] : NULL, Update) == -1) {
	perror(argc > 2 ? argv[2] : "cannot open psdatabase");
	exit(1);
    }

    set_maxcmd(width);
    read_globals();
    if (!no_header)
	puts(hdrs[fmt]);
    show_procs();
    exit(0);
}


usage()
{
    fprintf(stderr, "usage:  ps acehjlnrsSuUvwx{t<tty>,#} [system-path] [swap-path]\n");
    exit(1);
}


/*
 * set maximum chars displayed on a line
 */
set_maxcmd(w_opts)
{
    struct winsize win;
    int cols = 80;

    if (ioctl(1, TIOCGWINSZ, &win) != -1 && win.ws_col > 0)
	cols = win.ws_col;

    switch (w_opts) {
	case 0: break;
	case 1: cols += 52; break;	/* 80 -> 132 */
	case 2: cols *= 2; break;
	default: cols = MAXCMD;
    }
    maxcmd = cols - strlen(hdrs[fmt]) + 6;
}



show_procs()
{
    struct task_struct *taskp;
    union task_union task_buf;
    int tty, i, uid;
    off_t _task = k_addr("_task");

    uid = getuid();

    if (ctty)
	tty = tty_to_dev(ctty);

    for (i = first_task; i < NR_TASKS; ++i) {
	kmemread(&taskp, _task + 4*i, 4);
	if (taskp) {
	    kmemread(&task_buf, taskp, sizeof(task_buf));
			/* check if valid, proc may have exited */
	    if ((unsigned) task_buf.task.state > 4 ||
		    task_buf.task.pid <= 0 && i != 0)
		continue;

	    if (pid >= 0) {
		if (task_buf.task.pid != pid)
		    continue;
	    } else if (ctty) {
		if (task_buf.task.tty != tty)
		    continue;
	    } else
		if (!all && task_buf.task.uid != uid ||
		    !no_ctty && task_buf.task.tty == -1 ||
		    run_only && task_buf.task.state != TASK_RUNNING &&
			    task_buf.task.state != TASK_UNINTERRUPTIBLE)
			continue;

	    (fmt_fnc[fmt])(&task_buf);
	    if (fmt != PS_V && fmt != PS_M)
		show_time(&task_buf);
	    printf("%s\n", cmd_args(&task_buf));
	}
    }
}



show_short(task)
struct task_struct *task;
{
    printf("%5d %s %s",
	task->pid,
	dev_to_tty(task->tty),
	status(task));
}

show_long(task)
struct task_struct *task;
{
    long ppid;

    kmemread(&ppid, &task->p_pptr->pid, 4);

    printf("%2x %5d %5d %5d %3d %2d %4d %4d %-10.10s %s %s ",
	task->flags | (task->used_math ? 4 : 0),
	task->euid,
	task->pid,
	ppid,
	2 * PZERO - task->counter,	/* sort of priority */
	PZERO - task->priority,		/* nice value */
	VSIZE(task),
	task->rss * 4,
	(task->state == TASK_INTERRUPTIBLE ||
	 task->state == TASK_UNINTERRUPTIBLE ? 
	    wchan(task->tss.ebp, task) : ""),
	status(task),
	dev_to_tty(task->tty));
}

show_jobs(task)
struct task_struct *task;
{
    long ppid, tpgid;
    struct tty_struct *tt;
    int tty;

    if ((tty = task->tty) != -1) {
	tty &= 0xff;
	tt = (struct tty_struct *) k_addr("_tty_table");
	kmemread(&tpgid, &tt[tty - (tty < 64 ? 1 : 0)].pgrp, 4);
    } else
	tpgid = -1;

    kmemread(&ppid, &task->p_pptr->pid, 4);

    printf("%5d %5d %5d %5d %s %5d  %s %5d ",
	ppid,
	task->pid,
	task->pgrp,
	task->session,
	dev_to_tty(task->tty),
	tpgid,
	status(task),
	task->euid);
}

show_user(task)
struct task_struct *task;
{
    time_t now, start;
    int pcpu, pmem;

    if (num_outp)
	printf("%5d    ", task->euid);
    else
	printf("%-8s ", user_from_uid(task->euid));

    now = time(0L);
    start = startup_time + task->start_time / HZ;
    if (now == start)
	pcpu = 0;
    else
	pcpu = (task->utime + task->stime) * (1000/HZ) / (now - start);
    pmem = task->rss * 1000 / (main_mem / 4096);

    printf("%5d %2d.%d %2d.%d %4d %4d %s %s%.6s ",
	task->pid,
	pcpu / 10, pcpu % 10,
	pmem / 10, pmem % 10,
	VSIZE(task),
	task->rss * 4,
	dev_to_tty(task->tty),
	status(task),
 	ctime(&start) + (now - start > 3600*24 ? 4 : 10));
}

show_sig(task)
struct task_struct *task;
{
    unsigned long sigignore=0, sigcatch=0, bit=1;
    int i;

    for (i=0; i<32; ++i) {
	switch((int) task->sigaction[i].sa_handler) {
	    case 1: sigignore |= bit; break;
	    case 0: break;
	    default: sigcatch |= bit;
	}
	bit <<= 1;
    }
    printf("%5d %5d %08x %08x %08x %08x %s %s ",
	task->euid,
	task->pid,
	task->signal,
	task->blocked,
	sigignore,
	sigcatch,
	status(task),
	dev_to_tty(task->tty));
}

show_vm(task)
struct task_struct *task;
{
    int pmem;

    printf("%5d %2s %s",
	task->pid,
	dev_to_tty(task->tty),
	status(task));
    show_time(task);
    printf(" %6d %4d %4d %4d ",
	task->maj_flt + (Sum ? task->cmaj_flt : 0),
	task->end_code / 1024,
	SIZE(task), task->rss*4);
    if (task->rlim[RLIMIT_RSS].rlim_cur == RLIM_INFINITY)
	printf("   xx ");
    else
	printf("%5d ", task->rlim[RLIMIT_RSS].rlim_cur / 1024);
    pmem = task->rss * 1000 / (main_mem / 4096);
    printf("%2d.%d ", pmem / 10, pmem % 10);
}


show_m(task)
struct task_struct *task;
{
    int i;
    unsigned long buf[PAGE_SIZE/4], *pte;
    unsigned long pdir, ptbl;
    int size=0, resident=0;
    int share=0, trs=0, lrs=0, drs=0;
    int dt=0;
    int tpag = task->end_code / PAGE_SIZE;
    unsigned map_nr;

    if (memmap == NULL)
	get_memmap();

    pdir = task->tss.cr3 + (task->start_code >> 20);
    for (i = 0; i < 16; ++i, pdir += 4) {
	ptbl = get_kword(pdir);
	if (ptbl == 0) {
	    tpag -= 1024;
	    continue;
	}
	kmemread(buf, ptbl & 0xfffff000, sizeof buf);
	for (pte = buf; pte < &buf[1024]; ++pte) {
	    if (*pte != 0) {
		++size;
		if (*pte & 1) {
		    ++resident;
		    if (tpag > 0)
			++trs;
		    else
			++drs;

		    if (i == 15) {
			++lrs;
			if (*pte & 0x40)
			    ++dt;
			else
			    --drs;
		    }
		    map_nr = MAP_NR(*pte);
		    if (map_nr < pages && memmap[map_nr] > 1)
			++share;
		}
	    }
	    --tpag;
	}
    }
    printf("%5d %2s %6d %6d %4d %4d %4d %4d %4d %4d %4d %3d ", 
	task->pid,
	dev_to_tty(task->tty),
	task->maj_flt + (Sum ? task->cmaj_flt : 0),
	task->min_flt + (Sum ? task->cmin_flt : 0),
	/*task->end_code / 4096 << pg_shift,*/
	trs << pg_shift,
	drs << pg_shift,
	size << pg_shift, 
	size - resident << pg_shift,
	resident << pg_shift,
	share << pg_shift,
	lrs << pg_shift, 
	dt << pg_shift);
}

show_regs(task)
struct task_struct *task;
{
    printf("%2d %5d %8x %8x %8x ",
	task->start_code >> 26,
	task->pid,
	/**
	task->start_code >> 16,
	**/
	task->start_stack,
	KSTK_ESP(task),
	KSTK_EIP(task));

    prtime(task->timeout, jiffies);
    prtime(task->it_real_value, 0);

    printf("%s %2s ",
	status(task),
	dev_to_tty(task->tty));
}

prtime(t, rel)
unsigned long t, rel;
{
    if (t == 0) {
	printf("      ");
	return;
    }
    if ((long) t == -1) {
	printf("   xx ");
	return;
    }
    if ((long) (t -= rel) < 0)
	t = 0;
    
    if (t > 9999)
	printf("%5d ", t / 100);
    else
	printf("%2d.%02d ", t / 100, t % 100);
}

int (*fmt_fnc[])() = {
    show_short,
    show_long,
    show_user,
    show_jobs,
    show_sig,
    show_vm,
    show_m,
    show_regs
};


show_time(task)
struct task_struct *task;
{
    unsigned t;

    t = (task->utime + task->stime) / HZ;
    if (Sum)
	t += (task->cutime + task->cstime) / HZ;

    printf("%3d:%02d ", t / 60, t % 60);
}

char *
status(task)
struct task_struct *task;
{
    static char buf[5] = "    ";

    buf[0] = "RSDZT" [task->state];
    buf[1] = (task->rss == 0 && task->state != TASK_ZOMBIE ? 'W' : ' ');
    if (task->priority > PZERO)
	buf[2] = '<';
    else if (task->priority < PZERO)
	buf[2] = 'N';
    else
	buf[2] = ' ';
    return(buf);
}


char *
wchan(ebp, stack)
reg_t ebp;
reg_t *stack;
{
    reg_t eip;
    int bp;
    static char buf[16], *p;

    bp = (ebp & PAGE_MASK) >> 2;
    eip = stack[bp + 1];
    p = find_func(eip);
    if (strcmp(p, "sleep_on") == 0 ||
		strcmp(p, "interruptible_sleep_on") == 0)
	return(wchan(stack[bp], stack));
    
    if (num_outp) {
	sprintf(buf, "%x", eip);
	return(buf);
    }
    if (strncmp(p, "sys_", 4) == 0)
	p += 4;
    return(p);
}

get_memmap()
{
    static unsigned long _mem_map;

    if (memmap == NULL) {
	_mem_map = get_kword(k_addr("_mem_map"));
	memmap = (unsigned char *) xmalloc(pages);
    }
    kmemread(memmap, _mem_map, pages);
}

read_globals()
{

    main_mem = get_kword(k_addr("_high_memory"));
    pages = main_mem / 4096;
    startup_time = get_kword(k_addr("_startup_time"));
    jiffies = get_kword(k_addr("_jiffies"));
}
