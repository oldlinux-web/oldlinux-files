/* jobs.h -- structures and stuff used by the jobs.c file. */

#include "quit.h"

#if !defined (NO_WAIT_H)
#include <sys/wait.h>
#else

#ifdef LITTLE_ENDIAN
union wait
  {
    int	w_status;		/* used in syscall */

    /* Terminated process status. */
    struct
      {
	unsigned short
	  w_Termsig  : 7,	/* termination signal */
	  w_Coredump : 1,	/* core dump indicator */
	  w_Retcode  : 8,	/* exit code if w_termsig==0 */
	  w_Fill1    : 16;	/* high 16 bits unused */
      } w_T;

    /* Stopped process status.  Returned
       only for traced children unless requested
       with the WUNTRACED option bit. */
    struct
      {
	unsigned short
	  w_Stopval : 8,	/* == W_STOPPED if stopped */
	  w_Stopsig : 8,	/* actually zero on XENIX */
	  w_Fill2   : 16;	/* high 16 bits unused */
      } w_S;
  };

#else /* if !LITTLE_ENDIAN */

/* For big-endian machines. */

union wait
  {
    int	w_status;		/* used in syscall */

    /* Terminated process status. */
    struct
      {
	unsigned short w_Fill1    : 16;	/* high 16 bits unused */
	unsigned       w_Retcode  : 8;	/* exit code if w_termsig==0 */
	unsigned       w_Coredump : 1;	/* core dump indicator */
	unsigned       w_Termsig  : 7;	/* termination signal */
      } w_T;

    /* Stopped process status.  Returned
       only for traced children unless requested
       with the WUNTRACED option bit. */
    struct
      {
	unsigned short w_Fill2   : 16;	/* high 16 bits unused */
	unsigned       w_Stopsig : 8;	/* signal that stopped us */
	unsigned       w_Stopval : 8;	/* == W_STOPPED if stopped */
      } w_S;
  };

#endif /* LITTLE_ENDIAN */

#define	w_termsig w_T.w_Termsig
#define w_coredump w_T.w_Coredump
#define w_retcode w_T.w_Retcode
#define w_stopval w_S.w_Stopval
#define w_stopsig w_S.w_Stopsig

#define	WSTOPPED 0177
#define WIFSTOPPED(x) (((x) . w_stopval) == WSTOPPED)
#define WIFEXITED(x) ((! (WIFSTOPPED (x))) && (((x) . w_termsig) == 0))
#define WIFSIGNALED(x) ((! (WIFSTOPPED (x))) && (((x) . w_termsig) != 0))

#endif  /* NO_WAIT_H */

/* Make sure that parameters to wait3 are defined. */
#ifndef WNOHANG
#define WNOHANG 1
#define WUNTRACED 2
#endif

/* I looked it up.  For pretty_print_job ().  The real answer is 24. */
#define LONGEST_SIGNAL_DESC 20

/* We keep an array of jobs.  Each entry in the array is a linked list
   of processes that are piped together.  The first process encountered is
   the group leader. */

/* Each child of the shell is remembered in a STRUCT PROCESS.  A chain of
   such structures is a pipeline.  The chain is circular. */
typedef struct process {
  struct process *next;	/* Next process in the pipeline.  A circular chain. */
  int pid;		/* Process ID. */
  union wait status;	/* The status of this command as returned by wait. */
  int running;		/* Non-zero if this process is running. */
  char *command;	/* The particular program that is running. */
} PROCESS;

/* A description of a pipeline's state. */
typedef enum { JRUNNING, JSTOPPED, JDEAD, JMIXED } JOB_STATE;
#define JOBSTATE(job) (jobs[(job)]->state)

typedef struct job {
  char *wd;		/* The working directory at time of invocation. */
  PROCESS *pipe;	/* The pipeline of processes that make up this job. */
  int pgrp;		/* The process ID of the process group (necessary). */
  int foreground;	/* Non-zero if this is running in the foreground. */
  int notified;		/* Non-zero if already notified about job state. */
  JOB_STATE state;	/* The state that this job is in. */
  int job_control;	/* Non-zero if this job started under job control. */
#ifdef JOB_CONTROL
  COMMAND *deferred;	/* Commands that will execute when this job is done. */
#endif
} JOB;

#define NO_JOB -1	/* An impossible job array index. */
#define DUP_JOB -2	/* A possible return value for get_job_spec (). */

/* Stuff from the jobs.c file. */
extern int last_made_pid;
extern int current_job, previous_job;
extern int asynchronous_notification;
extern JOB **jobs;
extern int job_slots;

