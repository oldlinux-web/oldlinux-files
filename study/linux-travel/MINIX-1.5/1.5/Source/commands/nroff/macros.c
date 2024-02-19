/*
 *	macros.c - macro input/output processing for nroff word processor
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
 *	- Changed array index i from type long to type int (32000 is the
 *	  largest value anyhow) to prevent compiler warnings
 *	  by Wim 'Blue Baron' van Dorst (wsincc@tuerc3.urc.tue.nl)
 */

#undef NRO_MAIN					/* extern globals */

#include <stdio.h>
#include "nroff.h"



/*------------------------------*/
/*	defmac			*/
/*------------------------------*/
defmac (p, infp)
register char  *p;
FILE	       *infp;
{

/*
 *	Define a macro. top level, read from stream.
 *
 *	we should read macro without interpretation EXCEPT:
 *
 *	1) number registers are interpolated
 *	2) strings indicated by \* are interpolated
 *	3) arguments indicated by \$ are interpolated
 *	4) concealed newlines indicated by \(newline) are eliminated
 *	5) comments indicated by \" are eliminated
 *	6) \t and \a are interpreted as ASCII h tab and SOH.
 *	7) \\ is interpreted as backslash and \. is interpreted as a period.
 *
 *	currently, we do only 3. a good place to do it would be here before
 *	putmac, after colmac...
 */

	register char  *q;
	register int	i;
	char    	name[MNLEN];
	char    	defn[MXMLEN];
	char		newend[10];


	/*
	 *   skip the .de and get to the name...
	 */
	q = skipwd (p);
	q = skipbl (q);

	/*
	 *   ok, name now holds the name. make sure it is valid (i.e. first
	 *   char is alpha...). getwrd returns the length of the word.
	 */
	i = getwrd (q, name);
	if (!isprint (*name))
	{
		fprintf (err_stream,
			"***%s: missing or illegal macro definition name\n",
			myname);
		err_exit (-1);
	}

	/*
	 *   truncate to 2 char max name.
	 */
	if (i > 2)
		name[2] = EOS;


	/*
	 *   skip the name and see if we have a new end defined...
	 */
	q = skipwd (p);
	q = skipbl (q);
	for (i = 0; i < 10; i++)
		newend[i] = EOS;

	for (i = 0; (i < 10) && ( isalpha (q[i]) || isdigit (q[i]) ); i++)
	{
		newend[i] = q[i];
	}



	/*
	 *   read a line from input stream until we get the end of macro
	 *   command (.en or ..). actually. we should have read the next
	 *   field just above here to get the .de NA . or .de NA en string
	 *   to be new end of macro.
	 */
	i = 0;
	while (getlin (p, infp) != EOF)
	{
		if (p[0] == dc.cmdchr && newend[0] != EOS
		&&  p[1] == newend[0] && p[2] == newend[1])
		{
			/*
			 *   replacement end found
			 */
			break;
		}
		if (p[0] == dc.cmdchr && p[1] == 'e' && p[2] == 'n')
		{
			/*
			 *   .en found
			 */
			break;
		}
		if (p[0] == dc.cmdchr && p[1] == dc.cmdchr)
		{
			/*
			 *   .. found
			 */
			break;
		}


		/*
		 *   collect macro from the line we just read. all this does
		 *   is put it in the string defn.
		 */
		if ((i = colmac (p, defn, i)) == ERR)
		{
			fprintf (err_stream,
				"***%s: macro definition too long\n", myname);
			err_exit (-1);
		}
	}


	/*
	 *   store the macro
	 */
	if (!ignoring)
	{
		if (putmac (name, defn) == ERR)
		{
			fprintf (err_stream,
				"***%s: macro definition table full\n", myname);
			err_exit (-1);
		}
	}
}





/*------------------------------*/
/*	colmac			*/
/*------------------------------*/
colmac (p, d, i)
register char  *p;
char	       *d;
register int	i;
{

/*
 *	Collect macro definition from input stream
 */

	while (*p != EOS)
	{
		if (i >= MXMLEN - 1)
		{
			d[i - 1] = EOS;
			return (ERR);
		}
		d[i++] = *p++;
	}
	d[i] = EOS;
	return (i);
}





/*------------------------------*/
/*	putmac			*/
/*------------------------------*/
putmac (name, p)
char   *name;
char   *p;
{

/*
 *	Put macro definition into table
 *
 *	NOTE: any expansions of things like number registers SHOULD
 *	have been done already.
 */


	/*
	 *   any room left? (did we exceed max number of possible macros)
	 */
	if (mac.lastp >= MXMDEF)
		return (ERR);

	/*
	 *   will new one fit in big buffer?
	 */
	if (mac.emb + strlen (name) + strlen (p) + 1 > &mac.mb[MACBUF])
	{
		return (ERR);
	}


	/*
	 *   add it...
	 *
	 *   bump counter, set ptr to name, copy name, copy def.
	 *   finally increment end of macro buffer ptr (emb).
	 *
	 *   macro looks like this in mb:
	 *
	 *	mac.mb[MACBUF]		size of total buf
	 *	lastp < MXMDEF		number of macros possible
	 *	*mnames[MXMDEF]		-> names, each max length
	 *	..._____________________________...____________________...
	 *	    / / /|X|X|0|macro definition      |0| / / / / / / /
	 *	.../_/_/_|_|_|_|________________...___|_|/_/_/_/_/_/_/_...
	 *		  ^
	 *		  |
	 *		  \----- mac.mnames[mac.lastp] points here
	 *
	 *   both the 2 char name (XX) and the descripton are null term and
	 *   follow one after the other.
	 */
	++mac.lastp;
	mac.mnames[mac.lastp] = mac.emb;
	strcpy (mac.emb, name);
	strcpy (mac.emb + strlen (name) + 1, p);
	mac.emb += strlen (name) + strlen (p) + 2;

	return (OK);
}






/*------------------------------*/
/*	getmac			*/
/*------------------------------*/
char   *getmac (name)
register char  *name;
{

/*
 *	Get (lookup) macro definition from namespace
 */

	register int	i;

	/*
	 *   loop for all macros, starting with last one
	 */
	for (i = mac.lastp; i >= 0; --i)
	{
		/*
		 *   is this REALLY a macro?
		 */
		if (mac.mnames[i])
		{
			/*
			 *   if it compares, return a ptr to it
			 */
			if (!strcmp (name, mac.mnames[i]))
			{
/*!!!debug			puts (mac.mnames[i]);*/

				if (mac.mnames[i][1] == EOS)
					return (mac.mnames[i] + 2);
				else
					return (mac.mnames[i] + 3);
			}
		}
	}

	/*
	 *   none found, return null
	 */
	return (NULL_CPTR);
}






/*------------------------------*/
/*	maceval			*/
/*------------------------------*/
maceval (p, m)
register char  *p;
char	       *m;
{

/*
 *	Evaluate macro expansion
 */

	register int	i;
	register int	j;
	char	       *argp[15];
	char		c;
	int		xc;



	/*
	 *   replace command char with EOS
	 */
	*p++ = EOS;


	/* 
	 *   initialize argp array to substitute command
	 *   string for any undefined argument
	 *
	 *	NO!!! this is fixed...
	 */
/*	for (i = 0; i < 10; ++i)
		argp[i] = p;
*/
	/*
	 *   skip the command name
	 */
	p = skipwd (p);
	*p++ = EOS;


	/*
	 *   loop for all $n variables...
	 */
	for (i = 0; i < 10; ++i)
	{
		/*
		 *   get to substituted param and if no more, reset remaining
		 *   args to NULL and stop. using "i" here IS ok...
		 */
		p = skipbl (p);
		if (*p == '\r' || *p == '\n' || *p == EOS)
		{
			set_ireg (".$", i, 0);
			for ( ; i < 10; i++)
			{
				argp[i] = NULL_CPTR;
			}
			break;
		}


		/*
		 *   ...otherwise, see if this param is quoted. if it is,
		 *   it is all one parameter, even with blanks (but not
		 *   newlines...). look for another "c" (which is the quote).
		 *
		 *   if no quote, just read the arg as a single word and null
		 *   terminate it.
		 */
		if (*p == '\'' || *p == '"')
		{
			c = *p++;
			argp[i] = p;
			while (*p != c && *p != '\r' && *p != '\n' && *p != EOS)
				++p;
			*p++ = EOS;
		}
		else
		{
			argp[i] = p;
			p = skipwd (p);
			*p++ = EOS;
		}
	}


	/*
	 *   m contains text of the macro. p contained the input line.
	 *   here we start at the end of the macro def and see if there
	 *   are any $n thingies. go backwards.
	 */
	for (i = strlen (m) - 1; i >= 0; --i)
	{
		/*
		 *   found a $.
		 */
		if (i > 0 && m[i - 1] == '$')
		{
			if (!isdigit (m[i]))
			{
				/*
				 *   it wasn't a numeric replacement arg so
				 *   push this char back onto input stream
				 */
				putbak (m[i]);
			}
			else
			{
				/*
				 *   it WAS a numeric replacement arg. so we
				 *   want to push back the appropriate macro
				 *   invocation arg. m[i]-'0' is the numerical
				 *   value of the $1 thru $9. if the arg is
				 *   not there, argp[n] will be (char *) 0
				 *   and pbstr will do nothing.
				 */
				xc = m[i] - '1';
				if (argp[xc])
					pbstr (argp[xc]);
				--i;
			}
		}
		else
		{
			/*
			 *   no $ so push back the char...
			 */
			putbak (m[i]);
		}
	}

	/*
	 *   at this point, the iobuf will hold the new macro command, full
	 *   expanded for $n things. the return gets us right back to the
	 *   main loop in main() and we parse the (new) command just as if
	 *   it were read from a file.
	 */

}





/*------------------------------*/
/*	printmac		*/
/*------------------------------*/
printmac (opt)
int	opt;				/* 0=name&size,1=total size,2=full */
{

/*
 *	print all macros and strings and tabulate sizes
 */

	register int	i;
	register long	space;
 	register long	totalspace;
	register char  *pname;
	register char  *pdef;


	space      = 0L;
	totalspace = 0L;

	fflush (out_stream);
	fflush (err_stream);

	for (i = mac.lastp; i >= 0; --i)
	{
 		/*
		 *   is this REALLY a macro?
		 */
		if (mac.mnames[i])
		{
			pname = (char *) (mac.mnames[i]);
			pdef  = pname + 3;
			if (*(pname + 1) == '\0')
				pdef = pname + 2;

			space       = (long) strlen (pdef);
			totalspace += space;

			switch (opt)
			{
			case 0:
				fprintf (err_stream, "%s %ld\n", pname, space);
				break;
			case 2:
				fprintf (err_stream, "%s %ld\n", pname, space);
				fprintf (err_stream, "%s\n", pdef);
				break;
			case 1:
			default:
				break;
			}
		}
	}
	fprintf (err_stream, "Total space: %ld\n", totalspace);
	
}

