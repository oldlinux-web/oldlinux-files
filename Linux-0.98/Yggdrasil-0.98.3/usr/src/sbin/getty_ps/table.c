/*
**	$Id: table.c,v 2.0 90/09/19 20:18:46 paul Rel $
**
**	Routines to process the gettytab file.
*/

/*
**	Copyright 1989,1990 by Paul Sutcliffe Jr.
**
**	Permission is hereby granted to copy, reproduce, redistribute,
**	or otherwise use this software as long as: there is no monetary
**	profit gained specifically from the use or reproduction or this
**	software, it is not sold, rented, traded or otherwise marketed,
**	and this copyright notice is included prominently in any copy
**	made.
**
**	The author make no claims as to the fitness or correctness of
**	this software for any use whatsoever, and it is provided as is. 
**	Any use of this software is at the user's own risk.
*/

/*
**	$Log:	table.c,v $
**	Revision 2.0  90/09/19  20:18:46  paul
**	Initial 2.0 release
**	
*/


#include "getty.h"
#include "table.h"

#if defined(RCSID) && !defined(lint)
static char *RcsId =
"@(#)$Id: table.c,v 2.0 90/09/19 20:18:46 paul Rel $";
#endif


/*	Sane conditions.
 */
#define	ISANE	( BRKINT | IGNPAR | ISTRIP | ICRNL | IXON | IXANY )
#define	OSANE	( OPOST | ONLCR )
#define	CSANE	( DEF_CFL | CREAD | HUPCL )
#define	LSANE	( ISIG | ICANON | ECHO | ECHOE | ECHOK )

#define	CC_SANE	{ CINTR, CQUIT, CERASE, CKILL, CEOF, CNUL, CNUL, CNUL }


/*	States for gtabvalue()
 */
#define	ENTRY	0	/* looking for an entry line */
#define	QUIT	1	/* error occurred */


#define	NULLPTR		(char *) NULL


/*	All possible mode flags.
 */

SYMTAB	imodes[] = {
	{ "IGNBRK",	IGNBRK	},
	{ "BRKINT",	BRKINT	},
	{ "IGNPAR",	IGNPAR	},
	{ "PARMRK",	PARMRK	},
	{ "INPCK",	INPCK	},
	{ "ISTRIP",	ISTRIP	},
	{ "INLCR",	INLCR	},
	{ "IGNCR",	IGNCR	},
	{ "ICRNL",	ICRNL	},
	{ "IUCLC",	IUCLC	},
	{ "IXON",	IXON	},
	{ "IXANY",	IXANY	},
	{ "IXOFF",	IXOFF	},
	{  NULLPTR,	0	}
};

SYMTAB	omodes[] = {
	{ "OPOST",	OPOST	},
	{ "OLCUC",	OLCUC	},
	{ "ONLCR",	ONLCR	},
	{ "OCRNL",	OCRNL	},
	{ "ONOCR",	ONOCR	},
	{ "ONLRET",	ONLRET	},
	{ "OFILL",	OFILL	},
	{ "OFDEL",	OFDEL	},
	{ "NLDLY",	NLDLY	},
	{ "NL0",	NL0	},
	{ "NL1",	NL1	},
	{ "CRDLY",	CRDLY	},
	{ "CR0",	CR0	},
	{ "CR1",	CR1	},
	{ "CR2",	CR2	},
	{ "CR3",	CR3	},
	{ "TABDLY",	TABDLY	},
	{ "TAB0",	TAB0	},
	{ "TAB1",	TAB1	},
	{ "TAB2",	TAB2	},
	{ "TAB3",	TAB3	},
	{ "BSDLY",	BSDLY	},
	{ "BS0",	BS0	},
	{ "BS1",	BS1	},
	{ "VTDLY",	VTDLY	},
	{ "VT0",	VT0	},
	{ "VT1",	VT1	},
	{ "FFDLY",	FFDLY	},
	{ "FF0",	FF0	},
	{ "FF1",	FF1	},
	{  NULLPTR,	0	}
};

SYMTAB	cmodes[] = {
	{ "B0",		B0	},
	{ "B50",	B50	},
	{ "B75",	B75	},
	{ "B110",	B110	},
	{ "B134",	B134	},
	{ "B150",	B150	},
	{ "B200",	B200	},
	{ "B300",	B300	},
	{ "B600",	B600	},
	{ "B1200",	B1200	},
	{ "B1800",	B1800	},
	{ "B2400",	B2400	},
	{ "B4800",	B4800	},
	{ "B9600",	B9600	},
#ifdef	B19200
	{ "B19200",	B19200	},
#endif	/* B19200 */
#ifdef	B38400
	{ "B38400",	B38400	},
#endif	/* B38400 */
	{ "EXTA",	EXTA	},
	{ "EXTB",	EXTB	},
	{ "CS5",	CS5	},
	{ "CS6",	CS6	},
	{ "CS7",	CS7	},
	{ "CS8",	CS8	},
	{ "CSTOPB",	CSTOPB	},
	{ "CREAD",	CREAD	},
	{ "PARENB",	PARENB	},
	{ "PARODD",	PARODD	},
	{ "HUPCL",	HUPCL	},
	{ "CLOCAL",	CLOCAL	},
#ifdef	LOBLK
	{ "LOBLK",	LOBLK	},
#endif	/* LOBLK */
	{  NULLPTR,	0	},
};

SYMTAB	lmodes[] = {
	{ "ISIG",	ISIG	},
	{ "ICANON",	ICANON	},
	{ "XCASE",	XCASE	},
	{ "ECHO",	ECHO	},
	{ "ECHOE",	ECHOE	},
	{ "ECHOK",	ECHOK	},
	{ "ECHONL",	ECHONL	},
	{ "NOFLSH",	NOFLSH	},
#ifdef	XCLUDE
	{ "XCLUDE",	XCLUDE	},
#endif	/* XCLUDE */
	{  NULLPTR,	0	}
};

SYMTAB	ldiscs[] = {
	{ "LDISC0",	LDISC0	},
	{  NULLPTR,	0	}
};

/*
 *	Gettytab entry to use if no other can be determined
 */
GTAB	Default = {
	"default",
	{ 0, 0, ( SSPEED | CSANE ), 0, 0, CC_SANE, },
	{ ISANE, OSANE, ( SSPEED | CSANE ), LSANE, LDISC0, CC_SANE, },
	"login: ",
	"default"
};

#define	VALUE(cptr)	((cptr == (char *) NULL) ? "NULL" : cptr)

int	errors = 0;

int	nextentry(), parseGtab(), findsym();
char	*nextword();
void	addfield(), chkerr();


/*
**	gtabvalue() - find a gettytab entry that matches "id."
**
**	Returns (GTAB *)NULL if not found or an error occurs.
*/

GTAB *
gtabvalue(id, mode)
register char *id;
int mode;
{
	register int state;
	register char *p;
	register char *gettytab;	/* gettytab file to use */
	STDCHAR buf[MAXLINE+1];		/* buffer for Gtab entries */
	char buf_id[MAXID+1];		/* buffer to compare initial label */
	char *this = "First";		/* First or Next entry */
	static GTAB gtab;		/* structure to be returned */
	FILE *fp;

	debug3(D_GTAB, "gtabvalue(%s) called\n", VALUE(id));

	gettytab = (Check) ? CheckFile : GETTYTAB;
	debug3(D_GTAB, "gettytab=%s\n", gettytab);

	/* open the gettytab file
	 */
	if ((fp = fopen(gettytab, "r")) == (FILE *) NULL) {
		(void) sprintf(MsgBuf, "cannot open %s", gettytab);
		logerr(MsgBuf);
		return(&Default);
	}

	/* search through the file for "id", unless
	 * id is NULL, in which case we drop down
	 * to get the 'default' entry.
	 */
	state = (!Check && (id == (char *) NULL)) ? QUIT : ENTRY;

	while (state != QUIT && nextentry(buf, sizeof(buf), fp) == SUCCESS) {
		if (buf[0] == '#' || buf[0] == '\n')
			continue;	/* keep looking */
		if (Check) {
			(void) printf("*** %s Entry ***\n", this);
			(void) printf("%s\n", buf);
			this = "Next";
		}
		if (buf[strlen(buf)-1] != '\n') {
			/* last char not \n, line is too long */
			chkerr("line too long", FAIL);
			state = QUIT;
			continue;
		}
		/* get the first (label) field
		 */
		(void) strncpy(buf_id, buf, MAXID);
		if ((p = strtok(buf_id, "# \t")) != (char *) NULL)
			*(--p) = '\0';
		/* if Check is set, parse all entries;
		 * otherwise, parse only a matching entry
		 */
		if (Check || strequal(id, buf_id)) {
			if (parseGtab(&gtab, buf) == FAIL) {
				chkerr("*** Invalid Entry ***", FAIL);
				state = QUIT;
				continue;
			}
			if (!Check) {
				(void) fclose(fp);
				goto success;
			}
		}
	}

	if (Check) {
		if (errors)
			(void) printf("*** %d errors found ***\n", errors);
		(void) printf("*** Check Complete ***\n");
		(void) fclose(fp);
		return((GTAB *) NULL);
	}

	if (mode == G_FIND)
		return((GTAB *) NULL);

	if (id != (char *) NULL) {
		(void) sprintf(MsgBuf, "%s entry for \"%s\" not found",
				gettytab, id);
		logerr(MsgBuf);
	}

	/* matching entry not found or defective;
	 * use the first line of the file
	 */
	rewind(fp);
	(void) nextentry(buf, sizeof(buf), fp);
	(void) fclose(fp);
	if (parseGtab(&gtab, buf) == FAIL)
		return(&Default);	/* punt: first line defective */

    success:
	debug2(D_GTAB, "gtabvalue() successful\n");
	return(&gtab);
}


/*
**	nextentry() - retrieve next entry from gettytab file
**
**	Returns FAIL if an error occurs.
*/

int
nextentry(buf, len, stream)
register char *buf;
register int len;
FILE *stream;
{
	register int count = 0;
	STDCHAR line[MAXLINE+1];

	*buf = '\0';		/* erase buffer */

	while (fgets(line, sizeof(line), stream) != (char *) NULL) {
		debug2(D_GTAB, "line read = (");
		debug1(D_GTAB, line);
		debug2(D_GTAB, ")\n");
		if (count)
			buf[strlen(buf)-1] = '\0';
		if ((count += strlen(line)) >= len)
			return(FAIL);		/* entry too long */
		(void) strcat(buf, line);
		if ((line[0] == '\n') || (line[0] == '#'))
			return(SUCCESS);	/* blank line */
	}

	return (*buf == '\0' ? QUIT : SUCCESS);
}


/*
**	parseGtab() - fill in GTAB structure from buffer
**
**	Returns FAIL if an error occurs.
*/

int
parseGtab(gtab, line)
GTAB *gtab;
register char *line;
{
	register int field;
	register char *p;
	static int count;
	static char p_cur[MAXID+1], p_next[MAXID+1];
	static char p_login[MAXLOGIN+1];

	debug2(D_GTAB, "parseGtab() called\n");

	/* initialize gtab to empty
	 */
	gtab->cur_id = (char *) NULL;
	gtab->itermio.c_iflag = 0;
	gtab->itermio.c_oflag = 0;
	gtab->itermio.c_cflag = 0;
	gtab->itermio.c_lflag = 0;
	gtab->itermio.c_line  = 0;
	gtab->ftermio.c_iflag = 0;
	gtab->ftermio.c_oflag = 0;
	gtab->ftermio.c_cflag = 0;
	gtab->ftermio.c_lflag = 0;
	gtab->ftermio.c_line  = 0;
	gtab->login = (char *) NULL;
	gtab->next_id = (char *) NULL;

	if (LineD != (char *) NULL) {	/* line disc given on command line */
		addfield(&(gtab->itermio), LineD);
		addfield(&(gtab->ftermio), LineD);
	}

	/* parse the line
	 */
	debug2(D_GTAB, "parsing line:\n");
	field = 1;
	while (field != FAIL && field != SUCCESS) {
		if ((p = nextword(line, &count)) == (char *) NULL) {
			field = FAIL;
			continue;
		}
		debug4(D_GTAB, "field=%d, nextword=(%s)\n", field, p);
		switch (field) {
		case 1:
			/* cur_id label
			 */
			(void) strncpy(p_cur, p, MAXID);
			gtab->cur_id = p_cur;
			field++;
			break;
		case 2:
			/* '#' field separator
			 */
			if (*p != '#') {
				field = FAIL;
				continue;
			}
			field++;
			break;
		case 3:
			/* initial termio flags
			 */
			if (*p == '#')
				field++;
			else
				addfield(&(gtab->itermio), p);
			break;
		case 4:
			/* final termio flags
			 */
			if (*p == '#')
				field++;
			else
				addfield(&(gtab->ftermio), p);
			break;
		case 5:
			/* login message --
			 * nextword won't be the whole message; look
			 * ahead to the next '#' and terminate string there
			 */
			if ((p = index(line, '#')) == (char *) NULL) {
				field = FAIL;
				continue;
			}
			*p = '\0';
			p = line;		/* point p to line again */
			count = strlen(p)+1;	/* adjust count accordingly */
			debug3(D_GTAB, "login=(%s)\n", p);
			(void) strncpy(p_login, p, MAXLOGIN);
			gtab->login = p_login;
			field++;
			break;
		case 6:
			/* next_id label
			 */
			(void) strncpy(p_next, p, MAXID);
			gtab->next_id = p_next;
			field = SUCCESS;
			continue;
		}
		/* skip over word just processed
		 */
		line += count;
	}

	if (Check) {
		(void) printf("id: \"%s\"\n", gtab->cur_id);
		(void) printf("initial termio flags:\n");
		(void) printf(" iflag: %o, oflag: %o, cflag: %o, lflag: %o\n",
				gtab->itermio.c_iflag, gtab->itermio.c_oflag,
				gtab->itermio.c_cflag, gtab->itermio.c_lflag);
		(void) printf(" line disc: %o\n", gtab->itermio.c_line);
		(void) printf("final termio flags:\n");
		(void) printf(" iflag: %o, oflag: %o, cflag: %o, lflag: %o\n",
				gtab->ftermio.c_iflag, gtab->ftermio.c_oflag,
				gtab->ftermio.c_cflag, gtab->ftermio.c_lflag);
		(void) printf(" line disc: %o\n", gtab->ftermio.c_line);
		(void) printf("login prompt: \"%s\"\n", gtab->login);
		(void) printf("next id: \"%s\"\n\n", gtab->next_id);
	}

	return(field);
}


/*
**	nextword() - get next "word" from buffer
*/

char *
nextword(buf, count)
register char *buf;
register int *count;
{
	register int num = 0;
	register char *p;
	static char word[MAXLINE+1];

	while (*buf == ' ' || *buf == '\t' || *buf == '\\' ||
	       *buf == '\n') {	/* skip leading whitespace */
		buf++; num++;
	}
	p = word;
	if (*buf == '#') {	/* first char is '#' ? */
		*p++ = *buf;
		num++;
	} else {
		while (*buf != ' ' && *buf != '\t' && *buf != '\\' &&
		       *buf != '#' && *buf != '\n') {
			*p++ = *buf++;
			num++;
		}
	}
	*p = '\0';
	*count = num;
	return(word);
}


#define		TOGGLE(f,b)					\
	{if (inverted)  (f) &= ~(b); else (f) |= (b);}

/*
**	addfield() - add symbol to termio structure
*/

void
addfield(termio, field)
register TERMIO *termio;
register char *field;
{
	register int val;
	int inverted;

	if (strequal(field, "SANE")) {
		termio->c_iflag |= ISANE;
		termio->c_oflag |= OSANE;
		termio->c_cflag |= CSANE;
		termio->c_lflag |= LSANE;
	} else {
		if (*field == '-'){
			inverted = TRUE;
			field++;
			}
		else	inverted = FALSE;
		if ((val = findsym(field, imodes)) != FAIL)
/*			termio->c_iflag |= (ushort) val;	*/
			TOGGLE (termio->c_iflag, val)
		else if ((val = findsym(field, omodes)) != FAIL)
/*			termio->c_oflag |= (ushort) val;	*/
			TOGGLE (termio->c_oflag, val)
		else if ((val = findsym(field, cmodes)) != FAIL)
/*			termio->c_cflag |= (ushort) val;	*/
			TOGGLE (termio->c_cflag, val)
		else if ((val = findsym(field, lmodes)) != FAIL)
/*			termio->c_lflag |= (ushort) val;	*/
			TOGGLE (termio->c_lflag, val)
		else if ((val = findsym(field, ldiscs)) != FAIL)
/*			termio->c_line |= (ushort) val;		*/
			TOGGLE (termio->c_line, val)
		else if (Check) {
			(void) sprintf(MsgBuf, "undefined symbol: %s", field);
			chkerr(MsgBuf, OK);
		}
	}
}


/*
**	findsym() - look for field in SYMTAB list
*/

int
findsym(field, symtab)
register char *field;
register SYMTAB *symtab;
{
	for (; symtab->symbol != (char *) NULL; symtab++)
		if (strequal(symtab->symbol, field))
			return((int) symtab->value);

	return(FAIL);
}


/*
**	chkerr() - display error message from check routines
*/

void
chkerr(msg, status)
char *msg;
int status;
{
	(void) printf("*** parsing error: %s ***\n", msg);
	if (status)
		(void) printf("*** checking halted ***\n");
	else
		(void) printf("*** checking continued ***\n");

	errors++;
}


/* end of table.c */
