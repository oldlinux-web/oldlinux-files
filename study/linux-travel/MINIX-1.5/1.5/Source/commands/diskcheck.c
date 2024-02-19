/* diskcheck - test a disk for bad blocks	Author: Andy Tanenbaum */

#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <minix/config.h>
#include <minix/const.h>

#undef printf
#define OK 0
#define PRINTFREQ  10
#define N 30

char purgebuf[BLOCK_SIZE * N];
char buf[BLOCK_SIZE], zero[BLOCK_SIZE];
int pat1[BLOCK_SIZE / 2], pat2[BLOCK_SIZE / 2];
int blk = -1;			/* number of the block in buf, or -1 */
int pfd;			/* file descriptor for purging */
int fd;				/* file descriptor for data I/O */
unsigned initblock;		/* first block to test */
unsigned curblock;		/* current block */
unsigned limit;			/* first block beyond test zone */
unsigned errors;		/* # errors so far */
unsigned ct;			/* # blocks read so far */
int intflag;			/* set when signal seen */
extern errno;
long pos;
char *purgefile = "/dev/ram";

main(argc, argv)
int argc;
char *argv[];
{
  unsigned b;
  int i;
  void catch();

  signal(SIGINT, catch);
  signal(SIGQUIT, catch);
  if (argc != 4) usage();
  if ((fd = open(argv[1], O_RDWR)) < 0) {
	printf("Cannot open %s\n", argv[1]);
	exit(1);
  }
  if ((pfd = open(purgefile, O_RDWR)) < 0) {
	printf("Cannot open %s\n", purgefile);
	exit(1);
  }
  initblock = atoi(argv[2]);
  limit = initblock + atoi(argv[3]);
  if (limit <= initblock) usage();

  for (i = 0; i < BLOCK_SIZE / 2; i++) {
	pat1[i] = i;
	pat2[i] = 1000 - i;
  }

  for (b = initblock; b < limit; b++) {
	if (intflag) break;
	if (testblock(b) == ERROR) {
		errors++;
		if (blk == b) {
			/* Read ok, write failed; try to restore block. */
			lseek(fd, pos, SEEK_SET);
			write(fd, buf, BLOCK_SIZE);
		}
	}
	curblock = b;
	ct++;
	if (ct % PRINTFREQ == 0) status();
  }

  status();
  exit(0);
}


int testblock(b)
unsigned b;
{
/* Read block b in, save it in buf.  Then overwrite that block with a
 * known test pattern and read it back.  Finally, replace the block.
 * Return OK or ERROR.
 */

  int s;

  blk = -1;
  pos = (long) BLOCK_SIZE *(long) b;
  purge_cache();
  if (lseek(fd, pos, SEEK_SET) != pos) fatal("Cannot seek to block ", b);

  /* Read block b into 'buf'. */
  s = read(fd, buf, BLOCK_SIZE);

  /* Test for various outcomes of the read. */
  if (s == BLOCK_SIZE) {
	blk = b;
	if (wtest(pos, pat1) == ERROR) return(ERROR);
	if (wtest(pos, pat2) == ERROR) return (ERROR);
	lseek(fd, pos, SEEK_SET);
	if (write(fd, buf, BLOCK_SIZE) != BLOCK_SIZE) {
		nonfatal("Cannot rewrite block ", b);
		return(ERROR);
	} else {
		return(OK);
	}
  }
  if (s < 0) {
	if (errno == EIO)
		nonfatal("Read error on block ", b);
	else {
		printf("\nError.  Read returned %d.  errno=%d.   ", s, errno);
		fatal("Block ", b);
	}
	return(ERROR);
  }
  if (s == 0) fatal("End of file reached trying to read block ", b);


  nonfatal("Read size error on block ", b);
}

status()
{
  printf("%8u blocks tested, %u errors detected (last block tested = %5u)\r",
         ct, errors, curblock);
}

nonfatal(s, b)
char *s;
unsigned b;
{
  printf("\n%s%u\n", s, b);
}

fatal(s, b)
char *s;
unsigned b;
{
  printf("\n%s%u\n", s, b);
  status();
  exit(1);
}


void catch(sig)
int sig;			/* prototype says there has to be 1 arg */
{
  signal(SIGINT, catch);
  signal(SIGQUIT, catch);
  intflag = 1;
}


usage()
{
  printf("Usage: diskcheck device start-block block-count\n");
  exit(1);
}

wtest(pos, pat)
long pos;
int pat[];
{
  int testb[BLOCK_SIZE / 2];
  int i;

  lseek(fd, pos, SEEK_SET);
  if (write(fd, (char *) pat, BLOCK_SIZE) != BLOCK_SIZE) return(ERROR);
  sync();			/* force the write to the disk */
  purge_cache();
  lseek(fd, pos, SEEK_SET);
  if (read(fd, (char *) testb, BLOCK_SIZE) != BLOCK_SIZE) return(ERROR);
  for (i = 0; i < BLOCK_SIZE / 2; i++)
	if (testb[i] != pat[i]) {
		printf("%d %d\n", testb[i], pat[i]);
		return(ERROR);
	}
  return(OK);
}

purge_cache()
{
/* Do enough reads that the cache is purged. */

  int left, count, r;

  pfd = open(purgefile, O_RDONLY);
  left = NR_BUFS;
  while (left > 0) {
	count = (left < N ? left : N);
	if ((r = read(pfd, purgebuf, count * BLOCK_SIZE)) != count * BLOCK_SIZE) {
		printf("ERROR: count=%d  left=%d r=%d.  ", count, left, r);
		fatal("Cannot purge cache.  errno= ", errno);
	}
	left -= count;
  }
  close(pfd);
}
