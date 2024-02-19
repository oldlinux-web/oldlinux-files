/* cmp - compare two files	Authors: Paul Polderman & Michiel Huisjes */

/* 90-04-10 Schlenker
 *	Fixed incorrect handling of flags.
 *	Reduced buffer size to accommodate 7K pipes (Minix restriction).
 *	Better trapping of file reading errors.
 *	Considerable speedup when using -s flag.
 *	Buffering strategy remains seriously error prone; should be fixed.
 */

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define BLOCK_SIZE 6144

char *file_1, *file_2;
char buf1[BLOCK_SIZE];
char buf2[BLOCK_SIZE];
char lflag, sflag;

main(argc, argv)
int argc;
char *argv[];
{
  int fd1, fd2, i, exit_status;

  if (argc < 3 || argc > 4) usage();
  lflag = 0;
  sflag = 0;

  i = 1;
  if (strcmp(argv[i], "-l") == 0) {
	lflag++;
	i++;
  } else
  if (strcmp(argv[i], "-s") == 0) {
	sflag++;
	i++;
  }

  if (strcmp(argv[i], "-") == 0) {
	fd1 = 0;
	file_1 = "<stdin>";
  } else {
	if ((fd1 = open(argv[i], O_RDONLY)) < 0)
		cantopen(argv[i]);
	file_1 = argv[i];
  }
  i++;

  if (i == argc || (fd2 = open(argv[i], O_RDONLY)) < 0)
	cantopen(argv[i]);
  file_2 = argv[i];

  exit_status = sflag ? fastcmp(fd1, fd2) : cmp(fd1, fd2);

  close(fd1);
  close(fd2);

  exit(exit_status);
}

cmp(fd1, fd2)
int fd1, fd2;
{
  register unsigned long char_cnt, line_cnt;
  register int i;
  int n1, n2, n, exit_status;

  char_cnt = 1L;
  line_cnt = 1L;
  exit_status = 0;
  do {
	n1 = read(fd1, buf1, BLOCK_SIZE);
	n2 = read(fd2, buf2, BLOCK_SIZE);
	n = (n1 < n2) ? n1 : n2;
	if (n < 0) {
		printf("cmp: Error on %s\n", (n1 < 0) ? file_1 : file_2);
		return(1);
	}
	for (i = 0; i < n; i++) {	/* Check buffers for equality */
		if (buf1[i] != buf2[i]) {
			if (!lflag) {
				printf("%s %s differ: char %ld, line %ld\n",
				file_1, file_2, char_cnt, line_cnt);
				return(1);
			}
			printf("%10lu %03o %03o\n",
			       char_cnt, buf1[i] & 0377, buf2[i] & 0377);
			exit_status = 1;
		}
		if (buf1[i] == '\n') line_cnt++;
		char_cnt++;
	}
	if (n1 != n2) {		/* EOF on one of the input files. */
		printf("cmp: EOF on %s\n", (n1 < n2) ? file_1 : file_2);
		return(1);
	}
  } while (n > 0);		/* While not EOF on any file */
  return(exit_status);
}

fastcmp(fd1, fd2)
int fd1, fd2;
{
  int n1, n2;

  while (1) {
	n1 = read(fd1, buf1, BLOCK_SIZE);
	n2 = read(fd2, buf2, BLOCK_SIZE);
	if (n1 != n2) return(1);	/* Bug! - depends on buffering */
	if (n1 == 0) return(0);
	if (n1 < 0) return(1);
	if (memcmp((void *) buf1, (void *) buf2, (size_t) n1) != 0)
		return(1);
  }
}

usage()
{
  fprintf(stderr, "Usage: cmp [-l | -s] file1 file2\n");
  exit(2);
}

cantopen(s)
char *s;
{
  fprintf(stderr, "cmp: cannot open %s\n", s);
  exit(1);
}
