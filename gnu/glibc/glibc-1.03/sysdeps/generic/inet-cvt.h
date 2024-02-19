/* Generic Internet number representation conversion macros.  */

#undef	ntohl
#undef	ntohs
#undef	htonl
#undef	htons

extern unsigned long int EXFUN(ntohl, (unsigned long int));
extern unsigned short int EXFUN(ntohs, (unsigned short int));
extern unsigned long int EXFUN(htonl, (unsigned long int));
extern unsigned short int EXFUN(htons, (unsigned short int));

#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#define	htonl(x)	(x)
#define	htons(x)	(x)
