/*
 * scan.h -- scanner for calls
 *	<stdio> must be included before this file, and
 *	"main.h" is assumed the main.h defines BUCKET, also included before
 */

#define LCURLY	  '{'	        /*}*/	/* messes with vi's mind 	*/
#define RCURLY	 /*{*/ 	 	 '}'	/* to have curly in text	*/
#define LPAREN	  '('		/*)*/	/* same mess			*/
#define RPAREN   /*(*/		 ')'	/* as above			*/
#define LBRACK	  '['		/*]*/	/* more mess implies		*/
#define RBRACK	 /*[*/		 ']'	/* more mass			*/
#define BUCKET		100		/* number of objects to alloc	*/
#define MAXCHARS	80		/* max number of chars in ident	*/

typedef struct INnode {
	struct HTnode *pHTname;		/* namep;			*/
	struct INnode *pINnext;		/* pnext			*/
} INST;
#define nilINST	((INST *) 0)

typedef struct HTnode {
	char *pchname, *pchfile;	/* name & file declared		*/
	struct HTnode *pHTnext;		/* next in table (list)		*/
	struct INnode *pINcalls;	/* list of calls		*/
	short int
		listp,			/* 0 = don't, 1 = do, 2 = done	*/
		calledp,		/* have we ever been called	*/
		iline,			/* line output on		*/
		localp;			/* crude static function flag	*/
} HASH, *PHT;
#define nilHASH	((HASH *) 0)

extern void level1();
extern FILE *input;
extern HASH *newHASH(), *search(), *pHTRoot[2];
extern INST *newINST();
