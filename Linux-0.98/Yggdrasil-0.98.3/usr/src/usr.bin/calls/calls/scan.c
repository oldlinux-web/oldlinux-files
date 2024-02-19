/*
 * scan.c -- a simple scanner for C, pulls out the function
 *	calling pattern	(all by KSB)
 */

#ifdef pdp11
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <strings.h>
#define strsave(X)	strcpy((char *)malloc(strlen((X))+1), (X))

#include "scan.h"
#include "main.h"

int c;				/* parser look ahead			*/
FILE *input;			/* our input file pointer		*/
HASH *pHTRoot[2] =	 	/* our list of idents			*/
	{nilHASH, nilHASH};
static char 
	AUTO[] = "auto",	BREAK[] = "break",	CASE[] = "case",
	CHAR[] = "char",	CONTINUE[] = "continue",DEFAULT[] = "default",
	DO[] = "do",		DOUBLE[] = "double",	ELSE[] = "else",
	ENUM[] = "enum",	EXTERN[] = "extern",	FLOAT[] = "float",
	FOR[] = "for",		FORTRAN[] = "fortran",	GOTO[] = "goto",
	IF[] = "if",		INT[] = "int",		LONG[] = "long",
	REGISTER[] = "register",RETURN[] = "return",	SHORT[] = "short",
	SIZEOF[] = "sizeof",	STATIC[] = "static",	STRUCT[] = "struct",
	SWITCH[] = "switch",	TYPEDEF[] = "typedef",	UNION[] = "union",
	UNSIGNED[] = "unsigned",VOID[] = "void",	WHILE[] = "while";

HASH *
newHASH()	/* get a new hash node					*/
{
	extern char *calloc();
	register HASH *pHTRet;
	static HASH *pHTQueue = nilHASH;

	if (nilHASH == pHTQueue) {
		if (!(pHTRet = (HASH *)calloc(BUCKET, sizeof(HASH)))) {
			(void) fprintf(stderr, "out of mem\n");
			exit(2);
		}
		pHTQueue = (pHTRet+(BUCKET-1))->pHTnext = pHTRet;
	}
	pHTRet = pHTQueue;
	pHTQueue = pHTRet->pHTnext ? nilHASH : pHTRet+1;
	return pHTRet;
}

HASH *
search(name, Localp, pchFile)	/* translate name to hash node		*/
register char *name;
int Localp;			/* -> trying to make a local def	*/
char *pchFile;
{
	register HASH **ppHT, *pHT;
	register int i = 1;

	ppHT = & pHTRoot[1];	/* first search statics	*/
	while((pHT = *ppHT) && (i = strcmp(pHT->pchname, name)) <= 0) {
		if (0 == i && 0 == strcmp(pchFile, pHT->pchfile))
			break;	/* found a visible static function	*/
		ppHT = & pHT->pHTnext;
		i = 1;
	}

	if (0 != i && ! Localp) {
		ppHT = & pHTRoot[0];
		while((pHT = *ppHT) && (i = strcmp(pHT->pchname, name)) < 0)
			ppHT = & pHT->pHTnext;
	}

	if (0 != i) {
		pHT = newHASH();
		pHT->pchname = strsave(name);
#ifdef BADCALLOC		/* calloc does not zero mem?		*/
		pHT->pchfile = (char *) 0;
		pHT->listp = 0;
		pHT->calledp = 0;
		pHT->pINcalls = nilINST;
#endif BADCALLOC
		pHT->localp = Localp;
		pHT->pHTnext = *ppHT;
		*ppHT = pHT;
	}
	return pHT;
}

/*
 * here we don't assume that cpp takes out comments, really
 * paranoid of us, but I think that way
 * f is a flag we use to make the look ahead come out right
 * in all cases
 */
void
eatwhite(f)	/* skip blanks, comments, "strings", 'chars' in input	*/
register int f;
{
	if (f)
		c = getc(input);
	for(/* void */; /* c != EOF */; c = getc(input)) {
		if (isspace(c) || c == '\b') {
			continue;
		} else if ('/' == c) {		/* start of comment? */
			if ('*' == (c = getc(input))) {
				c = getc(input);	/* eat comment */
				for(;;) {
					while (c != '*')
						c = getc(input);
					if ('/' == (c = getc(input)))
						break;
				}
			} else {
				ungetc(c, input);
				c = '/';
				break;
			}
		} else if ('\'' == c || '"' == c) {
			while(c != (f = getc(input))) {
				if ('\\' == f)
					getc(input);
			}
		} else if ('#' == c) {
			while ('\n' != getc(input))
				/* void */;
		} else {
			break;
		}
	}
}

void
balance(l, r)	/* find balancing character				*/
register int l, r;
{
	register int brace = 1;

	do
		eatwhite(1);
	while (brace += (l == c) - (r == c));
}

int
getid(sb, ppchToken)	/* return 0 = var, 1 == func, 2 == keyword	*/
register char *sb;
char **ppchToken;
{
	static char *keywords[] = {
		AUTO, BREAK, CASE, CHAR, CONTINUE, DEFAULT,
		DO, DOUBLE, ELSE, ENUM, EXTERN, FLOAT, FOR,
		FORTRAN, GOTO, IF, INT, LONG, REGISTER,
		RETURN, SHORT, SIZEOF, STATIC, STRUCT, SWITCH,
		TYPEDEF, UNION, UNSIGNED, VOID, WHILE, (char *)0
	};
	register int i = 0;
	register char **psbKey = keywords;

	do {
		if (i < MAXCHARS)
			sb[i++] = c;
		c = getc(input);
	} while (isalpha(c) || isdigit(c) || '_' == c);
	sb[i] = '\000';		/* buffer really goes to MAXCHARS+1	*/
	eatwhite(0);	/* c == next char after id */

	while (*psbKey && 0 != strcmp(*psbKey, sb))
		++psbKey;

	if (*psbKey) {
		*ppchToken = *psbKey;
		return 2;
	}

	return LPAREN == c;
}

void
eatdecl(sb)	/* eat anything that starts with any keyword listed	*/
register char *sb;
{
	static char *which[] = {	/* keywords mark a declaration	*/
		AUTO, CHAR, STATIC, DOUBLE, ENUM, EXTERN, FLOAT, INT,
		LONG, REGISTER, SHORT, STATIC, STRUCT, TYPEDEF, UNION,
		UNSIGNED, VOID, (char *) 0};
	register char **psb = which;

	while(*psb)
		if (*psb++ == sb)
			break;
	if (*psb) {
		while ('=' != c && ';' != c && RPAREN != c) {
			if (LCURLY == c)
				balance(LCURLY, RCURLY);
			else if (LPAREN == c) {
				balance(LPAREN, RPAREN);
			}
			eatwhite(1);
		}
	}
}

INST *
newINST()	/* get a new instaniation  node				*/
{
	extern char *calloc();
	register INST *pINRet;
	static INST *pINQueue = nilINST;

	if (nilINST == pINQueue) {
		if (!(pINRet = (INST *)calloc(BUCKET, sizeof(INST)))) {
			(void) fprintf(stderr, "out of mem\n");
			exit(2);
		}
		pINQueue = (pINRet+(BUCKET-1))->pINnext = pINRet;
	}
	pINRet = pINQueue;
	pINQueue = pINRet->pINnext ? nilINST : pINRet+1;
	return pINRet;
}

void
level2(pHTCaller, pchFile)	/* inside a function looking for calls	*/
HASH *pHTCaller;
char *pchFile;
{
	static char buffer[MAXCHARS+1];
	register struct INnode *pINLoop;
	register int brace = 0;
	register HASH *pHTFound;
	register struct INnode **ppIN = & (pHTCaller->pINcalls);
	register int declp = 1;		/* eating declarations		*/
	auto char *pchToken;

	while (brace || declp) {
		if (isalpha(c) || '_' == c) {
			switch (getid(buffer, & pchToken)) {
			case 1:
				pHTFound = search(buffer, 0, pchFile);
				if (Allp)
					goto regardless;
				for(pINLoop = pHTCaller->pINcalls;
				    pINLoop;
				    pINLoop = pINLoop->pINnext)
					if (pHTFound == pINLoop->pHTname)
						break;
				if (! pINLoop) {
			regardless:
					pINLoop = *ppIN = newINST();
					pINLoop->pHTname = pHTFound;
					ppIN = & pINLoop->pINnext;
				}
				++pHTFound->calledp;
				break;
			case 2:
				eatdecl(pchToken);
				/* fall through */
			case 0:
				break;
			}
		} else {
			if (LCURLY == c)
				declp = 0, ++brace;
			else if (RCURLY == c)
				--brace;
			eatwhite(1);
		}
	}
	*ppIN = nilINST;
}

void
level1(filename)	/* in a C source program, looking for fnx(){..}	*/
register char *filename;
{
	static char buffer[MAXCHARS+1];
	static char *pchToken;
	register HASH *pHTTemp;
	register int parens = 0;
	register int Localp = 0;

	c = ' ';

	do {		/* looking to a function decl	*/
		if (isalpha(c) || '_' == c) {
			switch (getid(buffer, & pchToken)) {
			case 1:
				while (parens += (LPAREN == c) - (RPAREN == c))
					eatwhite(1);
				for (;;) {	/* eat complex stuff	*/
					eatwhite(1);
					if (LPAREN == c) {
						balance(LPAREN, RPAREN);
						continue;
					} else if (LBRACK == c) {
						balance(LBRACK, RBRACK);
						continue;
					} else {
						break;
					}
				}
				pHTTemp = search(buffer, Localp, filename);
				if (',' == c || ';' == c) {
					Localp = 0;
					break;
				}
				if (pHTTemp->pchfile && pHTTemp->pchfile != sbCmd &&
				    (pHTTemp->pchfile == filename ||
				    0 != strcmp(pHTTemp->pchfile, filename))) {
					fprintf(stderr, "%s is multiply defined [%s, %s]\n", pHTTemp->pchname, pHTTemp->pchfile, filename);
					exit(5);
				} else {
					pHTTemp->pchfile = filename;
					Localp = 0;
					level2(pHTTemp, filename);
				}
				continue;
			case 2:
				if (STATIC == pchToken)
					Localp = 1;
			case 0:
				continue;
			}
		} else if (LCURLY == c) {
			balance(LCURLY, RCURLY);
		} else if (LPAREN == c) {
			++parens;
		} else if (RPAREN == c) {
			--parens;
		} else if ('*' != c) {
			Localp = 0;
		}
		eatwhite(1);
	} while (EOF != c);
}
