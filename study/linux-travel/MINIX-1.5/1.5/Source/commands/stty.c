/* stty - set terminal mode	  	Author: Andy Tanenbaum */

#include <sgtty.h>
char *on[] = {"tabs", "cbreak", "raw", "-nl", "echo", "odd", "even"};
char *off[] = {"-tabs", "", "", "nl", "-echo", "", ""};
int k;

struct sgttyb args;
struct tchars tch;

#define STARTC	 021		/* CTRL-Q */
#define STOPC	 023		/* CTRL-S */
#define QUITC	 034		/* CTRL-\ */
#define EOFC	 004		/* CTRL-D */
#define DELC	0177		/* DEL */
#define BYTE    0377
#define FD         0		/* which file descriptor to use */

#define speed(s) args.sg_ispeed = s;  args.sg_ospeed = s
#define clr1 args.sg_flags &= ~(BITS5 | BITS6 | BITS7 | BITS8)
#define clr2 args.sg_flags &= ~(EVENP | ODDP)

main(argc, argv)
int argc;
char *argv[];
{

  /* Stty with no arguments just reports on current status. */
  if (ioctl(FD,TIOCGETP,&args)<0 || ioctl(FD,TIOCGETC,(struct sgttyb*)&tch)<0){
	prints("%s: can't read ioctl parameters from stdin\n", argv[0]);
	exit(1);
  }
  if (argc == 1) {
	report();
	exit(0);
  }

  /* Process the options specified. */
  k = 1;
  while (k < argc) {
	option(argv[k], k + 1 < argc ? argv[k + 1] : "");
	k++;
  }
  if (ioctl(FD,TIOCSETP,&args)<0 || ioctl(FD,TIOCSETC,(struct sgttyb*)&tch)<0){
	prints("%s: can't write ioctl parameters to stdin\n", argv[0]);
	exit(2);
  }
  exit(0);
}



report()
{
  int mode, ispeed, ospeed;

  mode = args.sg_flags;
  pr(mode & XTABS, 0);
  pr(mode & CBREAK, 1);
  pr(mode & RAW, 2);
  pr(mode & CRMOD, 3);
  pr(mode & ECHO, 4);
  pr(mode & ODDP, 5);
  pr(mode & EVENP, 6);

  ispeed = 100 * ((int) args.sg_ispeed & BYTE);
  ospeed = 100 * ((int) args.sg_ospeed & BYTE);
  prints("\nkill = ");
  prctl(args.sg_kill);
  prints("\nerase = ");
  prctl(args.sg_erase);
  prints("\nint = ");
  prctl(tch.t_intrc);
  prints("\nquit = ");
  prctl(tch.t_quitc);
  if (ispeed > 0) {
	prints("\nspeed = ");
	switch (ispeed) {
	    case 100:	prints("110");	break;
	    case 200:	prints("200");	break;
	    case 300:	prints("300");	break;
	    case 600:	prints("600");	break;
	    case 1200:	prints("1200");	break;
	    case 1800:	prints("1800");	break;
	    case 2400:	prints("2400");	break;
	    case 3600:	prints("3600");	break;
	    case 4800:	prints("4800");	break;
	    case 7200:	prints("7200");	break;
	    case 9600:	prints("9600");	break;
	    case 19200:	prints("19200");	break;
	    default:	prints("unknown");
	}
	switch (mode & BITS8) {
	    case BITS5:	prints("\nbits = 5");	break;
	    case BITS6:	prints("\nbits = 6");	break;
	    case BITS7:	prints("\nbits = 7");	break;
	    case BITS8:	prints("\nbits = 8");	break;
	}
  }
  prints("\n");
}

pr(f, n)
int f, n;
{
  if (f)
	prints("%s ", on[n]);
  else
	prints("%s ", off[n]);
}

option(opt, next)
char *opt, *next;
{
  if (match(opt, "-tabs")) {
	args.sg_flags &= ~XTABS;
	return;
  }
  if (match(opt, "-odd")) {
	args.sg_flags &= ~ODDP;
	return;
  }
  if (match(opt, "-even")) {
	args.sg_flags &= ~EVENP;
	return;
  }
  if (match(opt, "-raw")) {
	args.sg_flags &= ~RAW;
	return;
  }
  if (match(opt, "-cbreak")) {
	args.sg_flags &= ~CBREAK;
	return;
  }
  if (match(opt, "-echo")) {
	args.sg_flags &= ~ECHO;
	return;
  }
  if (match(opt, "-nl")) {
	args.sg_flags |= CRMOD;
	return;
  }
  if (match(opt, "tabs")) {
	args.sg_flags |= XTABS;
	return;
  }
  if (match(opt, "even")) {
	clr2;
	args.sg_flags |= EVENP;
	return;
  }
  if (match(opt, "odd")) {
	clr2;
	args.sg_flags |= ODDP;
	return;
  }
  if (match(opt, "raw")) {
	args.sg_flags |= RAW;
	return;
  }
  if (match(opt, "cbreak")) {
	args.sg_flags |= CBREAK;
	return;
  }
  if (match(opt, "echo")) {
	args.sg_flags |= ECHO;
	return;
  }
  if (match(opt, "nl")) {
	args.sg_flags &= ~CRMOD;
	return;
  }
  if (match(opt, "kill")) {
	args.sg_kill = *next;
	k++;
	return;
  }
  if (match(opt, "erase")) {
	args.sg_erase = *next;
	k++;
	return;
  }
  if (match(opt, "int")) {
	tch.t_intrc = *next;
	k++;
	return;
  }
  if (match(opt, "quit")) {
	tch.t_quitc = *next;
	k++;
	return;
  }
  if (match(opt, "5")) {
	clr1;
	args.sg_flags |= BITS5;
	return;
  }
  if (match(opt, "6")) {
	clr1;
	args.sg_flags |= BITS6;
	return;
  }
  if (match(opt, "7")) {
	clr1;
	args.sg_flags |= BITS7;
	return;
  }
  if (match(opt, "8")) {
	clr1;
	args.sg_flags |= BITS8;
	return;
  }
  if (match(opt, "110")) {
	speed(B110);
	return;
  }
  if (match(opt, "200")) {
	speed(2);
	return;
  }
  if (match(opt, "300")) {
	speed(B300);
	return;
  }
  if (match(opt, "600")) {
	speed(6);
	return;
  }
  if (match(opt, "1200")) {
	speed(B1200);
	return;
  }
  if (match(opt, "1800")) {
	speed(18);
	return;
  }
  if (match(opt, "2400")) {
	speed(B2400);
	return;
  }
  if (match(opt, "3600")) {
	speed(36);
	return;
  }
  if (match(opt, "4800")) {
	speed(B4800);
	return;
  }
  if (match(opt, "7200")) {
	speed(72);
	return;
  }
  if (match(opt, "9600")) {
	speed(B9600);
	return;
  }
  if (match(opt, "19200")) {
	speed(192);
	return;
  }
  if (match(opt, "default")) {
	args.sg_flags = ECHO | CRMOD | XTABS | BITS8;
	args.sg_ispeed = B1200;
	args.sg_ospeed = B1200;
	args.sg_kill = '@';
	args.sg_erase = '\b';
	tch.t_intrc = DELC;
	tch.t_quitc = QUITC;
	tch.t_startc = STARTC;
	tch.t_stopc = STOPC;
	tch.t_eofc = EOFC;
	return;
  }
  std_err("unknown mode: ");
  std_err(opt);
  std_err("\n");

}

int match(s1, s2)
char *s1, *s2;
{

  while (1) {
	if (*s1 == 0 && *s2 == 0) return(1);
	if (*s1 == 0 || *s2 == 0) return (0);
	if (*s1 != *s2) return (0);
	s1++;
	s2++;
  }
}

prctl(c)
char c;
{
  if (c < ' ')
	prints("^%c", 'A' + c - 1);
  else if (c == 0177)
	prints("DEL");
  else
	prints("%c", c);
}
