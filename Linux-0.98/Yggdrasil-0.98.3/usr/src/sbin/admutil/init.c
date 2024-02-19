/*
 * A System-V Init Clone for Minix.
 * Written by Miquel van Smoorenburg, Februari 1992.
 * Conforms to all standards I know of.
 * (As far as possible under Minix)
 *
 * (C) Miquel van Smoorenburg, miquels@maestro.htsa.aha.nl.
 *
 * Permission for redistribution granted, if
 *  1) You do not pretend you have written it
 *  2) You keep this header intact.
 *  3) You do not sell it or make any profit of it.
 */

/* 25-Mar-92: ported to linux by poe@daimi.aau.dk with little effort */

#ifdef linux
#  define USG 1
#endif

#include "config.h"
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#ifdef linux
#  include <termios.h>
#else
#  include <sgtty.h>
#endif
#include <utmp.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

/* To keep this thing less MINIX dependant, check some things */
#ifndef UTMP
#  ifdef UTMPFILE
#    define UTMP UTMPFILE		/* The real name */
#    define WTMP WTMPFILE
#  else
#    define UTMP "/etc/utmp"
#    define WTMP "/usr/adm/wtmp"
#  endif
#endif
#ifndef NO_PROCESS
#  define NO_PROCESS 0
#endif
#ifndef _NSIG
#  define _NSIG NSIG
#endif

#define DEBUG	    1
#define DEBUG2	    1
#define PIDMAX	   20			/* Number of slots in INITTAB */
#define INITTAB	   "/etc/sysvinittab"	/* the inittab to use */
#define CONSOLE	   "/dev/tty1"		/* For diagnostics & input */
#define NIL	   ((char *)0)
#define SHELL	   "/bin/sh"
#define INITLVL	   "/etc/initrunlvl"	/* Used with telinit */
#define INITPID	    1			/* pid of first process */
#define IOCTLSAVE  "/etc/ioctl.console" /* Tty settings of console */

/* Information about a process in the in-core inittab */
typedef struct {
  int flags;			/* Status of this entry */
  int pid;			/* Pid of this process */
  int status;			/* Status returned when child died */
  char id[5];			/* Inittab id (must be unique) */
  char rlevel[12];		/* run levels */
  int action;			/* what to do (see list below) */
  char process[80];		/* The command line */
} CHILD;

CHILD child[PIDMAX];		/* The in-core inittab */
char runlevel = 'S';		/* The current run level */
char lastlevel = 0;		/* The previous run level */
int got_hup = 0;		/* Set if we recieved the SIGHUP signal */
int powerpanic = 0;		/* Set if the power is failing */
int sys_level;			/* Special levels */

/* Actions to be taken by init */
#define RESPAWN			1
#define WAIT			2
#define ONCE			3
#define	BOOT			4
#define BOOTWAIT		5
#define POWERFAIL		6
#define POWERWAIT		7
#define OFF			8
#define	ONDEMAND		9
#define	INITDEFAULT	       10
#define SYSINIT		       11
#define PROCESS		       12

/* Values for the 'flags' field */
#define UNUSED			1	/* Free slot */
#define RUNNING			2	/* Process is still running */
#define KILLME			4	/* Kill this process */
#define DEMAND			8	/* "runlevels" a b c */

/* Special runlevels */
#define SYS_LEVEL		1	/* The pre - boot stage */
#define BOOT_LEVEL		2	/* We're booting */
#define NORMAL_LEVEL		3	/* And running */

/* ascii values for the `action' field. */
struct actions {
  char *name;
  int act;
} actions[] = {
  "respawn", 	RESPAWN,
  "wait",	WAIT,
  "once",	ONCE,
  "boot",	BOOT,
  "bootwait",	BOOTWAIT,
  "powerfail",	POWERFAIL,
  "powerwait",	POWERWAIT,
  "off",	OFF,
  "ondemand",	ONDEMAND,
  "initdefault",INITDEFAULT,
  "sysinit",	SYSINIT,
  "process",	PROCESS,
  NIL,		0,
};

/* Some useful macros */
#define isrunning(ch) ((ch)->flags & RUNNING)
#define resetbuf() (readline(-1, (char *)0, 0))

/* Forward declarations */
void print();
void read_level();
void read_itab();
void startup();
void sys_start();

#if DEBUG
int printf(str, a1, a2, a3, a4)
char *str;
{
  char buf[128];
  int len;

  sprintf(buf, str, a1, a2, a3, a4);
  len = strlen(buf);
  if(buf[len - 1] == '\n') buf[--len] = 0;

  print(buf, NIL, NIL);
  return(len);
}
#endif

#if DEBUG2
int printf2(str, a1, a2, a3, a4)
char *str;
{
  char buf[128];
  int len;
  int fd;

  sprintf(buf, str, a1, a2, a3, a4);
  len = strlen(buf);

  fd = open(CONSOLE, O_WRONLY);
  write(fd, buf, len);
  close(fd);

  return(len);
}
#endif

/*
 * PANIC!
 */
void panic(s)
char *s;
{
  int fd;
  
  fd = open(CONSOLE, O_WRONLY);
  
  write(fd, "\r\nINIT: ", 8);
  write(fd, s, strlen(s));
  write(fd, "\r\n", 2);
  while(1) pause();
}

/*
 * Print a message on the console device
 */
void print(s1, s2, s3)
char *s1, *s2, *s3;
{
  int fd;
  
#if PGRPBUG
  int pid, st;

  if ((pid = fork()) != 0) {
  	if (pid != -1) waitfor(pid, &st);
  	return;
  }
#endif  
  if ((fd = open(CONSOLE, O_WRONLY)) < 0)
#if PGRPBUG
	exit(1);
#else
  	return;
#endif

  write(fd, "INIT: ", 6);
  write(fd, s1, strlen(s1));
  if (s2) write(fd, s2, strlen(s2));
  if (s3) write(fd, s3, strlen(s3));
  write(fd, "\r\n", 2);
  close(fd);
#if PGRPBUG
  exit(0);
#endif  
}

void dummy_handler()
{
  signal(SIGUSR2, dummy_handler);
}

/*
 * Stop init from working (done by reboot and halt)
 * We can be waked up by either a SIGHUP (which means: new runlevel..)
 * or a SIGUSR2. But this should never be neccesary.
 */
void stop_handler()
{
  signal(SIGUSR1, SIG_IGN);
  
  pause();
  signal(SIGUSR1, stop_handler);
}

/*
 * Take care of terminal settings
 */
void read_modes()
{
#ifdef linux
	struct termios bla;
	int ifd, fd;

	fd = open(CONSOLE, O_RDWR);

	if ((ifd = open(IOCTLSAVE, O_RDONLY)) < 0 ||
	    read(ifd, (char *)&bla, sizeof(bla)) != sizeof(bla)) {
		bla.c_iflag = IGNPAR | ICRNL;
		bla.c_oflag = OPOST | ONLCR;
		bla.c_lflag = ICANON | ECHO | ECHOPRT | ECHOK | ECHOE;
		bla.c_cflag = B9600 | CLOCAL;
		bla.c_cc[VINTR] = 03;
		bla.c_cc[VERASE] = 8;
		bla.c_cc[VKILL] = 24;
		bla.c_cc[VQUIT] = 0x1c;
		bla.c_cc[VSTART] = 17;
		bla.c_cc[VSTOP] = 19;
		bla.c_cc[VEOF] = 4;
	}
	if (ifd >= 0) close(ifd);
	ioctl(fd, TCSETS, &bla);
	if (fd >= 0) close(fd);

#else
  int ifd, fd;
  struct sgttyb bla1;
  struct tchars bla2;

#if PGRPBUG
  int pid, st;

  if ((pid = fork()) != 0) {
  	if (pid > 0) waitfor(pid, &st);
  	return;
  }
#endif
  fd = open(CONSOLE, O_RDWR);

  if ((ifd = open(IOCTLSAVE, O_RDONLY)) < 0 ||
  	read(ifd, &bla1, sizeof(bla1)) != sizeof(bla1) ||
  	read(ifd, &bla2, sizeof(bla2)) != sizeof(bla2)) {
  		bla1.sg_ispeed = B300;
  		bla1.sg_ospeed = B300;
  		bla1.sg_erase  = 8;	/* ^H */
  		bla1.sg_kill   = 24;	/* ^X */
  		bla1.sg_flags  = BITS8 | CRMOD | ECHO | DCD;
  		bla2.t_intrc   = 127;	/* DEL */
  		bla2.t_quitc   = 0x1c;	/* ^\ */
  		bla2.t_startc  = 17;	/* ^Q */
  		bla2.t_stopc   = 19;	/* ^S */
  		bla2.t_eofc    = 4;	/* ^D */
  }
  if (ifd >= 0) close(ifd);
  ioctl(fd, TIOCSETP, &bla1);
  ioctl(fd, TIOCSETC, &bla2);
  if (fd >= 0) close(fd);
#if PGRPBUG
  exit(1);
#endif  
#endif /* ! linux */
}

/*
 * Save terminal settings until later.
 */
void save_modes()
{
#ifdef linux
	struct termios bla;
	int ifd, fd;
	
	fd = open(CONSOLE, O_RDWR);

	if ((ifd = open(IOCTLSAVE, O_WRONLY | O_CREAT, 0644)) >= 0) {
		ioctl(fd, TCGETS, &bla);
		write(ifd, (char *)&bla, sizeof(bla));
		close(ifd);
	}
	if (fd >= 0) close(fd);

#else
  struct sgttyb bla1;
  struct tchars bla2;
  int ifd, fd;
  
#if PGRPBUG
  int pid, st;

  if ((pid = fork()) != 0) {
  	if (pid > 0) waitfor(pid, &st);
  	return;
  }
#endif
  fd = open(CONSOLE, O_RDWR);

  if ((ifd = open(IOCTLSAVE, O_WRONLY | O_CREAT, 0644)) >= 0) {
  	ioctl(fd, TIOCGETP, &bla1);
  	ioctl(fd, TIOCGETC, &bla2);
  	write(ifd, &bla1, sizeof(bla1));
  	write(ifd, &bla2, sizeof(bla2));
  	close(ifd);
  }
  if (fd >= 0) close(fd);
#if PGRPBUG
  exit(0);
#endif  
#endif
}

/*
 * The SIGHUP handler
 */
void hup_handler()
{
  signal(SIGHUP, hup_handler);
  got_hup = 1;
}

#ifdef SIGPWR
/*
 * The SIGPWR handler (that Minix does not have)
 * nor does linux - poe
 */
void pwr_handler();
{
  int f;

  signal(SIGPWR, SIG_IGN);
  powerpanic = 1;
  if (runlevel != 'S') read_itab(1);
  powerpanic = 0;
  signal(SIGPWR, pwr_handler);
}
#endif

/*
 * Log an event ONLY in the wtmp file (reboot, runlevel)
 */
void wtmp_only(user, id, pid, type, line)
char *user;			/* name of user */
char *id;			/* inittab ID */
int pid;			/* PID of process */
int type;			/* TYPE of entry */
char *line;			/* Which line is this */
{
  int fd;
  struct utmp utmp;
  time_t t;

  if ((fd = open(WTMP, O_WRONLY)) < 0) return;
  /* Zero the fields */
  memset(utmp.ut_name, ' ', sizeof(utmp.ut_name));
  memset(utmp.ut_id,   ' ', sizeof(utmp.ut_id  ));
  memset(utmp.ut_line, ' ', sizeof(utmp.ut_line));

  /* Enter new fields */
  time(&t);
  utmp.ut_time = t;
  utmp.ut_pid  = pid;
  utmp.ut_type = type;
  strncpy(utmp.ut_name, user, sizeof(utmp.ut_name));
  strncpy(utmp.ut_id  , id  , sizeof(utmp.ut_id  ));
  strncpy(utmp.ut_line, line, sizeof(utmp.ut_line));

  lseek(fd, 0L, SEEK_END);
  write(fd, (char *)&utmp, sizeof(utmp));
  close(fd);
}

/*
 * Log an event into the WTMP and UTMP files.
 */
void wtmp(user, id, pid, type)
char *user;			/* name of user */
char *id;			/* inittab ID */
int pid;			/* PID of process */
int type;			/* TYPE of entry */
{
  struct utmp utmp;		/* UTMP/WTMP User Accounting */
  int fd = -1;			/* File Descriptor for UTMP */
  int fd2;			/* File Descriptor for WTMP */
  int found = 0;		/* Was the record found in UTMP */
  int freeentry = -1;		/* Was a free entry found during UTMP scan? */
  int lineno;			/* Offset into UTMP file */
  time_t t;			/* What's the time? */

  /* First read the utmp entry for this process */
  if ((fd = open(UTMP, O_RDWR)) >= 0) {
	lineno = 0;
        while (read(fd, (char *) &utmp, sizeof(utmp)) == sizeof(utmp)) {
		if (strncmp(utmp.ut_id, id, sizeof(utmp.ut_id)) == 0 &&
		    utmp.ut_type != NO_PROCESS) {
			(void) lseek(fd, (long) lineno, SEEK_SET);
			found++;
			break;
		}
		/* See if this is a free entry, save it for later */
		if (utmp.ut_pid == 0 || utmp.ut_type == 0)
			if (freeentry < 0) freeentry = lineno;
		lineno += sizeof(utmp);
	}
  }
  if (!found) { /* Enter some defaults */
	/* Zero the fields */
	memset(utmp.ut_name, ' ', sizeof(utmp.ut_name));
	memset(utmp.ut_id,   ' ', sizeof(utmp.ut_id  ));
	memset(utmp.ut_line, ' ', sizeof(utmp.ut_line));
	
	/* Enter new fields */
	utmp.ut_pid  = pid;
	strncpy(utmp.ut_name, user, sizeof(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , sizeof(utmp.ut_id  ));
	strcpy (utmp.ut_line, "");

	/* Where to write new utmp record */
	if (freeentry >= 0)
		lseek(fd, (long) freeentry, SEEK_SET);
  }

  /* Change the values of some fields */
  time(&t);
  utmp.ut_type = type;
  utmp.ut_time = t;

  /* Write the wtmp record */
  if ((fd2 = open(WTMP, O_WRONLY)) >= 0) {
	if (lseek(fd2, 0L, SEEK_END) >= 0L)
		(void) write(fd2, (char *) &utmp, sizeof(struct utmp));
	(void) close(fd2);
  }

  /* And write the utmp record, if needed */
  if (fd >= 0)  {
  	/* DEAD_PROCESS makes no sense in /etc/utmp */
  	if (utmp.ut_type == DEAD_PROCESS) {
  		utmp.ut_type = NO_PROCESS;
  		utmp.ut_pid  = 0;
  	}
	(void) write(fd, (char *) &utmp, sizeof(struct utmp));
	(void) close(fd);
  }
}

/*
 * Wait for a process 'pid'. If pid == 0, do a regular wait.
 */
int waitfor(pid, status) 
int pid;
int *status;
{
  int tmp, f, ourchild;
  CHILD *ch;

#if DEBUG2
  printf2("waitfor called with pid %d\n", pid);
#endif

  /* See if we stored the needed information before */
  for(f = 0; f < PIDMAX; f++) {
  	ch = &child[f];
	if (isrunning(ch) && ch->status != -1 && (pid==0 || ch->pid==pid)){
		*status = ch->status;
#if DEBUG		
		printf("waitfor: child found(%d, id:%s)\n", ch->pid, ch->id);
#endif
		return(ch->pid);
	}
  }
  
  /* No. Wait for the process to finish. */

  while(1) {
  	/* See if we got interrupted before doing the blocking wait*/
  	if (got_hup) {
  		errno = EINTR;
  		return(-1);
  	}
	/* Do the actual wait */
#if DEBUG2
	printf2("Waiting for %d...\n", pid);
#endif
	if((tmp = wait(status)) < 0) return(tmp);
	
#if DEBUG2
	printf2("waitfor: Got pid %d\n", tmp);
#endif
	/* Walk through our list */
	for(f = 0; f < PIDMAX; f++) {
		ch = &child[f];

		/* We found a matching entry */
		if (isrunning(ch) && ch->pid == tmp) {
			ch->status = *status;

			/* Is it what we wanted? */
			if (pid == 0 || tmp == pid) {
#if DEBUG
				printf("waitfor: waited for %d\n", tmp);
#endif
				return(tmp);
			}
#if DEBUG
			printf("waitfor: stored %d\n", tmp);
#endif
		}
	}
	/* Check again */
	if (tmp == pid) return(tmp);
#if DEBUG2
	printf2("waitfor: this child is not yet registered!\n");
#endif
  }
  /* NOTREACHED */
}


/*
 * Read one line from a file descriptor.
 * If the file descriptor is -1, that means "flush buffer".
 */
int readline(fd, buf, max)
int fd;
char *buf;
int max;
{
  static char buffer[512];
  int f;
  static int idx = 0;
  static int inbuf = 0;

  if(fd < 0) {
	idx = 0;
	inbuf = 0;
	return(0);
  }

  if (inbuf < 0) return(-1);
  max--; /* Need closing '\0' */

  for(f = 0; f < max; f++) {
	if (idx == inbuf) {
		inbuf = read(fd, buffer, 512);
		if (inbuf <= 0) {
			inbuf = -1;
			buf[f] = 0;
			return(f);
		}
		idx = 0;
	}
	buf[f] = buffer[idx++];
	if(buf[f] == '\n') break;
  }
  buf[f] = '\0';
  return(f);
}

/*
 * Decode the execution mode of a command.
 */
int get_mode(str)
char *str;
{
  int f;
  
  for(f = 0; actions[f].name; f++)
	if (!strcmp(str, actions[f].name)) return(actions[f].act);
	
  return(-1);
}

/*
 * Send signals to process groups that have to be killed.
 */
int sendthem(sig)
int sig;
{
  int f;
  int foundone = 0;

  for(f = 0; f < PIDMAX; f++) {
  	if (isrunning(&child[f]) && (child[f].flags & KILLME)) {
#if DEBUG
  		printf("Killing pid %d (%d)\n", child[f].pid, f);
#endif
#if KILLBUG
  		if (child[f].pid) kill(child[f].pid, sig);
#else
  		if (child[f].pid) kill(-child[f].pid, sig);
#endif
  		foundone = 1;
  	}
  }
  return(foundone);	
}

/*
 * Copy a string upto ':' or '\0', and return a pointer
 * to the start of the next field.
 */
char *copytocolon(s, buf)
char *s, *buf;
{
  char *old = s;

  if (*buf == 0) {
  	*s = 0;
  	return(buf);
  }
  while(*buf && *buf != ':') *s++ = *buf++;
  *s = 0;
  if (*buf == ':') buf++;
  return(buf);
}

/*
 * Read one line from INITTAB and place it into a CHILD structure.
 */
int decode_line(fd, ch)
int fd;
CHILD *ch;
{
  char act[16], tmp[128];
  char *ptr;

  do {
	if (readline(fd, tmp, 128) <= 0) return(0);

#if DEBUG
	printf("Just read: %s\n", tmp);
#endif
	/* Skip comments and empty lines */
	ptr = tmp;
	while(*ptr == ' ' || *ptr == '\t') ptr++;
  } while (*ptr == 0 || *ptr == '#') ;
  
  /* Break up into pieces */
  ptr = copytocolon(ch->id, ptr);
  ptr = copytocolon(ch->rlevel, ptr);
  ptr = copytocolon(act, ptr);
  ptr = copytocolon(ch->process, ptr);
  
  /* Check the runlevel field */
  for(ptr = ch->rlevel; *ptr; ptr++)
  	if (islower(*ptr)) *ptr = toupper(*ptr);
  if (ch->rlevel[0] == 0) strcpy(ch->rlevel, "0123456");

  /* Decode the 'action' field */
  if ((ch->action = get_mode(act)) < 0)
  	print("Invalid action in ", tmp, NIL);
  	
  /* Sysinit only runs single user */	
  if (ch->action == SYSINIT) strcpy(ch->rlevel, "S");

  /* Fill in the other fields */
  ch->pid = 0;
  ch->status = -1;
  ch->flags = 0;

  return(1);
}

/*
 * Ask the user on the console for a runlevel
 */
int ask_level()
{
  int lvl = -1, cfd;
  int st, pid, fd;
  char buf[8];

  read_modes();

#if PGRPBUG
  if ((pid = fork()) < 0) return('S');
  if (pid > 0) {
  	waitfor(pid, &st);
  	save_modes();
  	read_level(0);
  	return(runlevel);
  }
#endif
  
  if((cfd = open(CONSOLE, O_RDWR)) < 0) panic("cannot open console!");

  while(!strchr("0123456S", lvl)) {
  	write(cfd, "\nEnter runlevel: ", 17);
  	readline(cfd, buf, 4);
  	if (buf[0] != 0 && buf[1] == 0) lvl = buf[0];
  	if (lvl == 's') lvl = 'S';
  }
  close(cfd);
  resetbuf();

#if PGRPBUG
  fd = open(INITLVL, O_WRONLY | O_CREAT);
  write(fd, (char *)&lvl, 1);
  close(fd);
  exit(0);
#endif
  save_modes();
  return(lvl);
}

/*
 * Read INITTAB once to get the INITDEFAULT entry
 */
int get_initdefault()
{
  int lvl = -1;
  CHILD ch;
  char *ptr;
  int fd;

  /* Try to open inittab. If that fails: Single user mode. */
  if ((fd = open(INITTAB, O_RDONLY)) < 0) return('S');

  /* Read every line */
  while(decode_line(fd, &ch))
  	/* Until we find an initdefault entry */
  	if (ch.action == INITDEFAULT) {
  		/* Find the highest mentioned level */
  		ptr = ch.rlevel;
  		while(*ptr) {
  			/* Special case: S */
  			if ((strchr("0123456S", *ptr)  &&
  			    *ptr > lvl && *ptr != 'S') ||
  			    (lvl == -1 && *ptr == 'S')) lvl = *ptr;
  			ptr++;
  		}
  		break;
  	}
  /* close the inittab file */	
  close(fd);
  resetbuf();

  /* And return if we found a default entry */
  if (lvl >= 0) return(lvl);
  
  /* Ask for it */
  lvl = ask_level();
  return(lvl);
}


/*
 * Is any character from s2 in s1 ?
 */
int any(s1, s2)
char *s1, *s2;
{
  while(*s2) if (strchr(s1, *s2++)) return(1);
  return(0);
}


/*
 * Decide if this process is runnable because it's 'ondemand'.
 */
int isdemand(flags, rlevel)
int flags;
char *rlevel;
{
  if (runlevel == 'S' || !any(rlevel, "ABC") || !(flags & DEMAND))
  	return(0);
  return(1);
}

/*
 * (Re-) read the INITTAB file.
 */
void read_itab(doit)
int doit;
{
  int fd, f, g, status;
  CHILD new_child[PIDMAX];
  int printit;

  /* Try to open inittab. If this fails, go into single user mode. */
  if ((fd = open(INITTAB, O_RDONLY)) < 0) {
  	if (runlevel != 'S')  {
  		print("cannot open ", INITTAB, " going single-user");
		wtmp_only("runlevel", "~~", 'S', RUN_LVL, "~");
	}
  	fd = open("/dev/null", O_RDONLY);
  	runlevel = 'S';
  }

  /* Read the INITTAB file line-by-line. */

  /* First fake a Single-User entry */
  new_child[0].flags   = 0;
  new_child[0].action  = RESPAWN;
  new_child[0].pid     = 0;
  new_child[0].status  = -1;
  strcpy(new_child[0].id, "~~");
  strcpy(new_child[0].rlevel, "S");
  strcpy(new_child[0].process, "/bin/su");

  if (doit) {
	/* Read the INITTAB line by line into a new array */
	for(f = 1; f < PIDMAX; f++) {
		if (decode_line(fd, &new_child[f]) == 0) break;
		/* Check that the id's are unique */
		for(g = 0; g < f; g++)
			if (strncmp(new_child[f].id, new_child[g].id, 2) == 0){
				print("inittab id \"", new_child[f].id,
					"\" not unique");
				f--;
				break;
			}	
	}		
	close(fd);
	resetbuf();
  } else {
  	/* Fake we read INITTAB again.
  	 * This is because initially this function is executed
  	 * three times in a row.
  	 */
  	memcpy(new_child, child, sizeof(child));
  	for(f = 0; f < PIDMAX; f++)
  		if (new_child[f].flags != UNUSED) {
  			new_child[f].pid = 0;
  			new_child[f].flags = 0;
  			new_child[f].status = -1;
  		}
  	close(fd);
  	resetbuf();	
  }

  /* Mark the rest as unused */
  for(; f < PIDMAX; f++) new_child[f].flags = UNUSED;

  /* Mark all old children to be killed (we'll unmark them later) */
  for(f = 0; f < PIDMAX; f++) 
  	if (child[f].flags & RUNNING) {
  			child[f].flags |= KILLME;
  }

  /* See which children have the right to live, or have to be (re)started */
  for(f = 0; f < PIDMAX; f++)
  	for(g = 0; g < PIDMAX; g++)
  		if (child[f].flags != UNUSED &&
  		    new_child[g].flags != UNUSED &&
  		    strcmp(child[f].id, new_child[g].id) == 0 &&
  		    (strchr(new_child[g].rlevel, runlevel) ||
  		    isdemand(child[g].flags, new_child[g].rlevel))) {

#if DEBUG
			printf("(%s) matching!\n", child[f].id);
#endif
  		/* It can run in this level. If it already runs, AND
  		 * it's INITTAB entry did not change, don't touch it.
  		 * (this automatically implements the OFF field!)
  		 * BUT if it is NOT running restart it
  		 */
  		 	if (strcmp(child[f].process,
  		 	    new_child[g].process) == 0 &&
  		 	    child[f].action == new_child[g].action) {
  		 	    	/* Don't kill me */
#if DEBUG
  		 	    	printf("(%s) stays alive!\n", child[f].id);
#endif
  		 		child[f].flags &= ~KILLME;
  		 		if (child[f].flags & RUNNING) {
  		 			/* Copy some fields */
  		 			new_child[g].flags  = child[f].flags;
  		 			new_child[g].status = child[f].status;
  		 			new_child[g].pid    = child[f].pid;
  		 		}
  		 	}
  		 	/* See to it that the ondemand flag is copied */
  		 	new_child[g].flags |= (child[f].flags & DEMAND);
  		 }

  /* We now have two arrays:
   *     child[]     - marks which children we have to KILL
   *     new_child[] - the newest INITTAB.
   * Let's first kill unwanted children and collect their exit
   * statuses. Then we start up the new_children (marked by a pid
   * that is still zero)
   */
   
  /* Kill them */
  printit = 0;
  if (sendthem(SIGTERM)) {
  	if (lastlevel != runlevel) {
  		printit = 1;
  		print("Sending processes the warning signal...", NIL, NIL);
		sleep(10);
  		print("Sending processes the kill signal", NIL, NIL);
  	} else sleep(10);
	sendthem(SIGKILL);
  }
  /* And collect their exit statuses */
#if DEBUG
  printf("Collecting statuses..\n");
#endif
  for(f = 0; f < PIDMAX; f++) {
    	if (child[f].flags & KILLME) {
    		while (waitfor(child[f].pid, &status) < 0 && errno == EINTR)
    			;
#if DEBUG
    		printf("Waited for %d\n", child[f].pid);
#endif
    		child[f].flags &= ~RUNNING;
		wtmp("", child[f].id, child[f].pid, DEAD_PROCESS);
    	}
  }
#if DEBUG
  printf("Done\n");
  sleep(3);
#endif

  /* Allright, we're ready for a new runlevel! */
  
  memcpy(child, new_child, sizeof(child));
  
  if (printit) print("Starting the new runlevel.", NIL, NIL);

  /* Start up all new processes */
  for(f = 0; f < PIDMAX; f++)
  	if (child[f].flags != UNUSED && child[f].pid == 0 &&
  	    (strchr(child[f].rlevel, runlevel) ||
  	    isdemand(child[f].flags, child[f].rlevel))) {
  	    	/* See if we're in a normal runlevel */
		if (sys_level == NORMAL_LEVEL)
			startup(&child[f]);
		else
			sys_start(&child[f]);
	}
  lastlevel = runlevel;	
}

/*
 * Fork and execute.
 */
int spawn(ch)
CHILD *ch;
{
  char *args[16];
  char buf[136];
  int f, pid;
  char *ptr;

  /* Split up command line arguments */
  if (ch->action == PROCESS) {
  	args[1] = SHELL;
  	args[2] = "-c";
  	strcpy(buf, "exec ");
  	strcat(buf, ch->process);
  	args[3] = buf;
  	args[4] = NIL;
  } else {
  	strcpy(buf, ch->process);
  	ptr = buf;
  	for(f = 1; f < 15; f++) {
  		/* Skip white space */
  		while(*ptr == ' ' || *ptr == '\t') ptr++;
  		args[f] = ptr;
  		
  		/* Skip this `word' */
  		while(*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '#')
  			ptr++;
  		
  		/* If end-of-line, break */	
  		if (*ptr == '#' || *ptr == 0) {
  			f++;
  			*ptr = 0;
  			break;
  		}
  		/* End word with \0 and continue */
  		*ptr++ = 0;
  	}
  	args[f] = NIL;
  }
  args[0] = args[1];
  while(1) {
	if ((pid = fork()) == 0) {
#ifdef USG
		/* Should do release from controlling tty, 
		 * do a setpgrp() or setsid() and stuff here
		 */
		 
		setsid();
#endif
#if DEBUG
		pid = open("/dev/null", O_RDONLY);
		close(pid);
		if (pid != 3 && pid != 0)
			printf("Warning: some fd is still open (%d)\n", pid);
#endif
  		/* The single user entry needs to talk to the console */
  		if (strcmp(ch->id, "~~") == 0) {
  			read_modes();
  			f = open(CONSOLE, O_RDWR);
  			dup(f);
  			dup(f);
  			setuid(1); /* Force su to ask for a password */
  		}
  		/* Reset all the signals */
  		for(f = 1; f < _NSIG; f++) signal(f, SIG_DFL);
  		execv(args[1], args + 1);
  		if (errno == ENOEXEC || errno == EACCES)
  			execv(SHELL, args);
  		print("warning: cannot execute \"", args[1], "\"");
  		exit(1);
  	}
	if (pid == -1) {
		print("cannot fork, retry..", NIL, NIL);
		sleep(5);
		continue;
	}
	return(pid);
  }
}

/*
 * Start a special child (sysinit, boot & bootwait)
 */
void sys_start(ch)
CHILD *ch;
{
  int status;
  
  switch (ch->action) {
  	case BOOTWAIT:
  	case BOOT:
  		if (sys_level != BOOT_LEVEL) break;
  	case SYSINIT:
  		if (ch->action == SYSINIT && sys_level != SYS_LEVEL) break;
  		ch->flags = RUNNING;
		ch->status = -1;
  		ch->pid = spawn(ch);
  		wtmp("", ch->id, ch->pid, INIT_PROCESS);
		if (ch->action == BOOT) break;
  		while (waitfor(ch->pid, &status) < 0 && errno == EINTR)
  			;
  		ch->flags &= ~RUNNING;
		wtmp("", ch->id, ch->pid, DEAD_PROCESS);
  		break;
  	default:
  		break;
  }
}

/*
 * Start a child running!
 */
void startup(ch)
CHILD *ch;
{
  int status;

#if DEBUG
  printf("Starting id %s\n", ch->id);
#endif
  switch(ch->action) {
  	case ONCE:
  		if (lastlevel == runlevel) break;
  	case POWERFAIL:
  		if (ch->action == POWERFAIL && !powerpanic) break;
  	case RESPAWN:
  	case ONDEMAND:
  	case PROCESS:
  		ch->flags |= RUNNING;
		ch->status = -1;
  		ch->pid = spawn(ch);
  		wtmp("", ch->id, ch->pid, INIT_PROCESS);
  		break;
  	case POWERWAIT:
  		if (!powerpanic) break;
  	case WAIT:
  		if (ch->action == WAIT && runlevel == lastlevel) break;
		ch->status = -1;
  		ch->flags |= RUNNING;
  		ch->pid = spawn(ch);
  		wtmp("", ch->id, ch->pid, INIT_PROCESS);
  		while (waitfor(ch->pid, &status) < 0 && errno == EINTR)
  			;
  		ch->flags &= ~RUNNING;
		wtmp("", ch->id, ch->pid, DEAD_PROCESS);
  		break;
  }
}

/*
 * We got signaled: read the new level from INITLVL
 */
void read_level(verbose)
int verbose;
{
  int f, fd;
  char foo[2];

  if ((fd = open(INITLVL, O_RDONLY)) < 0) {
  	print("cannot open ", INITLVL, NIL);
  	return;
  }
  foo[0] = 'x';
  foo[1] = 0;

  read(fd, &foo[0], 1);
  close(fd);
  if (islower(foo[0])) foo[0] = toupper(foo[0]);

  if (strchr("QS0123456ABC", foo[0]) == NIL) {
  	print("bad runlevel: ", foo, NIL);
  	return;
  }
  if (foo[0] == 'Q') return;
  if (strchr("ABC", foo[0])) {
  	/* Start up those special tasks */
  	for(f = 0; f < PIDMAX; f++)
  		if (child[f].flags != UNUSED &&
  		    strchr(child[f].rlevel, foo[0])) {
  			child[f].flags |= DEMAND;
#if DEBUG
			printf("Marking (%s) as ondemand, flags %d\n",
				child[f].id, child[f].flags);
#endif
		}
  	return;
  }
  if (verbose) {
  	if (foo[0] != 'S')
  		print("new runlevel: ", foo, NIL);
  	else
  		print("going single user", NIL, NIL);
  }	
  runlevel = foo[0];
  if (verbose) wtmp_only("runlevel", "~~", runlevel, RUN_LVL, "~");
}

/*
 * The main loop
 */ 
init_main()
{
  int lvl, f, pid, status;
  int reread = 0;
  int did_boot = 0;

  /* Set up signals */
  for(f = 1; f <= _NSIG; f++) signal(f, SIG_IGN);
  signal(SIGHUP, hup_handler);
  signal(SIGUSR1, stop_handler);
  signal(SIGUSR2, dummy_handler);
#ifdef SIGPWR
  signal(SIGPWR, pwr_handler);
#endif  

  /* Initialize child structures */
  for(f = 0; f < PIDMAX; f++) child[f].flags = UNUSED;

  /* Initialize /etc/utmp */
  close(creat(UTMP, 0644));

  read_modes();

  /* First process entries of type 'sysinit' */
  sys_level = SYS_LEVEL;
  read_itab(1);
  
  /* Write a boot record (AFTER sysinit, 'cause sysinit sets the date) */
  wtmp_only("reboot", "~~", 0, BOOT_TIME, "~");

  /* Get our run level */
  runlevel = get_initdefault(0);

  /* Now process boot and bootwait */
  if (runlevel != 'S') {
  	sys_level = BOOT_LEVEL;
  	read_itab(0);
  	did_boot = 1;
  }
  sys_level = NORMAL_LEVEL;

  /* And go into default run level */
  wtmp_only("runlevel", "~~", runlevel, RUN_LVL, "~");
  read_itab(0);

  /* Wait for a process to terminate and try to restart it */
  while(1) {
  	pid = waitfor(0, &status);
  	if (pid < 0 && errno == EINTR && !got_hup) continue;

  	/* Only here we check if we got a signal */
  	if (got_hup) {
  		if (lastlevel == 'S') save_modes();
  		got_hup = 0;
  		read_level(1);
  		read_itab(1);
  		sync(); /* Least we can do */
  		continue;
  	}
  	/* No more children - This is bad! */
  	if (pid < 0) {
  		panic("No more children - hanging");
  	}

  	/* See which child this was */
  	for(f = 0; f < PIDMAX; f++)
  		if (child[f].flags != UNUSED && child[f].pid == pid) break;
  	if (f == PIDMAX) {
#if DEBUG
  		printf("?? Got status of stranger ??\n");
#endif
  		continue;
  	}
  	child[f].flags &= ~RUNNING;
	wtmp("", child[f].id, child[f].pid, DEAD_PROCESS);

	/* See if this was the single user entry */
	if (strcmp(child[f].id, "~~") == 0) {
		save_modes();
		runlevel = ask_level();
		if (runlevel != 'S' && did_boot == 0) {
			sys_level = BOOT_LEVEL;
			read_itab(1);
			sys_level = NORMAL_LEVEL;
			did_boot++;
		}
	}

  	read_itab(1);
  }
}

void std_err(s)
char *s;
{
  write(2, s, strlen(s));
}

void usage(s)
char *s;
{
  std_err("Usage: ");
  std_err(s);
  std_err(" 0123456SsQqAaBbCc\n");
  exit(1);
}

/*
 * Main entry for init and telinit.
 */
main(argc, argv)
int argc;
char **argv;
{
  char *p;
  int fd;

  /* Get my own name */
  if (p = strrchr(argv[0], '/'))
  	p++;
  else
  	p = argv[0];
  	
  /* See if I want to become "father of all processes" */
#if DEBUG
  if (strcmp(p, "telinit") != 0) init_main();
#endif
#ifdef notdef
  if (getpid() == INITPID) init_main();
#endif
  
  /* Nope, this is a change-run-level init */
  if (argc != 2 || strlen(argv[1]) != 1) usage(p);
  if (!strchr("0123456SsQqAaBbCc", argv[1][0])) usage(p);
  if ((fd = open(INITLVL, O_WRONLY | O_CREAT)) < 0) {
  	std_err(p);
  	std_err(": cannot create ");
  	std_err(INITLVL);
  	std_err("\n");
  }
  write(fd, argv[1], 1);
  close(fd);
  kill(INITPID, SIGHUP);
  exit(0);
}
