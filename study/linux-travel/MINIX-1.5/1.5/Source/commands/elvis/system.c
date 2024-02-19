/* system.c  -- UNIX version */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains a new version of the system() function and related stuff.
 *
 * Entry points are:
 *	system(cmd)	- run a single shell command
 *	wildcard(names)	- expand wildcard characters in filanames
 *	filter(m,n,cmd)	- run text lines through a filter program
 *
 * This is probably the single least portable file in the program.  The code
 * shown here should work correctly if it links at all; it will work on UNIX
 * and any O.S./Compiler combination which adheres to UNIX forking conventions.
 * For DOS, these three functions will have to be reimplemented from scratch.
 */

#include <sys/types.h>
#include <signal.h>
#include "vi.h"
extern		trapint();
extern char	**environ;


/* This is a new version of the system() function.  The only difference
 * between this one and the library one is: this one uses the o_shell option.
 */
int system(cmd)
	char	*cmd;	/* a command to run */
{
	int	status;	/* exit status of the command */

	switch (fork())
	{
	  case -1:						/* error */
		status = -1;
		break;

	  case 0:						/* child */
		execle(o_shell, o_shell, "-c", cmd, (char *)0, environ);
		exit(1); /* if we get here, the exec failed */

	  default:						/* parent */
		signal(SIGINT, SIG_IGN);
		wait(&status);
		signal(SIGINT, trapint);
	}

	return status;
}


/* This private function opens a pipe from a filter.  It is similar to the
 * system() function above, and to popen(cmd, "r").
 */
static int rpipe(cmd, in)
	char	*cmd;	/* the filter command to use */
	int	in;	/* the fd to use for stdin */
{
	int	r0w1[2];/* the pipe fd's */

	/* make the pipe */
	if (pipe(r0w1) < 0)
	{
		return -1;	/* pipe failed */
	}

	switch (fork())
	{
	  case -1:						/* error */
		return -1;

	  case 0:						/* child */
		/* close the "read" end of the pipe */
		close(r0w1[0]);

		/* redirect stdout to go to the "write" end of the pipe */
		close(1);
		dup(r0w1[1]);
		close(2);
		dup(r0w1[1]);
		close(r0w1[1]);

		/* redirect stdin */
		if (in != 0)
		{
			close(0);
			dup(in);
			close(in);
		}

		/* exec the shell to run the command */
		execle(o_shell, o_shell, "-c", cmd, (char *)0, environ);
		exit(1); /* if we get here, exec failed */

	  default:						/* parent */
		signal(SIGINT, SIG_IGN);	/* <- reset after the wait() */

		/* close the "write" end of the pipe */	
		close(r0w1[1]);

		return r0w1[0];
	}
}

/* This function closes the pipe opened by rpipe(), and returns 0 for success */
int rpclose(fd)
	int	fd;
{
	int	status;

	close(fd);
	wait(&status);
	signal(SIGINT, trapint);
	return status;
}

/* This function expands wildcards in a filename or filenames.  It does this
 * by running the "echo" command on the filenames via the shell; it is assumed
 * that the shell will expand the names for you.  If for any reason it can't
 * run echo, then it returns the names unmodified.
 */
char *wildcard(names)
	char	*names;
{
	int	i, j, fd;

	/* build the echo command */
	if (names != tmpblk.c)
	{
		/* the names aren't in tmpblk.c, so we can do it the easy way */
		strcpy(tmpblk.c, "echo ");
		strcat(tmpblk.c, names);
	}
	else
	{
		register char *s, *d;

		/* the names are already in tmpblk.c, so shift them to make
		 * room for the word "echo "
		 */
		for (s = names + strlen(names) + 1, d = s + 5; s > names; )
		{
			*--d = *--s;
		}
		strncpy(names, "echo ", 5);
	}

	/* run the command & read the resulting names */
	fd = rpipe(tmpblk.c, 0);
	if (fd < 0) return names;
	i = 0;
	do
	{
		j = read(fd, tmpblk.c, BLKSIZE - i);
		i += j;
	} while (j > 0);

	/* successful? */
	if (rpclose(fd) == 0 && j == 0 && i < BLKSIZE && i > 0)
	{
		tmpblk.c[i-1] = '\0'; /* "i-1" so we clip off the newline */
		return tmpblk.c;
	}
	else
	{
		return names;
	}
}

/* This function runs a range of lines through a filter program, and replaces
 * the original text with the filtered version.
 */
int filter(from, to, cmd)
	MARK	from, to;	/* the range of lines to filter */
	char	*cmd;		/* the filter command */
{
	int	scratch;	/* fd of the scratch file */
	int	fd;		/* fd of the pipe from the filter */
	int	i;

	/* write the lines to a temp file */
	cmd_write(from, to, CMD_BANG, 0, SCRATCHFILE);

	/* start the filter program */
	scratch = open(SCRATCHFILE, O_RDONLY);
	if (scratch < 0)
	{
		unlink(SCRATCHFILE);
		return -1;
	}
	fd = rpipe(cmd, scratch);
	if (fd < 0)
	{
		close(scratch);
		unlink(SCRATCHFILE);
		return -1;
	}

	ChangeText
	{
		/* delete the original lines.  Lines!  */
		from &= ~(BLKSIZE - 1);
		to &= ~(BLKSIZE - 1);
		to += BLKSIZE;
		delete(from, to);

		/* repeatedly read in new text and add it */
		while ((i = read(fd, tmpblk.c, BLKSIZE)) > 0)
		{
			tmpblk.c[i] = '\0';
			add(from, tmpblk.c);
			for (i = 0; tmpblk.c[i]; i++)
			{
				if (tmpblk.c[i] == '\n')
				{
					from = (from & ~(BLKSIZE - 1)) + BLKSIZE;
				}
				else
				{
					from++;
				}
			}
		}
	}

	/* cleanup */
	rpclose(fd);
	close(scratch);
	unlink(SCRATCHFILE);
	return 0;
}
