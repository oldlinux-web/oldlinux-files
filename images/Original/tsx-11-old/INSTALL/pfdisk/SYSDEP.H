/* communicate declarations from the files: s_*.c */

#define SECSIZE 0x200

extern int usage();		/* print a usage message */
	/* (char *progname) */

extern void getGeometry(); /* determine disk parameters */
	/* (char *dev, uint *cyls, uint *heads, uint *sectors) */

extern int getFile();	/* open, read, close, return(num-read) */
	/* (char *name, char *buf, int len) */

extern int putFile();	/* open, write, close, return(num-writen) */
	/* (char *name, char *buf, int len) */

extern int getBBlk();	/* open, read, close, return(num-read) */
	/* (char *dev, char *buf) */

extern int putBBlk();	/* open, write, close, return(num-writen) */
	/* (char *dev, char *buf) */

