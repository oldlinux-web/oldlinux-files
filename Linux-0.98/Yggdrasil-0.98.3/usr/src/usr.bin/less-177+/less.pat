*** command.c.orig	Wed Apr  3 12:35:44 1991
--- command.c	Wed Mar 11 23:01:12 1992
***************
*** 39,45 ****
  #if SHELL_ESCAPE
  static char *shellcmd = NULL;	/* For holding last shell command for "!!" */
  #endif
! static int mca;			/* The multicharacter command (action) */
  static int search_type;		/* The previous type of search */
  static int number;		/* The number typed by the user */
  static char optchar;
--- 39,45 ----
  #if SHELL_ESCAPE
  static char *shellcmd = NULL;	/* For holding last shell command for "!!" */
  #endif
! int mca;			/* The multicharacter command (action) */
  static int search_type;		/* The previous type of search */
  static int number;		/* The number typed by the user */
  static char optchar;
*** line.c.orig	Wed Apr  3 12:35:48 1991
--- line.c	Sat Mar 14 15:07:32 1992
***************
*** 4,10 ****
--- 4,13 ----
   * in preparation for output to the screen.
   */
  
+ #include <stdio.h>
+ #include <ctype.h>
  #include "less.h"
+ #include "regexp.h"
  
  static char linebuf[1024];	/* Buffer which holds the current output line */
  static char attr[1024];		/* Extension of linebuf to hold attributes */
***************
*** 14,19 ****
--- 17,23 ----
  static int overstrike;		/* Next char should overstrike previous char */
  static int is_null_line;	/* There is no current line */
  static char pendc;
+ int Compress_null_lines = 1;	/* null lines will not take screen asset */
  
  static int do_append();
  
***************
*** 28,33 ****
--- 32,39 ----
  extern int ul_s_width, ul_e_width;
  extern int bl_s_width, bl_e_width;
  extern int sc_width, sc_height;
+ extern int Last_pattern_is_caseless;
+ extern int caseless;
  
  /*
   * Rewind the line buffer.
***************
*** 363,368 ****
--- 369,376 ----
  pdone(endline)
  	int endline;
  {
+ 	extern regexp *Last_regexp;
+ 
  	if (pendc && (pendc != '\r' || !endline))
  		/*
  		 * If we had a pending character, put it in the buffer.
***************
*** 372,377 ****
--- 380,416 ----
  		(void) do_append(pendc);
  
  	/*
+ 	 * Modify the attribute for matched strings, do this before we
+ 	 * add a newline so that '$' will match end of line properly.
+ 	 */
+ 	if (Last_regexp != NULL) {
+ 		int i, len;
+ 		char *start, *p, *q, c;
+ 		char tmpbuf[1024];
+ 
+ 		start = p = linebuf;
+ 		linebuf[curr] = '\0';
+ 		if (caseless && Last_pattern_is_caseless) {
+ 			start = q = tmpbuf;
+ 			do	{
+ 				c = *p++;
+ 				*q++ = isupper(c) ? tolower(c) : c;
+ 			} while (c != '\0');
+ 			p = tmpbuf;	/*
+ 					 * use tmpbuf for regexp match
+ 					 * so that caseless match will
+ 					 * work properly
+ 					 */
+ 		}
+ 		while (regexec (Last_regexp, p)) {
+ 			i = Last_regexp->startp[0] - start;
+ 			len = Last_regexp->endp[0] - Last_regexp->startp[0];
+ 			memset (&attr[i], BLINK, len);
+ 			p = Last_regexp->endp[0];
+ 		}
+ 	}
+ 
+ 	/*
  	 * Add a newline if necessary,
  	 * and append a '\0' to the end of the line.
  	 */
***************
*** 397,402 ****
--- 436,442 ----
  {
  	if (is_null_line)
  	{
+ 		if (Compress_null_lines) return (0);
  		/*
  		 * If there is no current line, we pretend the line is
  		 * either "~" or "", depending on the "twiddle" flag.
*** position.c.orig	Thu Mar  7 00:02:16 1991
--- position.c	Wed Mar 11 09:28:17 1992
***************
*** 144,149 ****
--- 144,159 ----
  	return (1);
  }
  
+ 	public int
+ lines_on_screen()
+ {
+ 	register int i;
+ 
+ 	for (i = 0;  i < sc_height;  i++)
+ 		if (table[i] == NULL_POSITION) break;
+ 	return (i);
+ }
+ 
  /*
   * Get the current screen position.
   * The screen position consists of both a file position and
*** screen.c.orig	Wed Apr  3 12:35:48 1991
--- screen.c	Wed Mar 11 09:28:18 1992
***************
*** 51,56 ****
--- 51,57 ----
  	*sc_move,		/* General cursor positioning */
  	*sc_clear,		/* Clear screen */
  	*sc_eol_clear,		/* Clear to end of line */
+ 	*sc_eos_clear,		/* Clear to end of screen */
  	*sc_s_in,		/* Enter standout (highlighted) mode */
  	*sc_s_out,		/* Exit standout mode */
  	*sc_u_in,		/* Enter underline mode */
***************
*** 332,337 ****
--- 333,345 ----
  		sc_eol_clear = "";
  	}
  
+ 	sc_eos_clear = tgetstr("cd", &sp);
+ 	if (hard || sc_eos_clear == NULL || *sc_eos_clear == '\0')
+ 	{
+ 		cannot("clear to end of screen");
+ 		sc_eos_clear = "";
+ 	}
+ 
  	sc_clear = tgetstr("cl", &sp);
  	if (hard || sc_clear == NULL || *sc_clear == '\0')
  	{
***************
*** 555,560 ****
--- 563,569 ----
  	tputs(sc_addline, sc_height, putchr);
  }
  
+ #define	max(a,b)	(((a) > (b)) ? (a) : (b))
  /*
   * Move cursor to lower left corner of screen.
   */
***************
*** 561,567 ****
  	public void
  lower_left()
  {
! 	tputs(sc_lower_left, 1, putchr);
  }
  
  /*
--- 570,586 ----
  	public void
  lower_left()
  {
! 	unsigned line;
! 	char *p = sc_lower_left;
! 	extern int Compress_null_lines;
! 	extern void clear_eos();
! 
! 	if (Compress_null_lines && (line = lines_on_screen()) != sc_height) {
! 		p = tgoto (sc_move, 0, max (line-1, 0));
! 	}
! 	
! 	tputs(p, 1, putchr);
! 	if (Compress_null_lines) clear_eos ();
  }
  
  /*
***************
*** 604,609 ****
--- 623,638 ----
  clear_eol()
  {
  	tputs(sc_eol_clear, 1, putchr);
+ }
+ 
+ /*
+  * Clear from the cursor to the end of the screen.
+  * {{ This must not move the cursor. }}
+  */
+ 	public void
+ clear_eos()
+ {
+ 	tputs(sc_eos_clear, 1, putchr);
  }
  
  /*
*** search.c.orig	Wed Apr  3 12:35:48 1991
--- search.c	Sat Mar 14 14:59:12 1992
***************
*** 4,12 ****
  
  #include "less.h"
  #include "position.h"
! #if REGCOMP
  #include "regexp.h"
- #endif
  
  extern int sigs;
  extern int how_search;
--- 4,11 ----
  
  #include "less.h"
  #include "position.h"
! #include "cmd.h"
  #include "regexp.h"
  
  extern int sigs;
  extern int how_search;
***************
*** 13,19 ****
--- 12,22 ----
  extern int caseless;
  extern int linenums;
  extern int jump_sline;
+ extern int mca;
  
+ regexp *Last_regexp = NULL;		/* last search pattern */
+ int Last_pattern_is_caseless;
+ 
  /*
   * Search for the n-th occurrence of a specified pattern, 
   * either forward or backward.
***************
*** 37,59 ****
  	char *line;
  	int linenum;
  	int line_match;
! 	static int is_caseless;
! #if RECOMP
! 	char *re_comp();
! 	PARG parg;
! #else
! #if REGCMP
! 	char *regcmp();
! 	static char *cpattern = NULL;
! #else
! #if REGCOMP
! 	static struct regexp *regpattern = NULL;
! #else
  	static char lpbuf[100];
- 	static char *last_pattern = NULL;
- #endif
- #endif
- #endif
  
  	/*
  	 * Extract flags and type of search.
--- 40,47 ----
  	char *line;
  	int linenum;
  	int line_match;
! 	static regexp *regexp = NULL;
  	static char lpbuf[100];
  
  	/*
  	 * Extract flags and type of search.
***************
*** 61,67 ****
  	goforw = (SRCH_DIR(search_type) == SRCH_FORW);
  	want_match = !(search_type & SRCH_NOMATCH);
  
! 	if (pattern != NULL && *pattern != '\0' && (is_caseless = caseless))
  	{
  		/*
  		 * Search will ignore case, unless
--- 49,56 ----
  	goforw = (SRCH_DIR(search_type) == SRCH_FORW);
  	want_match = !(search_type & SRCH_NOMATCH);
  
! 	if (pattern != NULL && *pattern != '\0' &&
! 	    (Last_pattern_is_caseless = caseless))
  	{
  		/*
  		 * Search will ignore case, unless
***************
*** 70,167 ****
  		for (p = pattern;  *p != '\0';  p++)
  			if (*p >= 'A' && *p <= 'Z')
  			{
! 				is_caseless = 0;
  				break;
  			}
  	}
- #if RECOMP
  
! 	/*
! 	 * (re_comp handles a null pattern internally, 
! 	 *  so there is no need to check for a null pattern here.)
! 	 */
! 	if ((parg.p_string = re_comp(pattern)) != NULL)
! 	{
! 		error("%s", &parg);
! 		return (-1);
! 	}
! #else
! #if REGCMP
! 	if (pattern == NULL || *pattern == '\0')
! 	{
  		/*
  		 * A null pattern means use the previous pattern.
- 		 * The compiled previous pattern is in cpattern, so just use it.
  		 */
! 		if (cpattern == NULL)
! 		{
  			error("No previous regular expression", NULL_PARG);
  			return (-1);
  		}
- 	} else
- 	{
- 		/*
- 		 * Otherwise compile the given pattern.
- 		 */
- 		char *s;
- 		if ((s = regcmp(pattern, 0)) == NULL)
- 		{
- 			error("Invalid pattern", NULL_PARG);
- 			return (-1);
- 		}
- 		if (cpattern != NULL)
- 			free(cpattern);
- 		cpattern = s;
  	}
! #else
! #if REGCOMP
! 	if (pattern == NULL || *pattern == '\0')
! 	{
! 		/*
! 		 * A null pattern means use the previous pattern.
! 		 * The compiled previous pattern is in regpattern, 
! 		 * so just use it.
! 		 */
! 		if (regpattern == NULL)
! 		{
! 			error("No previous regular expression", NULL_PARG);
  			return (-1);
  		}
! 	} else
! 	{
! 		/*
! 		 * Otherwise compile the given pattern.
! 		 */
! 		struct regexp *s;
! 		if ((s = regcomp(pattern)) == NULL)
! 		{
! 			error("Invalid pattern", NULL_PARG);
! 			return (-1);
! 		}
! 		if (regpattern != NULL)
! 			free(regpattern);
! 		regpattern = s;
  	}
! #else
! 	if (pattern == NULL || *pattern == '\0')
  	{
- 		/*
- 		 * Null pattern means use the previous pattern.
- 		 */
- 		if (last_pattern == NULL)
- 		{
- 			error("No previous regular expression", NULL_PARG);
- 			return (-1);
- 		}
- 		pattern = last_pattern;
- 	} else
- 	{
  		strcpy(lpbuf, pattern);
- 		last_pattern = lpbuf;
  	}
- #endif
- #endif
- #endif
  
  	/*
  	 * Figure out where to start the search.
--- 59,95 ----
  		for (p = pattern;  *p != '\0';  p++)
  			if (*p >= 'A' && *p <= 'Z')
  			{
! 				Last_pattern_is_caseless = 0;
  				break;
  			}
  	}
  
! 	if (pattern == NULL || *pattern == '\0') {
  		/*
  		 * A null pattern means use the previous pattern.
  		 */
! 		if (regexp == NULL) {
  			error("No previous regular expression", NULL_PARG);
  			return (-1);
  		}
  	}
! 	else	{
! 		if (regexp) free (regexp);
! 		if ((regexp = regcomp (pattern)) == NULL) {
! 			error("bad regular expression", NULL_PARG);
  			return (-1);
  		}
! 		Last_regexp = regexp;
  	}
! 
! 	/*
! 	 * Save a copy so that we can use it later to highlight the pattern
! 	 * on the screen.
! 	 */
! 	if (pattern != NULL && *pattern != '\0')
  	{
  		strcpy(lpbuf, pattern);
  	}
  
  	/*
  	 * Figure out where to start the search.
***************
*** 268,274 ****
  			oldpos = pos;
  		}
  
! 		if (is_caseless)
  		{
  			/*
  			 * If this is a caseless search, convert 
--- 196,202 ----
  			oldpos = pos;
  		}
  
! 		if (Last_pattern_is_caseless)
  		{
  			/*
  			 * If this is a caseless search, convert 
***************
*** 294,315 ****
  
  		/*
  		 * Test the next line to see if we have a match.
- 		 * This is done in a variety of ways, depending
- 		 * on what pattern matching functions are available.
  		 */
! #if REGCMP
! 		line_match = (regex(cpattern, line) != NULL);
! #else
! #if RECOMP
! 		line_match = (re_exec(line) == 1);
! #else
! #if REGCOMP
! 		line_match = regexec(regpattern, line);
! #else
! 		line_match = match(pattern, line);
! #endif
! #endif
! #endif
  		/*
  		 * We are successful if want_match and line_match are
  		 * both true (want a match and got it),
--- 222,230 ----
  
  		/*
  		 * Test the next line to see if we have a match.
  		 */
! 		line_match = regexec (regexp, line);
! 
  		/*
  		 * We are successful if want_match and line_match are
  		 * both true (want a match and got it),
***************
*** 323,352 ****
  			break;
  	}
  
  	jump_loc(linepos, jump_sline);
  	return (0);
  }
- 
- #if (!REGCMP) && (!RECOMP) && (!REGCOMP)
- /*
-  * We have neither regcmp() nor re_comp().
-  * We use this function to do simple pattern matching.
-  * It supports no metacharacters like *, etc.
-  */
- 	static int
- match(pattern, buf)
- 	char *pattern, *buf;
- {
- 	register char *pp, *lp;
- 
- 	for ( ;  *buf != '\0';  buf++)
- 	{
- 		for (pp = pattern, lp = buf;  *pp == *lp;  pp++, lp++)
- 			if (*pp == '\0' || *lp == '\0')
- 				break;
- 		if (*pp == '\0')
- 			return (1);
- 	}
- 	return (0);
- }
- #endif
--- 238,250 ----
  			break;
  	}
  
+ 	if (mca == A_F_SEARCH || mca == A_B_SEARCH) {
+ 		/*
+ 		 * force refresh for new search. This will make the found
+ 		 * string displayed with proper attibutes.
+ 		 */
+ 		pos_clear();
+ 	}
  	jump_loc(linepos, jump_sline);
  	return (0);
  }
*** /dev/null	Sun Mar 15 00:32:54 1992
--- regexp.h	Sat Aug 13 17:13:12 1988
***************
*** 0 ****
--- 1,21 ----
+ /*
+  * Definitions etc. for regexp(3) routines.
+  *
+  * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
+  * not the System V one.
+  */
+ #define NSUBEXP  10
+ typedef struct regexp {
+ 	char *startp[NSUBEXP];
+ 	char *endp[NSUBEXP];
+ 	char regstart;		/* Internal use only. */
+ 	char reganch;		/* Internal use only. */
+ 	char *regmust;		/* Internal use only. */
+ 	int regmlen;		/* Internal use only. */
+ 	char program[1];	/* Unwarranted chumminess with compiler. */
+ } regexp;
+ 
+ extern regexp *regcomp();
+ extern int regexec();
+ extern void regsub();
+ extern void regerror();
*** /dev/null	Sun Mar 15 00:32:54 1992
--- regexp.c	Sat Aug 13 17:13:18 1988
***************
*** 0 ****
--- 1,1214 ----
+ /*
+  * regcomp and regexec -- regsub and regerror are elsewhere
+  * @(#)regexp.c	1.3 of 18 April 87
+  *
+  *	Copyright (c) 1986 by University of Toronto.
+  *	Written by Henry Spencer.  Not derived from licensed software.
+  *
+  *	Permission is granted to anyone to use this software for any
+  *	purpose on any computer system, and to redistribute it freely,
+  *	subject to the following restrictions:
+  *
+  *	1. The author is not responsible for the consequences of use of
+  *		this software, no matter how awful, even if they arise
+  *		from defects in it.
+  *
+  *	2. The origin of this software must not be misrepresented, either
+  *		by explicit claim or by omission.
+  *
+  *	3. Altered versions must be plainly marked as such, and must not
+  *		be misrepresented as being the original software.
+  *
+  * Beware that some of this code is subtly aware of the way operator
+  * precedence is structured in regular expressions.  Serious changes in
+  * regular-expression syntax might require a total rethink.
+  */
+ #include <stdio.h>
+ #include <regexp.h>
+ #include "regmagic.h"
+ 
+ /*
+  * The "internal use only" fields in regexp.h are present to pass info from
+  * compile to execute that permits the execute phase to run lots faster on
+  * simple cases.  They are:
+  *
+  * regstart	char that must begin a match; '\0' if none obvious
+  * reganch	is the match anchored (at beginning-of-line only)?
+  * regmust	string (pointer into program) that match must include, or NULL
+  * regmlen	length of regmust string
+  *
+  * Regstart and reganch permit very fast decisions on suitable starting points
+  * for a match, cutting down the work a lot.  Regmust permits fast rejection
+  * of lines that cannot possibly match.  The regmust tests are costly enough
+  * that regcomp() supplies a regmust only if the r.e. contains something
+  * potentially expensive (at present, the only such thing detected is * or +
+  * at the start of the r.e., which can involve a lot of backup).  Regmlen is
+  * supplied because the test in regexec() needs it and regcomp() is computing
+  * it anyway.
+  */
+ 
+ /*
+  * Structure for regexp "program".  This is essentially a linear encoding
+  * of a nondeterministic finite-state machine (aka syntax charts or
+  * "railroad normal form" in parsing technology).  Each node is an opcode
+  * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
+  * all nodes except BRANCH implement concatenation; a "next" pointer with
+  * a BRANCH on both ends of it is connecting two alternatives.  (Here we
+  * have one of the subtle syntax dependencies:  an individual BRANCH (as
+  * opposed to a collection of them) is never concatenated with anything
+  * because of operator precedence.)  The operand of some types of node is
+  * a literal string; for others, it is a node leading into a sub-FSM.  In
+  * particular, the operand of a BRANCH node is the first node of the branch.
+  * (NB this is *not* a tree structure:  the tail of the branch connects
+  * to the thing following the set of BRANCHes.)  The opcodes are:
+  */
+ 
+ /* definition	number	opnd?	meaning */
+ #define	END	0	/* no	End of program. */
+ #define	BOL	1	/* no	Match "" at beginning of line. */
+ #define	EOL	2	/* no	Match "" at end of line. */
+ #define	ANY	3	/* no	Match any one character. */
+ #define	ANYOF	4	/* str	Match any character in this string. */
+ #define	ANYBUT	5	/* str	Match any character not in this string. */
+ #define	BRANCH	6	/* node	Match this alternative, or the next... */
+ #define	BACK	7	/* no	Match "", "next" ptr points backward. */
+ #define	EXACTLY	8	/* str	Match this string. */
+ #define	NOTHING	9	/* no	Match empty string. */
+ #define	STAR	10	/* node	Match this (simple) thing 0 or more times. */
+ #define	PLUS	11	/* node	Match this (simple) thing 1 or more times. */
+ #define	OPEN	20	/* no	Mark this point in input as start of #n. */
+ 			/*	OPEN+1 is number 1, etc. */
+ #define	CLOSE	30	/* no	Analogous to OPEN. */
+ 
+ /*
+  * Opcode notes:
+  *
+  * BRANCH	The set of branches constituting a single choice are hooked
+  *		together with their "next" pointers, since precedence prevents
+  *		anything being concatenated to any individual branch.  The
+  *		"next" pointer of the last BRANCH in a choice points to the
+  *		thing following the whole choice.  This is also where the
+  *		final "next" pointer of each individual branch points; each
+  *		branch starts with the operand node of a BRANCH node.
+  *
+  * BACK		Normal "next" pointers all implicitly point forward; BACK
+  *		exists to make loop structures possible.
+  *
+  * STAR,PLUS	'?', and complex '*' and '+', are implemented as circular
+  *		BRANCH structures using BACK.  Simple cases (one character
+  *		per match) are implemented with STAR and PLUS for speed
+  *		and to minimize recursive plunges.
+  *
+  * OPEN,CLOSE	...are numbered at compile time.
+  */
+ 
+ /*
+  * A node is one char of opcode followed by two chars of "next" pointer.
+  * "Next" pointers are stored as two 8-bit pieces, high order first.  The
+  * value is a positive offset from the opcode of the node containing it.
+  * An operand, if any, simply follows the node.  (Note that much of the
+  * code generation knows about this implicit relationship.)
+  *
+  * Using two bytes for the "next" pointer is vast overkill for most things,
+  * but allows patterns to get big without disasters.
+  */
+ #define	OP(p)	(*(p))
+ #define	NEXT(p)	(((*((p)+1)&0377)<<8) + (*((p)+2)&0377))
+ #define	OPERAND(p)	((p) + 3)
+ 
+ /*
+  * See regmagic.h for one further detail of program structure.
+  */
+ 
+ 
+ /*
+  * Utility definitions.
+  */
+ #ifndef CHARBITS
+ #define	UCHARAT(p)	((int)*(unsigned char *)(p))
+ #else
+ #define	UCHARAT(p)	((int)*(p)&CHARBITS)
+ #endif
+ 
+ #define	FAIL(m)	{ regerror(m); return(NULL); }
+ #define	ISMULT(c)	((c) == '*' || (c) == '+' || (c) == '?')
+ #define	META	"^$.[()|?+*\\"
+ 
+ /*
+  * Flags to be passed up and down.
+  */
+ #define	HASWIDTH	01	/* Known never to match null string. */
+ #define	SIMPLE		02	/* Simple enough to be STAR/PLUS operand. */
+ #define	SPSTART		04	/* Starts with * or +. */
+ #define	WORST		0	/* Worst case. */
+ 
+ /*
+  * Global work variables for regcomp().
+  */
+ static char *regparse;		/* Input-scan pointer. */
+ static int regnpar;		/* () count. */
+ static char regdummy;
+ static char *regcode;		/* Code-emit pointer; &regdummy = don't. */
+ static long regsize;		/* Code size. */
+ 
+ /*
+  * Forward declarations for regcomp()'s friends.
+  */
+ #ifndef STATIC
+ #define	STATIC	static
+ #endif
+ STATIC char *reg();
+ STATIC char *regbranch();
+ STATIC char *regpiece();
+ STATIC char *regatom();
+ STATIC char *regnode();
+ STATIC char *regnext();
+ STATIC void regc();
+ STATIC void reginsert();
+ STATIC void regtail();
+ STATIC void regoptail();
+ #ifdef STRCSPN
+ STATIC int strcspn();
+ #endif
+ 
+ /*
+  - regcomp - compile a regular expression into internal code
+  *
+  * We can't allocate space until we know how big the compiled form will be,
+  * but we can't compile it (and thus know how big it is) until we've got a
+  * place to put the code.  So we cheat:  we compile it twice, once with code
+  * generation turned off and size counting turned on, and once "for real".
+  * This also means that we don't allocate space until we are sure that the
+  * thing really will compile successfully, and we never have to move the
+  * code and thus invalidate pointers into it.  (Note that it has to be in
+  * one piece because free() must be able to free it all.)
+  *
+  * Beware that the optimization-preparation code in here knows about some
+  * of the structure of the compiled regexp.
+  */
+ regexp *
+ regcomp(exp)
+ char *exp;
+ {
+ 	register regexp *r;
+ 	register char *scan;
+ 	register char *longest;
+ 	register int len;
+ 	int flags;
+ 	extern char *malloc();
+ 
+ 	if (exp == NULL)
+ 		FAIL("NULL argument");
+ 
+ 	/* First pass: determine size, legality. */
+ 	regparse = exp;
+ 	regnpar = 1;
+ 	regsize = 0L;
+ 	regcode = &regdummy;
+ 	regc(MAGIC);
+ 	if (reg(0, &flags) == NULL)
+ 		return(NULL);
+ 
+ 	/* Small enough for pointer-storage convention? */
+ 	if (regsize >= 32767L)		/* Probably could be 65535L. */
+ 		FAIL("regexp too big");
+ 
+ 	/* Allocate space. */
+ 	r = (regexp *)malloc(sizeof(regexp) + (unsigned)regsize);
+ 	if (r == NULL)
+ 		FAIL("out of space");
+ 
+ 	/* Second pass: emit code. */
+ 	regparse = exp;
+ 	regnpar = 1;
+ 	regcode = r->program;
+ 	regc(MAGIC);
+ 	if (reg(0, &flags) == NULL)
+ 		return(NULL);
+ 
+ 	/* Dig out information for optimizations. */
+ 	r->regstart = '\0';	/* Worst-case defaults. */
+ 	r->reganch = 0;
+ 	r->regmust = NULL;
+ 	r->regmlen = 0;
+ 	scan = r->program+1;			/* First BRANCH. */
+ 	if (OP(regnext(scan)) == END) {		/* Only one top-level choice. */
+ 		scan = OPERAND(scan);
+ 
+ 		/* Starting-point info. */
+ 		if (OP(scan) == EXACTLY)
+ 			r->regstart = *OPERAND(scan);
+ 		else if (OP(scan) == BOL)
+ 			r->reganch++;
+ 
+ 		/*
+ 		 * If there's something expensive in the r.e., find the
+ 		 * longest literal string that must appear and make it the
+ 		 * regmust.  Resolve ties in favor of later strings, since
+ 		 * the regstart check works with the beginning of the r.e.
+ 		 * and avoiding duplication strengthens checking.  Not a
+ 		 * strong reason, but sufficient in the absence of others.
+ 		 */
+ 		if (flags&SPSTART) {
+ 			longest = NULL;
+ 			len = 0;
+ 			for (; scan != NULL; scan = regnext(scan))
+ 				if (OP(scan) == EXACTLY && strlen(OPERAND(scan)) >= len) {
+ 					longest = OPERAND(scan);
+ 					len = strlen(OPERAND(scan));
+ 				}
+ 			r->regmust = longest;
+ 			r->regmlen = len;
+ 		}
+ 	}
+ 
+ 	return(r);
+ }
+ 
+ /*
+  - reg - regular expression, i.e. main body or parenthesized thing
+  *
+  * Caller must absorb opening parenthesis.
+  *
+  * Combining parenthesis handling with the base level of regular expression
+  * is a trifle forced, but the need to tie the tails of the branches to what
+  * follows makes it hard to avoid.
+  */
+ static char *
+ reg(paren, flagp)
+ int paren;			/* Parenthesized? */
+ int *flagp;
+ {
+ 	register char *ret;
+ 	register char *br;
+ 	register char *ender;
+ 	register int parno;
+ 	int flags;
+ 
+ 	*flagp = HASWIDTH;	/* Tentatively. */
+ 
+ 	/* Make an OPEN node, if parenthesized. */
+ 	if (paren) {
+ 		if (regnpar >= NSUBEXP)
+ 			FAIL("too many ()");
+ 		parno = regnpar;
+ 		regnpar++;
+ 		ret = regnode(OPEN+parno);
+ 	} else
+ 		ret = NULL;
+ 
+ 	/* Pick up the branches, linking them together. */
+ 	br = regbranch(&flags);
+ 	if (br == NULL)
+ 		return(NULL);
+ 	if (ret != NULL)
+ 		regtail(ret, br);	/* OPEN -> first. */
+ 	else
+ 		ret = br;
+ 	if (!(flags&HASWIDTH))
+ 		*flagp &= ~HASWIDTH;
+ 	*flagp |= flags&SPSTART;
+ 	while (*regparse == '|') {
+ 		regparse++;
+ 		br = regbranch(&flags);
+ 		if (br == NULL)
+ 			return(NULL);
+ 		regtail(ret, br);	/* BRANCH -> BRANCH. */
+ 		if (!(flags&HASWIDTH))
+ 			*flagp &= ~HASWIDTH;
+ 		*flagp |= flags&SPSTART;
+ 	}
+ 
+ 	/* Make a closing node, and hook it on the end. */
+ 	ender = regnode((paren) ? CLOSE+parno : END);	
+ 	regtail(ret, ender);
+ 
+ 	/* Hook the tails of the branches to the closing node. */
+ 	for (br = ret; br != NULL; br = regnext(br))
+ 		regoptail(br, ender);
+ 
+ 	/* Check for proper termination. */
+ 	if (paren && *regparse++ != ')') {
+ 		FAIL("unmatched ()");
+ 	} else if (!paren && *regparse != '\0') {
+ 		if (*regparse == ')') {
+ 			FAIL("unmatched ()");
+ 		} else
+ 			FAIL("junk on end");	/* "Can't happen". */
+ 		/* NOTREACHED */
+ 	}
+ 
+ 	return(ret);
+ }
+ 
+ /*
+  - regbranch - one alternative of an | operator
+  *
+  * Implements the concatenation operator.
+  */
+ static char *
+ regbranch(flagp)
+ int *flagp;
+ {
+ 	register char *ret;
+ 	register char *chain;
+ 	register char *latest;
+ 	int flags;
+ 
+ 	*flagp = WORST;		/* Tentatively. */
+ 
+ 	ret = regnode(BRANCH);
+ 	chain = NULL;
+ 	while (*regparse != '\0' && *regparse != '|' && *regparse != ')') {
+ 		latest = regpiece(&flags);
+ 		if (latest == NULL)
+ 			return(NULL);
+ 		*flagp |= flags&HASWIDTH;
+ 		if (chain == NULL)	/* First piece. */
+ 			*flagp |= flags&SPSTART;
+ 		else
+ 			regtail(chain, latest);
+ 		chain = latest;
+ 	}
+ 	if (chain == NULL)	/* Loop ran zero times. */
+ 		(void) regnode(NOTHING);
+ 
+ 	return(ret);
+ }
+ 
+ /*
+  - regpiece - something followed by possible [*+?]
+  *
+  * Note that the branching code sequences used for ? and the general cases
+  * of * and + are somewhat optimized:  they use the same NOTHING node as
+  * both the endmarker for their branch list and the body of the last branch.
+  * It might seem that this node could be dispensed with entirely, but the
+  * endmarker role is not redundant.
+  */
+ static char *
+ regpiece(flagp)
+ int *flagp;
+ {
+ 	register char *ret;
+ 	register char op;
+ 	register char *next;
+ 	int flags;
+ 
+ 	ret = regatom(&flags);
+ 	if (ret == NULL)
+ 		return(NULL);
+ 
+ 	op = *regparse;
+ 	if (!ISMULT(op)) {
+ 		*flagp = flags;
+ 		return(ret);
+ 	}
+ 
+ 	if (!(flags&HASWIDTH) && op != '?')
+ 		FAIL("*+ operand could be empty");
+ 	*flagp = (op != '+') ? (WORST|SPSTART) : (WORST|HASWIDTH);
+ 
+ 	if (op == '*' && (flags&SIMPLE))
+ 		reginsert(STAR, ret);
+ 	else if (op == '*') {
+ 		/* Emit x* as (x&|), where & means "self". */
+ 		reginsert(BRANCH, ret);			/* Either x */
+ 		regoptail(ret, regnode(BACK));		/* and loop */
+ 		regoptail(ret, ret);			/* back */
+ 		regtail(ret, regnode(BRANCH));		/* or */
+ 		regtail(ret, regnode(NOTHING));		/* null. */
+ 	} else if (op == '+' && (flags&SIMPLE))
+ 		reginsert(PLUS, ret);
+ 	else if (op == '+') {
+ 		/* Emit x+ as x(&|), where & means "self". */
+ 		next = regnode(BRANCH);			/* Either */
+ 		regtail(ret, next);
+ 		regtail(regnode(BACK), ret);		/* loop back */
+ 		regtail(next, regnode(BRANCH));		/* or */
+ 		regtail(ret, regnode(NOTHING));		/* null. */
+ 	} else if (op == '?') {
+ 		/* Emit x? as (x|) */
+ 		reginsert(BRANCH, ret);			/* Either x */
+ 		regtail(ret, regnode(BRANCH));		/* or */
+ 		next = regnode(NOTHING);		/* null. */
+ 		regtail(ret, next);
+ 		regoptail(ret, next);
+ 	}
+ 	regparse++;
+ 	if (ISMULT(*regparse))
+ 		FAIL("nested *?+");
+ 
+ 	return(ret);
+ }
+ 
+ /*
+  - regatom - the lowest level
+  *
+  * Optimization:  gobbles an entire sequence of ordinary characters so that
+  * it can turn them into a single node, which is smaller to store and
+  * faster to run.  Backslashed characters are exceptions, each becoming a
+  * separate node; the code is simpler that way and it's not worth fixing.
+  */
+ static char *
+ regatom(flagp)
+ int *flagp;
+ {
+ 	register char *ret;
+ 	int flags;
+ 
+ 	*flagp = WORST;		/* Tentatively. */
+ 
+ 	switch (*regparse++) {
+ 	case '^':
+ 		ret = regnode(BOL);
+ 		break;
+ 	case '$':
+ 		ret = regnode(EOL);
+ 		break;
+ 	case '.':
+ 		ret = regnode(ANY);
+ 		*flagp |= HASWIDTH|SIMPLE;
+ 		break;
+ 	case '[': {
+ 			register int class;
+ 			register int classend;
+ 
+ 			if (*regparse == '^') {	/* Complement of range. */
+ 				ret = regnode(ANYBUT);
+ 				regparse++;
+ 			} else
+ 				ret = regnode(ANYOF);
+ 			if (*regparse == ']' || *regparse == '-')
+ 				regc(*regparse++);
+ 			while (*regparse != '\0' && *regparse != ']') {
+ 				if (*regparse == '-') {
+ 					regparse++;
+ 					if (*regparse == ']' || *regparse == '\0')
+ 						regc('-');
+ 					else {
+ 						class = UCHARAT(regparse-2)+1;
+ 						classend = UCHARAT(regparse);
+ 						if (class > classend+1)
+ 							FAIL("invalid [] range");
+ 						for (; class <= classend; class++)
+ 							regc(class);
+ 						regparse++;
+ 					}
+ 				} else
+ 					regc(*regparse++);
+ 			}
+ 			regc('\0');
+ 			if (*regparse != ']')
+ 				FAIL("unmatched []");
+ 			regparse++;
+ 			*flagp |= HASWIDTH|SIMPLE;
+ 		}
+ 		break;
+ 	case '(':
+ 		ret = reg(1, &flags);
+ 		if (ret == NULL)
+ 			return(NULL);
+ 		*flagp |= flags&(HASWIDTH|SPSTART);
+ 		break;
+ 	case '\0':
+ 	case '|':
+ 	case ')':
+ 		FAIL("internal urp");	/* Supposed to be caught earlier. */
+ 		break;
+ 	case '?':
+ 	case '+':
+ 	case '*':
+ 		FAIL("?+* follows nothing");
+ 		break;
+ 	case '\\':
+ 		if (*regparse == '\0')
+ 			FAIL("trailing \\");
+ 		ret = regnode(EXACTLY);
+ 		regc(*regparse++);
+ 		regc('\0');
+ 		*flagp |= HASWIDTH|SIMPLE;
+ 		break;
+ 	default: {
+ 			register int len;
+ 			register char ender;
+ 
+ 			regparse--;
+ 			len = strcspn(regparse, META);
+ 			if (len <= 0)
+ 				FAIL("internal disaster");
+ 			ender = *(regparse+len);
+ 			if (len > 1 && ISMULT(ender))
+ 				len--;		/* Back off clear of ?+* operand. */
+ 			*flagp |= HASWIDTH;
+ 			if (len == 1)
+ 				*flagp |= SIMPLE;
+ 			ret = regnode(EXACTLY);
+ 			while (len > 0) {
+ 				regc(*regparse++);
+ 				len--;
+ 			}
+ 			regc('\0');
+ 		}
+ 		break;
+ 	}
+ 
+ 	return(ret);
+ }
+ 
+ /*
+  - regnode - emit a node
+  */
+ static char *			/* Location. */
+ regnode(op)
+ char op;
+ {
+ 	register char *ret;
+ 	register char *ptr;
+ 
+ 	ret = regcode;
+ 	if (ret == &regdummy) {
+ 		regsize += 3;
+ 		return(ret);
+ 	}
+ 
+ 	ptr = ret;
+ 	*ptr++ = op;
+ 	*ptr++ = '\0';		/* Null "next" pointer. */
+ 	*ptr++ = '\0';
+ 	regcode = ptr;
+ 
+ 	return(ret);
+ }
+ 
+ /*
+  - regc - emit (if appropriate) a byte of code
+  */
+ static void
+ regc(b)
+ char b;
+ {
+ 	if (regcode != &regdummy)
+ 		*regcode++ = b;
+ 	else
+ 		regsize++;
+ }
+ 
+ /*
+  - reginsert - insert an operator in front of already-emitted operand
+  *
+  * Means relocating the operand.
+  */
+ static void
+ reginsert(op, opnd)
+ char op;
+ char *opnd;
+ {
+ 	register char *src;
+ 	register char *dst;
+ 	register char *place;
+ 
+ 	if (regcode == &regdummy) {
+ 		regsize += 3;
+ 		return;
+ 	}
+ 
+ 	src = regcode;
+ 	regcode += 3;
+ 	dst = regcode;
+ 	while (src > opnd)
+ 		*--dst = *--src;
+ 
+ 	place = opnd;		/* Op node, where operand used to be. */
+ 	*place++ = op;
+ 	*place++ = '\0';
+ 	*place++ = '\0';
+ }
+ 
+ /*
+  - regtail - set the next-pointer at the end of a node chain
+  */
+ static void
+ regtail(p, val)
+ char *p;
+ char *val;
+ {
+ 	register char *scan;
+ 	register char *temp;
+ 	register int offset;
+ 
+ 	if (p == &regdummy)
+ 		return;
+ 
+ 	/* Find last node. */
+ 	scan = p;
+ 	for (;;) {
+ 		temp = regnext(scan);
+ 		if (temp == NULL)
+ 			break;
+ 		scan = temp;
+ 	}
+ 
+ 	if (OP(scan) == BACK)
+ 		offset = scan - val;
+ 	else
+ 		offset = val - scan;
+ 	*(scan+1) = (offset>>8)&0377;
+ 	*(scan+2) = offset&0377;
+ }
+ 
+ /*
+  - regoptail - regtail on operand of first argument; nop if operandless
+  */
+ static void
+ regoptail(p, val)
+ char *p;
+ char *val;
+ {
+ 	/* "Operandless" and "op != BRANCH" are synonymous in practice. */
+ 	if (p == NULL || p == &regdummy || OP(p) != BRANCH)
+ 		return;
+ 	regtail(OPERAND(p), val);
+ }
+ 
+ /*
+  * regexec and friends
+  */
+ 
+ /*
+  * Global work variables for regexec().
+  */
+ static char *reginput;		/* String-input pointer. */
+ static char *regbol;		/* Beginning of input, for ^ check. */
+ static char **regstartp;	/* Pointer to startp array. */
+ static char **regendp;		/* Ditto for endp. */
+ 
+ /*
+  * Forwards.
+  */
+ STATIC int regtry();
+ STATIC int regmatch();
+ STATIC int regrepeat();
+ 
+ #ifdef DEBUG
+ int regnarrate = 0;
+ void regdump();
+ STATIC char *regprop();
+ #endif
+ 
+ /*
+  - regexec - match a regexp against a string
+  */
+ int
+ regexec(prog, string)
+ register regexp *prog;
+ register char *string;
+ {
+ 	register char *s;
+ 	extern char *strchr();
+ 
+ 	/* Be paranoid... */
+ 	if (prog == NULL || string == NULL) {
+ 		regerror("NULL parameter");
+ 		return(0);
+ 	}
+ 
+ 	/* Check validity of program. */
+ 	if (UCHARAT(prog->program) != MAGIC) {
+ 		regerror("corrupted program");
+ 		return(0);
+ 	}
+ 
+ 	/* If there is a "must appear" string, look for it. */
+ 	if (prog->regmust != NULL) {
+ 		s = string;
+ 		while ((s = strchr(s, prog->regmust[0])) != NULL) {
+ 			if (strncmp(s, prog->regmust, prog->regmlen) == 0)
+ 				break;	/* Found it. */
+ 			s++;
+ 		}
+ 		if (s == NULL)	/* Not present. */
+ 			return(0);
+ 	}
+ 
+ 	/* Mark beginning of line for ^ . */
+ 	regbol = string;
+ 
+ 	/* Simplest case:  anchored match need be tried only once. */
+ 	if (prog->reganch)
+ 		return(regtry(prog, string));
+ 
+ 	/* Messy cases:  unanchored match. */
+ 	s = string;
+ 	if (prog->regstart != '\0')
+ 		/* We know what char it must start with. */
+ 		while ((s = strchr(s, prog->regstart)) != NULL) {
+ 			if (regtry(prog, s))
+ 				return(1);
+ 			s++;
+ 		}
+ 	else
+ 		/* We don't -- general case. */
+ 		do {
+ 			if (regtry(prog, s))
+ 				return(1);
+ 		} while (*s++ != '\0');
+ 
+ 	/* Failure. */
+ 	return(0);
+ }
+ 
+ /*
+  - regtry - try match at specific point
+  */
+ static int			/* 0 failure, 1 success */
+ regtry(prog, string)
+ regexp *prog;
+ char *string;
+ {
+ 	register int i;
+ 	register char **sp;
+ 	register char **ep;
+ 
+ 	reginput = string;
+ 	regstartp = prog->startp;
+ 	regendp = prog->endp;
+ 
+ 	sp = prog->startp;
+ 	ep = prog->endp;
+ 	for (i = NSUBEXP; i > 0; i--) {
+ 		*sp++ = NULL;
+ 		*ep++ = NULL;
+ 	}
+ 	if (regmatch(prog->program + 1)) {
+ 		prog->startp[0] = string;
+ 		prog->endp[0] = reginput;
+ 		return(1);
+ 	} else
+ 		return(0);
+ }
+ 
+ /*
+  - regmatch - main matching routine
+  *
+  * Conceptually the strategy is simple:  check to see whether the current
+  * node matches, call self recursively to see whether the rest matches,
+  * and then act accordingly.  In practice we make some effort to avoid
+  * recursion, in particular by going through "ordinary" nodes (that don't
+  * need to know whether the rest of the match failed) by a loop instead of
+  * by recursion.
+  */
+ static int			/* 0 failure, 1 success */
+ regmatch(prog)
+ char *prog;
+ {
+ 	register char *scan;	/* Current node. */
+ 	char *next;		/* Next node. */
+ 	extern char *strchr();
+ 
+ 	scan = prog;
+ #ifdef DEBUG
+ 	if (scan != NULL && regnarrate)
+ 		fprintf(stderr, "%s(\n", regprop(scan));
+ #endif
+ 	while (scan != NULL) {
+ #ifdef DEBUG
+ 		if (regnarrate)
+ 			fprintf(stderr, "%s...\n", regprop(scan));
+ #endif
+ 		next = regnext(scan);
+ 
+ 		switch (OP(scan)) {
+ 		case BOL:
+ 			if (reginput != regbol)
+ 				return(0);
+ 			break;
+ 		case EOL:
+ 			if (*reginput != '\0')
+ 				return(0);
+ 			break;
+ 		case ANY:
+ 			if (*reginput == '\0')
+ 				return(0);
+ 			reginput++;
+ 			break;
+ 		case EXACTLY: {
+ 				register int len;
+ 				register char *opnd;
+ 
+ 				opnd = OPERAND(scan);
+ 				/* Inline the first character, for speed. */
+ 				if (*opnd != *reginput)
+ 					return(0);
+ 				len = strlen(opnd);
+ 				if (len > 1 && strncmp(opnd, reginput, len) != 0)
+ 					return(0);
+ 				reginput += len;
+ 			}
+ 			break;
+ 		case ANYOF:
+ 			if (*reginput == '\0' || strchr(OPERAND(scan), *reginput) == NULL)
+ 				return(0);
+ 			reginput++;
+ 			break;
+ 		case ANYBUT:
+ 			if (*reginput == '\0' || strchr(OPERAND(scan), *reginput) != NULL)
+ 				return(0);
+ 			reginput++;
+ 			break;
+ 		case NOTHING:
+ 			break;
+ 		case BACK:
+ 			break;
+ 		case OPEN+1:
+ 		case OPEN+2:
+ 		case OPEN+3:
+ 		case OPEN+4:
+ 		case OPEN+5:
+ 		case OPEN+6:
+ 		case OPEN+7:
+ 		case OPEN+8:
+ 		case OPEN+9: {
+ 				register int no;
+ 				register char *save;
+ 
+ 				no = OP(scan) - OPEN;
+ 				save = reginput;
+ 
+ 				if (regmatch(next)) {
+ 					/*
+ 					 * Don't set startp if some later
+ 					 * invocation of the same parentheses
+ 					 * already has.
+ 					 */
+ 					if (regstartp[no] == NULL)
+ 						regstartp[no] = save;
+ 					return(1);
+ 				} else
+ 					return(0);
+ 			}
+ 			break;
+ 		case CLOSE+1:
+ 		case CLOSE+2:
+ 		case CLOSE+3:
+ 		case CLOSE+4:
+ 		case CLOSE+5:
+ 		case CLOSE+6:
+ 		case CLOSE+7:
+ 		case CLOSE+8:
+ 		case CLOSE+9: {
+ 				register int no;
+ 				register char *save;
+ 
+ 				no = OP(scan) - CLOSE;
+ 				save = reginput;
+ 
+ 				if (regmatch(next)) {
+ 					/*
+ 					 * Don't set endp if some later
+ 					 * invocation of the same parentheses
+ 					 * already has.
+ 					 */
+ 					if (regendp[no] == NULL)
+ 						regendp[no] = save;
+ 					return(1);
+ 				} else
+ 					return(0);
+ 			}
+ 			break;
+ 		case BRANCH: {
+ 				register char *save;
+ 
+ 				if (OP(next) != BRANCH)		/* No choice. */
+ 					next = OPERAND(scan);	/* Avoid recursion. */
+ 				else {
+ 					do {
+ 						save = reginput;
+ 						if (regmatch(OPERAND(scan)))
+ 							return(1);
+ 						reginput = save;
+ 						scan = regnext(scan);
+ 					} while (scan != NULL && OP(scan) == BRANCH);
+ 					return(0);
+ 					/* NOTREACHED */
+ 				}
+ 			}
+ 			break;
+ 		case STAR:
+ 		case PLUS: {
+ 				register char nextch;
+ 				register int no;
+ 				register char *save;
+ 				register int min;
+ 
+ 				/*
+ 				 * Lookahead to avoid useless match attempts
+ 				 * when we know what character comes next.
+ 				 */
+ 				nextch = '\0';
+ 				if (OP(next) == EXACTLY)
+ 					nextch = *OPERAND(next);
+ 				min = (OP(scan) == STAR) ? 0 : 1;
+ 				save = reginput;
+ 				no = regrepeat(OPERAND(scan));
+ 				while (no >= min) {
+ 					/* If it could work, try it. */
+ 					if (nextch == '\0' || *reginput == nextch)
+ 						if (regmatch(next))
+ 							return(1);
+ 					/* Couldn't or didn't -- back up. */
+ 					no--;
+ 					reginput = save + no;
+ 				}
+ 				return(0);
+ 			}
+ 			break;
+ 		case END:
+ 			return(1);	/* Success! */
+ 			break;
+ 		default:
+ 			regerror("memory corruption");
+ 			return(0);
+ 			break;
+ 		}
+ 
+ 		scan = next;
+ 	}
+ 
+ 	/*
+ 	 * We get here only if there's trouble -- normally "case END" is
+ 	 * the terminating point.
+ 	 */
+ 	regerror("corrupted pointers");
+ 	return(0);
+ }
+ 
+ /*
+  - regrepeat - repeatedly match something simple, report how many
+  */
+ static int
+ regrepeat(p)
+ char *p;
+ {
+ 	register int count = 0;
+ 	register char *scan;
+ 	register char *opnd;
+ 
+ 	scan = reginput;
+ 	opnd = OPERAND(p);
+ 	switch (OP(p)) {
+ 	case ANY:
+ 		count = strlen(scan);
+ 		scan += count;
+ 		break;
+ 	case EXACTLY:
+ 		while (*opnd == *scan) {
+ 			count++;
+ 			scan++;
+ 		}
+ 		break;
+ 	case ANYOF:
+ 		while (*scan != '\0' && strchr(opnd, *scan) != NULL) {
+ 			count++;
+ 			scan++;
+ 		}
+ 		break;
+ 	case ANYBUT:
+ 		while (*scan != '\0' && strchr(opnd, *scan) == NULL) {
+ 			count++;
+ 			scan++;
+ 		}
+ 		break;
+ 	default:		/* Oh dear.  Called inappropriately. */
+ 		regerror("internal foulup");
+ 		count = 0;	/* Best compromise. */
+ 		break;
+ 	}
+ 	reginput = scan;
+ 
+ 	return(count);
+ }
+ 
+ /*
+  - regnext - dig the "next" pointer out of a node
+  */
+ static char *
+ regnext(p)
+ register char *p;
+ {
+ 	register int offset;
+ 
+ 	if (p == &regdummy)
+ 		return(NULL);
+ 
+ 	offset = NEXT(p);
+ 	if (offset == 0)
+ 		return(NULL);
+ 
+ 	if (OP(p) == BACK)
+ 		return(p-offset);
+ 	else
+ 		return(p+offset);
+ }
+ 
+ #ifdef DEBUG
+ 
+ STATIC char *regprop();
+ 
+ /*
+  - regdump - dump a regexp onto stdout in vaguely comprehensible form
+  */
+ void
+ regdump(r)
+ regexp *r;
+ {
+ 	register char *s;
+ 	register char op = EXACTLY;	/* Arbitrary non-END op. */
+ 	register char *next;
+ 	extern char *strchr();
+ 
+ 
+ 	s = r->program + 1;
+ 	while (op != END) {	/* While that wasn't END last time... */
+ 		op = OP(s);
+ 		printf("%2d%s", s-r->program, regprop(s));	/* Where, what. */
+ 		next = regnext(s);
+ 		if (next == NULL)		/* Next ptr. */
+ 			printf("(0)");
+ 		else 
+ 			printf("(%d)", (s-r->program)+(next-s));
+ 		s += 3;
+ 		if (op == ANYOF || op == ANYBUT || op == EXACTLY) {
+ 			/* Literal string, where present. */
+ 			while (*s != '\0') {
+ 				putchar(*s);
+ 				s++;
+ 			}
+ 			s++;
+ 		}
+ 		putchar('\n');
+ 	}
+ 
+ 	/* Header fields of interest. */
+ 	if (r->regstart != '\0')
+ 		printf("start `%c' ", r->regstart);
+ 	if (r->reganch)
+ 		printf("anchored ");
+ 	if (r->regmust != NULL)
+ 		printf("must have \"%s\"", r->regmust);
+ 	printf("\n");
+ }
+ 
+ /*
+  - regprop - printable representation of opcode
+  */
+ static char *
+ regprop(op)
+ char *op;
+ {
+ 	register char *p;
+ 	static char buf[50];
+ 
+ 	(void) strcpy(buf, ":");
+ 
+ 	switch (OP(op)) {
+ 	case BOL:
+ 		p = "BOL";
+ 		break;
+ 	case EOL:
+ 		p = "EOL";
+ 		break;
+ 	case ANY:
+ 		p = "ANY";
+ 		break;
+ 	case ANYOF:
+ 		p = "ANYOF";
+ 		break;
+ 	case ANYBUT:
+ 		p = "ANYBUT";
+ 		break;
+ 	case BRANCH:
+ 		p = "BRANCH";
+ 		break;
+ 	case EXACTLY:
+ 		p = "EXACTLY";
+ 		break;
+ 	case NOTHING:
+ 		p = "NOTHING";
+ 		break;
+ 	case BACK:
+ 		p = "BACK";
+ 		break;
+ 	case END:
+ 		p = "END";
+ 		break;
+ 	case OPEN+1:
+ 	case OPEN+2:
+ 	case OPEN+3:
+ 	case OPEN+4:
+ 	case OPEN+5:
+ 	case OPEN+6:
+ 	case OPEN+7:
+ 	case OPEN+8:
+ 	case OPEN+9:
+ 		sprintf(buf+strlen(buf), "OPEN%d", OP(op)-OPEN);
+ 		p = NULL;
+ 		break;
+ 	case CLOSE+1:
+ 	case CLOSE+2:
+ 	case CLOSE+3:
+ 	case CLOSE+4:
+ 	case CLOSE+5:
+ 	case CLOSE+6:
+ 	case CLOSE+7:
+ 	case CLOSE+8:
+ 	case CLOSE+9:
+ 		sprintf(buf+strlen(buf), "CLOSE%d", OP(op)-CLOSE);
+ 		p = NULL;
+ 		break;
+ 	case STAR:
+ 		p = "STAR";
+ 		break;
+ 	case PLUS:
+ 		p = "PLUS";
+ 		break;
+ 	default:
+ 		regerror("corrupted opcode");
+ 		break;
+ 	}
+ 	if (p != NULL)
+ 		(void) strcat(buf, p);
+ 	return(buf);
+ }
+ #endif
+ 
+ /*
+  * The following is provided for those people who do not have strcspn() in
+  * their C libraries.  They should get off their butts and do something
+  * about it; at least one public-domain implementation of those (highly
+  * useful) string routines has been published on Usenet.
+  */
+ #ifdef STRCSPN
+ /*
+  * strcspn - find length of initial segment of s1 consisting entirely
+  * of characters not from s2
+  */
+ 
+ static int
+ strcspn(s1, s2)
+ char *s1;
+ char *s2;
+ {
+ 	register char *scan1;
+ 	register char *scan2;
+ 	register int count;
+ 
+ 	count = 0;
+ 	for (scan1 = s1; *scan1 != '\0'; scan1++) {
+ 		for (scan2 = s2; *scan2 != '\0';)	/* ++ moved down. */
+ 			if (*scan1 == *scan2++)
+ 				return(count);
+ 		count++;
+ 	}
+ 	return(count);
+ }
+ #endif
*** /dev/null	Sun Mar 15 00:32:54 1992
--- regerror.c	Sat Aug 13 17:13:20 1988
***************
*** 0 ****
--- 1,14 ----
+ #include <stdio.h>
+ 
+ void
+ regerror(s)
+ char *s;
+ {
+ #ifdef ERRAVAIL
+ 	error("regexp: %s", s);
+ #else
+ 	fprintf(stderr, "regexp(3): %s", s);
+ 	exit(1);
+ #endif
+ 	/* NOTREACHED */
+ }
*** less.nro.orig	Wed Apr  3 12:35:48 1991
--- less.nro	Sun Mar 15 00:45:43 1992
***************
*** 179,186 ****
  .IP /pattern
  Search forward in the file for the N-th line containing the pattern.
  N defaults to 1.
! The pattern is a regular expression, as recognized by
! .I ed.
  The search starts at the second line displayed
  (but see the -a and -j options, which change this).
  .sp
--- 179,186 ----
  .IP /pattern
  Search forward in the file for the N-th line containing the pattern.
  N defaults to 1.
! The pattern is a regular expression, see regular expression syntax
! below.
  The search starts at the second line displayed
  (but see the -a and -j options, which change this).
  .sp
***************
*** 928,933 ****
--- 928,999 ----
  The type of terminal on which
  .I less
  is being run.
+ 
+ .SH "REGULAR EXPRESSION SYNTAX"
+ This segment is extracted from regexp.3 in Henry Spenser's regexp package.
+ The entire regexp package can be obtained separately.
+ .PP
+ A regular expression is zero or more \fIbranches\fR, separated by `|'.
+ It matches anything that matches one of the branches.
+ .PP
+ A branch is zero or more \fIpieces\fR, concatenated.
+ It matches a match for the first, followed by a match for the second, etc.
+ .PP
+ A piece is an \fIatom\fR possibly followed by `*', `+', or `?'.
+ An atom followed by `*' matches a sequence of 0 or more matches of the atom.
+ An atom followed by `+' matches a sequence of 1 or more matches of the atom.
+ An atom followed by `?' matches a match of the atom, or the null string.
+ .PP
+ An atom is a regular expression in parentheses (matching a match for the
+ regular expression), a \fIrange\fR (see below), `.'
+ (matching any single character), `^' (matching the null string at the
+ beginning of the input string), `$' (matching the null string at the
+ end of the input string), a `\e' followed by a single character (matching
+ that character), or a single character with no other significance
+ (matching that character).
+ .PP
+ A \fIrange\fR is a sequence of characters enclosed in `[]'.
+ It normally matches any single character from the sequence.
+ If the sequence begins with `^',
+ it matches any single character \fInot\fR from the rest of the sequence.
+ If two characters in the sequence are separated by `\-', this is shorthand
+ for the full list of ASCII characters between them
+ (e.g. `[0-9]' matches any decimal digit).
+ To include a literal `]' in the sequence, make it the first character
+ (following a possible `^').
+ To include a literal `\-', make it the first or last character.
+ .SH AMBIGUITY
+ If a regular expression could match two different parts of the input string,
+ it will match the one which begins earliest.
+ If both begin in the same place	but match different lengths, or match
+ the same length in different ways, life gets messier, as follows.
+ .PP
+ In general, the possibilities in a list of branches are considered in
+ left-to-right order, the possibilities for `*', `+', and `?' are
+ considered longest-first, nested constructs are considered from the
+ outermost in, and concatenated constructs are considered leftmost-first.
+ The match that will be chosen is the one that uses the earliest
+ possibility in the first choice that has to be made.
+ If there is more than one choice, the next will be made in the same manner
+ (earliest possibility) subject to the decision on the first choice.
+ And so forth.
+ .PP
+ For example, `(ab|a)b*c' could match `abc' in one of two ways.
+ The first choice is between `ab' and `a'; since `ab' is earlier, and does
+ lead to a successful overall match, it is chosen.
+ Since the `b' is already spoken for,
+ the `b*' must match its last possibility\(emthe empty string\(emsince
+ it must respect the earlier choice.
+ .PP
+ In the particular case where no `|'s are present and there is only one
+ `*', `+', or `?', the net effect is that the longest possible
+ match will be chosen.
+ So `ab*', presented with `xabbbby', will match `abbbb'.
+ Note that if `ab*' is tried against `xabyabbbz', it
+ will match `ab' just after `x', due to the begins-earliest rule.
+ (In effect, the decision on where to start the match is the first choice
+ to be made, hence subsequent choices must respect it even if this leads them
+ to less-preferred alternatives.)
  
  .SH "SEE ALSO"
  lesskey(1)
*** version.c.orig	Wed Apr  3 12:35:50 1991
--- version.c	Sun Mar 15 00:48:34 1992
***************
*** 312,315 ****
   *	      Try /dev/tty for input before using fd 2.
   */
  
! char version[] = "@(#) less  version 177";
--- 312,315 ----
   *	      Try /dev/tty for input before using fd 2.
   */
  
! char version[] = "@(#) less  version 177+";
*** /dev/null	Sun Mar 15 01:01:45 1992
--- regmagic.h	Sat Aug 13 17:13:22 1988
***************
*** 0 ****
--- 1,5 ----
+ /*
+  * The first byte of the regexp internal "program" is actually this magic
+  * number; the start node begins in the second byte.
+  */
+ #define	MAGIC	0234
*** linstall.orig	Wed Apr  3 12:35:48 1991
--- linstall	Sun Mar 15 01:35:52 1992
***************
*** 742,748 ****
  # It is normally "-O" but may be, for example, "-g".
  OPTIM = -O
  
! CFLAGS = $(OPTIM)
  
  
  
--- 742,748 ----
  # It is normally "-O" but may be, for example, "-g".
  OPTIM = -O
  
! CFLAGS = $(OPTIM) -DSTRCSPN -I.
  
  
  
***************
*** 756,769 ****
  SRC3 =	charset.c filename.c lsystem.c output.c position.c ifile.c \
  	brac.c forwback.c jump.c search.c 
  SRC4 =	mark.c prompt.c screen.c signal.c tags.c ttyin.c version.c
  
! SRC =	$(SRC1) $(SRC2) $(SRC3) $(SRC4)
  
  OBJ =	brac.o ch.o charset.o cmdbuf.o command.o decode.o edit.o filename.o \
  	forwback.o help.o input.o jump.o line.o linenum.o \
  	lsystem.o main.o option.o optfunc.o opttbl.o os.o \
  	output.o position.o mark.o ifile.o prompt.o screen.o \
! 	search.o signal.o tags.o ttyin.o version.o
  
  
  ##########################################################################
--- 756,771 ----
  SRC3 =	charset.c filename.c lsystem.c output.c position.c ifile.c \
  	brac.c forwback.c jump.c search.c 
  SRC4 =	mark.c prompt.c screen.c signal.c tags.c ttyin.c version.c
+ SRC5 =  regexp.c regerror.c
  
! SRC =	$(SRC1) $(SRC2) $(SRC3) $(SRC4) $(SRC5)
  
  OBJ =	brac.o ch.o charset.o cmdbuf.o command.o decode.o edit.o filename.o \
  	forwback.o help.o input.o jump.o line.o linenum.o \
  	lsystem.o main.o option.o optfunc.o opttbl.o os.o \
  	output.o position.o mark.o ifile.o prompt.o screen.o \
! 	search.o signal.o tags.o ttyin.o version.o \
! 	regexp.o regerror.o
  
  
  ##########################################################################
***************
*** 841,846 ****
--- 843,849 ----
  	shar -v $(SRC2) > less4.shr
  	shar -v $(SRC3) less.hlp > less5.shr
  	shar -v $(SRC4) funcs.h > less6.shr
+ 	shar -v $(SRC5) regexp.h > less7.shr
  
  
