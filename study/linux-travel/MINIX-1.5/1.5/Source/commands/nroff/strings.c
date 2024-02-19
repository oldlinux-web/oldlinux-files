/*
 *	strings.c - String input/output processing for nroff word processor
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
 */

#undef NRO_MAIN					/* extern globals */

#include <stdio.h>
#include "nroff.h"



/*------------------------------*/
/*	defstr			*/
/*------------------------------*/
defstr (p)
register char  *p;
{

/*
 *	Define a string. top level, read from command line.
 *
 *	we should read string without interpretation EXCEPT:
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
 *	putstr, after colstr...
 */

	register char  *q;
	register int	i;
	char		name[MNLEN];
	char		defn[MXMLEN];



	name[0] = '\0';
	defn[0] = '\0';


	/*
	 *   skip the .ds and get to the name...
	 */
	q = skipwd (p);
	q = skipbl (q);

	/*
	 *   ok, name now holds the name. make sure it is valid (i.e. first
	 *   char is alpha...). getwrd returns the length of the word.
	 */
	i = getwrd (q, name);
	if (!name[0])
	{
		fprintf (err_stream,
			"***%s: missing or illegal string definition name\n",
			myname);
		err_exit (-1);
	}

	/*
	 *   truncate to 2 char max name.
	 */
	if (i > 2)
		name[2] = EOS;


	/*
	 *   skip the name to get to the string. it CAN start with a " to
	 *   have leading blanks...
	 */
	q = skipwd (q);
	q = skipbl (q);



	/*
	 *   read rest of line from input stream and collect string into
	 *   temp buffer defn
	 */
	if ((i = colstr (q, defn)) == ERR)
	{
		fprintf (err_stream,
			"***%s: string definition too long\n", myname);
		err_exit (-1);
	}


	/*
	 *   store the string
	 */
	if (putstr (name, defn) == ERR)
	{
		fprintf (err_stream,
			"***%s: string definition table full\n", myname);
		err_exit (-1);
	}
}





/*------------------------------*/
/*	colstr			*/
/*------------------------------*/
colstr (p, d)
register char  *p;
char	       *d;
{

/*
 *	Collect string definition from input stream
 */

	register int	i = 0;

	if (*p == '\"')
		p++;

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
/*	putstr			*/
/*------------------------------*/
putstr (name, p)
register char  *name;
register char  *p;
{

/*
 *	Put string definition into (macro) table
 *
 *	NOTE: any expansions of things like number registers SHOULD
 *	have been done already. strings and macros share mb buffer
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
	 *   string looks like this in mb:
	 *
	 *	mac.mb[MACBUF]		size of total buf
	 *	lastp < MXMDEF		number of macros/strings possible
	 *	*mnames[MXMDEF]		-> names, each max length
	 *	...______________________________...____________________...
	 *	    / / /|X|X|0|string definition      |0| / / / / / / /
	 *	.../_/_/_|_|_|_|_________________...___|_|/_/_/_/_/_/_/_...
	 *		    ^
	 *		    |
	 *		    \----- mac.mnames[mac.lastp] points here
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
/*	getstr			*/
/*------------------------------*/
char   *getstr (name)
register char  *name;
{

/*
 *	Get (lookup) string definition from namespace
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






