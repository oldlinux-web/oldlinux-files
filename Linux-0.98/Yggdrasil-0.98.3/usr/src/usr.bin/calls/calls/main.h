/*
 * calls   -- print out the calling struture of a C program.
 *
 *  takes these options:
 *	-a		show all calls (even duplicates)
 *	-e		include externals in index
 *	-i		normal index
 *	-t		terse form, no extra trees output
 *	-v		less verbose index
 *	-w nn		paper width  (default 96)
 *	-f name		function to start printing from
 *	-F name[/file]	static function to start printing from
 *
 *  arguments passed on to CPP:
 *	-D name		#define def
 *	-U name		#undef def
 *	-I file		#include path modifier
 */

#define MAXDEPTH	99		/* max output depth level	*/
#define PAPERWIDTH	96		/* limits tabbing		*/
#define TABWIDTH	8		/* width of a \t		*/

typedef struct CLnode {
	struct CLnode *pCLnext;
	struct HTnode *pHTlist;
} LIST;
#define nilCL	((LIST *) 0)
#define newCL()	((LIST *)malloc(sizeof(LIST)))

extern char sbCmd[];
extern int Allp;
