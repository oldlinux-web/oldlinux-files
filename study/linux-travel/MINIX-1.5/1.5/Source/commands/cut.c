/* cut - extract columns from a file or stdin. 	Author: Michael J. Holme
 *
 *	Copyright 1989, Michael John Holme, All rights reserved.
 *	This code may be freely distributed, provided that this notice
 *	remains intact.
 *
 *	V1.1: 6th September 1989
 *
 *	Bugs, criticisms, etc,
 *      c/o Mark Powell
 *          JANET sq79@uk.ac.liv
 *          ARPA  sq79%liv.ac.uk@nsfnet-relay.ac.uk
 *          UUCP  ...!mcvax!ukc!liv.ac.uk!sq79
 */

#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define MAX_FIELD	80	/* Pointers to the beginning of each field
			 * are stored in columns[], if a line holds
			 * more than MAX_FIELD columns the array
			 * boundary is exceed. But unlikely at 80 */
char *columns[MAX_FIELD];

#define MAX_ARGS	32	/* Maximum number of fields following -f or
			 * -c switches			  	  */
int args[MAX_ARGS * 2];
int num_args;

int mode;			/* 0 = dump stdin to stdout, 1=-f, 2=-c   */
int flagi;			/* 1=-i, 0=no -i 		  */
char delim = '\t';		/* default delimiting character	  */
FILE *fd;
char *name;
char line[BUFSIZ];
extern int errno;

void cuterror(err)
int err;
{
  static char *err_mes[] = {
			  "%s: syntax error\n",
			  "%s: position must be >0\n",
			  "%s: unknown option\n",
			  "%s: usage: cut [-f{args} [-i] [-d'x']]|[-c{args}] [filename [...]]\n",
			  "%s: line longer than BUFSIZ\n",
			  "%s: -d option not applicable\n",
			  "%s: -i option not applicable\n",
		 "%s: range must not decrease from left to right\n",
			  "%s: MAX_FIELD exceeded\n",
			  "%s: MAX_ARGS exceeded\n"
  };
  printf(err_mes[err - 1], name);
  exit(err);
}


void get_args()
{
  int i = 0;
  int arg_ptr = 0;
  int flag;

  num_args = 0;
  do {
	if (num_args == MAX_ARGS) cuterror(10);
	if (!isdigit(line[i]) && line[i] != '-') cuterror(1);

	args[arg_ptr] = 1;
	args[arg_ptr + 1] = BUFSIZ;
	flag = 1;

	while (line[i] != ',' && line[i] != 0) {
		if (isdigit(line[i])) {
			args[arg_ptr] = 0;
			while (isdigit(line[i]))
				args[arg_ptr] = 10 * args[arg_ptr] + line[i++] - '0';
			if (!args[arg_ptr]) cuterror(2);
			arg_ptr++;
		}
		if (line[i] == '-') {
			arg_ptr |= 1;
			i++;
			flag = 0;
		}
	}
	if (flag && arg_ptr & 1) args[arg_ptr] = args[arg_ptr - 1];
	if (args[num_args * 2] > args[num_args * 2 + 1]) cuterror(8);
	num_args++;
	arg_ptr = num_args * 2;
  }
  while (line[i++]);
}


void cut()
{
  int i, j, length, maxcol;

  while (fgets(line, BUFSIZ, fd)) {
	length = strlen(line) - 1;
	*(line + length) = 0;
	switch (mode) {
	    case 0:	printf("%s", line);	break;
	    case 1:
		maxcol = 0;
		columns[maxcol++] = line;
		for (i = 0; i < length; i++) {
			if (*(line + i) == delim) {
				*(line + i) = 0;
				if (maxcol == MAX_FIELD) cuterror(9);
				columns[maxcol] = line + i + 1;
				while (*(line + i + 1) == delim && flagi) {
					columns[maxcol]++;
					i++;
				}
				maxcol++;
			}
		}
		for (i = 0; i < num_args; i++) {
			for (j = args[i * 2]; j <= args[i * 2 + 1]; j++)
				if (j <= maxcol) {
					printf("%s", columns[j - 1]);
					if (i != num_args - 1 || j != args[i * 2 + 1])
						putchar(delim);
				}
		}
		break;
	    case 2:
		for (i = 0; i < num_args; i++) {
			for (j = args[i * 2]; j <= (args[i * 2 + 1] > length ? length :
					      args[i * 2 + 1]); j++)
				putchar(*(line + j - 1));
		}
	}
	putchar('\n');
  }
}


main(argc, argv)
int argc;
char *argv[];
{
  int flag;
  int i = 1;

  name = argv[0];
  if (argc == 1) cuterror(4);

  while (argv[i] != NULL && argv[i][0] == '-') {
	switch (argv[i][1]) {
	    case 'd':
		if (mode == 2) cuterror(6);
		sprintf(line, "%.1s", &argv[i++][2]);
		delim = line[0];
		break;
	    case 'f':
		sprintf(line, "%s", &argv[i++][2]);
		mode = 1;
		get_args();
		break;
	    case 'c':
		sprintf(line, "%s", &argv[i++][2]);
		mode = 2;
		get_args();
		break;
	    case 'i':
		if (mode == 2) cuterror(7);
		flagi = 1;
		i++;
		break;
	    default:	cuterror(3);
	}
  }

  if (i < argc)
	if ((fd = fopen(argv[i], "r")) == NULL) {
		printf("%s: couldn't open %s\n", name, argv[i]);
		exit(errno);
	} else {
		cut();
		fclose(fd);
	}
  else {
	fd = stdin;
	cut();
  }
  exit(0);
}
