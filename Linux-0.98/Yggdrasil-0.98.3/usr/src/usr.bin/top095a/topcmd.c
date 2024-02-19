/*
 * cmdline.c	- get command line arguments / environment
 *
 * Copyright (c) 1992 Branko Lankester
 *
 */
#include <termios.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <linux/sched.h>
#include "ps.h"

#define	MAXCMD	512	/* max # bytes to copy from args/env */

#define	STKBUF	1024

int maxcmd;


/*
 * return string of command and arguments of task
 */
char *
cmd_args(task)
struct task_struct *task;
{
    unsigned long stackp;	/* virtual address of data in buf */
    unsigned long envp = 0, stringp = 0;
    static unsigned long buf[2*STKBUF]; 
    int stacksize;		/* number of words on stack */
    int i, zeros = -1;
    int arg_cnt = 0, env_cnt = 0;
    int found_argc = 0;

    if (kern_comm) {
	buf[0] = 0;
	return(strncat((char *) buf, task->comm, 
		sizeof task->comm));
    }
    if (task->state == TASK_ZOMBIE)
	return("<defunct>");

    i = STKBUF/4 - 1;	/* highest word is always zero */
    stacksize = (LIBRARY_OFFSET - KSTK_ESP(task)) / 4;
    if (stacksize > ARG_MAX/4)
	stacksize = ARG_MAX/4;
    stackp = LIBRARY_OFFSET - STKBUF;
    if (memread(buf, task->start_code + stackp, STKBUF) != STKBUF)
	goto bad;
    while (--stacksize > 0) {
	unsigned long w;
	if (--i < 0) {
	    stackp -= STKBUF;
	    memcpy(buf+STKBUF/4, buf, STKBUF);
	    if (memread(buf, task->start_code + stackp, STKBUF) != STKBUF)
		goto bad;
	    i = STKBUF/4 - 1;
	}
	w = buf[i];

	if (zeros == -1) {
	    if (w != 0)		/* ignore zeros above string space */
		++zeros;
	    continue;
	}

	if (zeros < 2) {	/* at least 2 zeros before argv[] */
	    if (w == 0) {
		if (++zeros == 1) {
		    stringp = 4 * i + 4;
		    while (*((char *) buf + stringp) == '\0')
			++stringp;
		    stringp += stackp;
		} else
		    envp = stackp + 4*i + 4;
	    } else
		if (zeros == 1)	/* count envp[] pointers */
		    ++env_cnt;
	    continue;
	}
	if (w == 0) {
	    if (++zeros > 20)
		goto bad;
	    envp = stackp + 4*i + 4;
	    stringp = 0;
	    env_cnt = arg_cnt;
	    arg_cnt = 0;
	    continue;
	}

	if (w == stringp) {	/* found argv[0] */
	    found_argc = 1;
	    ++arg_cnt;
	    break;
	}
	if (w == envp) {
	    found_argc = 1;
	    if (stringp == 0)
		stringp = buf[i+1];
	}
	if (w == arg_cnt - 2 && arg_cnt > 2) {	/* found argc */
	    found_argc = 1;
	    arg_cnt -= 2;
	    if (stringp == 0 || buf[i+3] < stringp && buf[i+3] > stackp)
		stringp = buf[i+3];
	    break;
	}
	++arg_cnt;		/* count pointers in argv[] */
    }

    if (found_argc) {
	char *p, *commline;
	char *plim = (char *) &buf[STKBUF*2];

	if ((p = (char *) buf + stringp - stackp) > plim)
	    goto bad;;
	if (p + maxcmd < plim)
	    plim = p + maxcmd + 1;
	commline = p;

	if (show_env)
	    arg_cnt += env_cnt;

	while (arg_cnt-- && p < plim) {
	    for (; *p != '\0' && p < plim; ++p)
		if (!isprint(*p))
		    *p = ' ';
	    if (arg_cnt && p < plim)
		*p++ = ' ';
	}
	if (p == plim)
	    *(p-1) = '\0';
    /* if cmd empty or dash (login shell): also give real shell name */
	if (!show_env && (*commline == '-' || *commline == '\0') && 
			p < plim + sizeof task->comm + 4) {
	    strcat(p, " (");
	    strncat(p, task->comm, sizeof task->comm);
	    strcat(p, ")");
	}
	return(commline);
    }
bad:
    strcpy((char *) buf, " (");
    strncat((char *) buf, task->comm, sizeof task->comm);
    strcat((char *) buf, ")");
    return((char *) buf);
}
