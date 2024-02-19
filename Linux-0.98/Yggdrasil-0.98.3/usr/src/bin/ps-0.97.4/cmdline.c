/*
 * cmdline.c	- get command line arguments / environment
 *
 * Copyright (c) 1992 Branko Lankester
 *
 */
/* #define _LINUX_TYPES_H */
#include <termios.h>
#include <limits.h>
#include <string.h>
#include <linux/sched.h>
#include "ps.h"

#define	STKBUF	2048	/* must be bigger than MAXCMD */

int maxcmd = 132;


/*
 * return string of command and arguments of task
 */
char *
cmd_args(task)
struct task_struct *task;
{
    unsigned long st_stk;	/* virtual address of data in buf */
    static unsigned long buf[STKBUF/4 + 8]; 
    int i, n;
    int argc, argv0;
    char *p, *commline;

    if (task == NULL)
	return("");

    if (kern_comm) {
	buf[0] = 0;
	return(strncat((char *) buf, task->comm, 
		sizeof task->comm));
    }
    if (task->state == TASK_ZOMBIE)
	return("<defunct>");
    if (task->pid == 0)
	return("swapper");
    
    st_stk = task->start_stack;
    if ((n = vmread(buf, task->tss.cr3, task->start_code + st_stk, STKBUF)) <= 0)
	goto bad;

    argc = buf[0];
    if (argc <= 0 || argc + 3 >= n/4 || buf[argc + 3] != 0) {
	int envp = buf[2];
	if (envp > st_stk + 16 && envp < TASK_SIZE)
	    argc = (envp - st_stk - 16) / 4;
	else {
	    for (i = 3; i < n/4; ++i) {
		int badp = 0;
		if (buf[i] == 0) break;
		if (buf[i] < st_stk || buf[i] >= TASK_SIZE)
		    if (++badp == 4)
			goto bad;
	    }
	    argc = i - 3;
	}
    }
    if (argc == 0)
	goto bad;

    argv0 = buf[3];
    if (argv0 < st_stk + 24 || argv0 > TASK_SIZE) {
	for (i = argc + 4; i < n/4; ++i)
	    if (buf[i] == 0) break;
	if (buf[i] != 0)
	    goto bad;
	argv0 = st_stk + i*4 + 8;
    }
    if (argv0 - st_stk + maxcmd + 1 > STKBUF) {
	/*
	 * arg strings not in buffer
	 * read some bytes before argv[0] to find the real start
	 * value of argv[0] (prog may have incremented argv[0])
	 */
	st_stk = argv0 - 16;
	n = vmread(buf, task->tss.cr3, task->start_code + st_stk, maxcmd+16);
	if ((n -= 16) <= 0)
	    goto bad;
    } else {
	n -= (argv0 - st_stk) + 4;	/* 4 last bytes always 0 */
    }
    p = (char *) buf + argv0 - st_stk;
    for (i = 0; i < 16; ++i)
	if (*--p == '\0')
	    break;
    if (*p != '\0')
	p += 16;
    else {
	n += i;
	++p;
    }

    if (maxcmd < n)
	n = maxcmd;
    commline = p;
    p[n] = '\0';
    while (1) {
	for (; *p != '\0'; ++p)
	    if (*p < ' ' || *p > '\176')
		*p = ' ';
	if (p < commline + n && (--argc > 0 || show_env))
	    *p = ' ';
	else
	    break;
    }
    /* if cmd empty or dash (login shell): also give real shell name */
    if (!show_env && (*commline == '-' || *commline == '\0')) {
	strcat(commline, " (");
	strncat(commline, task->comm, sizeof task->comm);
	strcat(commline, ")");
	commline[maxcmd] = '\0';
    }
    return(commline);

bad:
    strcpy((char *) buf, " (");
    strncat((char *) buf, task->comm, sizeof task->comm);
    strcat((char *) buf, ")");
    return((char *) buf);
}


vmread(buf, pdir, addr, n)
char *buf;
unsigned long pdir;
unsigned long addr;
int n;
{
    
	unsigned long pde, pte, tmp;
	int cnt, count = n;
	static unsigned long high_mem;
	int rd;

	if (count <= 0)
		return(0);
	
	if (!high_mem) {
		high_mem = get_kword(k_addr("_high_memory"));
	}
rep:
	pde = addr >> 20 & 0xffc;
	if (((pte = get_kword(pdir + pde)) & 1) == 0)
		return(n - count);	/* page table not present */
	pte &= 0xfffff000;
	pte += addr >> 10 & 0xffc;

	cnt = 0x1000 - (addr & 0xfff);
	while (1) {
		if (cnt > count)
			cnt = count;
		if ((tmp = get_kword(pte)) == 0)
			break;
		if (tmp & 1) {
			tmp = (tmp & 0xfffff000) + (addr & 0xfff);
			if (tmp >= high_mem)
				break;
			rd = kmemread(buf, tmp, cnt);
		} else {
			rd = swapread(buf, (tmp << 11) + (addr & 0xfff), cnt);
		}
		if (rd != cnt) {
			if (rd > 0)
				count -= rd;
			break;
		}
		if ((count -= cnt) == 0)
			break;
		buf += cnt;
		addr += cnt;
		cnt = 0x1000;
		if (((pte += 4) & 0xfff) == 0)
			break; /* goto rep; */
	}
	return(n - count);
}

swapread(buf, pos, n)
char *buf;
off_t pos;
int n;
{
    static int swapfd = 0;

    if (!swapfd)
	swapfd = open(swappath, 0);

    if (swapfd == -1)
	return(0);

    lseek(swapfd, pos, 0);
    return(read(swapfd, buf, n));
}
