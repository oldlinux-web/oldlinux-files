/*
 * top.c		- control processes
 *
 * Copyright (c) 1992 Roger Binns
 * based entirely on ps which is
 * Copyright (c) 1992 Branko Lankester
 *
 */

/** gives problems with gcc 2.1
#include <stdlib.h>
#include <string.h>
**/
#include <linux/types.h>
#define _SYS_TYPES_H
#define _SIGNAL_H
#define _TIME_H
#define _SYS_RESOURCE_H
#define _SYS_TIME_H
#define _TERMIOS_H
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>
#include <ctype.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <termcap.h>
#include <termio.h>
#include "ps.h"

char *getenv();
char *strerror();
char *strchr();
extern int errno;

#undef toupper
#define toupper(x)	(((x) >= 'a' && (x) <= 'z') ? (x) - 0x20 : (x))

unsigned long main_mem;
int pg_shift=2;


#define RCFILE		".toprc"
#define MAXSCREENWIDTH	160
#define MAXSCREENHEIGHT	100

int first_task=1;
int newmap=1;
unsigned char *memmap;
unsigned pages;
unsigned long low_memory;
extern int maxcmd;
/* some dummy variables to get cmdline.c to link */
int kern_comm=0;
int show_env=0;
int idle,freemem;
char hdr[200];
char *title="TOP   by Roger Binns    Ps (c) 1992 Branko Lankester";

void do_it(struct task_struct *task, int num);
void do_key(char c);
char *cm, *clrtobot, *cl, *so, *se, *clrtoeol, *mb, *md, *us, *ue;
char *outp;
int vals[NR_TASKS];
int ticks[NR_TASKS];
int interval;
char *outstr;
int corm=1;
int cols, lines;
int Sum=0; /* child info included? */
int maxlines;
struct termio savetty;
struct termio rawtty;
int sleeptime=2;
char pflags[26];
enum { P_PID, P_PPID, P_UID, P_USER, P_PCPU, P_PMEM, P_TTY, P_PRI, P_NICE, P_PAGEIN, P_TSIZ, P_DSIZ, P_SIZE, 
	P_TRS, P_SWAP, P_SHARE, P_A, P_WP, P_D, P_RSS, P_WCHAN, P_STAT, P_TIME, P_COMMAND, P_END };

void show_title()
{
	char goodlook[100];
	memset(goodlook, ' ', sizeof goodlook);
	goodlook[(cols-strlen(title))/2] = '\0';
	printf("%s%s%s%s%s%s\n", cl, so, goodlook , title, goodlook, se);
}

void redraw(void)
{
	char *on, *off;
	on=so; off=se;

	show_title();
	printf("%s", tgoto(cm, 0, 1));	
	printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%c%s%s%s%s%s%s", 
		on, "q", off, "uit ", on, "k", off, "ill ", on, "n", off, "ice  Sort by ", on, "c", off, "pu or ", on,
		"m", off, "emory use ", on, "A-",'A'+P_END-1, off, " Fields ", on, "0-9", off, " update delay ^Write");
	printf("%s%s", tgoto(cm, 0, 4), hdr);	
}

void sort_vals(void)
{
	int it, highest, i, done=0;

	while(done<maxlines)
	{
	it=-1;
	highest=0;
	for(i=0; i<NR_TASKS; i++)
		{
			if(vals[i]<highest) continue;
			it=i; 
			highest=vals[i];
		}
	if(it<0) return;
	strcat(outstr, outp+it*MAXSCREENWIDTH);
	done++;
	vals[it]=-1;
	}
}

char *headers[]={"  PID "," PPID "," UID ","USER     ","%CPU ","%MEM ","TT ","PRI "," NI ","PAGEIN ",
		 "TSIZE ","DSIZE "," SIZE "," TRS ", "SWAP ", "SHARE ", "  A ", " WP ", "  D ", " RSS ",
		 "WCHAN     ","STAT ","  TIME ","COMMAND"
		};

void setp(void) /* set up which fields are printed */
{
	int i,c=0;
	for(i=0; i<P_END; i++)
		c+=pflags[i];
	if(!c) /* read from toprc */
		{
			int handle,a;
			char c[27];
			
			chdir(getenv("HOME"));
			handle=open(RCFILE, O_RDONLY);
			if(handle<0)
				for(i=0; i<P_END; i++) pflags[i]=1;	
			else 
				{
					read(handle, c, 27);
					sleeptime=c[0]-'0';
					if(sleeptime<0) sleeptime=0;
					for(a=1; a<27; a++)
						{
							pflags[a-1]=c[a]-('a'+a-1);
							if(pflags[a-1]) pflags[a-1]=1;
						}
					close(handle);
				}	
						
		}
	hdr[0]=0;
	for(i=0; i<P_END; i++)
	{
	if(!pflags[i]) continue;
	strcat(hdr, headers[i]);
	}
	strcat(hdr, "\n");
	maxcmd=cols-strlen(hdr)+8;
	if(maxcmd<5) maxcmd=5; /* will be ignored, dunno how cmdline will like it */
	hdr[cols]=0;
}

void sigcatch(int i)
{
	ioctl(0, TCSETAF, &savetty);
	printf(tgoto(cm, 0,lines-1));
	exit(1);
}

void window_init(int i)
{
    struct winsize ws;

    if (ioctl(1, TIOCGWINSZ, &ws) != -1) {
	cols = ws.ws_col;
	lines = ws.ws_row;
    } else {
	cols=tgetnum("co");
	lines=tgetnum("li");
    }
    if(cols>MAXSCREENWIDTH-2)
	cols=MAXSCREENWIDTH-2; /* leave space for \n and \0 */
    if(lines>MAXSCREENHEIGHT)
	lines=MAXSCREENHEIGHT;
    maxlines=lines-6;
    setp();
    redraw();
    signal(SIGWINCH, window_init);
}

void main(void)
{
	char *buffer=0; 
	char *termtype=getenv("TERM");
	int i;
	char c;
	struct termio newtty;
	int _jiffies, jiffies;
	unsigned long av[3], _aver;
	struct timeval tv;
	fd_set in;

	if(!termtype) { printf("TERM not set\n"); exit(1);}
	close(0);
	if(open("/dev/tty", O_RDONLY)) printf("stdin is not there\n");
	if(ioctl(0, TCGETA, &savetty) == -1)
		{
			printf("stdin must be a tty\n");
			exit(1);
		}
	signal(SIGHUP, sigcatch);
	signal(SIGINT, sigcatch);
	newtty=savetty;
	newtty.c_lflag&=~ICANON;
	newtty.c_lflag&=~ECHO;
	newtty.c_cc[VMIN]=1;
	newtty.c_cc[VTIME]=0;
	if(ioctl(0, TCSETAF, &newtty)==-1)
		{
			printf("cannot put tty into raw mode\n");
			exit(1);
		}
	ioctl(0, TCGETA, &rawtty);
	outp=(char*)calloc(NR_TASKS*MAXSCREENWIDTH, 1);
	outstr=(char*)calloc(MAXSCREENWIDTH*MAXSCREENHEIGHT,1);
	tgetent(buffer, termtype);
	cm=tgetstr("cm", 0);
	clrtobot=tgetstr("cd", 0);
	cl=tgetstr("cl",0);
	so=tgetstr("so",0);
	se=tgetstr("se",0);
	mb=tgetstr("mb",0);
	md=tgetstr("md",0);
	us=tgetstr("us",0);
	ue=tgetstr("ue",0);
	clrtoeol=tgetstr("ce", 0);
	open_psdb();
	_jiffies = k_addr("_jiffies");
	_aver = k_addr("_avenrun");
	read_globals();
	window_init(0);
	while(1) {
	    interval = -jiffies;
	    jiffies = get_kword(_jiffies);
	    interval += jiffies;
	    idle=freemem=1000;
	    show_procs();
	    outstr[0]=0;
	    sort_vals();
	    if(idle<0) idle=0;
	    if(freemem<0) freemem=0;
	    printf("%s%2d%% idle, %2d%% free core - sorted by %s", tgoto(cm,0,3), idle/10, freemem/10, corm?"CPU use    ":"Memory use "); /* can't have two tgoto's - they use a static buffer */
	    kmemread(av, _aver, sizeof av);
	    for (i=0; i<3; ++i) {
		av[i] *= 100;
		av[i] >>= 11;
	    }
	    printf("%s", tgoto(cm, cols-27, 3));
	    i = printf("load avg: %d.%02d, %d.%02d, %d.%02d%s",
		    av[0] / 100, av[0] % 100,
		    av[1] / 100, av[1] % 100,
		    av[2] / 100, av[2] % 100,
		    clrtoeol);
	    printf("%s%s%s", tgoto(cm,0,5), outstr, clrtobot);
	    /*
	    printf("%s%s%s", tgoto(cm,0,5), clrtobot, outstr);
	    */
	    
	    tv.tv_sec = sleeptime;
	    tv.tv_usec = 0;
	    FD_ZERO(&in);
	    FD_SET(0, &in);
	    if (select(16, &in, 0, 0, &tv) > 0)
		if (read(0, &c, 1) == 1)
		    do_key(c);
	}
	close_psdb();
}

void do_key(char c)
{
	int npid, nval;
	char spid[100];

	if (c == '?') {
		do_help();
		return;
	}
	if(c>='A' && c<='Z')
		{
			pflags[c-'A']=1-pflags[c-'A'];
			setp();
			redraw();
			return;
		}
	ioctl(0, TCSETA, &savetty);
	c=toupper(c);
	if(c=='N' || c=='K')
	{
		printf(tgoto(cm, 0, 3));
		printf(clrtoeol);
		if(c=='K')
		    printf("Kill pid:");
		else
		    printf("Nice pid:");
		if (fgets(spid, sizeof spid, stdin) == NULL || *spid == '\n')
		    goto ex;
		npid=atoi(spid);

		printf(tgoto(cm, 0, 3));
		printf(clrtoeol);
		printf((c=='N')?"Nice pid: %d Value:":"Kill pid: %d signal:",npid);
		if (fgets(spid, sizeof spid, stdin) == NULL)
		    goto ex;
		if (*spid == '\n')
		    nval = (c == 'N' ? 5 : 9);
		else
		    nval=atoi(spid);
		printf(tgoto(cm, 0, 3));
		printf(clrtoeol);
		printf((c=='N')?"Nice pid: %d Value: %d":"Kill pid: %d signal: %d",npid, nval);
		errno = 0;
		if(c=='N')
		    setpriority(PRIO_PROCESS, npid, nval);
		else
		    kill(npid, nval);
		if(errno)
		{
			printf(tgoto(cm, 0,3));
			printf(clrtoeol);
			printf("\007%s: %s", c=='N'?"nice":"kill",
				strerror(errno));
			fflush(stdout);
			sleep(3);
		}
	}
	else if(c=='L'-'A'+1) redraw(); /* ctrl L */
	else if(c=='W'-'A'+1)           /* ctrl W */
		{
			int handle,a;
			char c[27];
			chdir(getenv("HOME"));
			handle=open(RCFILE, O_WRONLY | O_CREAT | O_TRUNC, 0600);
			if(handle<0) 
				{
					perror("top");
					exit(1);
				}
			c[0]='0'+sleeptime;
			for(a=0; a<26; a++)
				c[a+1]=(pflags[a])?('A'+a):('a'+a);
			if(write(handle, c, 27)!=27)
				{
					perror("top");	
					exit(1);
				}	
			close(handle);
		}	
	else if(c=='M') corm=0;
	else if(c=='C') corm=1;
	else if(c=='F') kern_comm ^= 1;
	else if(c=='Q') kill(getpid(), 2);
	else if(c>='0' && c <='9')
		{
			sleeptime=c-'0';
			printf(tgoto(cm,0,3));
			printf(clrtoeol);
			printf(sleeptime?"Update now done every %d seconds\n":"No delays in updates now\n",sleeptime);
			sleep(2);
		}		
	else printf("%c", (char)7);
ex:
	printf("%s%s", tgoto(cm, 0,3), clrtoeol);
	ioctl(0, TCSETA, &rawtty);
}

do_help()
{
    int i, changed = 0;
    int row, col;
    char *p, c;

    show_title();
    for (i = 0; i < sizeof headers / sizeof headers[0]; ++i) {
	row = i % (lines-3) + 3;
	col = i / (lines-3) * 26;
	printf("%s", tgoto(cm, col, row));
	for (p = headers[i]; *p == ' '; ++p);
	printf("%c %c: %s", pflags[i] ? '*' : ' ', i + 'A', p);
    }
    while (1) {
	printf("%sToggle fields with %sa-x%s, any other key to return: ",
		tgoto(cm, 0, 1), so, se);
	fflush(stdout);
	read(0, &c, 1);
	i = toupper(c) - 'A';
	if (i >= 0 && i < sizeof headers / sizeof headers[0]) {
	    row = i % (lines-3) + 3;
	    col = i / (lines-3) * 26;
	    printf("%s", tgoto(cm, col, row));
	    if (pflags[i] ^= 1)
		putchar('*');
	    else
		putchar(' ');
	    changed = 1;
	} else
	    break;
    }
    if (changed)
	setp();
    redraw();
}
			

show_procs()
{
    struct task_struct *taskp;
    union task_union task_buf;
    int tty, i, uid;
    off_t _task = k_addr("_task");

    newmap = 1;
    for (i = first_task; i < NR_TASKS; ++i) {
	vals[i]=-1;
	outp[MAXSCREENWIDTH*i]=0;
	kmemread(&taskp, _task + 4*i, 4);
	if (taskp) {
	    kmemread(&task_buf, taskp, sizeof(task_buf));
			/* check if valid, proc may have exited */
	    if ((unsigned) task_buf.task.state > 4 ||
		    task_buf.task.pid <= 0 && i != 0)
		continue;
	    do_it((struct task_struct *)&task_buf, i);
	}
    }
}

void do_it(struct task_struct *task, int num)
{
	unsigned t, i;
    	time_t now, start;
    	int pcpu, pmem, p_ticks;
	long ppid;
	int TRS, SWAP, SHARE, A, WP, D;
	char tmp[200];
	
    	if(task->pid==0) return ; 
	kmemread(&ppid, &task->p_pptr->pid, 4);
 	t = task->utime + task->stime;
	if ((p_ticks = t - ticks[num]) < 0)
		p_ticks = t;	/* new process */
	ticks[num] = t;
	t /= HZ;
    	if (Sum)
		t += (task->cutime + task->cstime) / HZ;

	if (interval)
		pcpu = p_ticks * 1000 / interval;
	else
		pcpu = p_ticks;

	if(pflags[P_TRS] || pflags [P_SWAP] || pflags[P_SHARE] || pflags[P_A] || pflags[P_WP] || pflags[P_D])
		membreakdown(task, &TRS, &SWAP, &SHARE, &A, &WP, &D, &pmem);
	else
		pmem = task->rss * 1000 / (main_mem / 4096);
	if(pcpu>999) pcpu=999;
	idle-=pcpu;
	freemem-=pmem;
	if(corm) vals[num]=(pcpu<<8) + 256 - task->counter;
	else vals[num]=pmem;

	for(i=0; i<P_END; i++)
	{
		if(!pflags[i]) continue;
		tmp[0]=0;
		switch(i)
		{
		case P_PID: sprintf(tmp, "%5d ", task->pid); break;
		case P_PPID: sprintf(tmp, "%5d ", ppid); break;
		case P_UID: sprintf(tmp, "%4d ", task->euid); break;
		case P_USER: sprintf(tmp, "%-8.8s ", user_from_uid(task->euid)); break;
		case P_PCPU: sprintf(tmp, "%2d.%1d ", pcpu/10, pcpu%10); break;
		case P_PMEM: sprintf(tmp, "%2d.%1d ", pmem/10, pmem%10); break;
		case P_TTY: sprintf(tmp, "%-2.2s ", dev_to_tty(task->tty)); break;
		case P_PRI: sprintf(tmp, "%3d ", 2*PZERO-task->counter); break;
		case P_NICE: sprintf(tmp, "%3d ", PZERO-task->priority); break;
		case P_PAGEIN: sprintf(tmp, "%6d ", task->maj_flt+(Sum ? task->cmaj_flt : 0)); break;
		case P_TSIZ: sprintf(tmp, "%5d ", task->end_code/1024); break;
		case P_DSIZ: sprintf(tmp, "%5d ", SIZE(task)); break;
		case P_SIZE: sprintf(tmp, "%5d ", VSIZE(task)); break;
		case P_TRS: sprintf(tmp, "%4d ", TRS); break;
		case P_SWAP: sprintf(tmp, "%4d ", SWAP); break;
		case P_SHARE: sprintf(tmp, "%5d ", SHARE); break;
		case P_A: sprintf(tmp, "%3d ", A); break;
		case P_WP: sprintf(tmp, "%3d ", WP); break;
		case P_D: sprintf(tmp, "%3d ", D); break;
		case P_RSS: sprintf(tmp, "%4d ", task->rss*4); break;
		case P_WCHAN: sprintf(tmp, "%-9.9s ", 
					(task->state == TASK_INTERRUPTIBLE ||
	 				task->state == TASK_UNINTERRUPTIBLE ? 
	    				wchan(task->tss.ebp, task) : "")); break; 
		case P_STAT: sprintf(tmp, "%-4.4s ", status(task)); break;
		case P_TIME: sprintf(tmp, "%3d:%02d ", t/60, t%60); break;
		case P_COMMAND: strcpy(tmp, cmd_args(task)); break; 
		}
		strcat(outp+num*MAXSCREENWIDTH, tmp);
	}
	outp[num*MAXSCREENWIDTH+cols]=0;
	strcat(outp+num*MAXSCREENWIDTH, clrtoeol);
	strcat(outp+num*MAXSCREENWIDTH, "\n");
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
}


membreakdown(task, TRS, SWAP, SHARE, A, WP, D, PMEM)
struct task_struct *task;
int *TRS, *SWAP, *SHARE, *A, *WP, *D, *PMEM;
{
    int i;
    unsigned long buf[PAGE_SIZE/4], *pte;
    unsigned long pdir, ptbl;
    int size=0, resident=0;
    int wp=0, dirty=0, acc=0, share=0, trs=0, pmem=0;
    int tpag = task->end_code / PAGE_SIZE;
    unsigned map_nr;

    if (newmap)
	get_memmap();
    newmap = 0;

    pdir = task->tss.cr3 + (task->start_code >> 20);
    for (i = 0; i < 15; ++i, pdir += 4) {
	ptbl = get_kword(pdir);
	if (ptbl == 0) {
	    tpag -= 1024;
	    continue;
	}
	kmemread(buf, ptbl & 0xfffff000, sizeof buf);
	for (pte = buf; pte < &buf[1024]; ++pte) {
	    if (*pte != 0) {
		++size;
		if (tpag > 0)
		    ++trs;
		if (*pte & 1) {
		    ++resident;
		    if ((*pte & 2) == 0) {
			++wp;
		    }
		    map_nr = MAP_NR(*pte);
		    if (map_nr < pages && memmap[map_nr] > 1) {
			++share;
			pmem += 1000 / memmap[MAP_NR(*pte)];
		    } else
			pmem += 1000;
		    if (*pte & 0x40) ++dirty;
		    if (*pte & 0x20) ++acc;
		}
	    }
	    --tpag;
	}
    }
    pmem /= main_mem / 4096;

    *TRS=trs << pg_shift;
    *SWAP=size - resident << pg_shift;
    *SHARE=share << pg_shift;
    *A=acc;
    *WP=wp;
    *D=dirty;
    *PMEM=pmem;
}

