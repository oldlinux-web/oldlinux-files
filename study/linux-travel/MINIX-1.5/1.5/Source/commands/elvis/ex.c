/* ex.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains the code for reading ex commands. */

#include <ctype.h>
#include "vi.h"

/* This data type is used to describe the possible argument combinations */
typedef short ARGT;
#define FROM	1		/* allow a linespec */
#define	TO	2		/* allow a second linespec */
#define BANG	4		/* allow a ! after the command name */
#define EXTRA	8		/* allow extra args after command name */
#define XFILE	16		/* expand wildcards in extra part */
#define NOSPC	32		/* no spaces allowed in the extra part */
#define	DFLALL	64		/* default file range is 1,$ */
#define DFLNONE	128		/* no default file range */
#define NODFL	256		/* do not default to the current file name */
#define FILES	(XFILE + EXTRA)	/* multiple extra files allowed */
#define WORD1	(EXTRA + NOSPC)	/* one extra word allowed */
#define FILE1	(FILES + NOSPC)	/* 1 file allowed, defaults to current file */
#define NAMEDF	(FILE1 + NODFL)	/* 1 file allowed, defaults to "" */
#define NAMEDFS	(FILES + NODFL)	/* multiple files allowed, default is "" */
#define RANGE	(FROM + TO)	/* range of linespecs allowed */
#define NONE	0		/* no args allowed at all */

/* This array maps ex command names to command codes. The order in which
 * command names are listed below is significant -- ambiguous abbreviations
 * are always resolved to be the first possible match.  (e.g. "r" is taken
 * to mean "read", not "rewind", because "read" comes before "rewind")
 */
static struct
{
	char	*name;	/* name of the command */
	CMD	code;	/* enum code of the command */
	void	(*fn)();/* function which executes the command */
	ARGT	argt;	/* command line arguments permitted/needed/used */
}
	cmdnames[] =
{   /*	cmd name	cmd code	function	arguments */
	{"append",	CMD_APPEND,	cmd_append,	FROM		},
#ifdef DEBUG
	{"bug",		CMD_DEBUG,	cmd_debug,	RANGE+BANG+EXTRA},
#endif
	{"change",	CMD_CHANGE,	cmd_append,	RANGE		},
	{"delete",	CMD_DELETE,	cmd_delete,	RANGE+WORD1	},
	{"edit",	CMD_EDIT,	cmd_edit,	BANG+FILE1	},
	{"file",	CMD_FILE,	cmd_file,	NONE		},
	{"global",	CMD_GLOBAL,	cmd_global,	RANGE+BANG+EXTRA+DFLALL},
	{"insert",	CMD_INSERT,	cmd_append,	FROM		},
	{"join",	CMD_INSERT,	cmd_join,	RANGE		},
	{"k",		CMD_MARK,	cmd_mark,	FROM+WORD1	},
	{"list",	CMD_LIST,	cmd_list,	RANGE		},
	{"move",	CMD_MOVE,	cmd_move,	RANGE+EXTRA	},
	{"next",	CMD_NEXT,	cmd_next,	BANG+NAMEDFS	},
	{"Next",	CMD_PREVIOUS,	cmd_next,	BANG		},
	{"print",	CMD_PRINT,	cmd_print,	RANGE		},
	{"quit",	CMD_QUIT,	cmd_quit,	BANG		},
	{"read",	CMD_READ,	cmd_read,	FROM+NAMEDF	},
	{"substitute",	CMD_SUBSTITUTE,	cmd_substitute,	RANGE+EXTRA	},
	{"to",		CMD_COPY,	cmd_move,	RANGE+EXTRA	},
	{"undo",	CMD_UNDO,	cmd_undo,	NONE		},
	{"vglobal",	CMD_VGLOBAL,	cmd_global,	RANGE+EXTRA+DFLALL},
	{"write",	CMD_WRITE,	cmd_write,	RANGE+BANG+FILE1+DFLALL},
	{"xit",		CMD_XIT,	cmd_xit,	BANG		},
	{"yank",	CMD_YANK,	cmd_delete,	RANGE+WORD1	},

	{"!",		CMD_BANG,	cmd_shell,	RANGE+NAMEDFS+DFLNONE},
	{"<",		CMD_SHIFTL,	cmd_shift,	RANGE		},
	{">",		CMD_SHIFTR,	cmd_shift,	RANGE		},
	{"=",		CMD_FILE,	cmd_file,	RANGE		},

	{"args",	CMD_ARGS,	cmd_args,	NAMEDFS		},
	{"cd",		CMD_CD,		cmd_cd,		NAMEDF		},
	{"copy",	CMD_COPY,	cmd_move,	RANGE+EXTRA	},
	{"ex",		CMD_EDIT,	cmd_edit,	BANG+FILE1	},
	{"map",		CMD_MAP,	cmd_map,	BANG+EXTRA	},
	{"mkexrc",	CMD_MKEXRC,	cmd_mkexrc,	NONE		},
	{"put",		CMD_PUT,	cmd_put,	FROM+WORD1	},
	{"set",		CMD_SET,	cmd_set,	EXTRA		},
	{"shell",	CMD_SHELL,	cmd_shell,	NONE		},
	{"source",	CMD_SOURCE,	cmd_source,	NAMEDF		},
	{"tag",		CMD_TAG,	cmd_tag,	BANG+WORD1	},
	{"version",	CMD_VERSION,	cmd_version,	NONE		},
	{"visual",	CMD_VISUAL,	cmd_visual,	NONE		},
	{"wq",		CMD_XIT,	cmd_xit,	NONE		},

#ifdef DEBUG
	{"debug",	CMD_DEBUG,	cmd_debug,	RANGE+BANG+EXTRA},
	{"validate",	CMD_VALIDATE,	cmd_validate,	BANG		},
#endif
	{"chdir",	CMD_CD,		cmd_cd,		NAMEDF		},
	{"mark",	CMD_MARK,	cmd_mark,	FROM+WORD1	},
	{"previous",	CMD_PREVIOUS,	cmd_next,	BANG		},
	{"rewind",	CMD_REWIND,	cmd_rewind,	BANG		},
	{"unmap",	CMD_UNMAP,	cmd_map,	BANG+EXTRA	},

	{(char *)0}
};


/* This function parses a search pattern - given a pointer to a / or ?,
 * it replaces the ending / or ? with a \0, and returns a pointer to the
 * stuff that came after the pattern.
 */
char	*parseptrn(ptrn)
	register char	*ptrn;
{
	register char 	*scan;


	for (scan = ptrn + 1;
	     *scan && *scan != *ptrn;
	     scan++)
	{
		/* allow backslashed versions of / and ? in the pattern */
		if (*scan == '\\' && scan[1] != '\0')
		{
			scan++;
		}
	}
	if (*scan)
	{
		*scan++ = '\0';
	}

	return scan;
}


/* This function parses a line specifier for ex commands */
char *linespec(s, markptr)
	register char	*s;		/* start of the line specifier */
	MARK		*markptr;	/* where to store the mark's value */
{
	long		num;
	register char	*t;

	/* parse each ;-delimited clause of this linespec */
	do
	{
		/* skip an initial ';', if any */
		if (*s == ';')
		{
			s++;
		}

		/* skip leading spaces */
		while (isspace(*s))
		{
			s++;
		}
	
		/* dot means current position */
		if (*s == '.')
		{
			s++;
			*markptr = cursor;
		}
		/* '$' means the last line */
		else if (*s == '$')
		{
			s++;
			*markptr = movetoline(cursor, nlines);
		}
		/* digit means an absolute line number */
		else if (isdigit(*s))
		{
			for (num = 0; isdigit(*s); s++)
			{
				num = num * 10 + *s - '0';
			}
			*markptr = movetoline(cursor, num);
		}
		/* appostrophe means go to a set mark */
		else if (*s == '\'')
		{
			s++;
			*markptr = movetomark(cursor, 1L, (int)*s);
			s++;
		}
		/* slash means do a search */
		else if (*s == '/' || *s == '?')
		{
			/* put a '\0' at the end of the search pattern */
			t = parseptrn(s);
	
			/* search for the pattern */
			if (*s == '/')
			{
				pfetch(markline(*markptr));
				*markptr = (*markptr & ~(BLKSIZE - 1)) + plen - 1;
				*markptr = movefsrch(*markptr, s + 1);
			}
			else
			{
				*markptr &= ~(BLKSIZE - 1);
				*markptr = movebsrch(*markptr, s + 1);
			}
	
			/* adjust command string pointer */
			s = t;
		}
	
		/* if linespec was faulty, quit now */
		if (!*markptr)
		{
			return s;
		}
	
		/* maybe add an offset */
		if (*s == '-')
		{
			s++;
			for (num = 0; *s >= '0' && *s <= '9'; s++)
			{
				num = num * 10 + *s - '0';
			}
			if (num == 0)
			{
				num = 1;
			}
			*markptr = moveup(*markptr, num);
		}
		else if (*s == '+')
		{
			s++;
			for (num = 0; *s >= '0' && *s <= '9'; s++)
			{
				num = num * 10 + *s - '0';
			}
			if (num == 0)
			{
				num = 1;
			}
			*markptr = movedown(*markptr, num);
		}
	} while (*s == ';' || *s == '+' || *s == '-');

	return s;
}



/* This function reads an ex command and executes it. */
ex()
{
	char		cmdbuf[80];
	register int	cmdlen;

	/* read a line */
	cmdlen = vgets(':', cmdbuf, sizeof cmdbuf);
	if (cmdlen < 0)
	{
		return;
	}
	addch('\n');
	refresh();

	/* if empty line, assume ".+1" */
	if (cmdlen == 0)
	{
		strcpy(cmdbuf, ".+1");
	}

	/* parse & execute the command */
	doexcmd(cmdbuf);
}

doexcmd(cmdbuf)
	char		*cmdbuf;	/* string containing an ex command */
{
	register char	*scan;		/* used to scan thru cmdbuf */
	MARK		frommark;	/* first linespec */
	MARK		tomark;		/* second linespec */
	register int	cmdlen;		/* length of the command name given */
	CMD		cmd;		/* what command is this? */
	ARGT		argt;		/* argument types for this command */
	short		forceit;	/* bang version of a command? */
	register int	cmdidx;		/* index of command */
	register char	*build;		/* used while copying filenames */
	int		iswild;		/* boolean: filenames use wildcards? */
	int		isdfl;		/* using default line ranges? */


	/* ex commands can't be undone via the shift-U command */
	U_line = 0L;

	/* ignore command lines that start with "#" */
	if (*cmdbuf == '#' || *cmdbuf == '"')
	{
		return;
	}

	/* permit extra colons at the start of the line */
	while (*cmdbuf == ':')
	{
		cmdbuf++;
	}

	/* parse the line specifier */
	scan = cmdbuf;
	if (*scan == '%')
	{
		/* '%' means all lines */
		frommark = movetoline(cursor, 1L);
		tomark = movetoline(cursor, nlines);
		scan++;
	}
	else
	{
		frommark = cursor;
		scan = linespec(scan, &frommark);
		tomark = frommark;
		if (frommark && *scan == ',')
		{
			scan++;
			scan = linespec(scan, &tomark);
		}
		if (!tomark)
		{
			/* faulty line spec -- fault already described */
			return;
		}
		if (frommark > tomark)
		{
			msg("first address exceeds the second");
			return;
		}
	}
	isdfl = (scan == cmdbuf);

	/* skip whitespace */
	while (isspace(*scan))
	{
		scan++;
	}

	/* if no command, then just move the cursor to the mark & print */
	if (!*scan)
	{
		cursor = tomark;
		if (mode != MODE_EX)
		{
			return;
		}
		scan = "p";
	}

	/* figure out how long the command name is */
	if (!isalpha(*scan))
	{
		cmdlen = 1;
	}
	else
	{
		for (cmdlen = 1;
		     isalpha(scan[cmdlen]);
		     cmdlen++)
		{
		}
	}

	/* lookup the command code */
	for (cmdidx = 0;
	     cmdnames[cmdidx].name && strncmp(scan, cmdnames[cmdidx].name, cmdlen);
	     cmdidx++)
	{
	}
	argt = cmdnames[cmdidx].argt;
	cmd = cmdnames[cmdidx].code;
	if (cmd == CMD_NULL)
	{
		msg("Unknown command \"%.*s\"", cmdlen, scan);
		return;
	}

	/* if the command ended with a bang, set the forceit flag */
	scan += cmdlen;
	if ((argt & BANG) && *scan == '!')
	{
		scan++;
		forceit = 1;
	}
	else
	{
		forceit = 0;
	}

	/* skip any more whitespace, to leave scan pointing to arguments */
	while (isspace(*scan))
	{
		scan++;
	}

	/* a couple of special cases for filenames */
	if (argt & XFILE)
	{
		/* if names were given, process them */
		if (*scan)
		{
			for (build = tmpblk.c, iswild = FALSE; *scan; scan++)
			{
				switch (*scan)
				{
				  case '%':
					if (!*origname)
					{
						msg("No filename to substitute for %");
						return;
					}
					strcpy(build, origname);
					while (*build)
					{
						build++;
					}
					break;
	
				  case '#':
					if (!*prevorig)
					{
						msg("No filename to substitute for #");
						return;
					}
					strcpy(build, prevorig);
					while (*build)
					{
						build++;
					}
					break;
	
				  case '*':
				  case '?':
				  case '[':
				  case '`':
				  case '{':
				  case '$':
				  case '~':
					*build++ = *scan;
					iswild = TRUE;
					break;
	
				  default:
					*build++ = *scan;
				}
			}
			*build = '\0';
	
			if (iswild && cmd != CMD_BANG && tmpblk.c[0] != '>')
			{
				scan = wildcard(tmpblk.c);
			}
		}
		else /* no names given, maybe assume origname */
		{
			if (!(argt & NODFL))
			{
				strcpy(tmpblk.c, origname);
			}
			else
			{
				*tmpblk.c = '\0';
			}
		}

		scan = tmpblk.c;
	}

	/* bad arguments? */
	if (!(argt & FROM) && frommark != cursor)
	{
		msg("Can't use address with \"%s\" command.", cmdnames[cmdidx].name);
		return;
	}
	if (!(argt & TO) && tomark != frommark)
	{
		msg("Can't use a range with \"%s\" command.", cmdnames[cmdidx].name);
		return;
	}
	if (!(argt & EXTRA) && *scan)
	{
		msg("Extra characters after \"%s\" command.", cmdnames[cmdidx].name);
		return;
	}
	if (argt & NOSPC)
	{
		for (build = scan; *build; build++)
		{
			if (isspace(*build))
			{
				msg("Too many %s to \"%s\" command.",
					(argt & XFILE) ? "filenames" : "arguments",
					cmdnames[cmdidx].name);
				return;
			}
		}
	}

	/* some commands have special default ranges */
	if (isdfl && (argt & DFLALL))
	{
		frommark = MARK_FIRST;
		tomark = MARK_LAST;
	}
	else if (isdfl && (argt & DFLNONE))
	{
		frommark = tomark = 0L;
	}

	/* act on the command */
	(*cmdnames[cmdidx].fn)(frommark, tomark, cmd, forceit, scan);
}


/* This function executes EX commands from a file.  It returns 1 normally, or
 * 0 if the file could not be opened for reading.
 */
int doexrc(filename)
	char	*filename;	/* name of a ".exrc" file */
{
	int	fd;		/* file descriptor */
	int	len;		/* length of the ".exrc" file */
	char	*cmd;		/* start of a command */
	char	*end;		/* used to search for the end of cmd */
	char	buf[MAXRCLEN];	/* buffer, holds the entire .exrc file */

	/* open the file, read it, and close */
	fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		return 0;
	}
	len = read(fd, buf, MAXRCLEN);
	close(fd);

	/* find & do each command */
	for (cmd = buf; cmd < &buf[len]; cmd = end + 1)
	{
		/* find the end of the command */
		for (end = cmd; *end != '\n'; end++)
		{
		}
		*end = '\0';

		/* do it */
		doexcmd(cmd);
	}

	return 1;
}
