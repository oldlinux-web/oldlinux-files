/*
 * @(#)uurate.c 1.0 - Fri Feb  7 13:37:42 1992
 *
 * This program digests log ans stats files in the "Taylor" format
 * and outputs various statistical data to standard out.
 *
 * Author:
 *	Bob Denny (denny@alisa.com)
 *	Fri Feb  7 13:38:36 1992
 *
 * Original author:
 * 	Mark Pizzolato   mark@infopiz.UUCP
 *
 * Edits:
 *	Bob Denny - Fri Feb  7 15:04:54 1992
 *	Heavy rework for Taylor UUCP. This was the (very old) uurate from
 *	DECUS UUCP, which had a single logfile for activity and stats.
 *      Personally, I would have done things differently, with tables
 *      and case statements, but in the interest of time, I preserved
 *      Mark Pizzolato's techniques and style.
 */

#include "uucp.h"

char version[] = "@(#) Taylor UUCP Log File Summary Filter, Version 1.0";

#include <ctype.h>		/* Character Classification	*/
#include <string.h>
#include <math.h>

#include "sysdep.h"

#define DEBUGoff
	
/*
 * Direction of Calling and Data Transmission
 */
#define IN	0		/* Inbound		*/
#define OUT	1		/* Outbound 		*/

/*
 * Data structures used to collect information
 */
struct File_Stats
    {
    int files;			/* Files Transferred	*/
    unsigned long bytes;	/* Data Size Transferred*/
    double time;		/* Transmission Time	*/
    };

struct Phone_Call
    {
    int calls;			/* Call Count		*/
    int succs;			/* Successful calls     */
    double connect_time;	/* Connect Time Spent	*/
    struct File_Stats flow[2];	/* Rcvd & Sent Data  	*/
    };

struct Execution_Command
    {
    struct Execution_Command *next;
    char Commandname[64];
    int count;
    };

struct Host_entry
    {
    struct Host_entry *next;
    char Hostname[32];
    struct Execution_Command *cmds;	/* Local Activities */
    struct Phone_Call call[2];		/* In & Out Activities */
    };

/*
 * Stuff for getopt()
 */
extern int optind;		/* GETOPT : Option Index		*/
extern char *optarg;		/* GETOPT : Option Value		*/
extern void *calloc();

/*
 * Default files to read. Taken from Taylor compile-time configuration.
 * Must look like an argvec, hence the dummy argv[0].
 */
static char *(def_logs[3]) = { "", LOGFILE, STATFILE };

/*
 * Misc. strings for reports
 */
static char *(file_hdr[2]) = { "\nReceived file statistics:\n",
				 "\nSent file statistics\n" };
  
/*
 * BEGIN EXECUTION
 */
main(argc, argv)
int argc;
char *argv[];
{
  char c;
  char *p, *s;
  struct Host_entry *hosts = NULL;
  struct Host_entry *cur = NULL;
  struct Host_entry *e;
  struct Execution_Command *cmd;
  struct Execution_Command *ec;
  char Hostname[64];
  FILE *Log = NULL;
  char logline[1024];
  char *logmsg;
  int sent;
  int called;
  int show_files = 0;		/* I prefer boolean, but... */
  int show_calls = 0;
  int show_commands = 0;
  int show_efficiency = 0;
  int have_files = 0;
  int have_calls = 0;
  int have_commands = 0;  
  int use_stdin = 0;
  Hostname[0] = '\0';

  /*
   * I wish the compiler had the #error directive!
   */
#if !HAVE_TAYLOR_LOGGING
  fprintf(stderr, "uurate cannot be used with your configuration of\n");
  fprintf(stderr, "Taylor UUCP. To use uurate you must be using the\n");
  fprintf(stderr, "TAYLOR_LOGGING configuration.\n");
  exit(1);
#endif

  /*
   * Process the command line arguments
   */
  while((c = getopt(argc, argv, "h:cfexai")) != EOF)
    {
      switch(c)
	{
	case 'h':
	  strcpy(Hostname, optarg);
	  break;
	case 'c':
	  show_calls = 1;
	  break;
	case 'f':
	  show_files = 1;
	  break;
	case 'x':
	  show_commands = 1;
	  break;
        case 'e':
	  show_efficiency = 1;
	  break;
	case 'a':
	  show_calls = show_files = show_commands = show_efficiency = 1;
	  break;
	case 'i':
	  use_stdin = 1;
	  break;
        default :
	  goto usage;
	}
    }

  /*
   * If no report switches given, show call and file stats only
   */
  if (show_calls == 0 && show_files == 0
      && show_efficiency == 0 && show_commands == 0)
    show_calls = show_files = 1;
  
  /*
   * Adjust argv and argc to account for the args processed above.
   */
  argc -= (optind - 1);
  argv += (optind - 1);

  /*
   * If further args present, Assume rest are logfiles for us to process,
   * otherwise, take input from Log and Stat files provided in the
   * compilation environment of Taylor UUCP. If -i was given, Log already
   * points to stdin and no file args are accepted.
   */
  if(argc == 1)			/* No file arguments */
    {
      if (use_stdin)		/* If -i, read from stdin */
	{
	  argc = 2;
	  Log = stdin;
	}
      else			/* Read from current logs */
	{
          argc = 3;	        /* Bash argvec to default log/stat files */
          argv = &def_logs[0];
        }
    }
  else if (use_stdin)		/* File args with -i is an error */
    {
      fprintf(stderr, "uurate (error): file args given with '-i'\n");
      goto usage;
    }

  printf("\n");

  /*
   * MAIN LOGFILE PROCESSING LOOP
   */
  while (argc > 1)
    {

      if (!use_stdin && (Log = fopen(argv[1], "r")) == NULL)
	{
	  perror(argv[1]);
	  return;
	}

      printf("Reading %s...\n", (use_stdin ? "stdin" : argv[1]));
      
      /*
       * Read each line of the logfile and collect information
       */
      while (fgets(logline, sizeof(logline), Log))
	{
	  /*
	   * The host name of the other end of the connection is
	   * always the second field of the log line, whether we
	   * are reading a Log file or a Stats file. Set 'p' to
	   * point to the second field, null-terminated. Skip
	   * the line if something is funny.
	   */
	  if (NULL == (p = strchr(logline, ' ')))
	    continue;
	  ++p;
	  if (NULL != (s = strchr(p, ' ')))
	    *s = '\0';
	  for (s = p; *s; ++s)
	    if (isupper(*s))
	      *s = tolower(*s);
	  /*
	   * Skip this line if we got -h <host> and
	   * this line does not contain that host name.
	   */
	  if (Hostname[0] != '\0')
	    if (0 != strcmp(p, Hostname))
	      continue;
	  /*
	   * We are within a call block now. If this line is a file
	   * transfer record, determine the direction. If not then
	   * skip the line if it is not interesting.
	   */
	  if ((s = strchr(++s, ')')) == NULL)
	    continue;
	  logmsg = s + 2;		/* Message is 2 characters after ')' */
	  if (0 == strncmp(logmsg, "sent", 4))
	    sent = OUT;
	  else
	    if (0 == strncmp(logmsg, "received", 8))
	      sent = IN;
	    else
	      if ((0 != strncmp(logmsg, "Call complete", 13)) &&
		  (0 != strncmp(logmsg, "Calling system", 14)) &&
		  (0 != strncmp(logmsg, "Incoming call", 13)) &&
		  (0 != strncmp(logmsg, "Executing", 9)))
		continue;
	  /*
	   * Find the Host_entry for this host, or create a new
	   * one and link it on to the list.
	   */
	  if ((cur == NULL) || (0 != strcmp(p, cur->Hostname)))
	    {
	      for (cur = hosts; cur != NULL ; cur = cur->next)
		if (0 == strcmp(cur->Hostname, p))
		  break;
	      if (cur == NULL)
		{
		  cur = (struct Host_entry *)calloc(1, sizeof(*hosts));
		  strcpy(cur->Hostname, p);
		  if (hosts == NULL)
		    hosts = cur;
		  else
		    {
		      for (e = hosts; e->next != NULL; e = e->next);
		      e->next = cur;
		    }
		}
	    }
	  /*
	   * OK, if this is a uuxqt record, find the Execution_Command
	   * structure for the command being executed, or create a new
	   * one. Then count an execution of this command.
	   */
	  if (0 == strncmp(logmsg, "Executing", 9))
	    {
	      if (NULL == (p = strchr(logmsg, '(')))
		continue;
	      if ((s = strpbrk(++p, " )")) == NULL)
		continue;
	      *s = '\0';
	      for (cmd = cur->cmds; cmd != NULL; cmd = cmd->next)
		if (0 == strcmp(cmd->Commandname, p))
		  break;
	      if (cmd == NULL)
		{
		  cmd = (struct Execution_Command *)calloc(1, sizeof(*cmd));
		  strcpy(cmd->Commandname, p);
		  if (cur->cmds == NULL)
		    cur->cmds = cmd;
		  else
		    {
		      for (ec = cur->cmds; ec->next != NULL; ec = ec->next);
		      ec->next = cmd;
		    }
		}
	      ++cmd->count;
	      have_commands = 1;
	      continue;
	    }
	  /*
	   * Count start of outgoing call.
	   */
	  if (0 == strncmp(logmsg, "Calling system", 14))
	    {
	      called = OUT;
	      cur->call[called].calls += 1;
	      have_calls = 1;
	      continue;
	    }
	  /*
	   * Count start of incoming call.
	   */
	  if (0 == strncmp(logmsg, "Incoming call", 13))
	    {
	      called = IN;
	      cur->call[called].calls += 1;
	      have_calls = 1;
	      continue;
	    }
	  /*
	   * Handle end of call. Pick up the connect time.
	   */
	  if (0 == strncmp(logmsg, "Call complete", 13))
	    {
	      cur->call[called].succs += 1;
	      if (NULL == (s = strchr(logmsg, '(')))
		continue;
	      cur->call[called].connect_time += atof(s+1);
	      continue;
	    }
	  /*
	   * If we reached here, this must have been a file transfer
	   * record. Count it in the field corresponding to the
	   * direction of the transfer. Count bytes transferred and
	   * the time to transfer as well.
	   */
	  have_files = 1;
	  cur->call[called].flow[sent].files += 1;
	  if (NULL == (s = strchr(logmsg, ' ')))
	    continue;
	  cur->call[called].flow[sent].bytes += atol(++s);
	  if (NULL == (s = strchr(s, ' ')))
	    continue;
	  if (NULL == (s = strpbrk(s, "0123456789")))
	    continue;
	  cur->call[called].flow[sent].time += atof(s);
	}
      argc -= 1;
      argv += 1;
      if(Log != stdin)
	fclose(Log);
    }
  
  /*
   *     ***********
   *     * REPORTS *
   *     ***********
   *
   * Header
   */

  /*
   * Call statistics report
   */
  if(show_calls && have_calls)
    {
      printf("\nCall statistics:\n");
      printf("     sysname   callto    failto    totime    callfm    failfm    fmtime\n");
      printf("     --------  --------  --------  --------  --------  --------  --------\n");
      for (cur = hosts; cur != NULL; cur = cur->next)
	printf("     %-8s  %-8d  %-8d  %-8.0f  %-8d  %-8d  %-8.0f\n",
	       cur->Hostname,
	       cur->call[OUT].calls,
	       cur->call[OUT].calls - cur->call[OUT].succs,
	       cur->call[OUT].connect_time,
	       cur->call[IN].calls,
	       cur->call[IN].calls - cur->call[IN].succs,
	       cur->call[IN].connect_time);
    }

  /*
   * File statistics report
   */
  if(show_files && have_files)
    {
      for (sent = IN; sent <= OUT; ++sent)
	{
	  printf(file_hdr[sent]);
	  printf("     sysname   files     bytes     seconds   byte/sec\n");
	  printf("     --------  --------  --------  --------  --------\n");
	  for (cur = hosts; cur != NULL; cur = cur->next)
	    {
	      double rate;
	      double time;
	      
	      time = cur->call[IN].flow[sent].time +
		cur->call[OUT].flow[sent].time;
	      if (time == 0.0)
		continue;
	      rate = (cur->call[IN].flow[sent].bytes +
		      cur->call[OUT].flow[sent].bytes)/time;
	      printf("     %-8s  %-8d  %-8ld  %-8.0f  %-8.2f\n",
		     cur->Hostname,
		     cur->call[IN].flow[sent].files + 
		     cur->call[OUT].flow[sent].files,
		     cur->call[IN].flow[sent].bytes +
		     cur->call[OUT].flow[sent].bytes,
		     cur->call[IN].flow[sent].time +
		     cur->call[OUT].flow[sent].time,
		     rate);
	    }
	}
    }

  /*
   * Efficiency report
   */
  if (show_efficiency && have_files)
    {
      printf("\nEfficiency:\n");
      printf("     sysname   conntime  flowtime  ovhdtime  effcy (%%)\n");
      printf("     --------  --------  --------  --------  --------\n");
      for (cur = hosts; cur != NULL; cur = cur->next)
	{
	  double total, flow;
	  total = cur->call[IN].connect_time + cur->call[OUT].connect_time;
	  flow = cur->call[IN].flow[IN].time + cur->call[IN].flow[OUT].time +
	    cur->call[OUT].flow[IN].time + cur->call[OUT].flow[OUT].time;
	  printf("     %-8s  %-8.0f  %-8.0f  %-8.0f  %-2.2f%%\n",
		 cur->Hostname, total, flow,
		 (total - flow), ((flow / total) * 100.0));
	}
    }

  /*
   * Command execution report
   */  
  if (show_commands & have_commands)
    {
      printf("\nCommand executions:\n");
      printf("     sysname   rmail     rnews     other\n");
      printf("     --------  --------  --------  --------\n");
      for (cur = hosts; cur != NULL; cur = cur->next)
	{
          int rmail, rnews, other;
	  
	  if (cur->cmds == NULL)
	    continue;
          rmail = rnews = other = 0;
	  for (cmd = cur->cmds; cmd != NULL; cmd = cmd->next)
	    {
	      if (strcmp(cmd->Commandname, "rmail") == 0)
		rmail += cmd->count;
	      else if (strcmp(cmd->Commandname, "rnews") == 0)
		rnews += cmd->count;
	      else
		other += cmd->count;
	    }
	  printf("     %-8s  %-8d  %-8d  %-8d\n", cur->Hostname,
		 rmail, rnews, other);
	}
    }
  return;
  
 usage:
  fprintf(stderr,
      "Usage uurate [-cfexai] [-h hostname] [logfile ... logfile]\n");
  fprintf(stderr,"where:\t-c\tReport call statistics\n");
  fprintf(stderr, "\t-f\tReport file transfer statistics\n");
  fprintf(stderr, "\t-e\tReport efficiency statistics\n");
  fprintf(stderr, "\t-x\tReport command execution statistics\n");
  fprintf(stderr, "\t-a\tAll of the above reports\n");  
  fprintf(stderr, "\t-h host\tReport activities involving ONLY host\n");
  fprintf(stderr, "\t-i\tRead log info from standard input\n");
  fprintf(stderr,
      "If no report options given, reports call and file statistics\n\n");
  fprintf(stderr,
      "If neither -i nor logfiles given, defaults to reading from current\n");
  fprintf(stderr, "logfiles %s and %s\n\n", LOGFILE, STATFILE);
  fprintf(stderr,
      "NOTES:\tReports supressed if no relevant data in log file(s)\n");
  fprintf(stderr,
      "\tBoth \"Log\" and \"Stats\" data are needed for full reports\n");
}
