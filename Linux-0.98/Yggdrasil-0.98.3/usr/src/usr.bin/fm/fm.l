%{

/* 
 * (was file: fmt.c)
 * 04/88  bgray
 * file: fm.l
 * revised with (f)lex front end, 05/88  Greg Lee
 * revised for hyphenation, 05/92  Greg Lee
 *
 * This code is in the public domain.
 */

/* a simple text formatter */

#ifndef lint
static	char fm_sccsid[] = "@(#)fm.l <04/11/92>";
#endif lint


#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define MAXLINE 2048
#define MAXWORD 512

/* character attributes controlled by font-changing commands */
#define ROMAN  1	/* plain, from \rm or .R */
#define ITALIC 2	/* underlined, from \it or .I */
#define BOLD   3	/* bold, from \bf or .B */
#define TTYPE  4	/* reversed, from \tt or .S */
#define SLANTY 5	/* blinking, from \sl or .G */

int     attribute = ROMAN,	/* current character attribute */
        nextattribute = 0;	/* attribute to restore after next
				 * word for .RB, etc. or after line
				 * for \beginsection
				 */
char    nest[80];		/* stack of attributes for each current */
int	level = 1;		/* TeX {}-group */

/* global state of formatter */
char    iflag  = FALSE,		/* preserving left indentation (-i)? */
        jflag  = FALSE,		/* requested justification with -j? */
        oflag  = FALSE,		/* requested overprint with -o? */
        uflag  = FALSE,		/* requested terminal output with -u? */
        xflag  = FALSE,		/* requested use TeX commands w. -x? */
        mflag  = FALSE,		/* requested use man commands w. -m? */
        cflag  = FALSE,		/* requested format C code w. -c? */
        fmtflag  = FALSE,	/* called by name "fmt"? */
        crownflag  = FALSE,	/* requested crown margin mode w. -c? */
        sflag  = FALSE,		/* requested split only w. -s? */
        qtflag = FALSE,		/* saw odd " in processing .RB etc.? */
        ceflag = FALSE,		/* center next output line? */
        raflag = FALSE,		/* right adjust next output line? */
        obflag = FALSE,		/* not concatenating? */
        obsflag = FALSE,	/* preserve spacing? */
        njflag = FALSE,		/* not currently justifying? */
	startedflag = FALSE,	/* newline for file-initial \n? */
	usefmtindent = TRUE;	/* use indent of input line as indent? */

int     maxlen  = 72,		/* output line length */
        tabsize = 8,		/* separation of tab stops */
        indent  = 0,		/* white space at left, not counting offset */
        parindent  = 5,		/* paragraph indentation for tex mode */
        fmtindent  = 0,		/* input line indentation */
        fmtlastindent  = 0,	/* input line indentation of previous line */
        leftskip  = 0,		/* horizontal space at left, for tex mode */
        rightskip  = 0,		/* horizontal space at right, for tex mode */
        parskip  = 0,		/* vertical space between par., in points, for tex mode */
        vskip  = 0,		/* vertical space between par. yet to skip */
        cmargin0 = -2,		/* indent for first line of paragraph in crown mode */
        cmargin  = -2,		/* indent for other lines of paragraph in crown mode */
        offset  = 0,		/* white space at left before indentation */
        hlevel  = 1,		/* hyphenation level ( -h ) */
        tpstate = 0,		/* keep track for hanging indentation */
        tpvalue = 5,		/* amount of hang, from arg of .TP,.HP */
        spaces  = 0,		/* space to leave after current word */
        tempi;			/* misc. use in lex pattern section */

char   *progname = NULL;
char   *usage = "usage: %s [-width] [-ijcCxmousban] [-h n] [-p n] [-t n] [file ...]\n";

int     dir = FALSE;
int     holecnt = 0;
char   *holeptr[256];		/* holeptr[0] is unused */

char	oline[MAXLINE] = "\0",	/* output line buffer */
       *olp = oline,
        aline[MAXLINE] = "\0",	/* output line attribute buffer */
       *alp = aline,
        oword[MAXWORD] = "\0",	/* output word buffer */
       *owp = oword,
        aword[MAXWORD] = "\0",	/* output word attribute buffer */
       *awp = aword;

extern char *optarg;		/* from getopt */
extern int  optind;
extern int hyphenate ();

int     max ();
void    exit ();
char   *basename ();
void    copyindent ();
void    puttc ();
void    putts ();
void    lbreak ();
void    putword ();
void    justifyline ();
void    putline ();
void    sputline ();
void    skipline ();
void    addparens ();
void    terminit ();
void    termattr ();

    /* lex start state for flags: -x, -m, (none), -mx, respectively */
%}
%s	TEX MAN PLAIN ALL CCODE

wh	[ \t]*

%%

<MAN,ALL>^".sp"				skipline(1);

	/* match various TeX skip commands */
<TEX,ALL>\\[a-gi-z]+"skip"{wh}/[^a-z]	skipline(1);

    /* blank line in input gives blank line in output, except in tex mode */
\n{wh}$			if (xflag && !iflag && !obflag) lbreak();  else skipline(1);

<MAN,ALL>^"."("sp"|"ne")" "[0-9]+	skipline(atoi(yytext+4));

    /* traditional */
<PLAIN>^\..*\n {
			putword();   putline();
			(void)printf("%s",yytext);
}

    /* escaped special characters -- \\ for \ is not like real TeX */
<TEX,ALL>\\[$#{}&%^_~'"\\]		 puttc(yytext[1]);
<TEX,ALL>"--"("-")			puttc('-');

    /* some font-changing commands */

<TEX,ALL>"\\it"{wh}/[^a-z] |
<MAN,ALL>"\\f"(I|C|2)			attribute = ITALIC;

<TEX,ALL>"\\sl"{wh}/[^a-z] |
<MAN,ALL>"\\fG"     			attribute = SLANTY;

<TEX,ALL>"\\bf"{wh}/[^a-z] |
<MAN,ALL>"\\f"(B|3)     			attribute = BOLD;

<TEX,ALL>"\\rm"{wh}/[^a-z] |
<MAN,ALL>"\\f"(R|P|1)			attribute = ROMAN; /* \fP should not be like \fR */

<TEX,ALL>"\\tt"{wh}/[^a-z] |
<MAN,ALL>"\\fS"     			attribute = TTYPE;


<TEX,ALL>"\\TeX"{wh}/[^a-z]		putts("TeX");

    /* should these TeX commands all cause breaks? */
<TEX,ALL>\\"centerline"{wh}/[^a-z]	lbreak();  vskip = 0;  ceflag = level;
<TEX,ALL>\\"rightline"{wh}/[^a-z]	lbreak();  vskip = 0;  raflag = level;
<TEX,ALL>\\"raggedright"{wh}/[^a-z]	if (!njflag) njflag = level;
<TEX,ALL>\\"obeylines"{wh}/[^a-z]	lbreak();  vskip = 0;  if (!obflag) obflag = level;
<TEX,ALL>\\"obeyspaces"{wh}/[^a-z]	if (!obsflag) obsflag = level;  indent = 0;
<TEX,ALL>\\"parindent"{wh}(=)?{wh}("-")?[0-9]+"em"  {
		int i = 0;
		lbreak();
		while (!isdigit(yytext[i++])) ;
		if (yytext[i-2] == '-') i--;
		parindent = atoi(yytext+i-1);
		lbreak();
}
<TEX,ALL>\\"parskip"{wh}(=)?{wh}[0-9]+"pt"  {
		int i = 0;
		lbreak();
		while (!isdigit(yytext[i++])) ;
		parskip = atoi(yytext+i-1);
		lbreak();
}
<TEX,ALL>\\"hoffset"{wh}(=)?{wh}[0-9]+"em"  {
		int i = 0;
		lbreak();
		while (!isdigit(yytext[i++])) ;
		offset = atoi(yytext+i-1);
}
<TEX,ALL>\\"hsize"{wh}(=)?{wh}[0-9]+"em"  {
		int i = 0;
		lbreak();
		while (!isdigit(yytext[i++])) ;
		maxlen = atoi(yytext+i-1);
}
<TEX,ALL>\\"leftskip"{wh}(=)?{wh}[0-9]+"em"  {
		int i = 0;
		lbreak();
		while (!isdigit(yytext[i++])) ;
		leftskip = atoi(yytext+i-1);
}
<TEX,ALL>\\"rightskip"{wh}(=)?{wh}[0-9]+"em"  {
		int i = 0;
		lbreak();
		while (!isdigit(yytext[i++])) ;
		rightskip = atoi(yytext+i-1);
}
<TEX,ALL>\\"noindent"{wh}/[^a-z]	indent = 0;
<TEX,ALL>\\"par"{wh}/[^a-z]		lbreak();
<TEX,ALL>\\"break"{wh}/[^a-z]		putword();  putline();
<TEX,ALL>\\("bye"|"end"){wh}/[^a-z]	lbreak();  exit(0);
<TEX,ALL>"%".*\n 			;

<MAN,ALL>^".nf"				lbreak(); obflag = TRUE;
<MAN,ALL>^".fi"				lbreak(); obflag = FALSE;
<MAN,ALL>^".na"				njflag = TRUE;
<MAN,ALL>^".ad"(" b"|" n")?		njflag = FALSE;
<MAN,ALL>^".hy "[0-9]			hlevel = atoi(yytext+3);
<MAN,ALL>^".nh"				hlevel = 0;

<TEX,ALL>"{"				nest[level++] = attribute; /* push attr. onto stack */

<TEX,ALL>"}"  {		if (level < 2) {
				fprintf(stderr,"unmatched right brace\n");
				exit(1);
			}
			/* if emerging from {} in which \obeylines, \obeyspaces or
			 * \raggedright occurred, cancel  */
			if (obflag == level) obflag = FALSE;
			if (obsflag == level) obsflag = FALSE;
			if (njflag == level) njflag = FALSE;
			/* pop attribute off stack */
			attribute = nest[--level];
			/* } after \item signals end of tag; will not
			 * work right with {} inside tag */
			if (tpstate == 1)  {
				tpstate = 2;
				putword();
			}
			if (raflag == level || ceflag == level) {
				vskip = 0;
				lbreak();
				raflag = ceflag = FALSE;
			}
}


^[ \t]+ {
			startedflag = TRUE;
			putword();
			if (iflag || obflag) {
				if (xflag) lbreak();  else putline();
				tempi = attribute;
				/* for comments with indentation */
				if (cflag) attribute = ROMAN;
				if (obsflag) copyindent(yytext);
				attribute = tempi;
			}
			else if (mflag) putline();

			fmtindent = 0;
			for (; (isspace (*yytext)); yytext++) {
				fmtindent++;
				if (*yytext == '\t')
					for (; (fmtindent % tabsize); fmtindent++) ;
			}
			if (fmtindent > fmtlastindent) {
				if (fmtflag && !crownflag) putline();
				usefmtindent = TRUE;
			}
			if (iflag && xflag && !obsflag) indent = fmtindent;
			fmtlastindent = fmtindent;
}

<TEX,ALL>"\\ "		putword();

<MAN,ALL,CCODE>\n  {	/* for C code reverse all of multiline comments */
			if (!cflag) attribute = ROMAN;
			nextattribute = qtflag = FALSE;
			/* signal end of tag for hanging indent */
			if (tpstate == 1) tpstate = 2;
			putword();
			if (obflag) putline();
}

    /* more font-changing */

<MAN,ALL>^\.B[ \n]			attribute = BOLD;
<MAN,ALL>^\.(I|C|"Nm")[ \n] 		attribute = ITALIC;
<MAN,ALL>^\.R[ \n]			attribute = ROMAN;
<MAN,ALL>^\.("S"|"SM"|"SB")[ \n]	attribute = TTYPE;
<MAN,ALL>^\.G[ \n]			attribute = SLANTY;
<MAN,ALL>^".ft B"\n			attribute = BOLD;
<MAN,ALL>^".ft I"\n			attribute = ITALIC;
<MAN,ALL>^".ft"(" R")?\n		attribute = ROMAN;

<MAN,ALL>\\[\|\^&!{}\nacdeprtuz]		;

<MAN,ALL>^".if n "				;
<MAN,ALL>^".if t ".*\n				;
<MAN,ALL>^".de ".*\n".."			;
<MAN,ALL>^".de ".*\n.*\n".."			;
<MAN,ALL>^".de ".*\n.*\n.*\n".."		;
<MAN,ALL>^".de ".*\n.*\n.*\n.*\n".."		;

<MAN,ALL>\\.				puttc(yytext[1]);

<MAN,ALL>^".ti +"[0-9]+  {
	 		lbreak();
			for (tempi = 0; tempi < atoi(yytext+5) - 1; tempi++)
				puttc(' ');
}

<MAN,ALL>^\.br 				lbreak();

<MAN,ALL>^\.(P|L|"")P 			skipline(1); indent = 5;

    /* hanging indent stuff (awfully complicated) */

<MAN,ALL>^\.(T|H)"P "[0-9]+\n  {
			skipline(1);
			if (yytext[1] == 'H')
				tpstate = 4;
			else	tpstate = 1;
			tpvalue = atoi(yytext+4);
			indent = 5;
}

<TEX,ALL>\\"item"("item")?{wh}/[^a-z]  |
<MAN,ALL>^".HP ".*\n	|
<MAN,ALL>^".TP".*\n	|
<MAN,ALL>^".IP"[ \t]*\n?  {
			if (xflag) lbreak();  else skipline(1);
			if (yytext[1] == 'I' && yytext[3] == '\n')
				tpstate = 4;
			else	tpstate = 1;
			tpvalue = 5;
			indent = 5;
}

    /* various begin-end commands to set off blocks of text */

<MAN,ALL>^"."(R|D)(S|E)  {
			if (yytext[1] == 'D' && yytext[2] == 'S') {
				skipline(1);
				obflag = obsflag = TRUE;
			}
			else	lbreak();
			indent = 10;
			if (yytext[2] == 'E') {
				indent = 5;
				obflag = obsflag = FALSE;
			}
}

<MAN,ALL>^".E"(X|E)  {
			skipline(1);
			if (yytext[2] == 'E') {
				indent = 5;
				obflag = obsflag = FALSE;
			}
			else {	indent = 10;
				obflag = obsflag = TRUE;
			}
}


<MAN,ALL>^".N"(T|E).*  {
			skipline(1);
			if (yytext[2] == 'E') {
				indent = 5;
				maxlen += 5;
			}
			else {	indent = 10;
				maxlen -= 5;
				tempi = attribute;
				attribute = ITALIC;
				ceflag = TRUE;
				if (yyleng < 5) putts("NOTE");
				else putts(yytext+4);
				skipline(1);
			}
}

    /* those curious -man commands to join words, alternating fonts */

<MAN,ALL>^\.("IR"|"PN")" "	nextattribute = ROMAN; attribute = ITALIC;
<MAN,ALL>^\."BR "		nextattribute = ROMAN; attribute = BOLD;
<MAN,ALL>^\."RI "		nextattribute = ITALIC; attribute = ROMAN;
<MAN,ALL>^\."BI "		nextattribute = ITALIC; attribute = BOLD;
<MAN,ALL>^\."RB "		nextattribute = BOLD; attribute = ROMAN;
<MAN,ALL>^\."IB "		nextattribute = BOLD; attribute = ITALIC;

    /* section headings */

<TEX,ALL>\\"beginsection"{wh}  {
			skipline(1);
			nextattribute = attribute;
			attribute = BOLD;
}

<MAN,ALL>^\."S"(H|S)(" "|\n).* {
			skipline(1);
			obflag = FALSE;
			if (yytext[2] != 'S') indent = 0;
			attribute = BOLD;
			putts(yytext+4);
			lbreak();
			indent = 5;
			attribute = ROMAN;
}

<MAN,ALL>^\."TH ".* {
			lbreak();
			indent = 0;
			addparens(yytext+4, BOLD);
			raflag = TRUE;
			skipline(2);
			indent = 5;
}

    /* "Manual Section" reference */
<MAN,ALL>^".MS ".*	addparens(yytext+4, ITALIC);

<MAN,ALL>^".CT ".  {
			if (uflag || oflag) {
				tempi = attribute;
				attribute = TTYPE;
				puttc(toupper(yytext[4]));
				attribute = tempi;
			}
			else {
				putts("<CTRL/");
				puttc(yytext[4]);
				puttc('>');
			}
}

<MAN,ALL>^".V"(S|E).* |
<MAN,ALL>^".UC"(..)?  |
<MAN,ALL>^".IX".*     |
<MAN,ALL>^".fn "      |
<MAN,ALL>^\.([ ]+)?\\\".*	;

    /* for join-words commands, words can be quoted */
<MAN,ALL>\"  {
			if (nextattribute || tpstate == 1)  {
				qtflag = !qtflag;
			}
			else puttc('"');
}

<MAN,ALL,CCODE>" "  {
			if (nextattribute)  {
				if (qtflag) puttc(' ');
				else  {
					tempi = attribute;
					attribute = nextattribute;
					nextattribute = tempi;
				}
			}
			else {
				putword();
				if (obsflag) copyindent(yytext);
			}
}

<TEX,ALL,CCODE>\n  {
			putword();
			if (obflag || nextattribute) putline();
			if (nextattribute) {
				attribute = nextattribute;
				nextattribute = 0;
			}
}

<TEX,ALL>"~"			puttc(' ');

[ \t]+  {
			putword();
			if (obsflag) copyindent(yytext);
}

[\n\r\f]  {
			putword();
			if (sflag) {
				putline();
				usefmtindent = TRUE;
			}
			else if (!obflag && obsflag && olp > oline && *(olp-1) != ' ')
					copyindent(" ");
			if (cmargin == -3) cmargin = -2;
			else if (cmargin == -2) {
				cmargin = -1;
				if (cmargin0 == -2) cmargin0 = fmtindent;
			}
			else if (cmargin == -1) cmargin = fmtindent;
			fmtindent = 0;
}

<TEX,ALL>\\"backslash"{wh}/[^a-z]	puttc('\\');

<TEX>\\    |
<TEX,ALL>\$|\#				;
<TEX,ALL>\\[A-Za-z]+{wh}/[^a-z]		;

<MAN,ALL>\\"*(rq"			puttc('\'');
<MAN,ALL>\\"*(lq"			puttc('`');

<MAN,ALL>\\"("..  {
			tempi = attribute;
			attribute = TTYPE;
			if (yytext[2] != '*') puttc(yytext[2]);
			puttc(yytext[3]);
			attribute = tempi;
}

<MAN,ALL>\\"s"("-"|"+")?[0-9] |
<MAN,ALL>"\\*S"		|
<MAN,ALL>^".DT"		|
<MAN,ALL>^".ta ".*	|
<MAN,ALL>^".PD".*	|
<MAN,ALL>^".NX".*			;

<MAN,ALL>^"."..{wh}			;

<CCODE>\"	{	if (attribute != TTYPE) qtflag = !qtflag;
			puttc('"');
}

<CCODE>"/*"	{	puttc('/');
			if (!qtflag) attribute = TTYPE;
			puttc('*');
}

<CCODE>"*/"	{	puttc('*');
			if (!qtflag) attribute = ROMAN;
			puttc('/');
}

    /* control flow keywords are bold */
<CCODE>"continue"|"default"|"do"|"goto"|"return"  |
<CCODE>"if"|"else"|"while"|"break"|"switch"|"case"|"for"  {
			if (attribute != TTYPE && !qtflag) {
				attribute = BOLD;
				putts(yytext);
				attribute = ROMAN;
			} else putts(yytext);
}

    /* storage class keywords are underlined */
<CCODE>("auto"|"char"|"double"|"enum"|"extern"|"float")  |
<CCODE>("int"|"long"|"register"|"short"|"sizeof"|"static") |
<CCODE>("struct"|"typedef"|"union"|"unsigned"|"void") {

			if (attribute != TTYPE && !qtflag) {
				attribute = ITALIC;
				putts(yytext);
				attribute = ROMAN;
			} else putts(yytext);
}

    /* preprocessor keywords are blinking */
<CCODE>"#"[ \t]*("define"|"undef"|"include"|"if"|"ifdef")  |
<CCODE>"#"[ \t]*("ifndef"|"else"|"endif"|"line")  {

			if (attribute != TTYPE && !qtflag) {
				attribute = SLANTY;
				putts(yytext);
				attribute = ROMAN;
			} else putts(yytext);
}

    /* identifiers that happen to contain keywords are plain */
<CCODE>[a-zA-Z_]+	putts(yytext);

.			puttc(yytext[0]);



%%

/*
 * Characters, along with the currently active "font"= screen
 * attribute, are collected in a word buffer until we hit a
 * word break (white space), then moved to a line buffer.
 * Words are collected in the line buffer until we hit a line
 * break (end of paragraph) or the line is long enough, then
 * the line is output.
 */

main (argc, argv)
int     argc;
char   *argv[];
{
	int     c, i, hoffset = 4;

	progname = basename (argv[0]);
	if (!strcmp(progname,"nroff")) {
		mflag = TRUE;
		oflag = TRUE;
		jflag = TRUE;
	}
	else if (!strcmp(progname,"tex")) {
		xflag = TRUE;
		jflag = TRUE;
	}
	else if (!strcmp(progname,"fmt")) {
		hlevel = 0;
		obsflag = TRUE;
		fmtflag = TRUE;
	}
	while ((c = getopt (argc, argv, "ijcCxmousbanh:p:t:0:1:2:3:4:5:6:7:8:9:")) != EOF)
		switch (c) {
			case 'i': iflag = TRUE; parindent = 0; break;
			case 'j': jflag = TRUE; break;
			case 'c': crownflag = TRUE; break;
			case 'C': cflag = TRUE; break;
			case 'x': xflag = TRUE; break;
			case 'm': mflag = TRUE; break;
			case 'o': oflag = TRUE; break;
			case 'u': uflag = TRUE; break;
			case 's': sflag = TRUE; break;
			case 'b': fmtflag = TRUE; break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8':
			case '9': { char numarg[80];
				    numarg[0] = c;
				    for (i = 0; optarg[i] >= '0' && optarg[i] <= '9'; i++)
					numarg[i+1] = optarg[i];
				    numarg[i+1] = 0;
				    maxlen = max (0, atoi (numarg));
				    /* partial correction for processing single
				     * digit width */
				    if (i == 0) {
					if (optarg[i] == '-' || optind == 0) {
					   (void) fprintf (stderr, "please use leading 0 for width < 10\n");
					   exit (1);
					}
					optind--;
					argv[optind] = optarg;
				    }
				  }
				  break;
			case 'h': hlevel = max (0, atoi (optarg)); break;
			case 'p': hoffset = offset = max (0, atoi (optarg)); break;
			case 't': tabsize = max (1, atoi (optarg)); break;
			case 'a':
			case 'n': break;
			default: (void) fprintf (stderr, usage, progname);
				  exit (1);
				  break;
		}
		/* UCB fmt counts \n as part of line width */
		if (fmtflag && maxlen > 0) maxlen--;


/* set lex start state */
	if (xflag && mflag) {
		indent = parindent;
		offset = hoffset;
		BEGIN (ALL);
	}
	else if (xflag) {
		indent = parindent;
		offset = hoffset;
		BEGIN (TEX);
	}
	else if (mflag) {
		indent = 5;
		BEGIN (MAN);
	}
	else if (cflag) {
		obflag = TRUE;
		obsflag = TRUE;
		if (!oflag) uflag = TRUE;
		BEGIN (CCODE);
	}
	else {
		BEGIN (PLAIN);
	}

/* consult termcap for terminal controls */
	if (uflag) terminit ();

/* call lex scanner */
	if (optind >= argc) {
		(void) yylex ();
		lbreak ();
	} else for (; (optind < argc); optind++) {
		char *file = argv[optind];
		if (yyin == NULL) yyin = stdin;
		if (freopen (file, "r", yyin) == NULL) {
			char *tfname = (char *)malloc(strlen(file)+5);
			strcpy (tfname, file);
			strcat (tfname, ".tex");
			if (!xflag || (freopen (tfname, "r", yyin) == NULL)) {
				(void) fprintf (stderr,
			    	"Couldn't open file: %s\n", argv[optind]);
				exit (1);
			}
		}
		yy_init = 1;
		(void) yylex ();
		lbreak ();
	}

	return (0);
}				/* main */


/* preserve spacing due to spaces or tabs */
void	copyindent (ilp)
char	*ilp;
{
	int     col;

	col = (olp - oline) + 1;

	for (; (isspace (*ilp)); col++) {
		if (*ilp++ == '\t')
			for (; (col % tabsize); col++) {
				*olp++ = ' ';
				*alp++ = (cflag) ? attribute : ROMAN;
			}
		*olp++ = ' ';
		*alp++ = (cflag) ? attribute : ROMAN;
	}
}				/* copyindent */

/* one character goes into word buffer */
void puttc (ch)
char    ch;
{
	if (owp - oword >= MAXWORD) return;
	*owp++ = ch;
	*awp++ = attribute;
}				/* puttc */

/* put string in word buffer (might have spaces) */
void putts (s)
char   *s;
{
	while (*s)
		puttc (*s++);
}				/* putts */

/* finish pending word and line and put it out (no justification) */
void lbreak () {
	startedflag = TRUE;
	putword ();
	putline ();
	tpstate = 0;
	if (xflag && !obsflag) indent = parindent;
	fmtindent = fmtlastindent = 0;
	usefmtindent = TRUE;
	cmargin0 = cmargin = -2;
	vskip = parskip;
}				/* lbreak */

/* append pending word to line buffer */
void putword () {
	int     plen, tlen, hyph_len;
	char   *p;
	char   *q;

	/* just return if no characters have been accumulated yet */
	if (owp == oword) {
		/* this is for the special case in which a file
		 * starts with \n, which should count as  a "blank line"
		 */
		if (!startedflag) {
			putchar ('\n');
			startedflag = TRUE;
			cmargin = -3;
		}
		return;
	}
	startedflag = TRUE;

	puttc ('\0');
	p = oword;
	q = aword;
	plen = strlen (p);

	/* ugly way to ignore "0.3i" arg of .IP */
	if (tpstate == 2 && *p == '0' && p[plen - 1] == 'i') {
		spaces = plen = 0;
		*p = '\0';
	}

	/* to match UCB fmt program in crown margin mode, use indent
	 * before first input line of paragraph, then indent before
	 * second input line for rest of paragraph */
	if (crownflag) {
		if (cmargin0 >= 0) indent = cmargin0;
		else if (cmargin >= 0) indent = cmargin;
		else indent = fmtindent;
	}
	else if (iflag && !xflag) {
		if (cmargin0 >= 0) indent = cmargin0;
		else if (cmargin < 0) indent = fmtindent;
	}

	/* finish pending line if pending word won't fit */
	tlen = maxlen - leftskip - indent - rightskip; /* width for text */
	hyph_len = plen;
		/* ... unless width available for text is very small,
		 * in which case the line will just have to hang out past
		 * the right margin -- sorry about that */
	if (!obflag && (tlen > 1)) {
		/* how much room will be left for this word? */
		int future_room = tlen - ((olp - oline) + spaces);

		/* if short of room, try hyphenating */
		if (hlevel && (future_room < plen))
			hyph_len = hyphenate (oword, future_room, hlevel);

		/* if word is too long for line, make arbitrary break */
		if (hlevel < 2 && hyph_len > tlen) hyph_len = future_room;

		/* when doesn't fit, start a new line (if there's any hope) */
		if (future_room < hyph_len && hyph_len <= tlen) {
			hyph_len = plen;
			if ((jflag) && (holecnt) && !njflag)
				justifyline (tlen);
			putline ();
			/* signal input indent eligible to set
			 * indent when imitating UCB fmt */
			usefmtindent = TRUE;
			/* signal time to use hanging indent in
			 * crown margin mode */
			cmargin0 = -1;
		}
	}

	/* to match UCB fmt program, use indent before earliest word
	 * of current line */
	if (usefmtindent) {
		if (fmtflag && !crownflag) indent = fmtindent;
		usefmtindent = FALSE;
	}

	/* append spaces which were previously decided to go after
	 * last word put in line buffer */
	if (spaces) {
		/* don't use space inside tag of hanging indent to
		 * right justify */
		if (!tpstate || tpstate > 3)
			holeptr[++holecnt] = olp;
		/* if tag already padded out, permit justification
		 * of space after next word */
		if (tpstate == 3)
			tpstate = 4;
		for (; (spaces > 0); spaces--) {
			*olp++ = ' ';
			/* a matter of taste -- put just " = ROMAN"
			 * for no underlined spaces between words */
			*alp++ = (*(alp-1) == ITALIC && *q == ITALIC) ?
				ITALIC : ROMAN;
		}
	}

	/* append first part of hyphenated word to line */
	if (hyph_len < plen) {
		int j;
		for (j = 1; j < hyph_len; j++) {
			*olp++ = *p++;
			*alp++ = *q++;
			plen--;
		}
		/* for hyphenation break after '-', include it */
		if (hlevel && *p == '-') {
			*olp++ = *p++;
			*alp++ = *q++;
			plen--;
		}
		/* add hyphen */
		if (hlevel && *(olp-1) != '-' && *(olp-1) != ' ') {
			*olp++ = '-';
			*alp++ = *(q - 1);
		}
		/* line is now complete, so print and start a new one */
		if ((jflag) && (holecnt) && !njflag)
			justifyline (tlen);
		putline ();
		/* when remnant of word is too long for next line, move it back
                 * to beginning of word buffer and recurse */
		if (plen > olp - oline) {
			owp = oword;
			awp = aword;
			while (*p) {
				*owp++ = *p++;
				*awp++ = *q++;
			}
			putword ();
			return;
		}
	}

	/* figure spaces which should go after pending word which
	 * is just about to be appended in line buffer -- remember
	 * so they can be put before next word (cf. just above) */
	if (!obsflag) spaces = 1 + endofsentence (p, plen);

	/* append pending word in line buffer */
	while (*p) {
		*olp++ = *p++;
		*alp++ = *q++;
	}

	/* part of tortuous way of handling hanging indent */
		/* tpstate = 0: no hanging indent
		 *	     1: scanning tag to go in left margin
		 *	     2: have seen end of tag, time to
		 *		pad out tag to hanging indent value
		 *	     3: finished padding
		 *	     4: finished first line of hanging par.;
		 *		subsequent lines should be indented
		 *		to hanging indent value
		 */
	if (tpstate == 2) {	/* time to pad tag? */
		tpstate = 3;	/* signal padding done */
		if ((olp - oline) + spaces > tpvalue) /* will tag fit? */
			putline ();	/* no it won't, so start new line */
		else		/* yes it will, so pad it out */
			while ((olp - oline) + spaces < tpvalue)
				spaces++;
	}

	/* purge word buffer -- get ready for next word */
	owp = oword;
	awp = aword;
}				/* putword */


/* increase inter-word spacings for right justification */
void justifyline (width)
int width;
{
	int     n;
	char   *fp,
	       *tp,
	       *fa,
	       *ta;

	dir = (!(dir));
	fp = olp - 1;
	fa = alp - 1;
	olp = &oline[width];
	alp = &aline[width];
	tp = olp - 1;
	ta = alp - 1;
	while (tp > fp) {
		while (fp >= holeptr[holecnt]) {
			*tp-- = *fp--;
			*ta-- = *fa--;
		}
		if (dir)
			n = ((tp - fp) - 1) / holecnt + 1;
		else
			n = (tp - fp) / holecnt;
		while (n--) {
			*tp-- = ' ';
			*ta-- = ta[1];
		}
		holecnt--;
	}
}				/* justifyline */


/* put out pending line in line buffer */
void putline ()
{	int width = maxlen - leftskip - rightskip;

	/* when line is empty, do nothing */
	if (olp == oline) return;
	if (obsflag) {
		char *p;
		for (p = oline; p < olp && (*p == ' '); p++) ;
		if (p == olp) {
			olp = oline;
			alp = aline;
			holecnt = 0;
			spaces = 0;
			return;
		}
	}

	if (xflag && !obflag) while (vskip >= 6) {
		putchar ('\n');
		vskip -= 12;
	}
	if (ceflag) {
		spaces = max ((width - (olp - oline)) / 2, 1) + offset + leftskip;
		if (!xflag) ceflag = FALSE;
	}
	else if (raflag) {
		spaces = max ((width - (olp - oline)), 1) + offset + leftskip;
		if (!xflag) raflag = FALSE;
	}
	else	spaces = offset + leftskip + indent;
	if (spaces < 0) spaces = 0;

	if (tabsize == 8) while (spaces >= 8) {
		putchar('\t');
		spaces -= 8;
	}
	while (spaces > 0) {
		putchar(' ');
		spaces--;
	}

	*olp = '\0';
	if (*oline) {
		/* if -u or -x options, have to do it character
		 * by character */
		if (oflag || uflag)
			sputline ();
		else (void) printf ("%s\n", oline);
	}

	/* purge line buffer -- get ready for new line */
	olp = oline;
	alp = aline;
	holecnt = vskip = 0;

	if (xflag) indent = 0;
	/* if finished with first line of hanging par., increase
	 * left indent for subsequent lines */
	if (tpstate > 2) {
		indent = tpvalue + 5;
		tpstate = 4;
	}
}				/* putline */


/* display characters of line with overstriking or with special
 * terminal attributes */
void sputline () {
	char   *p;
	char   *q;
	char    last = ROMAN;


	for (p = oline, q = aline; *p; p++, q++)
		if (uflag) {
			last = newattr (last, *q);
			(void) printf ("%c", *p);
		} else
		switch (*q) {
			case ROMAN: 
				(void) printf ("%c", *p);
				break;
			case BOLD: 
			case TTYPE: 
				if (*p == ' ')
					printf("%c", *p);
				else	(void) printf ("%c%c%c", *p, 8, *p);
				break;
			case ITALIC: 
			case SLANTY: 
				(void) printf ("_%c%c", 8, *p);
				break;
		}
	if (uflag)
		last = newattr (last, ROMAN);
	putchar('\n');

}				/* sputline */

/* put blank lines */
void skipline (n)
int     n;
{
	lbreak ();
	for (; (n > 0); n--) putchar('\n');
	vskip -= 12*n;
	cmargin = -3;
}				/* skipline */


/* decide how many spaces go after word */
int     endofsentence (p, plen)
char   *p;
int     plen;
{
	if (plen < 3)
		return (FALSE);
	if (*(p + plen - 1) == '"') plen--;
	if (!strchr (".:?!", *(p + plen - 1)))
		return (FALSE);
	if (abbr (p))
		return (FALSE);
	return (TRUE);
}				/* endofsentence */


/* detect "." which is not real end of sentence */
int     abbr (s)
char   *s;
{
	char   *p,
	       *q,
	       *r;

	while (*s == '(')
		s++;
	q = ".i.e.g.dr.mr.mrs.st.";
	for (; (*q); q++) {
		p = q;
		r = s;
		while ((*r) && (*p++ == (*r++ | 0x20)));
		if (!*r)
			return (TRUE);
	}
	return (FALSE);
}				/* abbr */



char   *basename (s)
char   *s;
{
	char   *p;

	if (p = strrchr (s, '/'))
		return (++p);
	else
		return (s);
}				/* basename */



int     max (a, b)
int     a,
        b;
{
	return ((a > b) ? a : b);
}				/* max */


/* put parentheses around number of manual section */
void addparens (s, a)
char   *s;
int     a;
{
	nextattribute = attribute;
	attribute = a;
	while (*s && *s != ' ')
		puttc (*s++);
	if (*s == ' ')
		s++, puttc ('(');
	while (*s && *s != ' ')
		puttc (*s++);
	if (*s == ' ')
		s++;
	puttc (')');
	attribute = nextattribute;
	nextattribute = 0;
	if (*s && isalpha (*s))
		puttc (' ');
	while (*s)
		puttc (*s++);
}				/* addparens */


/* terminal commands -- accessing routines */

/*
 *  following derived from:
 *  tcap v1.0 Enhanced terminal control program
 *  Author: Eric Lorenzo Lim
 *  Modified by: Becca Thomas and Rik Farrow
 *  (appeared in Nov. '87 Unix World)
 */

#define SO 0
#define SE 1
#define MB 2
#define MD 3
#define MR 4
#define MH 5
#define ME 6
#define US 7
#define UE 8
char   *id[] = {
	"so", "se", "mb", "md", "mr", "mh", "me", "us", "ue",
	(char *) NULL,
};

#ifdef ANSI
void terminit () {}

/* put out a terminal handing command */
void termattr (i)
int     i;
{
	switch (i) {
		case SO: printf("%c[31m",27); break; /* red */
		case SE: printf("%c[0m",27); break;
		case MB: printf("%c[36m",27); break; /* cyan */
		case MD: printf("%c[1m",27); break;  /* bright */
		case MR: printf("%c[7m",27); break;  /* reverse */
		case MH: printf("%c[35m",27); break; /* magenta */
		case ME: printf("%c[0m",27); break;
		case US: printf("%c[32m",27); break; /* green */
		case UE: printf("%c[0m",27); break;
	}
}				/* termattr */

#else

char   *area[50];
char    bp[1024],
        strbuf[1024];

/* get for later use terminal handling commands we need */
void terminit () {
	int     id_size = 0;
	char   *str,
	       *tmpptr;
	extern char    *getenv (), *tgetstr (), *tgoto ();

	if ((tmpptr = getenv ("TERM")) == (char *) NULL) {
		(void) fprintf (stderr, "fm: TERM variable not set\n");
		exit (1);
	}

	str = strbuf;

	if (tgetent (bp, tmpptr))
		while (id[id_size] != (char *) NULL) {
			area[id_size] = tgetstr (id[id_size], &str);
			if (area[id_size] == (char *) NULL)
				area[id_size] = "";
			id_size++;
		}


}				/* terminit */

/* thanks to ken@cs.rochester.edu for the following,
 * which fixes the output statement in termattr() below
 */
void outch (c)
int	c;
{
	putchar(c);
}

char	PC = '\0';

/* put out a terminal handing command */
/* thanks to Dave Yearke, Sigma Systems Technology, Inc.
 *   for the SYSVR3 change below
 */
void termattr (i)
int     i;
{
#ifdef SYSVR3
	(void) putp (area[i]);
#else /* SYSVR3 */
	(void) tputs (area[i], 1, outch);
#endif /* SYSVR3 */
}				/* termattr */

#endif

/* check for change of "font" and send command to terminal
 * when there is a change */
int     newattr (l, a)
int     l,	/* last attribute */
        a;	/* new attribute */
{
	if (l != a) {
		if (l == BOLD || l == TTYPE || l == SLANTY)
			termattr (ME);
		else if (l == ITALIC)
			termattr (UE);

		switch (a) {
			case   BOLD:	termattr (MD); break;
			case ITALIC:	termattr (US); break;
			case  TTYPE:	termattr (MR); break;
			case SLANTY:	termattr (MB); break;
		}
	}
	return (a);
}				/* newattr */
