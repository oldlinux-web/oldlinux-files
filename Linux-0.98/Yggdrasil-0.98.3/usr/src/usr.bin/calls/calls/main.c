/*
 * main.c -- calls mainline, trace calling sequences of C programs
 */

#ifdef pdp11
#include <sys/types.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <strings.h>

#include "scan.h"
#include "getopt.h"
#include "main.h"

/* globals */
char
	sbCmd[] = "cmd line";	/* kludge to notify user of error	*/
int
	Allp = 0,		/* flag to show *all* function calls	*/
	Tersep = 0,		/* only requested trees			*/
	Externp = 0,		/* include externs in index		*/
	Indexp = 0,		/* output functions index		*/
	Verbosep = 1;		/* index functions not output		*/

/* locals */
static int
	linect = 0,		/* line number				*/
	activep = 0,		/* current function being output	*/
	iWidth = PAPERWIDTH;	/* default paper width			*/
static char
	*pchProg,		/* argv[0]				*/
	cppcommand[1024] =	/* cpp command string			*/
		"/lib/cpp -P ";
static HASH
	*activelist[MAXDEPTH];	/* list of current output names		*/


char *
syserrlist()	/*  the routine returns a string for a system error	*/
{
	extern char *sys_errlist[];
	extern int sys_nerr;
	extern int errno;
	register char *pchErr =
	    errno == 0 ? "errno = 0" :
		errno < sys_nerr ? sys_errlist[errno] : "errno out of range";

	errno = 0;
	return pchErr;
}

void
process(filename, outname)	/* invoke cpp on file, call level1	*/
register char *filename, *outname;
{
	extern FILE *popen();
	register char *sbNull;
	register int ret;

	if (access(filename, 04) != 0) {
		(void) fprintf(stderr, "%s: cannot open file '%s' (%s).\n", pchProg, filename, syserrlist());
		return;
	}
	sbNull = cppcommand + strlen(cppcommand);
	strcpy(sbNull, filename);
	if (NULL == (input = popen(cppcommand, "r"))) {
		(void) fprintf(stderr, "%s: fork of CPP command '%s' failed on file '%s' (%s).\n", pchProg, cppcommand, filename, syserrlist());
	} else {
		level1(outname);
		if (0 != (ret = pclose(input)))
			(void) fprintf(stderr, "%s: CPP command '%s' failed on file '%s' with return code %d.\n", pchProg, cppcommand, filename, ret);
	}
	*sbNull = '\0';
	return;
}

void
dostdin()	/* copy stdin to temp file, call process on file	*/
{
	extern char *mktemp();
	register int cc;
	register char *filename = mktemp("/tmp/callsXXXXXX");
	register FILE *ofileptr = fopen(filename, "w");
	register char *sbNull;

	if (NULL == ofileptr) {
		(void) fprintf(stderr, "%s: cannot open tempfile '%s' for writing (%s).\n", pchProg, filename, syserrlist());
	} else {
		while (EOF != (cc = getchar()))
			putc(cc, ofileptr);
		fclose(ofileptr);
		sbNull = cppcommand + strlen(cppcommand);
		strcpy(sbNull, "-I. ");
		process(filename, "stdin");
		*sbNull = '\000';
		unlink(filename);
	}
}

int
active(func)	/* check for recursive calls, prevents endless output	*/
register HASH *func;
{
	register int i;

	for (i = 0; i < activep-1; i++)
		if (func == activelist[i])
			return 1;
	return 0;
}

void
output(pHTFunc, tabc)	/* output a (sub)tree in pretty form		*/
register HASH *pHTFunc;
register int tabc;
{
	static char dashes[] = "\n----------";
	register INST *pINTemp;
	register int i;

	++linect;
	(void) printf("\n%5d\t", linect);
	if (activep < MAXDEPTH) {
		activelist[activep++] = pHTFunc;
	} else {
		(void) printf("   * nesting is too deep");
		return;
	}

	for (i = 0; i < tabc; i++ )
		putchar('\t');
	printf("%s", pHTFunc->pchname);

	if (active(pHTFunc)) {
		(void) printf(" <<< recursive >>>");
	} else if (pHTFunc->pchfile) {
		pINTemp = pHTFunc->pINcalls;
		if (pHTFunc->listp && tabc && 0 == pHTFunc->iline) {
			(void) printf(" [%s] [see below]", pHTFunc->pchfile);
		} else if (! pHTFunc->iline) {
			(void) printf(pHTFunc->localp ? " [static in %s]" : " [%s]", pHTFunc->pchfile);
			pHTFunc->iline = linect;
			if ((++tabc) * TABWIDTH >= iWidth) {
				(void) printf(dashes);
				tabc = 0;
			}
			while (pINTemp) {
				output(pINTemp->pHTname, tabc);
				pINTemp = pINTemp->pINnext;
			}
			if (! tabc)
				(void) printf(dashes);
		} else if (pINTemp || pHTFunc->localp) {
			(void) printf(" [see line %d]", pHTFunc->iline);
		}
	}
	activelist[activep--] = nilHASH;
}

int
main(argc, argv)	/* parse args, add files, call output		*/
int argc;
char *argv[];
{
	extern int atoi();
	static char sbOpts[] = "aehitvw:D:f:F:U:I:"; /* valid options	*/
	static char sbTemp[200];
	static LIST *pCLRoot;
	register HASH *pHTList;
	register int cOption;
	register LIST **ppCL;
	register char *pchSplit;

#ifndef pdp11
	setlinebuf(stdout);
#endif
	pchProg = argv[0];
	ppCL = & pCLRoot;

	while (EOF != (cOption = getopt(argc, argv, sbOpts))) {
		switch (cOption) {
		case 'a':
			Allp = 1;
			break;
		case 'e':
			Externp = 1;
			Indexp = 1;
			break;
		case 'F':
			if (0 != (pchSplit = index(optarg, '/'))) {
				*pchSplit++ = '\000';
			} else {
		case 'f':	
				pchSplit = sbCmd;
			}
			pHTList = search(optarg, 'F' == cOption, pchSplit);
			pHTList->listp = 1;
			pHTList->pchfile = pchSplit;
			*ppCL = newCL();
			(*ppCL)->pCLnext = pCLRoot;
			(*ppCL)->pHTlist = pHTList;
			ppCL = & (*ppCL)->pCLnext;
			break;
		case 't':	
			Tersep = 1;
			break;
		case 'v':
			Verbosep = 0;
			/*fallthrough*/
		case 'i':
			Indexp = 1;
			break;
		case 'w':
			if (0 >= (iWidth = atoi(optarg)))
				iWidth = PAPERWIDTH;
			break;
		case 'D':
			strcat(cppcommand, (const char *)
						sprintf(sbTemp, "-D%s ", optarg));
			break;
		case 'I':
			strcat(cppcommand, (const char *)
						sprintf(sbTemp, "-I%s ", optarg));
			break;
		case 'U':
			strcat(cppcommand, (const char *)
						sprintf(sbTemp, "-U%s ", optarg));
			break;
		case '?':
		case 'h':
			(void) fprintf(stderr, "usage: %s [-aehitv] [-f function] [-F function[/file.c]] [-w width]\n\
    [-D define] [-U undefine] [-I include-dir] [filename|-]*\n\
\ta\tprint all calls in every function body\n\
\te\tindex external functions too\n\
\tf,F\tstart calling trace at given function\n\
\th\tprint this message\n\
\ti\tprint an index of defined functions\n\
\tv\tlist only called functions in index output\n\
\tt\tterse, list only trees that are requested\n\
\tw\tset ouptut width\n\
\tD,U,I\tas in cpp\n", pchProg);
			exit('h' != cOption);
		}
	}
	*ppCL = nilCL;

	while (EOF != getarg(argc, argv)) {
		if ('-' == optarg[0] && '\000' == optarg[1])
			dostdin();
		else
			process(optarg, optarg);
	}

	while (pCLRoot) {		/* print requested trees	*/
		output(pCLRoot->pHTlist, 0);
		putchar('\n');
		pCLRoot = pCLRoot->pCLnext;
	}

	if (!Tersep) {			/* print other trees		*/
		for (cOption = 0; cOption < 2; ++cOption) {
			for (pHTList = pHTRoot[cOption]; pHTList; pHTList = pHTList->pHTnext) {
				if (!pHTList->calledp && NULL != pHTList->pchfile && 0 == pHTList->iline) {
					output(pHTList, 0);
					putchar('\n');
				}
			}
		}
	}

	if (Indexp) {			/* print index			*/
		printf("\fIndex:\n");
		while (nilHASH != pHTRoot[0] || nilHASH != pHTRoot[1]) {
			
			if (nilHASH == pHTRoot[0] || (nilHASH != pHTRoot[1] && strcmp(pHTRoot[0]->pchname, pHTRoot[1]->pchname) >= 0)) {
				pHTList = pHTRoot[1];
				pHTRoot[1] = pHTRoot[1]->pHTnext;
			} else {
				pHTList = pHTRoot[0];
				pHTRoot[0] = pHTRoot[0]->pHTnext;
			}
			if (!Externp && NULL == pHTList->pchfile)
				continue;
			if (!Verbosep && 0 == pHTList->iline)
				continue;
			putchar('\t');
			fputs(pHTList->pchname, stdout);
			if (pHTList->localp) {
				printf(" [static in %s]", pHTList->pchfile);
			} else if (((char *) 0) != pHTList->pchfile) {
				printf(" [%s]", pHTList->pchfile);
			}
			if (0 != pHTList->iline) {
				printf(" [see line %d]", pHTList->iline);
			}
			putchar('\n');
		}
	}
	exit(0);
}
