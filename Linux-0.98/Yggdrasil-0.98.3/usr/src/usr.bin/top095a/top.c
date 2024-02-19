/*
 * top.c		- control processes
 *
 * Copyright (c) 1992 Roger Binns
 * based entirely on ps which is
 * Copyright (c) 1992 Branko Lankester
 *
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <fcntl.h>
#include <ctype.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <termcap.h>
#include <termio.h>
#include "ps.h"

unsigned long main_mem;
unsigned long startup_time;
int pg_shift=2;

union task_union {
    struct task_struct task;
    reg_t stack[PAGE_SIZE/4];
};

#define RCFILE		".toprc"
#define MAXSCREENWIDTH	160
#define MAXSCREENHEIGHT	100
#define	_SSIZE(task)	(LIBRARY_OFFSET - KSTK_ESP(task))
#define	SSIZE(task)	((task)->pid == 0 ? 0 : _SSIZE(task))
#define	VSIZE(task)	(((task)->brk + 1023 + SSIZE(task)) / 1024)
#define	SIZE(task)	(((task)->brk - (task)->end_code + 1023 + \
			  SSIZE(task)) / 1024)

int first_task=1;
unsigned char *memmap;
unsigned pages;
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

void redraw(void)
{
	char *on, *off;
	char goodlook[100];
	int i;
	on=so; off=se;
	goodlook[0]=0;
	for(i=0; i<(cols-strlen(title))/2; i++) strcat(goodlook, " ");
	printf("%s%s%s%s%s%s\n", cl, so, goodlook , title, goodlook, se);
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
	
void main(void)
{
	char *buffer=0; 
	char *termtype=getenv("TERM");
	int width, i;
	char c;
	struct termio newtty;

	if(!termtype) { printf("TERM not set\n"); exit(1);}
	close(0);
	if(open("/dev/tty", O_RDONLY)) printf("stdin is not there\n");
	if(ioctl(0, TCGETA, &savetty) == -1)
		{
			printf("stdin must be a tty\n");
			exit(1);
		}
	signal(SIGHUP, sigcatch);
	newtty=savetty;
	newtty.c_lflag&=~ICANON;
	newtty.c_lflag&=~ECHO;
	newtty.c_cc[VMIN]=0;
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
	cols=width=tgetnum("co");
	lines=tgetnum("li");
	if(cols>MAXSCREENWIDTH-2) cols=MAXSCREENWIDTH-2; /* leave space for \n and \0 */
	if(lines>MAXSCREENHEIGHT) lines=MAXSCREENHEIGHT;
	maxlines=lines-5;
	open_psdb();
	read_globals();
	setp();
	redraw();
	while(1)
	{
	idle=freemem=1000;
	show_procs();
	outstr[0]=0;
	sort_vals();
	if(idle<0) idle=0;
	if(freemem<0) freemem=0;
	printf("%s%2d%% idle, %2d%% free core - sorted by %s", tgoto(cm,0,3), idle/10, freemem/10, corm?"CPU use    ":"Memory use "); /* can't have two tgoto's - they use a static buffer */
	printf("%s%s%s", tgoto(cm,0,5), clrtobot, outstr);
	/* a select should work here - it doesn't hence the following */
	
	for(i=0; i<sleeptime; i++) { if(read(0, &c, 1)==1) do_key(c); sleep(1);}
	if(read(0, &c, 1)==1) do_key(c); 
	/* flush keyb buffers  - don't ask */
	while(read(0, &c, 1)==1) ;
	
	}
	close_psdb();
}

void do_key(char c)
{
	int errno=0;
	int npid, nval;
	char spid[100];

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
		if(c=='K')
		{
		printf(tgoto(cm, 0, 3));
		printf(clrtoeol);
		printf("Kill pid:");
		gets(spid);
		npid=atoi(spid);
		}
		else npid=getpid();

		printf(tgoto(cm, 0, 3));
		printf(clrtoeol);
		printf((c=='N')?"Nice pid: %d Value:":"Kill pid: %d signal:",npid);
		gets(spid);
		nval=atoi(spid);
		printf(tgoto(cm, 0, 3));
		printf(clrtoeol);
		printf((c=='N')?"Nice pid: %d Value: %d":"Kill pid: %d signal: %d",npid, nval);
		sleep(2);
		if(c=='N') nice(nval);
		else kill(npid, nval);
		if(errno)
		{
			printf(tgoto(cm, 0,3));
			printf(clrtoeol);
			printf(strerror(errno));
			sleep(2);
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
	printf("%s%s", tgoto(cm, 0,3), clrtoeol);
	ioctl(0, TCSETA, &rawtty);
}
			

show_procs()
{
    struct task_struct *taskp;
    union task_union task_buf;
    int tty, i, uid;
    off_t _task = k_addr("_task");

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
    	int pcpu, pmem;
	long ppid;
	int TRS, SWAP, SHARE, A, WP, D;
	char tmp[200];
	
    	if(task->pid==0) return ; 
	kmemread(&ppid, &task->p_pptr->pid, 4);
 	t = (task->utime + task->stime) / HZ;
    	if (Sum)
		t += (task->cutime + task->cstime) / HZ;

    	now = time(0L);
    	start = startup_time + task->start_time / HZ;
    	if (now == start)
		pcpu = 0;
    	else
		pcpu = (task->utime + task->stime) * (1000/HZ) / (now - start);
    	pmem = task->rss * 1000 / (main_mem / 4096);
	if(pcpu>999) pcpu=999;
	idle-=pcpu;
	freemem-=pmem;
	if(corm) vals[num]=pcpu;
	else vals[num]=pmem;
	if(pflags[P_TRS] || pflags [P_SWAP] || pflags[P_SHARE] || pflags[P_A] || pflags[P_WP] || pflags[P_D])
		membreakdown(task, &TRS, &SWAP, &SHARE, &A, &WP, &D);

	for(i=0; i<P_END; i++)
	{
		if(!pflags[i]) continue;
		tmp[0]=0;
		switch(i)
		{
		case P_PID: sprintf(tmp, "%5d ", task->pid); break;
		case P_PPID: sprintf(tmp, "%5d ", ppid); break;
		case P_UID: sprintf(tmp, "%4d ", task->euid); break;
		case P_USER: sprintf(tmp, "%-8.8s ", getpwuid(task->euid)->pw_name); break;
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
    if (memmap == NULL) {
	pages = (get_kword(k_addr("_HIGH_MEMORY")) - LOW_MEM) >> 12;
	memmap = (unsigned char *) xmalloc(pages);
    }
    kmemread(memmap, k_addr("_mem_map"), pages);
}

read_globals()
{
    main_mem =  get_kword(k_addr("_memory_end")) -
		get_kword(k_addr("_main_memory_start"));
    startup_time = get_kword(k_addr("_startup_time"));
}


/*
 * ttynames:
 * 	ttya0 a1 a2...	virtual consoles
 *	ttyA0 A1 	serial lines
 *	ttyp0 p1 p2...	pty's
 */

static char *ttgrp = "abcdABCDpqrsPQRS";
static char *ttsub = "0123456789abcdef";

char *
dev_to_tty(int dev)
{
    static char tty[3];

    if (dev == -1)
	return "? ";
    if (dev == 0)
	return "co";

    tty[0] = ttgrp[(dev >> 4) & 017];
    tty[1] = ttsub[dev & 017];
    return(tty);
}

tty_to_dev(tty)
char *tty;
{
    char *p, *q;

    if (*tty == '\0') {		/* empty string: controlling tty */
	struct stat buf;
	if (fstat(0, &buf) != -1)
	    return(buf.st_rdev & 0xff);
	else
	    return -1;
    }
    if (tty[1] == '\0' && isdigit(*tty))
	return(*tty - '0');
    if (strcmp(tty, "co") == 0)
	return 0;
    if ((p = strchr(ttgrp, *tty)) != NULL &&
	(q = strchr(ttsub, tty[1])) != NULL)
	return(((p - ttgrp) << 4) | (q - ttsub));
    else
	return -1;
}

membreakdown(task, TRS, SWAP, SHARE, A, WP, D)
struct task_struct *task;
int *TRS, *SWAP, *SHARE, *A, *WP, *D;
{
    int i;
    unsigned long buf[PAGE_SIZE/4], *pte;
    unsigned long pdir, ptbl;
    int size=0, resident=0;
    int wp=0, dirty=0, acc=0, share=0, trs=0;
    int tpag = task->end_code / PAGE_SIZE;

    if (memmap == NULL)
	get_memmap();

    pdir = task->start_code >> 20;
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
		    if (memmap[MAP_NR(*pte)] > 1)
			++share;
		    if (*pte & 0x40) ++dirty;
		    if (*pte & 0x20) ++acc;
		}
	    }
	    --tpag;
	}
    }
	*TRS=trs << pg_shift;
	*SWAP=size - resident << pg_shift;
	*SHARE=share << pg_shift;
	*A=acc;
	*WP=wp;
	*D=dirty;
}

