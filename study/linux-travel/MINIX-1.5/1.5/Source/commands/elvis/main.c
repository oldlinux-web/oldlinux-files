/* main.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains the main() function of vi */

#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>
#include "vi.h"

extern		trapint(); /* defined below */
extern char	*getenv();
jmp_buf		jmpenv;

/*---------------------------------------------------------------------*/

main(argc, argv)
	int	argc;
	char	*argv[];
{
	int	i;
	char	*cmd = (char *)0;
	char	*tag = (char *)0;
	char	*str;

	/* set mode to MODE_VI or MODE_EX depending on program name */
	switch (argv[0][strlen(argv[0]) - 1])
	{
	  case 'x':
		mode = MODE_EX;
		break;

	  case 'w':
		mode = MODE_VI;
		*o_readonly = TRUE;
		break;

	  default:
		mode = MODE_VI;
	}

	/* start curses */
	initscr();
	cbreak();
	noecho();
	scrollok(stdscr, TRUE);

#ifdef DEBUG
	/* normally, we ignore SIGQUIT.  SIGINT is trapped later */
	signal(SIGQUIT, SIG_IGN);
#endif

	/* map the arrow keys.  The KU,KD,KL,and KR variables correspond to
	 * the :ku=: (etc.) termcap capabilities.  The variables are defined
	 * as part of the curses package.
	 */
	if (KU) mapkey(KU, "k", WHEN_VICMD, "<up>");
	if (KD) mapkey(KD, "j", WHEN_VICMD, "<down>");
	if (KL) mapkey(KL, "h", WHEN_VICMD, "<left>");
	if (KR) mapkey(KR, "l", WHEN_VICMD, "<right>");
	if (PU) mapkey(PU, "\002", WHEN_VICMD, "<PgUp>");
	if (PD) mapkey(PD, "\006", WHEN_VICMD, "<PgDn>");
	if (HM) mapkey(HM, "\031", WHEN_VICMD, "<Home>");
	if (EN) mapkey(EN, "\005", WHEN_VICMD, "<End>");

	/* initialize the options */
	initopts();

	/* process any flags */
	for (i = 1; i < argc && *argv[i] == '-'; i++)
	{
		switch (argv[i][1])
		{
		  case 'R':	/* readonly */
			*o_readonly = TRUE;
			break;

		  case 'r':	/* recover */
			addstr("Use the `virecover` command to recover lost files\n");
			refresh();
			endwin();
			exit(0);
			break;

		  case 't':	/* tag */
			if (argv[i][2])
			{
				tag = argv[i] + 2;
			}
			else
			{
				i++;
				tag = argv[i];
			}
			break;

		  case 'v':	/* vi mode */
			mode = MODE_VI;
			break;

		  case 'e':	/* ex mode */
			mode = MODE_EX;
			break;
		}
	}

	/* if we were given an initial ex command, save it... */
	if (i < argc && *argv[i] == '+')
	{
		if (argv[i][1])
		{
			cmd = argv[i++] + 1;
		}
		else
		{
			cmd = "$"; /* "vi + file" means start at EOF */
			i++;
		}
	}

	/* the remaining args are file names. */
	nargs = argc - i;
	if (nargs > 0)
	{
		strcpy(args, argv[i]);
		while (++i < argc)
		{
			strcat(args, " ");
			strcat(args, argv[i]);
		}
		argno = 1;
	}
	argno = 0;

	/* search for a tag now, if desired */
	blkinit();
	if (tag)
	{
		cmd_tag(MARK_FIRST, MARK_FIRST, CMD_TAG, 0, tag);
	}

	/* if no tag, or tag failed, then start with first arg */
	if (tmpfd < 0 && tmpstart(argv[i - nargs]) == 0 && *origname)
	{
		ChangeText
		{
		}
		clrflag(file, MODIFIED);
	}

	/* perform the .exrc files and EXINIT environment variable */
	str = getenv("HOME");
	if (str)
	{
		sprintf(tmpblk.c, "%s%c%s", str, SLASH, HMEXRC);
		doexrc(tmpblk.c);
	}
	doexrc(EXRC);
	str = getenv("EXINIT");
	if (str)
	{
		doexcmd(str);
	}

	/* now we do the immediate ex command that we noticed before */
	if (cmd)
	{
		doexcmd(cmd);
	}

	/* repeatedly call ex() or vi() (depending on the mode) until the
	 * mode is set to MODE_QUIT
	 */
	while (mode != MODE_QUIT)
	{
  		setjmp(jmpenv);
		signal(SIGINT, trapint);

		switch (mode)
		{
		  case MODE_VI:
			vi();
			break;

		  case MODE_EX:
			ex();
			break;
#ifdef DEBUG
		  default:
			msg("mode = %d?", mode);
			mode = MODE_QUIT;
#endif
		}
	}

	/* free up the cut buffers */
	cutend();

	/* end curses */
	endwin();

	exit(0);
}


trapint(signo)
	int	signo;
{
	resume_curses(FALSE);
	longjmp(jmpenv, 1);
}
