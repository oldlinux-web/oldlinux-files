#include <sys/types.h>
#include <sys/uio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>

extern void _sethtent(int f);
extern void _endhtent(void);
extern struct hostent *_gethtent(void);
extern struct hostent *_gethtbyname(const char *name);
extern struct hostent *_gethtbyaddr(const char *addr, int len,
		 int type);
extern int _validuser(FILE *hostf, const char *rhost,
		 const char *luser, const char *ruser, int baselen);
extern int _checkhost(const char *rhost, const char *lhost, int len);
extern void putlong(u_long l, u_char *msgp);
extern void putshort(u_short l, u_char *msgp);
extern u_long _getlong(u_char *msgp);
extern u_short _getshort(u_char *msgp);
extern void p_query(char *msg);
extern void fp_query(char *msg, FILE *file);
extern char *p_cdname(char *cp, char *msg, FILE *file);
extern char *p_rr(char *cp, char *msg, FILE *file);
extern char *p_type(int type);
extern char * p_class(int class);
extern char *p_time(u_long value);
extern char * hostalias(const char *name);
extern void sethostfile(char *name);
extern void _res_close (void);
extern void ruserpass(const char *host, char **aname, char **apass);
