/*
 * intrface.h -- part of LHa for UNIX
 *
 *	modified : Dec. 5, 1991, Masaru Oki.
 */

typedef short  node;

struct interfacing {
	FILE *infile;
	FILE *outfile;
	unsigned long original;
	unsigned long packed;
	int dicbit;
	int method;
};

extern struct interfacing interface;

/* from error.c
extern void error();
extern void fileerror(); */

/* from slide.c */ 
extern node *next;

extern int encode_alloc();
extern void encode();
extern void decode();

/* from crcio.c */
extern unsigned short crc;

extern void make_crctable();
extern unsigned short calccrc();

/* from append.c */
extern void start_indicator();
extern void finish_indicator();
extern void finish_indicator2();
