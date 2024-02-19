/* time - time a command	Authors: Andy Tanenbaum & Michiel Huisjes */

#include <sys/types.h>
#include <sys/times.h>
#include <limits.h>
#include <time.h>
#include <signal.h>

#define HZ CLOCKS_PER_SEC

char **args;
char *name;

int digit_seen;
char a[] = "        .  \n";

main(argc, argv)
int argc;
char *argv[];
{

  struct tms pre_buf, post_buf;
  int status, pid;
  long start_time, end_time;

  if (argc == 1) exit(0);

  args = &argv[1];
  name = argv[1];

  /* Get real time at start of run. */
  (void) time(&start_time);

  /* Fork off child. */
  if ((pid = fork()) < 0) {
	std_err("Cannot fork\n");
	exit(1);
  }
  if (pid == 0) execute();

  /* Parent is the time program.  Disable interrupts and wait. */
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);

  do {
	times(&pre_buf);
  } while (wait(&status) != pid);
  (void) time(&end_time);

  if ((status & 0377) != 0) std_err("Command terminated abnormally.\n");
  times(&post_buf);

  /* Print results. */
  print_time("real ", (end_time - start_time) * HZ);
  print_time("user ", post_buf.tms_cutime - pre_buf.tms_cutime);
  print_time("sys  ", post_buf.tms_cstime - pre_buf.tms_cstime);
  exit(status >> 8);
}

print_time(mess, t)
char *mess;
register long t;
{
/* Print the time 't' in hours: minutes: seconds.  't' is in ticks. */
  int hours, minutes, seconds, hundredths, i;

  digit_seen = 0;
  for (i = 0; i < 8; i++) a[i] = ' ';
  hours = (int) (t / (3600L * (long) HZ));
  t -= (long) hours * 3600L * (long) HZ;
  minutes = (int) (t / (60L * (long) HZ));
  t -= (long) minutes * 60L * (long) HZ;
  seconds = (int) (t / (long) HZ);
  t -= (long) seconds * (long) HZ;
  hundredths = (int) (t * 100L / (long) HZ);

  std_err(mess);

  if (hours) {
	twin(hours, &a[0]);
	a[2] = ':';
  }
  if (minutes || digit_seen) {
	twin(minutes, &a[3]);
	a[5] = ':';
  }
  if (seconds || digit_seen)
	twin(seconds, &a[6]);
  else
	a[7] = '0';
  a[9] = hundredths / 10 + '0';
  a[10] = hundredths % 10 + '0';
#ifndef HUNDREDTHS		/* tenths used to be enough */
  a[10] = '\n'; a[11] = 0;
#endif
  std_err(a);
}

twin(n, p)
int n;
char *p;
{
  char c1, c2;
  c1 = (n / 10) + '0';
  c2 = (n % 10) + '0';
  if (digit_seen == 0 && c1 == '0') c1 = ' ';
  *p++ = c1;
  *p++ = c2;
  if (n > 0) digit_seen = 1;
}

execute()
{
  execvp(name, args);
  std_err("Cannot execute ");
  std_err(name);
  std_err("\n");
  exit(-1);
}
