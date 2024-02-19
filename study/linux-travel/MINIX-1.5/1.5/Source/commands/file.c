/* file - report on file type.		Author: Andy Tanenbaum */

#include <blocksize.h>
#include <ar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <minix/config.h>

#if (CHIP == M68000)
#define A_OUT		0x410	/* magic number for executables */
#define A_OUT_SPLIT	0x420	/* magic number for executables split I/D */
#define A_OUT_SEC	0x301	/* second check for executables */
#define OBJECT		0x102	/* minix/st object */
#else
#define A_OUT 001401		/* magic number for executables */
#define SPLIT 002040		/* second word on split I/D binaries */
#endif
#define XBITS 00111		/* rwXrwXrwX (x bits in the mode) */
#define ENGLISH 25		/* cutoff for determining if text is Eng. */
char buf[BLOCK_SIZE];

main(argc, argv)
int argc;
char *argv[];
{
/* This program uses some heuristics to try to guess about a file type by
 * looking at its contents.
 */

  int i;

  if (argc < 2) usage();
  for (i = 1; i < argc; i++) file(argv[i]);
}

file(name)
char *name;
{
  int i, fd, n, magic, second, mode, nonascii, special, funnypct, etaoins;
  int symbols;
  long engpct;
  char c;
  struct stat st_buf;

  printf("%s: ", name);

  /* Open the file, stat it, and read in 1 block. */
  fd = open(name, O_RDONLY);
  if (fd < 0) {
	printf("cannot open\n");
	return;
  }
  n = fstat(fd, &st_buf);
  if (n < 0) {
	printf("cannot stat\n");
	close(fd);
	return;
  }
  mode = st_buf.st_mode;

  /* Check for directories and special files. */
  if ((mode & S_IFMT) == S_IFDIR) {
	printf("directory\n");
	close(fd);
	return;
  }
  if ((mode & S_IFMT) == S_IFCHR) {
	printf("character special file\n");
	close(fd);
	return;
  }
  if ((mode & S_IFMT) == S_IFBLK) {
	printf("block special file\n");
	close(fd);
	return;
  }
  n = read(fd, buf, BLOCK_SIZE);
  if (n < 0) {
	printf("cannot read\n");
	close(fd);
	return;
  }

  /* Check to see if file is an archive. */
#if (CHIP == M68000)
  magic = (buf[0] << 8) | (buf[1] & 0377);
  if (magic == 026377) {
#else
  magic = (buf[1] << 8) | (buf[0] & 0377);
  if (magic == ARMAG) {
#endif
	printf("archive\n");
	close(fd);
	return;
  }
#if (CHIP == M68000)
  /* Check to see if file is an object file. */
  if (magic == OBJECT) {
	printf("MINIX/ST object file\n");
	close(fd);
	return;
  }
#endif

  /* Check to see if file is an executable binary. */
#if (CHIP != M68000)
  if (magic == A_OUT) {
	/* File is executable.  Check for split I/D. */
	printf("MINIX/PC executable");
	second = (buf[3] << 8) | (buf[2] & 0377);
	if (second == SPLIT)
		printf("   separate I & D space");
	else
		printf("   combined I & D space");
#else
  if (magic == A_OUT || magic == A_OUT_SPLIT) {
	/* File is executable.  Check for split I/D. */
	printf("MINIX/ST executable");
	second = (buf[2] << 8) | (buf[3] & 0377);
	if (second == A_OUT_SEC) {
		if (magic == A_OUT_SPLIT)
			printf("   separate I & D space");
		else
			printf("   combined I & D space");
	}
#endif
	symbols = buf[28] | buf[29] | buf[30] | buf[31];
	if (symbols != 0)
		printf("   not stripped\n");
	else
		printf("   stripped\n");
	close(fd);
	return;
  }

  /* Check to see if file is a shell script. */
  if (mode & XBITS) {
	/* Not a binary, but executable.  Probably a shell script. */
	printf("shell script\n");
	close(fd);
	return;
  }

  /* Check for ASCII data and certain punctuation. */
  nonascii = 0;
  special = 0;
  etaoins = 0;
  for (i = 0; i < n; i++) {
	c = buf[i];
	if (c & 0200) nonascii++;
	if (c == ';' || c == '{' || c == '}' || c == '#') special++;
	if (c == '*' || c == '<' || c == '>' || c == '/') special++;
	if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
	if (c == 'e' || c == 't' || c == 'a' || c == 'o') etaoins++;
	if (c == 'i' || c == 'n' || c == 's') etaoins++;
  }

  if (nonascii == 0) {
	/* File only contains ASCII characters.  Continue processing. */
	funnypct = 100 * special / n;
	engpct = 100L * (long) etaoins / n;
	if (funnypct > 1) {
		printf("C program\n");
	} else {
		if (engpct > (long) ENGLISH)
			printf("English text\n");
		else
			printf("ASCII text\n", engpct);
	}
	close(fd);
	return;
  }

  /* Give up.  Call it data. */
  printf("data\n");
  close(fd);
  return;
}

usage()
{
  printf("Usage: file name ...\n");
  exit(1);
}
