/* chmem - set total memory size for execution	Author: Andy Tanenbaum */

#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

/* Should include a.out.h or exec.h.  Here is an approximation. */
#define HLONG            8	/* header size in longs */
#define TEXT             2	/* where is text size in header */
#define DATA             3	/* where is data size in header */
#define BSS              4	/* where is bss size in header */
#define TOT              6	/* where in header is total allocation */
#define TOTPOS          24	/* where is total in header */
#define SEPBIT  0x00200000	/* this bit is set for separate I/D */
#define MAGIC       0x0301	/* magic number for executable progs */
#define MAX_8086   0x10000L	/* maximum allocation size for 8086 */
#define MAX_386 0x7FFFFFFFL	/* etc */
#define MAX_68K 0x7FFFFFFFL
#define CPU_8086         4	/* CPU code for 8086 executables */
#define CPU_386       0x10	/* etc */
#define CPU_68K       0x0B	/* from Minix-PC a.out.h - unreliable */

main(argc, argv)
int argc;
char *argv[];
{
/* The 8088 architecture does not make it possible to catch stacks that grow
 * big.  The only way to deal with this problem is to let the stack grow down
 * towards the data segment and the data segment grow up towards the stack.
 * Normally, a total of 64K is allocated for the two of them, but if the
 * programmer knows that a smaller amount is sufficient, he can change it
 * using chmem.
 *
 * chmem =4096 prog  sets the total space for stack + data growth to 4096
 * chmem +200  prog  increments the total space for stack + data growth by 200
 */

  char *p;
  int fd, separate;
  long lsize, olddynam, newdynam, newtot, overflow, header[HLONG];
  char cpu;
  long max;

  if (argc != 3) usage();
  p = argv[1];
  if (*p != '=' && *p != '+' && *p != '-') usage();
  lsize = atol(p + 1);

  fd = open(argv[2], O_RDWR);
  if (fd < 0) stderr3("chmem: can't open ", argv[2], "\n");

  if (read(fd, (char *) header, sizeof(header)) != sizeof(header))
	stderr3("chmem: ", argv[2], "bad header\n");
  if ((header[0] & 0xFFFFL) != MAGIC)
	stderr3("chmem: ", argv[2], " not executable\n");
  separate = (header[0] & SEPBIT ? 1 : 0);

  cpu = (char) (header[0] >> 24);	/* cpu byte is most significant */
  if (cpu == CPU_8086 && *(unsigned short *) &header[0] != MAGIC)
	cpu = CPU_68K;		/* 8086 code with 68K byte order == 68K */
  switch(cpu) {
	case CPU_8086:	max = MAX_8086;	break;
	case CPU_386:	max = MAX_386;	break;
	case CPU_68K:	max = MAX_68K;	break;
	default:	stderr3("chmem: ", argv[2], "bad CPU type\n");
  }  

  if (lsize < 0) stderr3("chmem: ", p+1, " negative size not allowed\n");
  if (lsize > max) stderr3("chmem: ", p + 1, " too large\n");

  olddynam = header[TOT] - header[DATA] - header[BSS];
  if (separate == 0) olddynam -= header[TEXT];

  if (*p == '=')
	newdynam = lsize;
  else if (*p == '+')
	newdynam = olddynam + lsize;
  else if (*p == '-')
	newdynam = olddynam - lsize;

  newtot = header[DATA] + header[BSS] + newdynam;
  if (separate == 0) newtot += header[TEXT];
  overflow = (newtot > max ? newtot - max : 0);
  newdynam -= overflow;
  newtot -= overflow;
  lseek(fd, (long) TOTPOS, SEEK_SET);
  if (write(fd, (char *) &newtot, 4) < 0)
	stderr3("chmem: can't modify ", argv[2], "\n");
  printf("%s: Stack+malloc area changed from %ld to %ld bytes.\n",
         argv[2], olddynam, newdynam);
  exit(0);
}

usage()
{
  std_err("chmem {=+-} amount file\n");
  exit(1);
}

stderr3(s1, s2, s3)
char *s1, *s2, *s3;
{
  std_err(s1);
  std_err(s2);
  std_err(s3);
  exit(1);
}
