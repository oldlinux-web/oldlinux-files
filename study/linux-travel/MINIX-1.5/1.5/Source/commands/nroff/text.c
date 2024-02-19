/*
 *	text.c - text output processing portion of nroff word processor
 *
 *	adapted for atariST/TOS by Bill Rosenkranz 11/89
 *	net:	rosenkra@hall.cray.com
 *	CIS:	71460,17
 *	GENIE:	W.ROSENKRANZ
 *
 *	original author:
 *
 *	Stephen L. Browning
 *	5723 North Parker Avenue
 *	Indianapolis, Indiana 46220
 *
 *	history:
 *
 *	- Originally written in BDS C;
 *	- Adapted for standard C by W. N. Paul
 *	- Heavily hacked up to conform to "real" nroff by Bill Rosenkranz
 *	- Adapted the justification of lines with escape codes 
 *	  by Wim 'Blue Baron' van Dorst (wsincc@tuerc3.urc.tue.nl)
 */

#undef NRO_MAIN					/* extern globals */

#include <stdio.h>
#include "nroff.h"


/*------------------------------*/
/*	text			*/
/*------------------------------*/
text (p)
register char  *p;
{

/*
 *	main text processing
 */

	register int	i;
	char		wrdbuf[MAXLINE];


	/*
	 *   skip over leading blanks if in fill mode. we indent later.
	 *   since leadbl does a robrk, do it if in .nf mode
	 */
	if (dc.fill == YES)
	{
		if (*p == ' ' || *p == '\n' || *p == '\r')
			leadbl (p);
	}
	else
		robrk ();


	/*
	 *   expand escape sequences
	 */
	expesc (p, wrdbuf);


	/*
	 *   test for how to output
	 */
	if (dc.ulval > 0)
	{
		/*
		 *   underline (.ul)
		 *
		 *   Because of the way underlining is handled,
		 *   MAXLINE should be declared to be three times
		 *   larger than the longest expected input line
		 *   for underlining.  Since many of the character
		 *   buffers use this parameter, a lot of memory
		 *   can be allocated when it may not really be
		 *   needed.  A MAXLINE of 180 would allow about
		 *   60 characters in the output line to be
		 *   underlined (remember that only alphanumerics
		 *   get underlined - no spaces or punctuation).
		 */
		underl (p, wrdbuf, MAXLINE);
		--dc.ulval;
	}
	if (dc.cuval > 0)
	{
		/*
		 *   continuous underline (.cu)
		 */
		underl (p, wrdbuf, MAXLINE);
		--dc.cuval;
	}
	if (dc.boval > 0)
	{
		/*
		 *   bold (.bo)
		 */
		bold (p, wrdbuf, MAXLINE);
		--dc.boval;
	}
	if (dc.ceval > 0)
	{
		/*
		 *   centered (.ce)
		 */
		center (p);
		put (p);
		--dc.ceval;
	}
	else if ((*p == '\r' || *p == '\n') && dc.fill == NO)
	{
		/*
		 *   all blank line
		 */
		put (p);
	}
	else if (dc.fill == NO)
	{
		/*
		 *   unfilled (.nf)
		 */
		put (p);
	}
	else
	{
		/*
		 *   anything else...
		 */

		/*
		 *   get a word and put it out. increment ptr to the next
		 *   word.
		 */
		while ((i = getwrd (p, wrdbuf)) > 0)
		{
			putwrd (wrdbuf);
			p += i;
		}
	}
}




/*------------------------------*/
/*	bold			*/
/*------------------------------*/
bold (p0, p1, size)
register char  *p0;
register char  *p1;
int		size;
{

/*
 *	insert bold face text (by overstriking)
 */

	register int	i;
	register int	j;

	j = 0;
	for (i = 0; (p0[i] != '\n') && (j < size - 1); ++i)
	{
		if (isalpha (p0[i]) || isdigit (p0[i]))
		{
			p1[j++] = p0[i];
			p1[j++] = '\b';
		}
		p1[j++] = p0[i];
	}
	p1[j++] = '\n';
	p1[j] = EOS;
	while (*p1 != EOS)
		*p0++ = *p1++;
	*p0 = EOS;
}






/*------------------------------*/
/*	center			*/
/*------------------------------*/
center (p)
register char  *p;
{

/*
 *	center a line by setting tival
 */

	dc.tival = max ((dc.rmval + dc.tival - width (p)) >> 1, 0);
}




/*------------------------------*/
/*	expand			*/
/*------------------------------*/
expand (p0, c, s)
register char  *p0;
char		c;
register char  *s;
{

/*
 *	expand title buffer to include character string
 */

	register char  *p;
	register char  *q;
	register char  *r;
	char    	tmp[MAXLINE];

	p = p0;
	q = tmp;
	while (*p != EOS)
	{
		if (*p == c)
		{
			r = s;
			while (*r != EOS)
				*q++ = *r++;
		}
		else
			*q++ = *p;
		++p;
	}
	*q = EOS;
	strcpy (p0, tmp);		/* copy it back */
}




/*------------------------------*/
/*	justcntr		*/
/*------------------------------*/
justcntr (p, q, limit)
register char  *p;
char	       *q;
int	       *limit;
{

/*
 *	center title text into print buffer
 */

	register int	len;

	len = width (p);
	q   = &q[(limit[RIGHT] + limit[LEFT] - len) >> 1];
	while (*p != EOS)
		*q++ = *p++;
}





/*------------------------------*/
/*	justleft		*/
/*------------------------------*/
justleft (p, q, limit)
register char  *p;
char	       *q;
int		limit;
{

/*
 *	left justify title text into print buffer
 */

	q = &q[limit];
	while (*p != EOS)
		*q++ = *p++;
}




/*------------------------------*/
/*	justrite		*/
/*------------------------------*/
justrite (p, q, limit)
register char  *p;
char	       *q;
int     	limit;
{

/*
 *	right justify title text into print buffer
 */

	register int	len;

	len = width (p);
	q = &q[limit - len];
	while (*p != EOS)
		*q++ = *p++;
}






/*------------------------------*/
/*	leadbl			*/
/*------------------------------*/
leadbl (p)
register char  *p;
{

/*
 *	delete leading blanks, set tival
 */

	register int	i;
	register int	j;

	/*
	 *   end current line and reset co struct
	 */
	robrk ();

	/*
	 *   skip spaces
	 */
	for (i = 0; p[i] == ' ' || p[i] == '\t'; ++i)
		;

	/*
	 *   if not end of line, reset current temp indent
	 */
#ifdef GEMDOS
	if (p[i] != '\n' && p[i] != '\r')
		dc.tival = i;
#else
	if (p[i] != '\n' && p[i] != '\r')
		dc.tival = i;
#endif

	/*
	 *   shift string
	 */
	for (j = 0; p[i] != EOS; ++j)
		p[j] = p[i++];
	p[j] = EOS;
}





/*------------------------------*/
/*	pfoot			*/
/*------------------------------*/
pfoot ()
{

/*
 *	put out page footer
 */

	if (dc.prflg == TRUE)
	{
		skip (pg.m3val);
		if (pg.m4val > 0)
		{
			if ((pg.curpag % 2) == 0)
			{
				puttl (pg.efoot, pg.eflim, pg.curpag);
			}
			else
			{
				puttl (pg.ofoot, pg.oflim, pg.curpag);
			}
			skip (pg.m4val - 1);
		}
	}
}





/*------------------------------*/
/*	phead			*/
/*------------------------------*/
phead ()
{

/*
 *	put out page header
 */

	pg.curpag = pg.newpag;
	if (pg.curpag >= pg.frstpg && pg.curpag <= pg.lastpg)
	{
		dc.prflg = TRUE;
	}
	else
	{
		dc.prflg = FALSE;
	}
	++pg.newpag;
	set_ireg ("%", pg.newpag, 0);
	if (dc.prflg == TRUE)
	{
		if (pg.m1val > 0)
		{
			skip (pg.m1val - 1);
			if ((pg.curpag % 2) == 0)
			{
				puttl (pg.ehead, pg.ehlim, pg.curpag);
			}
			else
			{
				puttl (pg.ohead, pg.ohlim, pg.curpag);
			}
		}
		skip (pg.m2val);
	}
	/* 
	 *	initialize lineno for the next page
	 */
	pg.lineno = pg.m1val + pg.m2val + 1;
	set_ireg ("ln", pg.lineno, 0);
}




/*------------------------------*/
/*	puttl			*/
/*------------------------------*/
puttl (p, lim, pgno)
register char  *p;
int	       *lim;
int		pgno;
{

/*
 *	put out title or footer
 */

	register int	i;
	char		pn[8];
	char		t[MAXLINE];
	char		h[MAXLINE];
	char		delim;

	itoda (pgno, pn, 6);
	for (i = 0; i < MAXLINE; ++i)
		h[i] = ' ';
	delim = *p++;
	p = getfield (p, t, delim);
	expand (t, dc.pgchr, pn);
	justleft (t, h, lim[LEFT]);
	p = getfield (p, t, delim);
	expand (t, dc.pgchr, pn);
	justcntr (t, h, lim);
	p = getfield (p, t, delim);
	expand (t, dc.pgchr, pn);
	justrite (t, h, lim[RIGHT]);
	for (i = MAXLINE - 4; h[i] == ' '; --i)
		h[i] = EOS;
	h[++i] = '\n';
#ifdef GEMDOS
	h[++i] = '\r';
#endif
	h[++i] = EOS;
	if (strlen (h) > 2)
	{
		for (i = 0; i < pg.offset; ++i)
			prchar (' ', out_stream);
	}
	putlin (h, out_stream);
}





/*------------------------------*/
/*	putwrd			*/
/*------------------------------*/
putwrd (wrdbuf)
register char  *wrdbuf;
{

/*
 *	put word in output buffer
 */

	register char  *p0;
	register char  *p1;
	int     	w;
	int     	last;
	int     	llval;
	int     	nextra;



	/*
	 *   check if this word puts us over the limit
	 */
	w     = width (wrdbuf);
	last  = strlen (wrdbuf) + co.outp;
	llval = dc.rmval - dc.tival;
	co.outesc += countesc (wrdbuf);
	if (((co.outp > 0) && ((co.outw + w - co.outesc) > llval))
	||   (last > MAXLINE))
	{
		/*
		 *   last word exceeds limit so prepare to break line, print
		 *   it, and reset outbuf.
		 */
		last -= co.outp;
		if (dc.juval == YES)
		{
			nextra = llval - co.outw + 1;

			/*
			 *	Do not take in the escape char of the
			 * 	word that didn't fit on this line anymore
			 */
			 co.outesc -= countesc (wrdbuf);
			
			/* 
			 *	Check whether last word was end of
			 *	sentence and modify counts so that
			 *	it is right justified.
			 */
			if (co.outbuf[co.outp - 2] == ' ')
			{
				--co.outp;
				++nextra;
			}
			spread (co.outbuf, co.outp - 1, nextra, 
							co.outwds, co.outesc);
			if ((nextra + co.outesc > 0) && (co.outwds > 1))
			{
				co.outp += (nextra + co.outesc - 1);
			}
		}

		/*
		 *   break line, output it, and reset all co members. reset
		 *   esc count.
		 */
		robrk ();

		co.outesc = countesc (wrdbuf);
	}


	/*
	 *   copy the current word to the out buffer which may have been
	 *   reset
	 */
	p0 = wrdbuf;
	p1 = co.outbuf + co.outp;
	while (*p0 != EOS)
		*p1++ = *p0++;

	co.outp              = last;
	co.outbuf[co.outp++] = ' ';
	co.outw             += w + 1;
	co.outwds           += 1;
}




/*------------------------------*/
/*	skip			*/
/*------------------------------*/
skip (n)
register int	n;
{

/*
 *	skips the number of lines specified by n.
 */

	register int	i;

	if (dc.prflg == TRUE && n > 0)
	{
		for (i = 0; i < n; ++i)
		{
			prchar ('\n', out_stream);
		}
#ifdef GEMDOS
		prchar ('\r', out_stream);
#endif
	}
}





/*------------------------------*/
/*	spread			*/
/*------------------------------*/
spread (p, outp, nextra, outwds, escapes)
register char  *p;
int     	outp;
int		nextra;
int		outwds;
int		escapes;
{

/*
 *	spread words to justify right margin
 */

	register int	i;
	register int	j;
	register int	nb;
	register int	ne;
	register int	nholes;
	int		jmin;


	/*
	 *   quick sanity check...
	 */
	if ((nextra + escapes < 1) || (outwds < 2))
		return;

	/*
	 *   set up for the spread and do it...
	 */
	dc.sprdir = ~dc.sprdir;
	ne        = nextra + escapes;
	nholes    = outwds - 1;			/* holes between words */
	i         = outp - 1;			/* last non-blank character */
	j         = min (MAXLINE - 3, i + ne);	/* leave room for CR,LF,EOS */

	while (i < j)
	{
		p[j] = p[i];
		if (p[i] == ' ')
		{
			if (dc.sprdir == 0)
				nb = (ne - 1) / nholes + 1;
			else
				nb = ne / nholes;
			ne -= nb;
			--nholes;
			for (; nb > 0; --nb)
			{
				--j;
				p[j] = ' ';
			}
		}
		--i;
		--j;
	}
}





/*------------------------------*/
/*	strkovr			*/
/*------------------------------*/
strkovr (p, q)
register char  *p;
register char  *q;
{

/*
 *	split overstrikes (backspaces) into seperate buffer
 */

	register char  *pp;
	int		bsflg;

	bsflg = FALSE;
	pp = p;
	while (*p != EOS)
	{
		*q = ' ';
		*pp = *p;
		++p;
		if (*p == '\b')
		{
			if (*pp >= ' ' && *pp <= '~')
			{
				bsflg = TRUE;
				*q = *pp;
				++p;
				*pp = *p;
				++p;
			}
		}
		++q;
		++pp;
	}
	*q++ = '\r';
	*q = *pp = EOS;

	return (bsflg);
}





/*------------------------------*/
/*	underl			*/
/*------------------------------*/
underl (p0, p1, size)
register char  *p0;
register char  *p1;
int		size;
{

/*
 *	underline a line
 */

	register int	i;
	register int	j;

	j = 0;
	for (i = 0; (p0[i] != '\n') && (j < size - 1); ++i)
	{
		if (p0[i] >= ' ' && p0[i] <= '~')
		{
			if (isalpha (p0[i]) || isdigit (p0[i]) || dc.cuval > 0)
			{
				p1[j++] = '_';
				p1[j++] = '\b';
			}
		}
		p1[j++] = p0[i];
	}
	p1[j++] = '\n';
	p1[j] = EOS;
	while (*p1 != EOS)
		*p0++ = *p1++;
	*p0 = EOS;
}




/*------------------------------*/
/*	width			*/
/*------------------------------*/
width (s)
register char  *s;
{

/*
 *	compute width of character string
 */

	register int	w;

	w = 0;
	while (*s != EOS)
	{
		if (*s == '\b')
			--w;
		else if (*s != '\n' && *s != '\r')
			++w;
		++s;
	}

	return (w);
}
