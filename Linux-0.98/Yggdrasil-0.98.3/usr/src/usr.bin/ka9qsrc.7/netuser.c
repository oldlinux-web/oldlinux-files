/* Miscellaneous format conversion subroutines */

#include <ctype.h>
#include <stdio.h>
#include "global.h"
#include "netuser.h"
#ifdef UNIX
#include "unix.h"
#include <string.h>
#endif /* UNIX */
#ifdef	BSD
char *sprintf();
#endif

int net_error;

#define LINELEN 256

/* Convert Internet address in ascii dotted-decimal format (44.0.0.1) to
 * binary IP address
 */
int32
aton(s)
register char *s;
{
	int32 n;
	int atoi();
	register int i;

	n = 0;
	for(i=24;i>=0;i -= 8){
		n |= (int32)atoi(s) << i;
		if((s = index(s,'.')) == NULLCHAR)
		break;
		s++;
	}
	return n;
}
/* Resolve a host name into an IP address. IP addresses in dotted-decimal
 * notation are distinguished from domain names by enclosing them in
 * brackets, e.g., [44.64.0.1]
 */
int32
xresolve(host)
char *host;
{
	register char *cp,*cp1;
	int i;
	char hostent[LINELEN];
	FILE *sfile;
	static struct {
		char *name;
		int32 address;
	} cache;

	if(*host == '['){
		/* Brackets indicate IP address in dotted-decimal form */
		return aton(host + 1);
	}
	if(cache.name != NULLCHAR && strcmp(cache.name,host) == 0)
		return cache.address;

	/* Not a numerical IP address, search the host table */
	if((sfile = fopen(hosts,"r")) == NULL){
		return 0;
	}
	while (!feof(sfile)){
		fgets(hostent,LINELEN,sfile);
		rip(hostent);
		cp = hostent;
		if(*cp == '#' || !isdigit(*cp))
			continue;	/* Comment or invalid line */
		while(cp != NULLCHAR){
			/* Skip white space */
			while(*cp == ' ' || *cp == '\t')
				cp++;
			if(*cp == '\0')
				break;
			/* Look for next token, find length of this one */
			if((cp1 = index(cp,'\t')) != NULLCHAR){
				i = cp1 - cp;
			} else if((cp1 = index(cp,' ')) != NULLCHAR) {
				i = cp1 - cp;
			} else {
				i = strlen(cp);
			}
			if(strlen(host) == i && strncasecmp(host,cp,i) == 0){
				/* Found it, put in cache */
				fclose(sfile);
				if(cache.name != NULLCHAR)
					free(cache.name);
				cache.name = malloc((unsigned)strlen(host)+1);
				strcpy(cache.name,host);
				cache.address = aton(hostent);
				return cache.address;
			}
			/* That one didn't match, try the next one */
			cp = cp1;
		}
	}
	/* No address found */
	fclose(sfile);
	return 0;
}

/* Convert an internet address (in host byte order) to a dotted decimal ascii
 * string, e.g., 255.255.255.255\0
 */
char *
inet_ntoa(a)
int32 a;
{
	static char buf[16];

	sprintf(buf,"%u.%u.%u.%u",
		hibyte(hiword(a)),
		lobyte(hiword(a)),
		hibyte(loword(a)),
		lobyte(loword(a)) );
	return buf;
}
/* Convert a socket (address + port) to an ascii string of the form
 * aaa.aaa.aaa.aaa:ppppp
 */
char *
psocket(s)
struct socket *s;
{
	static char buf[30];

	sprintf(buf,"%s:%u",inet_ntoa(s->address),s->port);
	return buf;
}
/* Convert hex-ascii string to long integer */
long
htol(s)
char *s;
{
	long ret;
	char c;

	ret = 0;
	while((c = *s++) != '\0'){
#if	(!ATARI_ST && !LATTICE)	/* DG2KK "ik versta er heelemal niets van!" */
		c &= 0x7f;
#endif
		if(c >= '0' && c <= '9')
			ret = ret*16 + (c - '0');
		else if(c >= 'a' && c <= 'f')
			ret = ret*16 + (10 + c - 'a');
		else if(c >= 'A' && c <= 'F')
			ret = ret*16 + (10 + c - 'A');
		else
			break;
	}
	return ret;
}
