
/* Added by Jon LaBadie jon@jonlab.UUCP
   to implement the H (hex search) option */

#include	<stdio.h>
#include	<ctype.h>
#include	<curses.h>
#define		beep()		putchar(7)

/*
** hex_2_byte returns the integer value of a byte
** represented by the two characters passed to it.
** For example, passed an '8' and an 'A', it will
** return 170 (8 * 16 + 10).  Returns -1 on any error.
*/

int
hex_2_byte(a, b)
char a, b;
{
	int v = 0;

	if (!isxdigit(a) || !isxdigit(b))
		return -1;
	
	a = toupper(a);
	b = toupper(b);

	if (isdigit(a))
		v = (a - '0') * 16;
	else
		v = (a - 'A' + 10) * 16;
	
	if (isdigit(b))
		v += (b - '0');
	else
		v += (b - 'A' + 10);
	
	return v;
}


/* Take two strings as arguments.
** First is a sequence of hex digit pairs.
** Each pair is to be converted into the
** equivalent unsigned 1 byte value and
** stored in the second array.
*/

int
cvt_str(s, h)
char *s;
unsigned char *h;
{
	int c;
	int len = 0;

	while(*s != '\0')
	{
		if (*(s+1) == '\0')
			return -1;
		c = hex_2_byte(*s, *(s+1));
		if (c >= 0)
			*h++ = c;
		else
			return -1;
		len++;
		s += 2;
	}
	*h = '\0';
	return len;
}

find_hex()
{
	int 	stlen;
	char 	string[60];
	char	*strstart;
static	char	laststring[60];
static	int	re_search = 0, old_filpos;
	unsigned char hexstr[30];
	unsigned char *up;
	int	found;
	int 	searchpos;
	extern	char secbuf[];
	extern	long filpos;

	move(2,0);
	clrtoeol();
	printw("HEX string to search for: ");
	refresh();
	echo();
	string[0] = '0';
	getstr(&string[1]);
	if (strlen(string) == 1) {
		if (strlen(laststring) > 0)
			strcpy(string, laststring);
		else {
			beep();
			return;
		}
	}
	else {
		strcpy(laststring, string);
	}
	noecho();
	move(2,0);
	clrtoeol();
	if (strlen(string) % 2)
		strstart = &string[1];
	else
		strstart = &string[0];
	stlen = cvt_str(strstart, hexstr);
	if (stlen < 0)
	{
		printw("Invalid Hex string: %s", strstart);
		refresh();
		sleep(1);
		return;
	}
	printw("Searching for '%s'", strstart);
	refresh();
	found = 0;
	searchpos = 1;
	while (found == 0) {
		while ((256 - searchpos) >= stlen) {
			if (secbuf[searchpos] != hexstr[0] || memcmp(secbuf + searchpos + 1, hexstr + 1, stlen - 1))
				searchpos++;
			else {
				filpos += searchpos;
#ifdef	CLINES
				if (filpos >= 16*CLINES) {
					filpos -= 16*CLINES;
				}
				else {
					filpos = 0;
				}
#endif	/* context lines */
#ifdef	ALLIGN
				filpos &= ~0xf;
#endif	/* allign */
				re_search = old_filpos - filpos + 1;
				old_filpos = filpos;
				found = 1;
				break;
				}
			}
		if (found == 0) {
			filpos += searchpos;
			searchpos = 0;
			}
		if (rdsec() == 0) {
			found = 1;	
			}
		refresh();
		}
	move (2,0);
	clrtoeol();
}
