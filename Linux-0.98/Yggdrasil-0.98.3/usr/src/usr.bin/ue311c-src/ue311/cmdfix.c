/*	CMDFIX.C:	Translate pre 3.10 command and startup
			files to 3.10 files
			(C)Copyright 1988 by Daniel Lawrence
			All Rights reserved

	The names of function key command strings have been changed to
	help make these names machine independant.  This program translates
	old files to new.

*/

#include <stdio.h>
#include "estruct.h"

pascal fix();

/*	Table of command key name changes */

char *trans[][2] = {
	"FN ",	"A-D",
	"FN!",	"A-F",
	"FN#",	"A-H",
	"FN$",	"A-J",
	"FN%",	"A-K",
	"FN&",	"A-L",
	"FN,",	"A-Z",
	"FN-",	"A-X",
	"FN.",	"A-C",
	"FN/",	"A-V",
	"FN0",	"A-B",
	"FN1",	"A-N",
	"FN2",	"A-M",
	"FN;",	"FN1",
	"FN<",	"FN2",
	"FN=",	"FN3",
	"FN>",	"FN4",
	"FN?",	"FN5",
	"FN@",	"FN6",
	"FNA",	"FN7",
	"FNB",	"FN8",
	"FNC",	"FN9",
	"FND",	"FN0",
	"FNG",	"FN<",
	"FNH",	"FNP",
	"FNI",	"FNZ",
	"FNK",	"FNB",
	"FNM",	"FNF",
	"FNO",	"FN>",
	"FNP",	"FNN",
	"FNQ",	"FNV",
	"FNR",	"FNC",
	"FNS",	"FND",
	"FNT",	"S-FN1",
	"FNU",	"S-FN2",
	"FNV",	"S-FN3",
	"FNW",	"S-FN4",
	"FNX",	"S-FN5",
	"FNY",	"S-FN6",
	"FNZ",	"S-FN7",
	"FN[",	"S-FN8",
	"FN\"",	"A-G",
	"FN\\",	"S-FN9",
	"FN]",	"S-FN0",
	"FN^",	"FN^1",
	"FN_",	"FN^2",
	"FN`",	"FN^3",
	"FNa",	"FN^4",
	"FNb",	"FN^5",
	"FNc",	"FN^6",
	"FNd",	"FN^7",
	"FNe",	"FN^8",
	"FNf",	"FN^9",
	"FNg",	"FN^0",
	"FNh",	"A-FN1",
	"FNi",	"A-FN2",
	"FNj",	"A-FN3",
	"FNk",	"A-FN4",
	"FNl",	"A-FN5",
	"FNm",	"A-FN6",
	"FNn",	"A-FN7",
	"FNo",	"A-FN8",
	"FNp",	"A-FN9",
	"FNq",	"A-FN0",
	"FNs",	"FN^B",
	"FNt",	"FN^F",
	"FNu",	"FN^>",
	"FNv",	"FN^V",
	"FNw",	"FN^<",
	"FN„",	"FN^Z",
};

#define NUMTRAN	70

char *trans1[][2] = {
	"FN^P",	"A-Q",
	"FN^Q",	"A-W",
	"FN^R",	"A-E",
	"FN^S",	"A-R",
	"FN^T",	"A-T",
	"FN^U",	"A-Y",
	"FN^V",	"A-U",
	"FN^W",	"A-I",
	"FN^X",	"A-O",
	"FN^Y",	"A-P",
	"FN^^",	"A-A",
	"FN^_",	"A-S",
};

#define NUMTRAN1	13

/* some other globals */

FILE *infile;	/* current input file handle */
FILE *outfile;	/* current output file handle */

main(argc, argv)

int argc;	/* number of command line arguments */
char *argv[];	/* text of command line arguments */

{
	register carg;	/* current command line argument */

	puts("CMDFIX:	Startup and Command file updater");
	printf("		for MicroEMACS version %s\n", VERSION);

	/* help people out! */
	if (argc < 2) {
		puts("Converts MicroEMACS 3.9n and earlier keybindings to");
		puts("version 3.10 keybindings.");
		puts("\nUsage:	cmdfix <file list>");
		exit(-1);
	}

	/* loop through each input file and convert it */
	carg = 1;
	while (carg < argc) {
		fix(argv[carg]);
		++carg;
	}
	exit(0);
}

pascal fix(fname)

char *fname;	/* file to fix */

{
	register int inp;
	register int index;
	int nseq;	/* number of fixed function sequences */
	int buf[4];	/* buffer for characters going through */

	printf("Fixing %s\n", fname);

	/***	FIRST PASS   ***/
	/* first open the input file */
	infile = fopen(fname, "r");
	if (infile == NULL) {
		printf("%%No such file as %s\n", fname);
		return;
	}

	/* open the temporary output file */
	outfile = fopen("tempfile", "w");
	if (outfile == NULL) {
		puts("%Can not open output file");
		fclose(infile);
		return;
	}


	/* scan through the file... holding on FN sequences */
	inp = 0;
	buf[0] = 0;
	while (buf[0] != EOF) {
		if (inp == 4) {
			/* try to translate */
			if ((buf[0] == 'F') && (buf[1] == 'N') && (buf[2] == 0x5e)) {
				for (index = 0; index < NUMTRAN1; index++)
					if (trans1[index][0][3] == buf[3]) {
						fprintf(outfile, "%s", trans1[index][1]);
						inp = 0;
						buf[0] = 0;
						break;
					}
			}
		}
		if (inp == 4) {
			/* cycle a character */
			putc(buf[0], outfile);
			buf[0] = buf[1];
			buf[1] = buf[2];
			buf[2] = buf[3];
			if (buf[3] != EOF)
				buf[3] = getc(infile);
		} else {
			buf[inp++] = getc(infile);
		}
	}

	/* close the files */
	fclose(infile);
	fclose(outfile);

	/***	Second PASS   ***/
	/* first open the temporary input file */
	infile = fopen("tempfile", "r");
	if (infile == NULL) {
		printf("%%No such file as %s\n", fname);
		return;
	}

	/* open the output file */
	outfile = fopen(fname, "w");
	if (outfile == NULL) {
		puts("%Can not open output file");
		fclose(infile);
		return;
	}

	/* scan through the file... holding on FN sequences */
	inp = 0;
	nseq = 0;
	buf[0] = 0;
	while (buf[0] != EOF) {
		if (inp == 3) {
			/* try to translate */
			if (buf[0] == 'F' && buf[1] == 'N') {
				for (index = 0; index < NUMTRAN; index++)
					if (trans[index][0][2] == buf[2]) {
						fprintf(outfile, "%s", trans[index][1]);
						inp = 0;
						buf[0] = 0;
						++nseq;
						break;
					}
			}
		}
		if (inp == 3) {
			/* cycle a character */
			putc(buf[0], outfile);
			buf[0] = buf[1];
			buf[1] = buf[2];
			if (buf[2] != EOF)
				buf[2] = getc(infile);
		} else {
			buf[inp++] = getc(infile);
		}
	}

	/* close the files */
	fclose(infile);
	fclose(outfile);
	unlink("tempfile");

	/* and report */
	printf("[%u keys fixed in %s]\n", nseq, fname);
	return;
}
