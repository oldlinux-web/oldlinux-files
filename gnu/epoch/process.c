/* Asynchronous subprocess control for GNU Emacs.
   Copyright (C) 1985, 1986, 1987, 1988 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU Emacs General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU Emacs, but only under the conditions described in the
GNU Emacs General Public License.   A copy of this license is
supposed to have been given to you along with GNU Emacs so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */


#include <signal.h>

#include "config.h"

#ifdef subprocesses
/* The entire file is within this conditional */

#include <stdio.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/types.h>		/* some typedefs are used in sys/file.h */
#include <sys/file.h>
#include <sys/stat.h>

#ifdef HAVE_SOCKETS	/* TCP connection support, if kernel can do it */
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#endif /* HAVE_SOCKETS */

#if defined(BSD) || defined(STRIDE)
#include <sys/ioctl.h>
#if !defined (O_NDELAY) && defined (HAVE_PTYS)
#include <fcntl.h>
#endif /* HAVE_PTYS and no O_NDELAY */
#endif /* BSD or STRIDE */
#ifdef USG
#include <termio.h>
#include <fcntl.h>
#endif /* USG */

#ifdef NEED_BSDTTY
#include <sys/bsdtty.h>
#endif

#ifdef IRIS
#include <sys/sysmacros.h>	/* for "minor" */
#include <sys/time.h>
#else
#ifdef UNIPLUS
#include <sys/time.h>

#else /* not IRIS, not UNIPLUS */
#ifdef HAVE_TIMEVAL
#if defined(USG) && !defined(IBMRTAIX)
#include <time.h>
#else /* IBMRTAIX or not USG */
#include <sys/time.h>
#endif /* IBMRTAIX or not USG */
#endif /* HAVE_TIMEVAL */

#endif /* not UNIPLUS */
#endif /* not IRIS */

#if defined (HPUX) && defined (HAVE_PTYS)
#include <sys/ptyio.h>
#endif

#ifdef SYSV_PTYS
#include <sys/tty.h>
#include <sys/pty.h>
#endif

#undef NULL
#include "lisp.h"
#include "window.h"
#include "buffer.h"
#include "process.h"
#include "termhooks.h"
#include "termopts.h"
#include "commands.h"


/* a process object is a network connection when its childp field is neither
   Qt nor Qnil but is instead a string (name of foreign host we
   are connected to + name of port we are connected to) */

#ifdef HAVE_SOCKETS
#define NETCONN_P(p) (XGCTYPE (XPROCESS (p)->childp) == Lisp_String)
#else
#define NETCONN_P(p) 0
#endif /* HAVE_SOCKETS */

/* Define SIGCHLD as an alias for SIGCLD.  There are many conditionals
   testing SIGCHLD.  */

#if !defined (SIGCHLD) && defined (SIGCLD)
#define SIGCHLD SIGCLD
#endif /* SIGCLD */

/* Define the structure that the wait system call stores.
   On many systems, there is a structure defined for this.
   But on vanilla-ish USG systems there is not.  */

#ifndef WAITTYPE
#if !defined (BSD) && !defined (UNIPLUS) && !defined (STRIDE) && !(defined (HPUX) && !defined (NOMULTIPLEJOBS))
#define WAITTYPE int
#define WIFSTOPPED(w) ((w&0377) == 0177)
#define WIFSIGNALED(w) ((w&0377) != 0177 && (w&~0377) == 0)
#define WIFEXITED(w) ((w&0377) == 0)
#define WRETCODE(w) (w >> 8)
#define WSTOPSIG(w) (w >> 8)
#define WCOREDUMP(w) ((w&0200) != 0)
#define WTERMSIG(w) (w & 0377)
#else
#ifdef BSD4_1
#include <wait.h>
#else
#include <sys/wait.h>
#endif /* not BSD 4.1 */
#define WAITTYPE union wait
#define WRETCODE(w) w.w_retcode
#define WCOREDUMP(w) w.w_coredump
#ifndef WTERMSIG
#define WTERMSIG(w) w.w_termsig
#endif
#ifndef WSTOPSIG
#define WSTOPSIG(w) w.w_stopsig
#endif
#endif /* BSD or UNIPLUS or STRIDE */
#endif /* no WAITTYPE */

extern errno;
extern sys_nerr;
extern char *sys_errlist[];

#ifndef BSD4_1
extern char *sys_siglist[];
#else
char *sys_siglist[] =
  {
    "bum signal!!",
    "hangup",
    "interrupt",
    "quit",
    "illegal instruction",
    "trace trap",
    "iot instruction",
    "emt instruction",
    "floating point exception",
    "kill",
    "bus error",
    "segmentation violation",
    "bad argument to system call",
    "write on a pipe with no one to read it",
    "alarm clock",
    "software termination signal from kill",
    "status signal",
    "sendable stop signal not from tty",
    "stop signal from tty",
    "continue a stopped process",
    "child status has changed",
    "background read attempted from control tty",
    "background write attempted from control tty",
    "input record available at control tty",
    "exceeded CPU time limit",
    "exceeded file size limit"
    };
#endif

#ifdef vipc

#include "vipc.h"
extern int comm_server;
extern int net_listen_address;
#endif /* vipc */

/* t means use pty, nil means use a pipe,
   maybe other values to come.  */
Lisp_Object Vprocess_connection_type;

#ifdef SKTPAIR
#ifndef HAVE_SOCKETS
#include <sys/socket.h>
#endif
#endif /* SKTPAIR */

int  child_changed;		/* Flag when a child process has ceased
				   to be */

/* Mask of bits indicating the descriptors that we wait for input on */

int input_wait_mask;

int delete_exited_processes;

#define MAXDESC 32

/* Indexed by descriptor, gives the process (if any) for that descriptor */
Lisp_Object chan_process[MAXDESC];

/* Alist of elements (NAME . PROCESS) */
Lisp_Object Vprocess_alist;

Lisp_Object Qprocessp;

Lisp_Object get_process ();

/* Buffered-ahead input char from process, indexed by channel.
   -1 means empty (no char is buffered).
   Used on sys V where the only way to tell if there is any
   output from the process is to read at least one char.
   Always -1 on systems that support FIONREAD.  */

int proc_buffered_char[MAXDESC];

#ifdef HAVE_PTYS

/* Open an available pty, putting descriptor in *ptyv,
  and return the file name of the pty.  Return 0 if none available.  */

char ptyname[24];

char *
pty (ptyv)
     int *ptyv;
{
  struct stat stb;
  register c, i;

#ifdef PTY_ITERATION
  PTY_ITERATION
#else
  for (c = FIRST_PTY_LETTER; c <= 'z'; c++)
    for (i = 0; i < 16; i++)
#endif
      {
#ifdef PTY_NAME_SPRINTF
	PTY_NAME_SPRINTF
#else
#ifdef HPUX
	sprintf (ptyname, "/dev/ptym/pty%c%x", c, i);
#else
#ifdef RTU
	sprintf (ptyname, "/dev/pty%x", i);
#else
	sprintf (ptyname, "/dev/pty%c%x", c, i);
#endif /* not RTU */
#endif /* not HPUX */
#endif /* no PTY_NAME_SPRINTF */

#ifndef IRIS
	if (stat (ptyname, &stb) < 0)
	  return 0;
	*ptyv = open (ptyname, O_RDWR | O_NDELAY, 0);
#else /* Unusual IRIS code */
 	*ptyv = open ("/dev/ptc", O_RDWR | O_NDELAY, 0);
 	if (*ptyv < 0)
 	  return 0;
	if (fstat (*ptyv, &stb) < 0)
	  return 0;
#endif /* IRIS */

	if (*ptyv >= 0)
	  {
	    /* check to make certain that both sides are available
	       this avoids a nasty yet stupid bug in rlogins */
#ifdef PTY_TTY_NAME_SPRINTF
	    PTY_TTY_NAME_SPRINTF
#else
	    /* In version 19, make these special cases use the macro above.  */
#ifdef HPUX
            sprintf (ptyname, "/dev/pty/tty%c%x", c, i);
#else
#ifdef RTU
            sprintf (ptyname, "/dev/ttyp%x", i);
#else
#ifdef IRIS
 	    sprintf (ptyname, "/dev/ttyq%d", minor (stb.st_rdev));
#else
            sprintf (ptyname, "/dev/tty%c%x", c, i);
#endif /* not IRIS */
#endif /* not RTU */
#endif /* not HPUX */
#endif /* no PTY_TTY_NAME_SPRINTF */
#ifndef UNIPLUS
	    if (access (ptyname, 6) != 0)
	      {
		close (*ptyv);
#ifndef IRIS
		continue;
#else
		return (0);
#endif /* IRIS */
	      }
#endif /* not UNIPLUS */
	    /*
		* If the following statement is included,
		* then a 0 length record is EOT, but no other
		* control characters can be sent down the pty
		* (e.g., ^S/^Q, ^O, etc.).  If it is not
		* included, then sending ^D down the pty-pipe
		* makes a pretty good EOF.
		*/
	    /* I'm told that TOICREMOTE does not mean control chars
	       "can't be sent" but rather that they don't have
	       input-editing or signaling effects.
	       That should be good, because we have other ways
	       to do those things in Emacs.
	       However, telnet mode seems not to work on 4.2.
	       So TIOCREMOTE is turned off now. */

	    /* Under hp-ux, if TIOCREMOTE is turned on, some calls
	       will hang.  In particular, the "timeout" feature (which
	       causes a read to return if there is no data available)
	       does this.  Also it is known that telnet mode will hang
	       in such a way that Emacs must be stopped (perhaps this
	       is the same problem).

	       If TIOCREMOTE is turned off, then there is a bug in
	       hp-ux which sometimes loses data.  Apparently the
	       code which blocks the master process when the internal
	       buffer fills up does not work.  Other than this,
	       though, everything else seems to work fine.

	       Since the latter lossage is more benign, we may as well
	       lose that way.  -- cph */
#ifdef HPUX
#if 0
#define DID_REMOTE
	    ioctl (*ptyv, TIOCREMOTE, 1);
/* Yes, HPUX has an incompatible interface for this.
   Also, using it makes telnet.el fail (Emacs hangs sending text to
   it).  */
#endif
#else /* not HPUX */
#if 0
#ifdef TIOCREMOTE
	    {
	      int on = 1;
	      ioctl (*ptyv, TIOCREMOTE, &on);
	    }
#endif
#endif
#endif /* not HPUX */
/* this is said to be unecessary, and to be harmful in 4.3.  */
/*	    ioctl (*ptyv, FIONBIO, &on);  */
#ifdef FIONBIO
#ifdef SYSV_PTYS
	    {
	      int on = 1;
	      ioctl (*ptyv, FIONBIO, &on);
	    }
#endif
#endif
#ifdef IBMRTAIX
   /* On AIX, the parent gets SIGHUP when a pty attached child dies.  So, we */
   /* ignore SIGHUP once we've started a child on a pty.  Note that this may */
   /* cause EMACS not to die when it should, i.e., when its own controlling  */
   /* tty goes away.  I've complained to the AIX developers, and they may    */
   /* change this behavior, but I'm not going to hold my breath.             */
	    signal (SIGHUP, SIG_IGN);
#endif
	    return ptyname;
	  }
      }
  return 0;
}

#endif /* HAVE_PTYS */

Lisp_Object
make_process (name)
     Lisp_Object name;
{
  register Lisp_Object val, tem, name1;
  register struct Lisp_Process *p;
  char suffix[10];
  register int i;

  /* size of process structure includes the vector header,
     so deduct for that.  But struct Lisp_Vector includes the first
     element, thus deducts too much, so add it back.  */
  val = Fmake_vector (make_number ((sizeof (struct Lisp_Process)
				    - sizeof (struct Lisp_Vector)
				    + sizeof (Lisp_Object))
				   / sizeof (Lisp_Object)),
		      Qnil);
  XSETTYPE (val, Lisp_Process);

  p = XPROCESS (val);
  XFASTINT (p->infd) = 0;
  XFASTINT (p->outfd) = 0;
  XFASTINT (p->pid) = 0;
  XFASTINT (p->flags) = 0;
  XFASTINT (p->reason) = 0;
  p->mark = Fmake_marker ();

  /* If name is already in use, modify it until it is unused.  */

  name1 = name;
  for (i = 1; ; i++)
    {
      tem = Fget_process (name1);
      if (NULL (tem)) break;
      sprintf (suffix, "<%d>", i);
      name1 = concat2 (name, build_string (suffix));
    }
  name = name1;
  p->name = name;
  Vprocess_alist = Fcons (Fcons (name, val), Vprocess_alist);
  return val;
}

remove_process (proc)
     register Lisp_Object proc;
{
  register Lisp_Object pair;

  pair = Frassq (proc, Vprocess_alist);
  Vprocess_alist = Fdelq (pair, Vprocess_alist);
  Fset_marker (XPROCESS (proc)->mark, Qnil, Qnil);

  deactivate_process (proc);
}

DEFUN ("processp", Fprocessp, Sprocessp, 1, 1, 0,
  "Return t if OBJECT is a process.")
  (obj)
     Lisp_Object obj;
{
  return XTYPE (obj) == Lisp_Process ? Qt : Qnil;
}

DEFUN ("get-process", Fget_process, Sget_process, 1, 1, 0,
  "Return the process named NAME, or nil if there is none.")
  (name)
     register Lisp_Object name;
{
  if (XTYPE (name) == Lisp_Process)
    return name;
  CHECK_STRING (name, 0);
  return Fcdr (Fassoc (name, Vprocess_alist));
}

DEFUN ("get-buffer-process", Fget_buffer_process, Sget_buffer_process, 1, 1, 0,
  "Return the (or, a) process associated with BUFFER.\n\
BUFFER may be a buffer or the name of one.")
  (name)
     register Lisp_Object name;
{
  register Lisp_Object buf, tail, proc;

  if (NULL (name)) return Qnil;
  buf = Fget_buffer (name);
  if (NULL (buf)) return Qnil;

  for (tail = Vprocess_alist; !NULL (tail); tail = Fcdr (tail))
    {
      proc = Fcdr (Fcar (tail));
      if (XTYPE (proc) == Lisp_Process && EQ (XPROCESS (proc)->buffer, buf))
	return proc;
    }
  return Qnil;
}

/* This is how commands for the user decode process arguments */

Lisp_Object
get_process (name)
     register Lisp_Object name;
{
  register Lisp_Object proc;
  if (NULL (name))
    proc = Fget_buffer_process (Fcurrent_buffer ());
  else
    {
      proc = Fget_process (name);
      if (NULL (proc))
	proc = Fget_buffer_process (Fget_buffer (name));
    }

  if (!NULL (proc))
    return proc;

  if (NULL (name))
    error ("Current buffer has no process");
  else
    error ("Process %s does not exist", XSTRING (name)->data);
  /* NOTREACHED */
}

DEFUN ("delete-process", Fdelete_process, Sdelete_process, 1, 1, 0,
  "Delete PROCESS: kill it and forget about it immediately.\n\
PROCESS may be a process or the name of one, or a buffer name.")
  (proc)
     register Lisp_Object proc;
{
  proc = get_process (proc);
  if (NETCONN_P (proc))
    XFASTINT (XPROCESS (proc)->flags) = EXITED | CHANGED;
  else if (XFASTINT (XPROCESS (proc)->infd))
    {
      Fkill_process (proc, Qnil);
      /* Do this now, since remove_process will make child_sig do nothing.  */
      XFASTINT (XPROCESS (proc)->flags) = SIGNALED | CHANGED;
      change_msgs ();
    }
  remove_process (proc);
  return Qnil;
}

DEFUN ("process-status", Fprocess_status, Sprocess_status, 1, 1, 0,
  "Return the status of PROCESS: a symbol, one of these:\n\
run  -- for a process that is running.\n\
stop -- for a process stopped but continuable.\n\
exit -- for a process that has exited.\n\
signal -- for a process that has got a fatal signal.\n\
open -- for a network stream connection that is open.\n\
closed -- for a network stream connection that is closed.\n\
nil -- if arg is a process name and no such process exists.")
/* command -- for a command channel opened to Emacs by another process.\n\
   external -- for an i/o channel opened to Emacs by another process.\n\  */
  (proc)
     register Lisp_Object proc;
{
  register struct Lisp_Process *p;
  proc = Fget_process (proc);
  if (NULL (proc))
    return proc;
  p = XPROCESS (proc);

  switch (XFASTINT (p->flags) & PROC_STATUS)
    {
    case RUNNING:
      if (NETCONN_P (proc))
	return intern ("open");
      else if (!NULL (p->childp))
	return intern ("run");
      else if (!NULL (p->command_channel_p))
	return intern ("command");
      return intern ("external");

    case EXITED:
      if (NETCONN_P (proc))
	return intern ("closed");
      return intern ("exit");

    case SIGNALED:
      return intern ("signal");

    case STOPPED:
      return intern ("stop");
    }

  /* NOTREACHED */
}

DEFUN ("process-exit-status", Fprocess_exit_status, Sprocess_exit_status,
       1, 1, 0,
  "Return the exit status of PROCESS or the signal number that killed it.\n\
If PROCESS has not yet exited or died, return 0.")
  (proc)
     register Lisp_Object proc;
{
  CHECK_PROCESS (proc, 0);
  return XPROCESS (proc)->reason;
}

DEFUN ("process-id", Fprocess_id, Sprocess_id, 1, 1, 0,
  "Return the process id of PROCESS.\n\
This is the pid of the Unix process which PROCESS uses or talks to.\n\
For a network connection, this value is nil.")
  (proc)
     register Lisp_Object proc;
{
  CHECK_PROCESS (proc, 0);
  return XPROCESS (proc)->pid;
}

DEFUN ("process-name", Fprocess_name, Sprocess_name, 1, 1, 0,
  "Return the name of PROCESS, as a string.\n\
This is the name of the program invoked in PROCESS,\n\
possibly modified to make it unique among process names.")
  (proc)
     register Lisp_Object proc;
{
  CHECK_PROCESS (proc, 0);
  return XPROCESS (proc)->name;
}

DEFUN ("process-command", Fprocess_command, Sprocess_command, 1, 1, 0,
  "Return the command that was executed to start PROCESS.\n\
This is a list of strings, the first string being the program executed\n\
and the rest of the strings being the arguments given to it.\n\
For a non-child channel, this is nil.")
  (proc)
     register Lisp_Object proc;
{
  CHECK_PROCESS (proc, 0);
  return XPROCESS (proc)->command;
}

DEFUN ("set-process-buffer", Fset_process_buffer, Sset_process_buffer,
  2, 2, 0,
  "Set buffer associated with PROCESS to BUFFER (a buffer, or nil).")
  (proc, buffer)
     register Lisp_Object proc, buffer;
{
  CHECK_PROCESS (proc, 0);
  if (!NULL (buffer))
    CHECK_BUFFER (buffer, 1);
  XPROCESS (proc)->buffer = buffer;
  return buffer;
}

DEFUN ("process-buffer", Fprocess_buffer, Sprocess_buffer,
  1, 1, 0,
  "Return the buffer PROCESS is associated with.\n\
Output from PROCESS is inserted in this buffer\n\
unless PROCESS has a filter.")
  (proc)
     register Lisp_Object proc;
{
  CHECK_PROCESS (proc, 0);
  return XPROCESS (proc)->buffer;
}

DEFUN ("process-mark", Fprocess_mark, Sprocess_mark,
  1, 1, 0,
  "Return the marker for the end of the last output from PROCESS.")
  (proc)
     register Lisp_Object proc;
{
  CHECK_PROCESS (proc, 0);
  return XPROCESS (proc)->mark;
}

DEFUN ("set-process-filter", Fset_process_filter, Sset_process_filter,
  2, 2, 0,
  "Give PROCESS the filter function FILTER; nil means no filter.\n\
When a process has a filter, each time it does output\n\
the entire string of output is passed to the filter.\n\
The filter gets two arguments: the process and the string of output.\n\
If the process has a filter, its buffer is not used for output.")
  (proc, filter)
     register Lisp_Object proc, filter;
{
  CHECK_PROCESS (proc, 0);
  XPROCESS (proc)->filter = filter;
  return filter;
}

DEFUN ("process-filter", Fprocess_filter, Sprocess_filter,
  1, 1, 0,
  "Returns the filter function of PROCESS; nil if none.\n\
See set-process-filter for more info on filter functions.")
  (proc)
     register Lisp_Object proc;
{
  CHECK_PROCESS (proc, 0);
  return XPROCESS (proc)->filter;
}

DEFUN ("set-process-sentinel", Fset_process_sentinel, Sset_process_sentinel,
  2, 2, 0,
  "Give PROCESS the sentinel SENTINEL; nil for none.\n\
The sentinel is called as a function when the process changes state.\n\
It gets two arguments: the process, and a string describing the change.")
  (proc, sentinel)
     register Lisp_Object proc, sentinel;
{
  CHECK_PROCESS (proc, 0);
  XPROCESS (proc)->sentinel = sentinel;
  return sentinel;
}

DEFUN ("process-sentinel", Fprocess_sentinel, Sprocess_sentinel,
  1, 1, 0,
  "Return the sentinel of PROCESS; nil if none.\n\
See set-process-sentinel for more info on sentinels.")
  (proc)
     register Lisp_Object proc;
{
  CHECK_PROCESS (proc, 0);
  return XPROCESS (proc)->sentinel;
}

DEFUN ("process-kill-without-query", Fprocess_kill_without_query,
  Sprocess_kill_without_query, 1, 1, 0,
  "Say no query needed if this process is running when Emacs is exited.")
  (proc)
     register Lisp_Object proc;
{
  CHECK_PROCESS (proc, 0);
  XPROCESS (proc)->kill_without_query = Qt;
  return Qt;
}

Lisp_Object
list_processes_1 ()
{
  register Lisp_Object tail, tem;
  Lisp_Object proc, minspace, tem1;
  register struct buffer *old = bf_cur;
  register struct Lisp_Process *p;
  register int state;
  char tembuf[80];

  XFASTINT (minspace) = 1;

  SetBfp (XBUFFER (Vstandard_output));
  Fbuffer_flush_undo (Vstandard_output);

  bf_cur->truncate_lines = Qt;

  write_string ("\
Proc         Status   Buffer         Command\n\
----         ------   ------         -------\n", -1);

  for (tail = Vprocess_alist; !NULL (tail); tail = Fcdr (tail))
    {
      proc = Fcdr (Fcar (tail));
      p = XPROCESS (proc);
      if (NULL (p->childp))
	continue;

      Finsert (1, &p->name);
      Findent_to (make_number (13), minspace);

      state = XFASTINT (p->flags) & PROC_STATUS;
      switch (state)
	{
	case RUNNING:
	  if (NETCONN_P (proc))
	    write_string ("Open", -1);
	  else
	    write_string ("Run", -1);
	  break;

	case STOPPED:
	  write_string ("Stop", -1);
	  break;

	case EXITED:
	  if (NETCONN_P (proc))
	    write_string ("Closed", -1);
	  else
	    write_string ("Exit", -1);
	  if (XFASTINT (p->reason))
	    {
	      sprintf (tembuf, " %d", XFASTINT (p->reason));
	      write_string (tembuf, -1);
	    }
	  remove_process (proc);
	  break;

	case SIGNALED:
	  if (XFASTINT (p->reason) < NSIG)
	    write_string (sys_siglist [XFASTINT (p->reason)], -1);
	  else
	    write_string ("Signal", -1);
	  remove_process (proc);
	}

      Findent_to (make_number (22), minspace);
      if (NULL (p->buffer))
	InsStr ("(none)");
      else if (NULL (XBUFFER (p->buffer)->name))
	InsStr ("(Killed)");
      else
	Finsert (1, &XBUFFER (p->buffer)->name);

      Findent_to (make_number (37), minspace);

      if (NETCONN_P (proc))
        {
	  sprintf (tembuf, "(network stream connection to %s)\n",
		   XSTRING (p->childp)->data);
	  InsStr (tembuf);
        }
      else 
	{
	  tem = p->command;
	  while (1)
	    {
	      tem1 = Fcar (tem);
	      Finsert (1, &tem1);
	      tem = Fcdr (tem);
	      if (NULL (tem))
		break;
	      InsStr (" ");
	    }
	  InsStr ("\n");
       }
    }

  SetBfp (old);
  return Qnil;
}

DEFUN ("list-processes", Flist_processes, Slist_processes, 0, 0, "",
  "Display a list of all processes.\n\
\(Any processes listed as Exited or Signaled are actually eliminated\n\
after the listing is made.)")
  ()
{
  internal_with_output_to_temp_buffer ("*Process List*",
				       list_processes_1, Qnil);
  return Qnil;
}

DEFUN ("process-list", Fprocess_list, Sprocess_list, 0, 0, 0,
  "Return a list of all processes.")
  ()
{
  return Fmapcar (Qcdr, Vprocess_alist);
}

DEFUN ("start-process", Fstart_process, Sstart_process, 3, MANY, 0,
  "Start a program in a subprocess.  Return the process object for it.\n\
Args are NAME BUFFER PROGRAM &rest PROGRAM-ARGS\n\
NAME is name for process.  It is modified if necessary to make it unique.\n\
BUFFER is the buffer or (buffer-name) to associate with the process.\n\
 Process output goes at end of that buffer, unless you specify\n\
 an output stream or filter function to handle the output.\n\
 BUFFER may be also nil, meaning that this process is not associated\n\
 with any buffer\n\
Third arg is program file name.  It is searched for as in the shell.\n\
Remaining arguments are strings to give program as arguments.")
  (nargs, args)
     int nargs;
     register Lisp_Object *args;
{
  Lisp_Object buffer, name, program, proc, tem;
  register unsigned char **new_argv;
  register int i;

  name = args[0];
  CHECK_STRING (name, 0);

  buffer = args[1];
  program = args[2];

  CHECK_STRING (program, 2);

  new_argv = (unsigned char **) alloca ((nargs - 1) * sizeof (char *));

  for (i = 3; i < nargs; i++)
    {
      tem = args[i];
      CHECK_STRING (tem, i);
      new_argv[i - 2] = XSTRING (tem)->data;
    }
  new_argv[i - 2] = 0;
  new_argv[0] = XSTRING (program)->data;

  /* If program file name is not absolute, search our path for it */
  if (new_argv[0][0] != '/')
    {
      tem = Qnil;
      openp (Vexec_path, program, "", &tem, 1);
      if (NULL (tem))
	report_file_error ("Searching for program", Fcons (program, Qnil));
      new_argv[0] = XSTRING (tem)->data;
    }

  if (!NULL (buffer))
    buffer = Fget_buffer_create (buffer);
  proc = make_process (name);

  XPROCESS (proc)->childp = Qt;
  XPROCESS (proc)->command_channel_p = Qnil;
  XPROCESS (proc)->buffer = buffer;
  XPROCESS (proc)->sentinel = Qnil;
  XPROCESS (proc)->filter = Qnil;
  XPROCESS (proc)->command = Flist (nargs - 2, args + 2);

  create_process (proc, new_argv);

  return proc;
}

create_process_1 (signo)
     int signo;
{
#ifdef USG
  /* USG systems forget handlers when they are used;
     must reestablish each time */
  signal (signo, create_process_1);
#endif /* USG */
}

create_process (process, new_argv)
     Lisp_Object process;
     char **new_argv;
{
  int pid, inchannel, outchannel, forkin, forkout;
  int sv[2];
#ifdef SIGCHLD
  int (*sigchld)();
#endif
  register char *ptyname = 0;
  char **env;
  extern char **environ;

#ifdef MAINTAIN_ENVIRONMENT
  env = (char **) alloca (size_of_current_environ ());
  get_current_environ (env);
#else
  env = environ;
#endif /* MAINTAIN_ENVIRONMENT */

#ifdef HAVE_PTYS
  if (EQ (Vprocess_connection_type, Qt))
    ptyname = pty (&inchannel);

  outchannel = inchannel;
  if (ptyname)
    {
#ifndef USG
      /* On USG systems it does not work to open
	 the pty's tty here and then close and reopen it in the child.  */
      forkout = forkin = open (ptyname, O_RDWR, 0);
      if (forkin < 0)
	report_file_error ("Opening pty", Qnil);
#else
      forkin = forkout = -1;
#endif
    }
  else
#endif /* HAVE_PTYS */
#ifdef SKTPAIR
    {
      if (socketpair (AF_UNIX, SOCK_STREAM, 0, sv) < 0)
	report_file_error ("Opening socketpair", Qnil);
      outchannel = inchannel = sv[0];
      forkout = forkin = sv[1];
    }
#else /* not SKTPAIR */
    {
      pipe (sv);
      inchannel = sv[0];
      forkout = sv[1];
      pipe (sv);
      outchannel = sv[1];
      forkin = sv[0];
    }
#endif /* not SKTPAIR */

#if 0
  /* Replaced by close_process_descs */
  set_exclusive_use (inchannel);
  set_exclusive_use (outchannel);
#endif

/* Stride people say it's a mystery why this is needed
   as well as the O_NDELAY, but that it fails without this.  */
#ifdef STRIDE
  {
    int one = 1;
    ioctl (inchannel, FIONBIO, &one);
  }
#endif

#ifdef O_NDELAY
  fcntl (inchannel, F_SETFL, O_NDELAY);
#endif

  /* Record this as an active process, with its channels.
     As a result, child_setup will close Emacs's side of the pipes.  */
  chan_process[inchannel] = process;
  XFASTINT (XPROCESS (process)->infd) = inchannel;
  XFASTINT (XPROCESS (process)->outfd) = outchannel;
  XFASTINT (XPROCESS (process)->flags) = RUNNING;

  /* Delay interrupts until we have a chance to store
     the new fork's pid in its process structure */
#ifdef SIGCHLD
#ifdef BSD4_1
  sighold (SIGCHLD);
#else /* not BSD4_1 */
#ifdef HPUX
  sigsetmask (1 << (SIGCHLD - 1));
#endif /* HPUX */
#if defined (BSD) || defined (UNIPLUS)
  sigsetmask (1 << (SIGCHLD - 1));
#else /* ordinary USG */
  sigchld = (int (*)()) signal (SIGCHLD, SIG_DFL);
#endif /* ordinary USG */
#endif /* not BSD4_1 */
#endif /* SIGCHLD */

  {
    /* child_setup must clobber environ on systems with true vfork.
       Protect it from permanent change.  */
    char **save_environ = environ;

    pid = vfork ();
    if (pid == 0)
      {
	int xforkin = forkin;
	int xforkout = forkout;

	/* Make the pty be the controlling terminal of the process.  */
#ifdef HAVE_PTYS
	/* First, disconnect its current controlling terminal.  */
#ifdef USG
	/* It's very important to call setpgrp() here and no time
	   afterwards.  Otherwise, we lose our controlling tty which
	   is set when we open the pty. */
	setpgrp ();
#endif /* USG */
#ifdef TIOCNOTTY
	/* In 4.3BSD, the TIOCSPGRP bug has been fixed, and now you
	   can do TIOCSPGRP only to the process's controlling tty.  */
	if (ptyname)
	  {
	    /* I wonder: would just ioctl (0, TIOCNOTTY, 0) work here? 
	       I can't test it since I don't have 4.3.  */
	    int j = open ("/dev/tty", O_RDWR, 0);
	    ioctl (j, TIOCNOTTY, 0);
	    close (j);
	  }
#endif /* TIOCNOTTY */

#if !defined (RTU) && !defined (UNIPLUS)
/*** There is a suggestion that this ought to be a
     conditional on TIOCSPGRP.  */
	/* Now close the pty (if we had it open) and reopen it.
	   This makes the pty the controlling terminal of the subprocess.  */
	if (ptyname)
	  {
	    /* I wonder if close (open (ptyname, ...)) would work?  */
	    if (xforkin >= 0)
	      close (xforkin);
	    xforkout = xforkin = open (ptyname, O_RDWR, 0);

	    if (xforkin < 0)
	      abort ();
	  }
#endif /* not UNIPLUS and not RTU */
#ifdef IBMRTAIX
	/* On AIX, we've disabled SIGHUP above once we start a child on a pty.
	   Now reenable it in the child, so it will die when we want it to.  */
	if (ptyname)
	  signal (SIGHUP, SIG_DFL);
#endif
#endif /* HAVE_PTYS */
	child_setup_tty (xforkout);
	child_setup (xforkin, xforkout, xforkout, new_argv, env);
      }
    environ = save_environ;
  }

  input_wait_mask |= ChannelMask (inchannel);

  /* If the subfork execv fails, and it exits,
     this close hangs.  I don't know why.
     So have an interrupt jar it loose.  */
  signal (SIGALRM, create_process_1);
  alarm (1);
  if (forkin >= 0)
    close (forkin);
  alarm (0);
  if (forkin != forkout && forkout >= 0)
    close (forkout);

  if (pid < 0)
    {
      remove_process (process);
      report_file_error ("Doing vfork", Qnil);
    }

  XFASTINT (XPROCESS (process)->pid) = pid;

#ifdef SIGCHLD
#ifdef BSD4_1
  sigrelse (SIGCHLD);
#else /* not BSD4_1 */
#ifdef HPUX
  sigsetmask (0);
#endif /* HPUX */
#if defined (BSD) || defined (UNIPLUS)
  sigsetmask (0);
#else /* ordinary USG */
  signal (SIGCHLD, sigchld);
#endif /* ordinary USG */
#endif /* not BSD4_1 */
#endif /* SIGCHLD */
}

#ifdef HAVE_SOCKETS

/* open a TCP network connection to a given HOST/SERVICE.  Treated
   exactly like a normal process when reading and writing.  Only
   differences are in status display and process deletion.  A network
   connection has no PID; you cannot signal it.  All you can do is
   deactivate and close it via delete-process */

#ifdef HAVE_UNIX_DOMAIN
#include <sys/un.h>
#endif HAVE_UNIX_DOMAIN

DEFUN ("open-network-stream", Fopen_network_stream, Sopen_network_stream, 
       4, 4, 0,
#ifdef HAVE_UNIX_DOMAIN
  "Open a TCP connection for a service to a host.\n\
Returns a subprocess-object to represent the connection.\n\
Input and output work as for subprocesses; `delete-process' closes it.\n\
Args are NAME BUFFER HOST SERVICE.\n\
If SERVICE is 0, then HOST is taken to be the name of a socket file, and a\n\
Unix domain socket is opened.\n\
NAME is name for process.  It is modified if necessary to make it unique.\n\
BUFFER is the buffer (or buffer-name) to associate with the process.\n\
 Process output goes at end of that buffer, unless you specify\n\
 an output stream or filter function to handle the output.\n\
 BUFFER may be also nil, meaning that this process is not associated\n\
 with any buffer\n\
Third arg is name of the host to connect to.\n\
Fourth arg SERVICE is name of the service desired, or an integer\n\
 specifying a port number to connect to."
#else
  "Open a TCP connection for a service to a host.\n\
Returns a subprocess-object to represent the connection.\n\
Input and output work as for subprocesses; `delete-process' closes it.\n\
Args are NAME BUFFER HOST SERVICE.\n\
NAME is name for process.  It is modified if necessary to make it unique.\n\
BUFFER is the buffer (or buffer-name) to associate with the process.\n\
 Process output goes at end of that buffer, unless you specify\n\
 an output stream or filter function to handle the output.\n\
 BUFFER may be also nil, meaning that this process is not associated\n\
 with any buffer\n\
Third arg is name of the host to connect to.\n\
Fourth arg SERVICE is name of the service desired, or an integer\n\
 specifying a port number to connect to."
#endif HAVE_UNIX_DOMAIN
       )
   (name, buffer, host, service)
      Lisp_Object name, buffer, host, service;
{
  Lisp_Object proc;
  register int i;
  struct sockaddr_in address;
#ifdef HAVE_UNIX_DOMAIN
  struct sockaddr_un server;
  int unix_domain = 0;
#endif HAVE_UNIX_DOMAIN
  struct servent *svc_info;
  struct hostent *host_info;
  int s, outch, inch;
  char errstring[80];
  int port;

  CHECK_STRING (name, 0);
  CHECK_STRING (host, 0);
  if (XTYPE(service) == Lisp_Int)
    {
#ifdef HAVE_UNIX_DOMAIN
	if (XINT (service) == 0)
	  unix_domain = 1;
	else
#endif HAVE_UNIX_DOMAIN
	  port = htons ((unsigned short) XINT (service));
    }
  else
    {
      CHECK_STRING (service, 0);
      svc_info = getservbyname (XSTRING (service)->data, "tcp");
      if (svc_info == 0)
	error ("Unknown service \"%s\"", XSTRING (service)->data);
      port = svc_info->s_port;
    }

#ifdef HAVE_UNIX_DOMAIN
  if (unix_domain)
    {
	server.sun_family = AF_UNIX;
	strcpy (server.sun_path, XSTRING (host)->data);
    }
  else
#endif HAVE_UNIX_DOMAIN
    {
	host_info = gethostbyname (XSTRING (host)->data);
	if (host_info == 0)
	  error ("Unknown host \"%s\"", XSTRING(host)->data);

	bzero (&address, sizeof address);
	bcopy (host_info->h_addr, (char *) &address.sin_addr, host_info->h_length);
	address.sin_family = host_info->h_addrtype;
	address.sin_port = port;
    }

#ifdef HAVE_UNIX_DOMAIN
  if (unix_domain)
    s = socket (AF_UNIX, SOCK_STREAM, 0);
  else
#endif HAVE_UNIX_DOMAIN
    s = socket (host_info->h_addrtype, SOCK_STREAM, 0);
  
  if (s < 0)
    report_file_error ("error creating socket", Fcons (name, Qnil));

#ifdef HAVE_UNIX_DOMAIN
  if (unix_domain)
    {
	if (connect (s, &server, strlen (server.sun_path) + 2) < 0)
	  error ("connect failed for socket: \"%s\"", XSTRING (host)->data);
    }
  else
#endif HAVE_UNIX_DOMAIN
    {
	if (connect (s, &address, sizeof address) == -1)
	  error ("Host \"%s\" not responding", XSTRING (host)->data);
    }

  inch = s;
  outch = dup (s);
  if (outch < 0) 
    report_file_error ("error duplicating socket", Fcons (name, Qnil));

  if (!NULL (buffer))
    buffer = Fget_buffer_create (buffer);
  proc = make_process (name);

  chan_process[inch] = proc;

#ifdef O_NDELAY
  fcntl (inch, F_SETFL, O_NDELAY);
#endif

  XPROCESS (proc)->childp = host;
  XPROCESS (proc)->command_channel_p = Qnil;
  XPROCESS (proc)->buffer = buffer;
  XPROCESS (proc)->sentinel = Qnil;
  XPROCESS (proc)->filter = Qnil;
  XPROCESS (proc)->command = Qnil;
  XPROCESS (proc)->pid = Qnil;
  XPROCESS (proc)->kill_without_query = Qt;
  XFASTINT (XPROCESS (proc)->infd) = s;
  XFASTINT (XPROCESS (proc)->outfd) = outch;
  XFASTINT (XPROCESS (proc)->flags) = RUNNING;
  input_wait_mask |= ChannelMask (inch);
  return proc;
}

#endif	/* HAVE_SOCKETS */

deactivate_process (proc)
     Lisp_Object proc;
{
  register int inchannel, outchannel;
  register struct Lisp_Process *p = XPROCESS (proc);

  inchannel = XFASTINT (p->infd);
  outchannel = XFASTINT (p->outfd);

  if (inchannel)
    {
      /* Beware SIGCHLD hereabouts. */
      flush_pending_output (inchannel);
      close (inchannel);
      if (outchannel  &&  outchannel != inchannel)
 	close (outchannel);

      XFASTINT (p->infd) = 0;
      XFASTINT (p->outfd) = 0;
      chan_process[inchannel] = Qnil;
      input_wait_mask &= ~ChannelMask (inchannel);
    }
}

/* Close all descriptors currently in use for communication
   with subprocess.  This is used in a newly-forked subprocess
   to get rid of irrelevant descriptors.  */

close_process_descs ()
{
  int i;
  for (i = 0; i < MAXDESC; i++)
    {
      Lisp_Object process;
      process = chan_process[i];
      if (!NULL (process))
	{
	  int in = XFASTINT (XPROCESS (process)->infd);
	  int out = XFASTINT (XPROCESS (process)->outfd);
	  close (in);
	  if (in != out)
	    close (out);
	}
    }
}

DEFUN ("accept-process-output", Faccept_process_output, Saccept_process_output,
  0, 2, 0,
  "Allow any pending output from subprocesses to be read by Emacs.\n\
It is read into the process' buffers or given to their filter functions.\n\
Non-nil arg PROCESS means do not return until some output has been received\n\
from PROCESS.  Non-nil arg TIMEOUT means wait for that many seconds, -1\n\
return immediately.")
  (proc, timeout)
     register Lisp_Object proc, timeout;
{
  if (NULL (proc)) {
      if (XTYPE(timeout) == Lisp_Int)
        timeout = XINT(timeout);
      else
        timeout = -1;
      wait_reading_process_input (timeout, 0, 0);
    }
  else
    {
      if (XTYPE(timeout) == Lisp_Int)
        timeout = XINT(timeout);
      else
        timeout = 0;
      proc = get_process (proc);
      wait_reading_process_input (timeout, XPROCESS (proc), 0);
    }
  return Qnil;
}


/* This variable is different from waiting_for_input in keyboard.c.
   It is used to communicate to a lisp process-filter/sentinel (via the
   function Fwaiting_for_user_input_p below) whether emacs was waiting
   for user-input when that process-filter was called.
   waiting_for_input cannot be used as that is by definition 0 when
   lisp code is being evalled */
static int waiting_for_user_input_p;

#include <X11/Xlib.h>
extern Display *XD_display;
extern int x_in_handler_hook;
extern Lisp_Object Vx_event_handler;

/* Read and dispose of subprocess output
 while waiting for timeout to elapse and/or keyboard input to be available.

 time_limit is the timeout in seconds, or zero for no limit.
 -1 means gobble data available immediately but don't wait for any.

 read_kbd is 1 to return when input is available.
 -1 means caller will actually read the input.
 A pointer to a struct Lisp_Process means wait until
 something arrives from that process.

 do_display means redisplay should be done to show
 subprocess output that arrives.  */

wait_reading_process_input (time_limit, read_kbd, do_display)
     int time_limit, read_kbd, do_display;
{
  register int channel, nfds, m;
  int Available = 0;
  int Exception;
  int xerrno;
  Lisp_Object proc;
#ifdef HAVE_TIMEVAL
  struct timeval timeout, end_time, garbage;
#else
  long timeout, end_time, temp;
#endif /* not HAVE_TIMEVAL */
  int Atemp;
  int wait_channel = 0;
  struct Lisp_Process *wait_proc = 0;
  extern kbd_count;
  extern int eqlength;			/* length of internal X event Q */

  /* Detect when read_kbd is really the address of a Lisp_Process.  */
  if (read_kbd > 10 || read_kbd < -1)
    {
      wait_proc = (struct Lisp_Process *) read_kbd;
      wait_channel = XFASTINT (wait_proc->infd);
      read_kbd = 0;
    }
  else if (x_in_handler_hook) time_limit = -1; /* dd/amc */

  waiting_for_user_input_p = read_kbd;

  /* Since we may need to wait several times,
     compute the absolute time to return at.  */
  if (time_limit)
    {
#ifdef HAVE_TIMEVAL
      gettimeofday (&end_time, &garbage);
      end_time.tv_sec += time_limit;
#else /* not HAVE_TIMEVAL */
      time (&end_time);
      end_time += time_limit;
#endif /* not HAVE_TIMEVAL */
    }

  if (fix_screen_hook) (*fix_screen_hook) (); /* dd/amc */

  while (1)
    {
      /* If calling from keyboard input, do not quit
	 since we want to return C-g as an input character.
	 Otherwise, do pending quit if requested.  */
      if (read_kbd >= 0)
	QUIT;

      /* If status of something has changed, and no input is available,
	 notify the user of the change right away */
      if (child_changed && do_display)
	{
	  Atemp = input_wait_mask;
#ifdef HAVE_TIMEVAL
	  timeout.tv_sec=0; timeout.tv_usec=0;
#else /* not HAVE_TIMEVAL */
	  timeout = 0;
#endif /* not HAVE_TIMEVAL */
	  if (select (MAXDESC, &Atemp, 0, 0, &timeout) <= 0)
	    change_msgs();
	}

      /* Don't wait for output from a non-running process.  */
      if (wait_proc != 0
	  && (XFASTINT (wait_proc->flags) & PROC_STATUS) != RUNNING)
	break;

      /* at this point, we're going to wait unless read_kbd and kbd_count
       * are set, so it's a good time to process the X-event Q, before the
       * current time is done
       */

      if (!x_in_handler_hook && x_epoch_hook() && do_display)
	  DoDsp(1);

      /* Compute time from now till when time limit is up */
      /* Exit if already run out */
      if (time_limit == -1)
	{
	  /* -1 specified for timeout means
	     gobble output available now
	     but don't wait at all. */
#ifdef HAVE_TIMEVAL
	  timeout.tv_sec = 0;
	  timeout.tv_usec = 0;
#else
	  timeout = 0;
#endif /* not HAVE_TIMEVAL */
	}
      else if (time_limit)
	{
#ifdef HAVE_TIMEVAL
	  gettimeofday (&timeout, &garbage);
	  timeout.tv_sec = end_time.tv_sec - timeout.tv_sec;
	  timeout.tv_usec = end_time.tv_usec - timeout.tv_usec;
	  if (timeout.tv_usec < 0)
	    timeout.tv_usec += 1000000,
	    timeout.tv_sec--;
	  if (timeout.tv_sec < 0)
	    break;
#else /* not HAVE_TIMEVAL */
          time (&temp);
	  timeout = end_time - temp;
	  if (timeout < 0)
	    break;
#endif /* not HAVE_TIMEVAL */
	}
      else
	{
#ifdef HAVE_TIMEVAL
	  /* If no real timeout, loop sleeping with a big timeout
	     so that input interrupt can wake us up by zeroing it  */
	  timeout.tv_sec = 100;
	  timeout.tv_usec = 0;
#else /* not HAVE_TIMEVAL */
          timeout = 100000;	/* 100000 recognized by the select emulator */
#endif /* not HAVE_TIMEVAL */
	}

      /* Cause C-g and alarm signals to take immediate action,
	 and cause input available signals to zero out timeout */
      if (read_kbd < 0)
	set_waiting_for_input (&timeout);

      /* Wait till there is something to do */

      Available = Exception = input_wait_mask;
      if (!read_kbd)
	Available &= ~1;

      if (read_kbd && kbd_count)
	nfds = 0;
      else if (read_kbd && ! x_in_handler_hook)
	{
	  extern int sigio_critical;
/*	  sigio_critical = 1; */
	  if (!EQ(Qnil,Vx_event_handler) && (eqlength || QLength(XD_display)))
	    timeout.tv_sec = timeout.tv_usec = 0;
#ifdef IBMRTAIX
	  nfds = select (MAXDESC, &Available, 0, 0, &timeout);
#else
#ifdef HPUX
	  nfds = select (MAXDESC, &Available, 0, 0, &timeout);
#else
	  nfds = select (MAXDESC, &Available, 0, &Exception, &timeout);
#endif
#endif
	  sigio_critical = 0;
	}
      else
#ifdef IBMRTAIX
	nfds = select (MAXDESC, &Available, 0, 0, &timeout);
#else
#ifdef HPUX
	nfds = select (MAXDESC, &Available, 0, 0, &timeout);
#else
	nfds = select (MAXDESC, &Available, 0, &Exception, &timeout);
#endif
#endif
      xerrno = errno;

/*      if (fix_screen_hook)
	(*fix_screen_hook) ();
*/

      /* Make C-g and alarm signals set flags again */
      clear_waiting_for_input ();

      if (time_limit && nfds == 0)	/* timeout elapsed */
	break;
      if (nfds < 0)
	{
	  if (xerrno == EINTR)
	    Available = 0;
#ifdef ALLIANT
	  /* This happens for no known reason on ALLIANT.
	     I am guessing that this is the right response. -- RMS.  */
	  else if (xerrno == EFAULT)
	    Available = 0;
#endif
	  else if (xerrno == EBADF)
	    abort ();
	  else
	    error("select error: %s", sys_errlist[xerrno]);
	}
#ifdef sun
      else if (nfds > 0 && (Available & 1) && interrupt_input)
	/* System sometimes fails to deliver SIGIO.  */
	kill (getpid (), SIGIO);
#endif

      /* Check for keyboard input */
      /* If there is any, return immediately
	 to give it higher priority than subprocesses */

      if (read_kbd && (x_in_handler_hook || detect_input_pending ()))
	break;

#ifdef vipc
      /* Check for connection from other process */

      if (Available & ChannelMask (comm_server))
	{
	  Available &= ~(ChannelMask (comm_server));
	  create_commchan ();
	}
#endif vipc

      /* Check for data from a process or a command channel */

      for (channel = 3; Available && channel < MAXDESC; channel++)
	{
	  m = ChannelMask (channel);
	  if (m & Available)
	    {
	      Available &= ~m;
	      /* If waiting for this channel,
		 arrange to return as soon as no more input
		 to be processed.  No more waiting.  */
	      if (wait_channel == channel)
		{
		  wait_channel = 0;
		  time_limit = -1;
		}
	      proc = chan_process[channel];
	      if (NULL (proc))
		continue;

#ifdef vipc
	      /* It's a command channel */
	      if (!NULL (XPROCESS (proc)->command_channel_p))
		{
		  ProcessCommChan (channel, proc);
		  if (NULL (XPROCESS (proc)->command_channel_p))
		    {
		      /* It has ceased to be a command channel! */
		      int bytes_available;
		      if (ioctl (channel, FIONREAD, &bytes_available) < 0)
			bytes_available = 0;
		      if (bytes_available)
			Available |= m;
		    }
		  continue;
		}
#endif vipc

	      /* Read data from the process, starting with our
		 buffered-ahead character if we have one.  */

	      if (read_process_output (proc, channel) > 0)
		{
		  if (do_display)
		    DoDsp (1);
		}
	      else
		{
		  /* Preserve status of processes already terminated.  */
		  child_changed++;
		  deactivate_process (proc);

/*
 * With ptys: when the parent process of a pty exits we are notified,
 * just as we would be with any of our other children.  After the process
 * exits, select() will indicate that we can read the channel.  When we
 * do this, read() returns 0.  Upon receiving this, we close the channel.
 *
 * For external channels, when the peer closes the connection, select()
 * will indicate that we can read the channel.  When we do this, read()
 * returns -1 with errno = ECONNRESET.  Since we never get notified of
 * this via wait3(), we must explictly mark the process as having exited.
 */
		  if ((XFASTINT (XPROCESS (proc)->flags) & PROC_STATUS)
		      == RUNNING)
		    {
		      XFASTINT (XPROCESS (proc)->flags) = EXITED | CHANGED;
		      XFASTINT (XPROCESS (proc)->reason) = 0;
		    }
		}
	    }
	} /* end for */
    } /* end while */
}

/* Read pending output from the process channel,
   starting with our buffered-ahead character if we have one.
   Yield number of characters read.

   This function reads at most 1024 characters.
   If you want to read all available subprocess output,
   you must call it repeatedly until it returns zero.  */

read_process_output (proc, channel)
     Lisp_Object proc;
     register int channel;
{
  register int nchars;
  char chars[1024];
  register Lisp_Object outstream;
  register struct buffer *old = bf_cur;
  register struct Lisp_Process *p = XPROCESS (proc);
  register int opoint;

  if (proc_buffered_char[channel] < 0)
    nchars = read (channel, chars, sizeof chars);
  else
    {
      chars[0] = proc_buffered_char[channel];
      proc_buffered_char[channel] = -1;
      nchars = read (channel, chars + 1, sizeof chars - 1);
      if (nchars < 0)
	nchars = 1;
      else
	nchars = nchars + 1;
    }

  if (nchars <= 0) return 0;

  outstream = p->filter;
  if (!NULL (outstream))
    {
      call2 (outstream, proc, make_string (chars, nchars));
      return nchars;
    }

  /* If no filter, write into buffer if it isn't dead.  */
  if (!NULL (p->buffer) && !NULL (XBUFFER (p->buffer)->name))
    {
      Lisp_Object tem;

      Fset_buffer (p->buffer);
      opoint = point;

      /* Insert new output into buffer
	 at the current end-of-output marker,
	 thus preserving logical ordering of input and output.  */
      if (XMARKER (p->mark)->buffer)
	SetPoint (marker_position (p->mark));
      else
	SetPoint (NumCharacters + 1);
      if (point <= opoint)
	opoint += nchars;

      tem = bf_cur->read_only;
      bf_cur->read_only = Qnil;
      InsCStr (chars, nchars);
      bf_cur->read_only = tem;
      Fset_marker (p->mark, make_number (point), p->buffer);
      RedoModes++;

      SetPoint (opoint);
      SetBfp (old);
    }
  return nchars;
}

DEFUN ("waiting-for-user-input-p", Fwaiting_for_user_input_p, Swaiting_for_user_input_p,
       0, 0, 0,
  "Returns non-NIL if emacs is waiting for input from the user.\n\
This is intended for use by asynchronous process output filters and sentinels.")
       ()
{
  return ((waiting_for_user_input_p) ? Qt : Qnil);
}

/* Sending data to subprocess */

jmp_buf send_process_frame;

send_process_trap ()
{
#ifdef BSD4_1
  sigrelse (SIGPIPE);
  sigrelse (SIGALRM);
#endif /* BSD4_1 */
  longjmp (send_process_frame, 1);
}

send_process_1 (proc, buf, len)
     Lisp_Object proc;
     char *buf;
     int len;
{
  /* Don't use register vars; longjmp can lose them.  */
  int rv;
  unsigned char *procname = XSTRING (XPROCESS (proc)->name)->data;

  if ((XFASTINT (XPROCESS (proc)->flags) & PROC_STATUS) != RUNNING)
    error ("Process %s not running", procname);


  if (!setjmp (send_process_frame))
    while (len > 0)
      {
	signal (SIGPIPE, send_process_trap);
	rv = write (XFASTINT (XPROCESS (proc)->outfd), buf, len);
	signal (SIGPIPE, SIG_DFL);
	if (rv < 0)
	  {
#ifdef EWOULDBLOCK
	    if (errno == EWOULDBLOCK)
	      {
		/* It would be nice to accept process output here,
		   but that is difficult.  For example, it could
		   garbage what we are sending if that is from a buffer.  */
		immediate_quit = 1;
		QUIT;
		sleep (1);
		immediate_quit = 0;
		continue;
	      }
#endif
	    report_file_error ("writing to process", Fcons (proc, Qnil));
	  }
	buf += rv;
	len -= rv;
      }
  else
    {
      XFASTINT (XPROCESS (proc)->flags) =  EXITED | CHANGED;
      deactivate_process (proc);
      error ("SIGPIPE raised on process %s; closed it", procname);
    }
}

/*** Is it really safe for this to get an error ?  */

send_process (proc, buf, count)
     Lisp_Object proc;
     char *buf;
     int count;
{
#ifdef vipc
  struct { int checkword, type, datalen; } header;

  if (!NULL (XPROCESS (proc)->command_channel_p))
    {
      checkword = UNIQUE_FROB;
      type = VIPC_MESG;
      datalen = count;
      send_process_1 (proc, &header, sizeof header);
    }
#endif vipc
  send_process_1 (proc, buf, count);
}

DEFUN ("process-send-region", Fprocess_send_region, Sprocess_send_region,
  3, 3, 0,
  "Send current contents of region as input to PROCESS.\n\
PROCESS may be a process name.\n\
Called from program, takes three arguments, PROCESS, START and END.")
  (process, start, end)
     Lisp_Object process, start, end;
{
  Lisp_Object proc;
  int start1;

  proc = get_process (process);
  validate_region (&start, &end);

  if (XINT (start) < bf_s1 && XINT (end) >= bf_s1)
    move_gap (start);

  start1 = XINT (start);
  send_process (proc, &CharAt (start1), XINT (end) - XINT (start));

  return Qnil;
}

DEFUN ("process-send-string", Fprocess_send_string, Sprocess_send_string,
  2, 2, 0,
  "Send PROCESS the contents of STRING as input.\n\
PROCESS may be a process name.")
  (process, string)
     Lisp_Object process, string;
{
  Lisp_Object proc;
  CHECK_STRING (string, 1);
  proc = get_process (process);
  send_process (proc, XSTRING (string)->data, XSTRING (string)->size);
  return Qnil;
}

/* send a signal number SIGNO to PROCESS.
   CURRENT_GROUP means send to the process group that currently owns
   the terminal being used to communicate with PROCESS.
   This is used for various commands in shell mode.
   If NOMSG is zero, insert signal-announcements into process's buffers
   right away.  */

sig_process (process, signo, current_group, nomsg)
     Lisp_Object process;
     int signo;
     Lisp_Object current_group;
     int nomsg;
{
  Lisp_Object proc;
  register struct Lisp_Process *p;
  int gid;

  proc = get_process (process);
  p = XPROCESS (proc);

  if (!EQ (p->childp, Qt))
    error ("Process %s is not a subprocess",
	   XSTRING (p->name)->data);
  if (!XFASTINT (p->infd))
    error ("Process %s is not active",
	   XSTRING (p->name)->data);

#ifdef TIOCGPGRP		/* Not sure about this! (fnf) */
  /* If we are using pgrps, get a pgrp number and make it negative.  */
  if (!NULL (current_group))
    {
      ioctl (XFASTINT (p->infd), TIOCGPGRP, &gid);
      gid = - gid;
    }
  else
    gid = - XFASTINT (p->pid);
#else /* not using pgrps */
  /* Can't select pgrps on this system, so we know that
     the child itself heads the pgrp.  */
  gid = - XFASTINT (p->pid);
#endif /* not using pgrps */

  switch (signo)
    {
#ifdef SIGCONT
    case SIGCONT:
      XFASTINT (p->flags) = RUNNING | CHANGED;
      child_changed++;
      break;
#endif
    case SIGINT:
    case SIGQUIT:
    case SIGKILL:
      flush_pending_output (XFASTINT (p->infd));
      break;
    }
  /* gid may be a pid, or minus a pgrp's number */
#ifdef BSD
  /* On bsd, [man says] kill does not accept a negative number to kill a pgrp.
     Must do that differently.  */
  killpg (-gid, signo);
#else /* Not BSD.  */
  kill (gid, signo);
#endif /* Not BSD.  */

  /* Put notices in buffers now, since it is safe now.
     Because of this, we know that a process we have just killed
     will never need to use its buffer again.  */
  if (!nomsg)
    change_msgs ();
}

DEFUN ("interrupt-process", Finterrupt_process, Sinterrupt_process, 0, 2, 0,
  "Interrupt process PROCESS.  May be process or name of one.\n\
Nil or no arg means current buffer's process.\n\
Second arg CURRENT-GROUP non-nil means send signal to\n\
the current process-group of the process's controlling terminal\n\
rather than to the process's own process group.\n\
If the process is a shell, this means interrupt current subjob\n\
rather than the shell.")
  (process, current_group)
     Lisp_Object process, current_group;
{
  sig_process (process, SIGINT, current_group, 0);
  return process;
}

DEFUN ("kill-process", Fkill_process, Skill_process, 0, 2, 0,
  "Kill process PROCESS.  May be process or name of one.\n\
See function interrupt-process for more details on usage.")
  (process, current_group)
     Lisp_Object process, current_group;
{
  sig_process (process, SIGKILL, current_group, 0);
  return process;
}

DEFUN ("quit-process", Fquit_process, Squit_process, 0, 2, 0,
  "Send QUIT signal to process PROCESS.  May be process or name of one.\n\
See function interrupt-process for more details on usage.")
  (process, current_group)
     Lisp_Object process, current_group;
{
  sig_process (process, SIGQUIT, current_group, 0);
  return process;
}

DEFUN ("stop-process", Fstop_process, Sstop_process, 0, 2, 0,
  "Stop process PROCESS.  May be process or name of one.\n\
See function interrupt-process for more details on usage.")
  (process, current_group)
     Lisp_Object process, current_group;
{
#ifndef SIGTSTP
  error ("no SIGTSTP support");
#else
  sig_process (process, SIGTSTP, current_group, 0);
#endif
  return process;
}

DEFUN ("continue-process", Fcontinue_process, Scontinue_process, 0, 2, 0,
  "Continue process PROCESS.  May be process or name of one.\n\
See function interrupt-process for more details on usage.")
  (process, current_group)
     Lisp_Object process, current_group;
{
#ifdef SIGCONT
    sig_process (process, SIGCONT, current_group, 0);
#else
    error ("no SIGCONT support");
#endif
  return process;
}

DEFUN ("process-send-eof", Fprocess_send_eof, Sprocess_send_eof, 0, 1, 0,
  "Make PROCESS see end-of-file in its input.\n\
Eof comes after any text already sent to it.\n\
nil or no arg means current buffer's process.")
  (process)
     Lisp_Object process;
{
  Lisp_Object proc;

  proc = get_process (process);
  /* Sending a zero-length record is supposed to mean eof
     when TIOCREMOTE is turned on.  */
#ifdef DID_REMOTE
  {
    char buf[1];
    write (XFASTINT (XPROCESS (proc)->outfd), buf, 0);
  }
#else /* did not do TOICREMOTE */
  send_process (proc, "\004", 1);
#endif /* did not do TOICREMOTE */
  return process;
}

/* Kill all processes associated with `buffer'.
 If `buffer' is nil, kill all processes  */

kill_buffer_processes (buffer)
     Lisp_Object buffer;
{
  Lisp_Object tail, proc;

  for (tail = Vprocess_alist; XGCTYPE (tail) == Lisp_Cons;
       tail = XCONS (tail)->cdr)
    {
      proc = XCONS (XCONS (tail)->car)->cdr;
      if (XGCTYPE (proc) == Lisp_Process
	  && (NULL (buffer) || EQ (XPROCESS (proc)->buffer, buffer)))
	{
	  if (NETCONN_P (proc))
	    deactivate_process (proc);
	  else if (XFASTINT (XPROCESS (proc)->infd))
	    sig_process (proc, SIGHUP, Qnil, 1);
	}
    }
}

count_active_processes ()
{
  register Lisp_Object tail, proc;
  register int count = 0;

  for (tail = Vprocess_alist; !NULL (tail); tail = Fcdr (tail))
    {
      proc = Fcdr (Fcar (tail));

      if ((1 << (XFASTINT (XPROCESS (proc)->flags) & PROC_STATUS)
	   & ((1 << RUNNING) | (1 << STOPPED)))
	  && NULL (XPROCESS (proc)->kill_without_query))
	count++;
    }

  return count;
}

/* On receipt of a signal that a child status has changed,
 loop asking about children with changed statuses until
 the system says there are no more.
   All we do is change the flags components;
 we do not run sentinels or print notifications.
 That is saved for the next time keyboard input is done,
 in order to avoid timing errors.  */

/** WARNING: this can be called during garbage collection.
 Therefore, it must not be fooled by the presence of mark bits in
 Lisp objects.  */

/** USG WARNING:  Although it is not obvious from the documentation
 in signal(2), on a USG system the SIGCLD handler MUST NOT call
 signal() before executing at least one wait(), otherwise the handler
 will be called again, resulting in an infinite loop.  The relevant
 portion of the documentation reads "SIGCLD signals will be queued
 and the signal-catching function will be continually reentered until
 the queue is empty".  Invoking signal() causes the kernel to reexamine
 the SIGCLD queue.   Fred Fish, UniSoft Systems Inc. */

child_sig (signo)
     int signo;
{
  register int pid;
  WAITTYPE w;
  Lisp_Object tail, proc;
  register struct Lisp_Process *p;
  int old_errno = errno;

#ifdef BSD4_1
  extern int synch_process_pid;
  extern int sigheld;
  sigheld |= sigbit (SIGCHLD);
#endif

loop: 

#ifdef WNOHANG
#ifndef WUNTRACED
#define WUNTRACED 0
#endif /* no WUNTRACED */
  pid = wait3 (&w, WNOHANG | WUNTRACED, 0);
  if (pid <= 0)
    {
      if (errno == EINTR)
	{
	  errno = 0;
	  goto loop;
	}
  /* USG systems forget handlers when they are used;
     must reestablish each time */
#ifdef USG
      signal (signo, child_sig);   /* WARNING - must come after wait3() */
#endif
#ifdef  BSD4_1
      sigheld &= ~sigbit (SIGCHLD);
      sigrelse (SIGCHLD);
#endif
      errno = old_errno;
      return;
    }
#else
  pid = wait (&w);
#endif /* no WNOHANG */

#ifdef BSD4_1
  if (synch_process_pid == pid)
    synch_process_pid = 0;         /* Zero it to show process has died. */
#endif

  for (tail = Vprocess_alist; XSYMBOL (tail) != XSYMBOL (Qnil); tail = XCONS (tail)->cdr)
    {
      proc = XCONS (XCONS (tail)->car)->cdr;
      p = XPROCESS (proc);
      if (EQ (p->childp, Qt) && XFASTINT (p->pid) == pid)
	break;
    }

  if (XSYMBOL (tail) == XSYMBOL (Qnil))
#if defined (USG) && ! (defined (HPUX) && defined (WNOHANG))
    goto ignore;
#else
    goto loop;		/* We don't know who this is */
#endif

  child_changed++;
  if (WIFSTOPPED (w))
    {
      XFASTINT (p->flags) = STOPPED | CHANGED;
      XFASTINT (p->reason) = WSTOPSIG (w);
    }
  else if (WIFEXITED (w))
    {
      XFASTINT (p->flags) = EXITED | CHANGED;
      if (WCOREDUMP (w))
	XFASTINT (p->flags) |= COREDUMPED;
      XFASTINT (p->reason) = WRETCODE (w);
    }
  else if (WIFSIGNALED (w))
    {
      XFASTINT (p->flags) = SIGNALED | CHANGED;
      if (WCOREDUMP (w))
	XFASTINT (p->flags) |= COREDUMPED;
      XFASTINT (p->reason) = WTERMSIG (w);
    }
#if !defined (USG) || (defined (HPUX) && defined (WNOHANG))
  goto loop;
#else
 ignore:
#ifdef USG
  signal (signo, child_sig);
#endif
  errno = old_errno;
#endif /* not USG, or HPUX with WNOHANG */
}

/* Find all process marked as "changed"
  and notify the user in a suitable fashion
  (either run the sentinel or output a message).
  This is done while Emacs is waiting for keyboard input */

change_msgs()
{
  register Lisp_Object tail, proc, buffer;
  register struct Lisp_Process *p;
  char line[50];

  child_changed = 0;

  for (tail = Vprocess_alist; !NULL (tail); tail = Fcdr (tail))
    {
      proc = Fcdr (Fcar (tail));
      p = XPROCESS (proc);

      if (!(XFASTINT (p->flags) & CHANGED))
	continue;

      /* If process is still active, read any output that remains.  */
      if (XFASTINT (p->infd))
	while (read_process_output (proc, XFASTINT (p->infd)) > 0);

      XFASTINT (p->flags) &= ~CHANGED;

      line[0] = 0;
      buffer = p->buffer;

      if ((XFASTINT (p->flags) & PROC_STATUS) == SIGNALED
	  || (XFASTINT (p->flags) & PROC_STATUS) == STOPPED)
	{
	  sprintf (line, "%s%s\n",
		   XFASTINT (p->reason) < NSIG
		     ? sys_siglist[XFASTINT (p->reason)] : "unknown",
		   XFASTINT (p->flags) & COREDUMPED ? " (core dumped)" : "");
	  line[0] = DOWNCASE (line[0]);

	  if ((XFASTINT (p->flags) & PROC_STATUS) == SIGNALED)
	    if (delete_exited_processes)
	      remove_process (proc);
	    else
	      deactivate_process (proc);
	}
      else if ((XFASTINT (p->flags) & PROC_STATUS) == EXITED)
	{
	  if (XFASTINT (p->reason))
	    sprintf (line, "exited abnormally with code %d\n",
		     XFASTINT (p->reason));
	  else
	    sprintf (line, "finished\n");

	  if (delete_exited_processes)
	    remove_process (proc);
	  else
	    deactivate_process (proc);
	}

      if (!NULL (p->sentinel))
	exec_sentinel (proc, build_string (line));
      else if (line[0] && !NULL (buffer))
	{
	  Lisp_Object ro = XBUFFER (buffer)->read_only;
	  struct buffer *old = bf_cur;
	  int opoint;

	  /* Avoid error if buffer is deleted
	     (probably that's why the process is dead, too) */
	  if (NULL (XBUFFER (buffer)->name))
	    continue;
	  Fset_buffer (buffer);
	  opoint = point;
	  SetPoint (NumCharacters + 1);
	  if (point == opoint)
	    opoint = -1;
	  bf_cur->read_only = Qnil;
	  InsStr ("\nProcess ");
	  Finsert (1, &p->name);
	  InsStr (" ");
	  InsStr (line);
	  bf_cur->read_only = ro;
	  if (opoint > 0)
	    SetPoint (opoint);
	  SetBfp (old);
	}
    } /* end for */

  RedoModes++;  /* in case buffers use %s in mode-line-format */
  DoDsp (1);
}

exec_sentinel (proc, reason)
     Lisp_Object proc, reason;
{
  Lisp_Object sentinel;
  register struct Lisp_Process *p = XPROCESS (proc);

  sentinel = p->sentinel;
  if (NULL (sentinel))
    return;

  p->sentinel = Qnil;
  call2 (sentinel, proc, reason);
  p->sentinel = sentinel;
}

init_process ()
{
  register int i;

#ifdef SIGCHLD
#ifndef CANNOT_DUMP
  if (! noninteractive || initialized)
#endif
    signal (SIGCHLD, child_sig);
#endif

  input_wait_mask = ChannelMask(0);
  Vprocess_alist = Qnil;
  for (i = 0; i < MAXDESC; i++)
    {
      chan_process[i] = Qnil;
      proc_buffered_char[i] = -1;
    }
}

syms_of_process ()
{
  Qprocessp = intern ("processp");
  staticpro (&Qprocessp);

  staticpro (&Vprocess_alist);

  DEFVAR_BOOL ("delete-exited-processes", &delete_exited_processes,
    "*Non-nil means delete processes immediately when they exit.\n\
nil means don't delete them until `list-processes' is run.");

  delete_exited_processes = 1;

  DEFVAR_LISP ("process-connection-type", &Vprocess_connection_type,
    "Control type of device used to communicate with subprocesses.\n\
Values are nil to use a pipe, t for a pty (or pipe if ptys not supported).\n\
Value takes effect when `start-process' is called.");
  Vprocess_connection_type = Qt;

  defsubr (&Sprocessp);
  defsubr (&Sget_process);
  defsubr (&Sget_buffer_process);
  defsubr (&Sdelete_process);
  defsubr (&Sprocess_status);
  defsubr (&Sprocess_exit_status);
  defsubr (&Sprocess_id);
  defsubr (&Sprocess_name);
  defsubr (&Sprocess_command);
  defsubr (&Sset_process_buffer);
  defsubr (&Sprocess_buffer);
  defsubr (&Sprocess_mark);
  defsubr (&Sset_process_filter);
  defsubr (&Sprocess_filter);
  defsubr (&Sset_process_sentinel);
  defsubr (&Sprocess_sentinel);
  defsubr (&Sprocess_kill_without_query);
  defsubr (&Slist_processes);
  defsubr (&Sprocess_list);
  defsubr (&Sstart_process);
#ifdef HAVE_SOCKETS
  defsubr (&Sopen_network_stream);
#endif /* HAVE_SOCKETS */
  defsubr (&Saccept_process_output);
  defsubr (&Sprocess_send_region);
  defsubr (&Sprocess_send_string);
  defsubr (&Sinterrupt_process);
  defsubr (&Skill_process);
  defsubr (&Squit_process);
  defsubr (&Sstop_process);
  defsubr (&Scontinue_process);
  defsubr (&Sprocess_send_eof);
  defsubr (&Swaiting_for_user_input_p);
}

#endif subprocesses
