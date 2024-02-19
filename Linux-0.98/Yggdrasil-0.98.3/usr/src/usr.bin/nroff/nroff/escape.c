/*
 *	escape.c - Escape and special character input processing portion of
 *	           nroff word processor
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
/*	expesc			*/
/*------------------------------*/
expesc (p, q)
char   *p;
char   *q;
{

/*
 *	Expand escape sequences
 */

	register char  *s;
	register char  *t;
	register char  *pstr;
	register int	i;
	register int	val;
	register int	autoinc;
	char		c;
	char		fs[5];				/* for font change */
	char		nrstr[20];
	char		fmt[20];
	char		name[10];
	int		nreg;
	char	       *pfs;
	int		inc;
	int		tmp;
	char		delim;


	s = p;
	t = q;


	/*
	 *   if escape parsing is not on, just copy string
	 */
	if (dc.escon == NO)
	{
		while (*s != EOS)
		{
			*t++ = *s++;
		}
		*t = EOS;
		strcpy (p, q);

		return;
	}


	/*
	 *   do it...
	 */
	while (*s != EOS)
	{
		if (*s != dc.escchr)
		{
			/*
			 *   not esc, continue...
			 */
			*t++ = *s++;
		}


		else if (*(s + 1) == dc.escchr)
		{
			/*
			 *   \\			escape escape
			 */
			*t++ = *s++;
			++s;
		}


		else if (*(s + 1) == 'n')
		{
			/*
			 *   \nx, \n(xx		register
			 *
			 *   first check for \n+... or \n-... (either form)
			 */
			s += 2;
			autoinc = 0;
			if (*s == '+')
			{
				autoinc = 1;
				s += 1;
			}
			if (*s == '-')
			{
				autoinc = -1;
				s += 1;
			}



			/*
			 *   was this \nx or \n(xx form?
			 */
			if (isalpha (*s))
			{
				/*
				 *   \nx form. find reg (a-z)
				 */
				nreg = tolower (*s) - 'a';


				/*
				 *   was this \n+x or \n-x? if so, do the
				 *   auto incr
				 */
				if (autoinc > 0)
					dc.nr[nreg] += dc.nrauto[nreg];
				else if (autoinc < 0)
					dc.nr[nreg] -= dc.nrauto[nreg];

				/*
				 *   display format
				 */
				if (dc.nrfmt[nreg] == '1')
				{
					/*
					 *   normal decimal digits
					 */
					t += itoda (dc.nr[nreg], t, 6) - 1;
				}
				else if (dc.nrfmt[nreg] == 'i')
				{
					/*
					 *   lower roman
					 */
					t += itoroman (dc.nr[nreg], t, 24) - 1;
				}
				else if (dc.nrfmt[nreg] == 'I')
				{
					/*
					 *   upper roman
					 */
					t += itoROMAN (dc.nr[nreg], t, 24) - 1;
				}
				else if (dc.nrfmt[nreg] == 'a')
				{
					/*
					 *   lower letters
					 */
					t += itoletter (dc.nr[nreg], t, 12) - 1;
				}
				else if (dc.nrfmt[nreg] == 'A')
				{
					/*
					 *   upper letters
					 */
					t += itoLETTER (dc.nr[nreg], t, 12) - 1;
				}
				else if (dc.nrfmt[nreg] & 0x80)
				{
					/*
					 *   zero-filled decimal
					 */
					sprintf (fmt, "%%0%dld",
						(int)(dc.nrfmt[nreg] & 0x7F));
					fmt[5] = '\0';
					sprintf (nrstr, fmt, (long) dc.nr[nreg]);
					tmp = dc.nrfmt[nreg] & 0x7F;
					nrstr[tmp] = '\0';

					strcpy (t, nrstr);
					t += strlen (nrstr);
				}
				else
				{
					/*
					 *   normal (default)
					 */
					t += itoda (dc.nr[nreg], t, 6) - 1;
				}
				++s;
			}
			else if (*s == '%')
			{
				/*
				 *   \n% form. find index into reg struct
				 */
				nreg = findreg ("%");
				if (nreg < 0)
				{
					fprintf (err_stream,
						"***%s: no register match\n",
						myname);
					err_exit (-1);
				}


				/*
				 *   was this \n+% or \n-%? if so, do the
				 *   auto incr
				 */
				if (autoinc > 0)
					rg[nreg].rval += rg[nreg].rauto;
				else if (autoinc < 0)
					rg[nreg].rval -= rg[nreg].rauto;


				/*
				 *   display format
				 */
				if (rg[nreg].rfmt == '1')
				{
					/*
					 *   normal decimal digits
					 */
					t += itoda (rg[nreg].rval, t, 6) - 1;
				}
				else if (rg[nreg].rfmt == 'i')
				{
					/*
					 *   lower roman
					 */
					t += itoroman (rg[nreg].rval, t, 24) - 1;
				}
				else if (rg[nreg].rfmt == 'I')
				{
					/*
					 *   upper roman
					 */
					t += itoROMAN (rg[nreg].rval, t, 24) - 1;
				}
				else if (rg[nreg].rfmt == 'a')
				{
					/*
					 *   lower letters
					 */
					t += itoletter (rg[nreg].rval, t, 12) - 1;
				}
				else if (rg[nreg].rfmt == 'A')
				{
					/*
					 *   upper letters
					 */
					t += itoLETTER (rg[nreg].rval, t, 12) - 1;
				}
				else if (rg[nreg].rfmt & 0x80)
				{
					/*
					 *   zero-filled decimal
					 */
					sprintf (fmt, "%%0%dld",
						(int)(rg[nreg].rfmt & 0x7F));
					fmt[5] = '\0';
					sprintf (nrstr, fmt, (long) rg[nreg].rval);
					tmp = rg[nreg].rfmt & 0x7F;
					nrstr[tmp] = '\0';

					strcpy (t, nrstr);
					t += strlen (nrstr);
				}
				else
				{
					/*
					 *   normal (default)
					 */
					t += itoda (rg[nreg].rval, t, 6) - 1;
				}
				s += 1;
			}
			else if (*s == '(')
			{
				/*
				 *   \n(xx form. find index into reg struct
				 */
				s += 1;
				name[0] = *s;
				name[1] = *(s + 1);
				if (name[1] == ' '  || name[1] == '\t'
				||  name[1] == '\n' || name[1] == '\r')
					name[1] = '\0';
				name[2] = '\0';
				nreg = findreg (name);
				if (nreg < 0)
				{
					fprintf (err_stream,
						"***%s: no register match\n",
						myname);
					err_exit (-1);
				}
				

				/*
				 *   was this \n+(xx or \n-(xx? if so, do the
				 *   auto incr
				 */
				if (rg[nreg].rflag & RF_WRITE)
				{
					if (autoinc > 0)
						rg[nreg].rval += rg[nreg].rauto;
					else if (autoinc < 0)
						rg[nreg].rval -= rg[nreg].rauto;
				}


				/*
				 *   display format
				 */
				if (rg[nreg].rfmt == '1')
				{
					/*
					 *   normal decimal digits
					 */
					t += itoda (rg[nreg].rval, t, 6) - 1;
				}
				else if (rg[nreg].rfmt == 'i')
				{
					/*
					 *   lower roman
					 */
					t += itoroman (rg[nreg].rval, t, 24) - 1;
				}
				else if (rg[nreg].rfmt == 'I')
				{
					/*
					 *   upper roman
					 */
					t += itoROMAN (rg[nreg].rval, t, 24) - 1;
				}
				else if (rg[nreg].rfmt == 'a')
				{
					/*
					 *   lower letters
					 */
					t += itoletter (rg[nreg].rval, t, 12) - 1;
				}
				else if (rg[nreg].rfmt == 'A')
				{
					/*
					 *   upper letters
					 */
					t += itoLETTER (rg[nreg].rval, t, 12) - 1;
				}
				else if (rg[nreg].rfmt & 0x80)
				{
					/*
					 *   zero-filled decimal
					 */
					sprintf (fmt, "%%0%dld",
						(int)(rg[nreg].rfmt & 0x7F));
					fmt[5] = '\0';
					sprintf (nrstr, fmt, (long) rg[nreg].rval);
					tmp = rg[nreg].rfmt & 0x7F;
					nrstr[tmp] = '\0';

					strcpy (t, nrstr);
					t += strlen (nrstr);
				}
				else
				{
					/*
					 *   normal (default)
					 */
					t += itoda (rg[nreg].rval, t, 6) - 1;
				}
				s += 2;
			}
		}


		else if (*(s + 1) == '\"')
		{
			/*
			 *   \"			comment
			 */
			*s = EOS;
			*t = *s;

			return;
		}


		else if (*(s + 1) == '*')
		{
			/*
			 *   \*x, \*(xx		string
			 */
			s += 2;
			if (*s == '(')
			{
				/*
				 *   \*(xx form
				 */
				s += 1;
				name[0] = *s;
				name[1] = *(s + 1);
				name[2] = '\0';
				pstr = getstr (name);
				if (!pstr)
				{
					fprintf (err_stream,
						"***%s: string not found\n",
						myname);
					err_exit (-1);
				}
				while (*pstr)
					*t++ = *pstr++;
				s += 2;
			}
			else
			{
				/*
				 *   \*x form
				 */
				name[0] = *s;
				name[1] = '\0';
				pstr = getstr (name);
				if (!pstr)
				{
					fprintf (err_stream,
						"***%s: string not found\n",
						myname);
					err_exit (-1);
				}
				while (*pstr)
					*t++ = *pstr++;
				s += 1;
			}
		}


		else if (*(s + 1) == 'f')
		{
			/*
			 *   \fx		font
			 */
			s += 2;
			pfs = fs;		/* set up ret string */
			fs[0] = '\0';

			/*
			 *  it parses 1-2 char of s and returns esc seq for
			 *  \fB and \fR (\fI is same as \fB)
			 */
			fontchange (*s, pfs);

			/*
			 *   imbed the atari (vt52) escape seq
			 */
			while (*pfs)
				*t++ = *pfs++;
			++s;			/* skip B,I,R,S,P */
		}


		else if (*(s + 1) == '(')
		{
			/*
			 *   \(xx		special char
			 */
			s  += 2;

			/*
			 *   it returns num char to skip and sets c to
			 *   the ascii value of the char
			 */
			inc = specialchar (s, &c);

			/*
			 *   skip proper num char in s and add c to target
			 */
			if (inc)
			{
				s   += inc;
				*t++ = c;
			}
		}


		else if (*(s + 1) == 'e')
		{
			/*
			 *   \e		printable version of escape
			 */
			*t++ = dc.escchr;
			s   += 2;
		}


		else if (*(s + 1) == '`')
		{
			/*
			 *   \`		grave, like \(ga
			 */
			*t++ = 0x60;
			s  += 2;
		}


		else if (*(s + 1) == '\'')
		{
			/*
			 *   \'		accute, like \(aa
			 */
			s  += 2;
			*t++ = 0xBA;
		}


		else if (*(s + 1) == '-')
		{
			/*
			 *   \-		minus
			 */
			s  += 2;
			*t++ = 0x2D;
		}


		else if (*(s + 1) == '.')
		{
			/*
			 *   \.		period
			 */
			s  += 2;
			*t++ = 0x2E;
		}


		else if (*(s + 1) == ' ')
		{
			/*
			 *   \(space)	space
			 */
			s  += 2;
			*t++ = 0x20;
		}


		else if (*(s + 1) == '0')
		{
			/*
			 *   \0		digital width space
			 */
			s  += 2;
			*t++ = 0x20;
		}


		else if (*(s + 1) == '|')
		{
			/*
			 *   \|		narrow width char (0 in nroff)
			 */
			s  += 2;
		}


		else if (*(s + 1) == '^')
		{
			/*
			 *   \^		narrow width char (0 in nroff)
			 */
			s  += 2;
		}


		else if (*(s + 1) == '&')
		{
			/*
			 *   \&		non-printing zero width
			 */
			s  += 2;
		}


		else if (*(s + 1) == '!')
		{
			/*
			 *   \!		transparent copy line
			 */
			s  += 2;
		}


		else if (*(s + 1) == '$')
		{
			/*
			 *   \$N	interpolate arg 1<=N<=9
			 */
			s  += 2;
		}


		else if (*(s + 1) == '%')
		{
			/*
			 *   \%		hyphen
			 */
			s  += 2;
			*t++ = 0x2D;
			*t++ = 0x2D;
		}


		else if (*(s + 1) == 'a')
		{
			/*
			 *   \a
			 */
			s  += 2;
		}


		else if (*(s + 1) == 'b')
		{
			/*
			 *   \b'abc...'
			 */
			s  += 2;
		}


		else if (*(s + 1) == 'c')
		{
			/*
			 *   \c
			 */
			s  += 2;
		}


		else if (*(s + 1) == 'd')
		{
			/*
			 *   \d
			 */
			s  += 2;
		}


		else if (*(s + 1) == 'h')
		{
			/*
			 *   \h'N'	horiz motion
			 */
			s    += 2;
			delim = *s++;
			val   = atoi (s);
			for (i = 0; i < val; i++)
				*t++ = ' ';
			while (*s != delim)
			{
				if (*s == 0)
					break;
				s++;
			}
			if (*s)
				s++;
			
		}


		else if (*(s + 1) == 'k')
		{
			/*
			 *   \kx
			 */
			s  += 2;
		}


		else if (*(s + 1) == 'l')
		{
			/*
			 *   \l'Nc'
			 */
			s  += 2;
		}


		else if (*(s + 1) == 'L')
		{
			/*
			 *   \L'Nc'
			 */
			s  += 2;
		}


		else if (*(s + 1) == 'o')
		{
			/*
			 *   \o'abc...'		overstrike
			 */
			s  += 2;
			delim = *s++;
			while (*s != EOS && *s != delim)
			{
				*t++ = *s++;
				*t++ = 0x08;
			}
			s++;
		}


		else if (*(s + 1) == 'p')
		{
			/*
			 *   \p
			 */
			s  += 2;
		}


		else if (*(s + 1) == 'r')
		{
			/*
			 *   \r
			 */
			s  += 2;
		}


		else if (*(s + 1) == 's')
		{
			/*
			 *   \sN,\s+-N
			 */
			s  += 2;
		}


		else if (*(s + 1) == 't')
		{
			/*
			 *   \t		horizontal tab
			 */
			s  += 2;
			*t++ = 0x09;
		}


		else if (*(s + 1) == 'u')
		{
			/*
			 *   \u
			 */
			s  += 2;
		}


		else if (*(s + 1) == 'v')
		{
			/*
			 *   \v'N'	vert tab
			 */
			s    += 2;
			delim = *s++;
			val   = atoi (s);
			for (i = 0; i < val; i++)
				*t++ = 0x0A;
			while (*s != delim)
			{
				if (*s == 0)
					break;
				s++;
			}
			if (*s)
				s++;
		}


		else if (*(s + 1) == 'w')
		{
			/*
			 *   \w'str'
			 */
			s  += 2;
		}


		else if (*(s + 1) == 'x')
		{
			/*
			 *   \x'N'
			 */
			s  += 2;
		}


		else if (*(s + 1) == 'z')
		{
			/*
			 *   \zc	print c w/o spacing
			 */
			s  += 2;
			*t++ = *s++;
			*t++ = 0x08;
		}


		else if (*(s + 1) == '{')
		{
			/*
			 *   \{
			 */
			s  += 2;
		}


		else if (*(s + 1) == '}')
		{
			/*
			 *   \}
			 */
			s  += 2;
		}


		else if (*(s + 1) == '\n' || *(s + 1) == '\r')
		{
			/*
			 *   \(newline)	ignore newline
			 */
			s  += 2;
		}


		else
		{
			/*
			 *   \X		any other character not above
			 */
			s   += 1;
			*t++ = *s++;
		}

	}

	/*
	 *   end the string and return it in original buf
	 */
	*t = EOS;
	strcpy (p, q);
}




/*------------------------------*/
/*	specialchar		*/
/*------------------------------*/
specialchar (s, c)
register char  *s;
register char  *c;
{

/*
 *	handles \(xx escape sequences for special characters (atari-specific)
 */

	register char	c1;
	register char	c2;

	c1 = *s;
	c2 = *(s+1);

	/*
	 *   symbols (std font)
	 */
	if (c1 == 'e' && c2 == 'm') {*c = 0x2D; return (2);}	/* dash */
	if (c1 == 'h' && c2 == 'y') {*c = 0x2D; return (2);}	/* hyphen */
	if (c1 == 'b' && c2 == 'u') {*c = 0xF9; return (2);}	/* bullet */
	if (c1 == 's' && c2 == 'q') {*c = 0xF9; return (2);}	/* square */
	if (c1 == 'r' && c2 == 'u') {*c = 0x5F; return (2);}	/* rule */
	if (c1 == '1' && c2 == '2') {*c = 0xAB; return (2);}	/* 1/2 */
	if (c1 == '1' && c2 == '4') {*c = 0xAC; return (2);}	/* 1/4 */
	if (c1 == 'd' && c2 == 'e') {*c = 0xF8; return (2);}	/* degree */
	if (c1 == 'd' && c2 == 'g') {*c = 0xBB; return (2);}	/* dagger */
	if (c1 == 'f' && c2 == 'm') {*c = 0xBA; return (2);}	/* dagger */
	if (c1 == 'c' && c2 == 't') {*c = 0x9B; return (2);}	/* cent */
	if (c1 == 'c' && c2 == 'o') {*c = 0xBD; return (2);}	/* copyrite */
	if (c1 == 'r' && c2 == 'g') {*c = 0xBE; return (2);}	/* registered */
	if (c1 == 't' && c2 == 'm') {*c = 0xBF; return (2);}	/* trademark */

	if (c1 == 'p' && c2 == '2') {*c = 0xFD; return (2);}	/* ^2 */
	if (c1 == 'p' && c2 == '3') {*c = 0xFE; return (2);}	/* ^3 */
	if (c1 == 'p' && c2 == 'n') {*c = 0xFC; return (2);}	/* ^n */
	if (c1 == 'a' && c2 == 'a') {*c = 0xBA; return (2);}	/* acute */
	if (c1 == 'g' && c2 == 'a') {*c = 0x60; return (2);}	/* grave */
	if (c1 == 'd' && c2 == 't') {*c = 0xFA; return (2);}	/* dot */
	if (c1 == 'p' && c2 == 'p') {*c = 0xBC; return (2);}	/* paragraph */
	if (c1 == '^' && c2 == 'g') {*c = 0x07; return (2);}	/* ring bell */
	if (c1 == 'u' && c2 == 'a') {*c = 0x01; return (2);}	/* up arrow */
	if (c1 == 'd' && c2 == 'a') {*c = 0x02; return (2);}	/* dn arrow */
	if (c1 == '-' && c2 == '>') {*c = 0x03; return (2);}	/* rt arrow */
	if (c1 == '<' && c2 == '-') {*c = 0x04; return (2);}	/* lf arrow */
	if (c1 == 'd' && c2 == 'i') {*c = 0xF6; return (2);}	/* divide */
	if (c1 == 's' && c2 == 'r') {*c = 0xFB; return (2);}	/* sq root */
	if (c1 == '=' && c2 == '=') {*c = 0xF0; return (2);}	/* == */
	if (c1 == '>' && c2 == '=') {*c = 0xF2; return (2);}	/* >= */
	if (c1 == '<' && c2 == '=') {*c = 0xF3; return (2);}	/* <= */
	if (c1 == '+' && c2 == '-') {*c = 0xF1; return (2);}	/* +- */
	if (c1 == '~' && c2 == '=') {*c = 0xF7; return (2);}	/* ~= */
	if (c1 == 'a' && c2 == 'p') {*c = 0x7E; return (2);}	/* approx */
	if (c1 == 'n' && c2 == 'o') {*c = 0xAA; return (2);}	/* not */
	if (c1 == 'm' && c2 == 'o') {*c = 0xEE; return (2);}	/* member */
	if (c1 == 'c' && c2 == 'a') {*c = 0xEF; return (2);}	/* intersect */
	if (c1 == 'c' && c2 == 'u') {*c = 0x55; return (2);}	/* union */
	if (c1 == 'i' && c2 == '1') {*c = 0xF4; return (2);}	/* integral1 */
	if (c1 == 'i' && c2 == '2') {*c = 0xF5; return (2);}	/* integral2 */

	if (c1 == 'b' && c2 == 'r') {*c = 0x7C; return (2);}	/* box v rule */
	if (c1 == 'b' && c2 == 'v') {*c = 0x7C; return (2);}	/* bold vert */
	if (c1 == 'p' && c2 == 'l') {*c = 0x2B; return (2);}	/* math plus */
	if (c1 == 'm' && c2 == 'i') {*c = 0x2D; return (2);}	/* math minus */
	if (c1 == 'e' && c2 == 'q') {*c = 0x3D; return (2);}	/* math equal */
	if (c1 == '*' && c2 == '*') {*c = 0x2A; return (2);}	/* math star */
	if (c1 == 's' && c2 == 'l') {*c = 0x2F; return (2);}	/* slash */
	if (c1 == 'u' && c2 == 'l') {*c = 0x5F; return (2);}	/* underrule */
	if (c1 == 's' && c2 == 'c') {*c = 0xDD; return (2);}	/* section */


	/*
	 *   greek
	 */
	if (c1 == '*' && c2 == 'a') {*c = 0xE0; return (2);}	/* alpha */
	if (c1 == '*' && c2 == 'b') {*c = 0xE1; return (2);}	/* beta */
	if (c1 == '*' && c2 == 'g') {*c = 0xE2; return (2);}	/* gamma */
	if (c1 == '*' && c2 == 'd') {*c = 0x7F; return (2);}	/* delta */
	if (c1 == '*' && c2 == 's') {*c = 0xE4; return (2);}	/* sigma */
	if (c1 == '*' && c2 == 'p') {*c = 0xE3; return (2);}	/* pi */
	if (c1 == '*' && c2 == 'm') {*c = 0xE6; return (2);}	/* mu */

	*c = ' ';
	return (0);	
}




/*------------------------------*/
/*	fontchange		*/
/*------------------------------*/
fontchange (fnt, s)
char	fnt;
char   *s;
{

/*
 *	handles \fx font change escapes for R,B,I,S,P (atari-specific)
 *	resets current and last font in dc struct (last used for .ft
 *	with no args)
 */

	int	tmp;

	*s = '\0';
	switch (fnt)
	{
	case 'R':				/* Times Roman */
		if (dc.dofnt == YES)
			strcpy (s, e_standout);
		dc.lastfnt = dc.thisfnt;
		dc.thisfnt = 1;
		break;
	case 'I':				/* Times italic */
		if (dc.dofnt == YES)
			strcpy (s, s_standout);
		dc.lastfnt = dc.thisfnt;
		dc.thisfnt = 2;
		break;
	case 'B':				/* Times bold */
		if (dc.dofnt == YES)
			strcpy (s, s_standout);
		dc.lastfnt = dc.thisfnt;
		dc.thisfnt = 3;
		break;
	case 'S':				/* math/special */
		*s = '\0';
		dc.lastfnt = dc.thisfnt;
		dc.thisfnt = 4;
		break;
	case 'P':				/* previous (exchange) */
		if (dc.dofnt == YES)
		{
			if (dc.lastfnt == 1)
				strcpy (s, e_standout);	/* to R */
			else if (dc.lastfnt == 2)
				strcpy (s, s_standout);	/* to I */
			else if (dc.lastfnt == 3)
				strcpy (s, s_standout);	/* to B */
			else
				*s = '\0';		/* nothing */
		}

		tmp        = dc.thisfnt;		/* swap this/last */
		dc.thisfnt = dc.lastfnt;
		dc.lastfnt = tmp;
		break;
	default:
		*s = '\0';
		break;
	}

	set_ireg (".f", dc.thisfnt, 0);
}





/*------------------------------*/
/*	findreg			*/
/*------------------------------*/
findreg (name)
register char  *name;
{

/*
 *	find register named 'name' in pool. return index into array or -1
 *	if not found.
 */

	register int	i;
	register char  *prname;

	for (i = 0; i < MAXREGS; i++)
	{
		prname = rg[i].rname;
		if (*prname == *name && *(prname + 1) == *(name + 1))
			break;
	}

	return ((i < MAXREGS) ? i : -1);
}




