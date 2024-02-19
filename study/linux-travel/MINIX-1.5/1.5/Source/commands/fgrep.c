/* fgrep - fast grep			Author: Jan Christiaan van Winkel */

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

int argc;
char **argv;
int stringlen, offset, boundary;
int i, j, k, count, linum;
char stringarea[1024];
int strptr;
int strcount;
unsigned char tbl[32][256];
int lengths[32];
char *tststring[32];
char string[512];
char tmpstring[512];
int vflag, cflag, nofname, hadone, lflag, nflag, sflag, hflag, eflag, fflag;
int fp;

main(oargc, oargv)
int oargc;
char *oargv[];

{
  int find();
  void exparg();
  void getargs();
  void gotone();

  argc = oargc;
  argv = oargv;
  getargs();

  fp = 0;
  if (i >= argc - 2 || hflag)	/* stdin, 1 file, hflag */
	nofname = 1;

  do {
	if (i < argc && (fp = open(argv[i], O_RDONLY)) < 0) {
		fprintf(stderr, "%s: can't open %s\n", argv[0], argv[i]);
		continue;
	}
	count = 0;
	linum = 0;

	while ((stringlen = getlin(fp, string, 512)) != EOF) {
		linum++;
		for (j = 0; j < strcount; j++) {
			if (find(tststring[j], tbl[j], lengths[j]) != vflag) {
				gotone();
				break;
			}
		}
		if (lflag && count) break;
	}
	close(fp);

	if (cflag) {
		printf("%s: %d times\n", argv[i], count);
	}
	if (lflag && count > 0) {
		printf("%s\n", argv[i]);
	}
  } while (++i < argc);

  fflush(stdout);
  if (hadone)
	exit(0);
  else
	exit(1);
}

void getargs()
{
  int tmp;
  void maktbl();
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
	switch (argv[i][1]) {
	    case 'e':
		eflag = 1;
		if (fflag) {
			fprintf(stderr, "%s: can't have -e and -f at the same time\n", argv[0]);
			exit(2);
		}
		if (i < argc - 1) {
			i++;
			tststring[0] = argv[i];
			strcount = 1;
		} else {
			fprintf(stderr, "%s: not enough arguments\n");
			exit(2);
		}
		break;
	    case 'v':	vflag = 1;	break;
	    case 'c':	cflag = 1;	break;
	    case 'l':	lflag = 1;	break;
	    case 's':	sflag = 1;	break;
	    case 'h':	hflag = 1;	break;
	    case 'n':	nflag = 1;	break;
	    case 'f':
		fflag = 1;
		if (eflag) {
		    fprintf(stderr, 
		       "%s: can't have -e and -f at the same time\n", argv[0]);
		    exit(2);
		}
		if (i >= argc - 1) {
			fprintf(stderr, "%s: not enough arguments\n");
			exit(2);
		} else {
			i++;
			if ((fp = open(argv[i], O_RDONLY)) < 0) {
				fprintf(stderr, 
				      "%s: can't open %s\n", argv[0], argv[i]);
				exit(2);
			}
			strcount = 0;
			while ((tmp = getlin(fp, &stringarea[strptr], 128)) != EOF) {
				tststring[strcount++] = &stringarea[strptr];
				strptr = strptr + tmp + 1;
				if (strptr >= 1024 - 128 || strcount == 32) {
					fprintf(stderr, "%s: not enough room\n", argv[0]);
					exit(2);
				}
			}
			close(fp);
		}
		break;
	    default:
		fprintf(stderr, "%s: invalid command line option\n", argv[0]);
		exit(2);
		break;
	}
	if (cflag && lflag) {
		fprintf(stderr, "%s: cannot have -l and -c at the same time\n", argv[0]);
		exit(2);
	}
  }

  if (!eflag && !fflag) {
	if (i < argc) {
		tststring[0] = argv[i++];
		strcount = 1;
	} else {
		fprintf(stderr, "%s: no search string.\n", argv[0]);
		exit(2);
	}
  }
  for (j = 0; j < strcount; j++) {
	if (tststring[j][0] == '"') {
		count = strlen(tststring[j]);
		movmem(&tststring[j][1], tststring[j], count - 2);
		tststring[j][count - 2] = '\0';
	}
	maktbl(tststring[j], tbl[j], &lengths[j]);
  }
}


movmem(src, dst, len)
char *src, *dst;
int len;
{
  while (len--) *dst++ = *src++;
}

setmem(mem, len, filler)
char *mem;
int len;
char filler;
{
  while (len--) *mem++ = filler;
}


int find(findword, table, wordlen)
unsigned char *findword;
unsigned char *table;
int wordlen;
{
  auto int lastletter, tmp;

  boundary = stringlen - wordlen;
  lastletter = wordlen - 1;
  offset = 0;
  while (offset <= boundary) {
	tmp = table[string[offset + lastletter]];
	if (tmp) {
		offset += tmp;
	} else {
		for (k = lastletter - 1; k >= 0; k--) {
			if ((string[k + offset]) != findword[k]) {
				offset++;
				break;
			}
		}
		if (k < 0) return(1);
	}
  }
  return(0);
}


void maktbl(findword, table, wordlen)
unsigned char *findword;
unsigned char *table;
int *wordlen;
{

  auto int i, len;

  *wordlen = len = strlen(findword);
  setmem(table, 256, len);
  for (i = 0; i < len; i++) table[findword[i]] = len - i - 1;
}


void gotone()
{
  hadone = 1;

  if (cflag || lflag || sflag) {
	count++;
	return;
  }
  if (!nofname) printf("%s:", argv[i]);

  if (nflag) printf("%d:", linum);

  printf("%s\n", string);
}


int getlin(infile, buf, maxlen)
int infile;
char *buf;
int maxlen;
{
  static char mybuf[2048];
  static char *low;
  static char *high;

  auto int status;
  auto char *p = buf;
  auto int endline;

  *p = '\0';
  maxlen--;
  while (1) {
	endline = 0;
	while (low < high && !endline) {
		if (p >= &buf[maxlen]) {	/* overflow, skip all
						 * until \n */
			while (low < high && *low != '\n') low++;
			endline = (*low == '\n');
		} else
			endline = ((*p++ = *low++) == '\n');

	}			/* exhausted buffer or found \n */

	/* Don't continue if \n found */
	if (endline) {
		*(p - 1) = '\0';
		return(p - buf - 1);
	}
	status = read(infile, mybuf, 2048);
	if (status <= 0) break;

	low = mybuf;
	high = &mybuf[status];

  }

  /* Empty line or a bit filled ? */
  *p = '\0';
  if (status < 0) {
	perror("read error");
	return(EOF);
  }
  if (p - buf) return(p - buf);
  return(EOF);

}				/* of getlin() */
