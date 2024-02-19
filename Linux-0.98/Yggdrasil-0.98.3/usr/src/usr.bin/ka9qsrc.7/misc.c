/* Miscellaneous machine independent utilities */

#include "global.h"
#include <ctype.h>
#ifdef	UNIX
#undef	toupper
#undef	tolower
#endif

/* Convert hex-ascii to integer */
int
htoi(s)
char *s;
{
	int i = 0;
	char c;

	while((c = *s++) != '\0'){
		if(c == 'x')
			continue;	/* allow 0x notation */
		if('0' <= c && c <= '9')
			i = (i * 16) + (c - '0');
		else if('a' <= c && c <= 'f')
			i = (i * 16) + (c - 'a' + 10);
		else if('A' <= c && c <= 'F')
			i = (i * 16) + (c - 'A' + 10);
		else
			break;
	}
	return i;
}
/* replace terminating end of line marker(s) with null */
rip(s)
register char *s;
{
	register char *cp;

	if((cp = index(s,'\r')) != NULLCHAR)
		*cp = '\0';
	if((cp = index(s,'\n')) != NULLCHAR)
		*cp = '\0';
}
/* Case-insensitive string comparison */
strncasecmp(a,b,n)
register char *a,*b;
register int n;
{
	char a1,b1;

	while(n-- != 0 && (a1 = *a++) != '\0' && (b1 = *b++) != '\0'){
		if(a1 == b1)
			continue;	/* No need to convert */
		a1 = tolower(a1);
		b1 = tolower(b1);
		if(a1 == b1)
			continue;	/* NOW they match! */
		if(a1 > b1)
			return 1;
		if(a1 < b1)
			return -1;
	}
	return 0;
}

int16
get16(cp)
register char *cp;
{
	register int16 x;

	x = uchar(*cp++);
	x <<= 8;
	x |= uchar(*cp);
	return x;
}

/* Machine-independent, alignment insensitive network-to-host long conversion */
int32
get32(cp)
register char *cp;
{
	int32 rval;

	rval = uchar(*cp++);
	rval <<= 8;
	rval |= uchar(*cp++);
	rval <<= 8;
	rval |= uchar(*cp++);
	rval <<= 8;
	rval |= uchar(*cp);

	return rval;
}

/* Copy a string to a malloc'ed buffer. Turbo C has this one in its
 * library, but it doesn't call mallocw() and can therefore return NULL.
 * NOS uses of strdup() generally don't check for NULL, so they need this one.
 */
char *
strdup(s)
const char *s;
{
	register char *out;
	register int len;

	if(s == NULLCHAR)
		return NULLCHAR;
	len = strlen(s);
	out = malloc(len+1);
	if (!out) {
		printf("strdup: malloc failed\n");
		exit(1);
	}
	/* This is probably a tad faster than strcpy, since we know the len */
	memcpy(out,s,len);
	out[len] = '\0';
	return out;
}
