/* Generic Internet number representation conversion macros.  */

#undef	ntohl
#undef	ntohs
#undef	htonl
#undef	htons

extern unsigned long int ntohl __P ((unsigned long int));
extern unsigned short int ntohs __P ((unsigned short int));
extern unsigned long int htonl __P ((unsigned long int));
extern unsigned short int htons __P ((unsigned short int));

#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#define	htonl(x)	(x)
#define	htons(x)	(x)
