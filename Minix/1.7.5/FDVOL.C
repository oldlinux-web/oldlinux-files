/* fdvol.c - A stripped down version of the MINIX vol program.  It takes
 * a file and breaks into into floppy-sized chunks, writing each one raw
 * to a floppy.
 *
 *      Usage:          fdvol file drive KB [slow]
 *
 *	Examples:	fdvol 1440 a: foo.taz		# 1.4 MB floppy a:
 *			fdvol 1200 b: foo.taz		# 1.2 MB floppy b:
 *                      fdvol slow 360 a: foo.taz	# old machine
 *                      fdvol 1440 a: foo bar		# concatenate
 *
 * The optional 'slow' parameter forces the program to write in units of 3
 * sectors. Folk tradition has it that this works around some buggy BIOSes.
 *
 * This code borrows heavily from Mark Becker's RaWrite program.
 */


#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TEST 0

#if !TEST
#include <alloc.h>
#include <bios.h>
#include <dir.h>
#include <dos.h>
#include <io.h>
#endif

#define FALSE	0
#define TRUE	(!FALSE)

#define SECTORSIZE	512

#define	RESET	0
#define	LAST	1
#define	READ	2
#define	WRITE	3
#define	VERIFY	4
#define	FORMAT	5

int done;
char buffer[18*SECTORSIZE];	/* do I/O in units of up to 18 sectors */
char testbuf[SECTORSIZE];	/* to do a test read of the first sector */


int handler(void)
{
/* Catch CTRL-C and CTRL-Break. */

  done = 1;
  return(0);
}

void msg(char (*s))
{
/* Print an error message and quit. */

  fprintf(stderr, "%s\n", s);
  _exit(1);
}

void Error(int status, int cyl, int head, int sector)
{
/* Identify the error code with a real error message. */


  fprintf(stderr, "\nError occured while writing cyl %d, head=%d, sector=%d\n", cyl,head,sector+1);
  switch (status) {
    case 0x00:	msg("Operation Successful");				break;
    case 0x01:	msg("Bad command");					break;
    case 0x02:	msg("Address mark not found");				break;
    case 0x03:	msg("Attempt to write on write-protected disk");	break;
    case 0x04:	msg("Sector not found");				break;
    case 0x05:	msg("Reset failed (hard disk)");			break;
    case 0x06:	msg("Disk changed since last operation");		break;
    case 0x07:	msg("Drive parameter activity failed");			break;
    case 0x08:	msg("DMA overrun");					break;
    case 0x09:	msg("Attempt to DMA across 64K boundary");		break;
    case 0x0A:	msg("Bad sector detected");				break;
    case 0x0B:	msg("Bad track detected");				break;
    case 0x0C:	msg("Unsupported track");				break;
    case 0x10:	msg("Bad CRC/ECC on disk read");			break;
    case 0x11:	msg("CRC/ECC corrected data error");			break;
    case 0x20:	msg("Controller has failed");				break;
    case 0x40:	msg("Seek operation failed");				break;
    case 0x80:	msg("Attachment failed to respond");			break;
    case 0xAA:	msg("Drive not ready (hard disk only");			break;
    case 0xBB:	msg("Undefined error occurred (hard disk only)");	break;
    case 0xCC:	msg("Write fault occurred");				break;
    case 0xE0:	msg("Status error");					break;
    case 0xFF:	msg("Sense operation failed");				break;
  }
  exit(1);
}


void main(int argc, char *argv[])
{
  int disknr = 1, fdin, count, drive, head, cyl, status, sector;
  int max_cyl, chunk, nsectors, ct;
  long offset, drive_size, r, cyl_size;
  char *p, c;
  int slow;
  int kbsize;
  char **files;
  int nfiles, i;

#if !TEST
  /* Catch breaks. */
  ctrlbrk(handler);
#endif

#if 0 /* Do we have to clear the screen? */
  fprintf(stderr, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
#endif

  if (argc > 1 && strcmp(argv[1], "slow") == 0) {	/* Lousy BIOS? */
	slow = 1;
	argc--;
	argv++;
  } else {
	slow = 0;
  }

  /* Check the arguments for validity. */
  if (argc < 4)
        msg("Usage: fdvol [slow] #kilobytes drive-letter file1 [file2 ...]");

  kbsize = atoi(argv[1]);

  p = argv[2];
  c = *p;
  if (c == 'a' || c == 'A')
	drive = 0;
  else if (c == 'b' || c == 'B')
        drive = 1;
  else
	msg("fdvol: Second parameter must be drive, either   a:   or   b:");

  files = argv + 3;
  nfiles = argc - 3;

  switch(kbsize) {
      case 360:
        cyl_size = 9*2*SECTORSIZE;      /* bytes/cylinder */
        max_cyl = 39;                   /* zero-based counting */
	drive_size = cyl_size * (max_cyl+1);
        chunk = (!slow ? 9 * SECTORSIZE : 3 * SECTORSIZE);
        nsectors = chunk/SECTORSIZE;
        break;

      case 720:
        cyl_size = 9*2*SECTORSIZE;      /* bytes/cylinder */
        max_cyl = 79;                   /* zero-based counting */
	drive_size = cyl_size * (max_cyl+1);
        chunk = (!slow ? 9 * SECTORSIZE : 3 * SECTORSIZE);
        nsectors = chunk/SECTORSIZE;
        break;

      case 1200:
        cyl_size = 15*2*SECTORSIZE;          /* bytes/cylinder */
        max_cyl = 79;                   /* zero-based counting */
	drive_size = cyl_size * (max_cyl+1);
        chunk = (!slow ? 15 * SECTORSIZE : 3 * SECTORSIZE);
        nsectors = chunk/SECTORSIZE;
        break;

      case 1440:
        cyl_size = 18*2*SECTORSIZE;     /* bytes/cylinder */
        max_cyl = 79;                   /* zero-based counting */
	drive_size = cyl_size * (max_cyl+1);
        chunk = (!slow ? 18 * SECTORSIZE : 3 * SECTORSIZE);
        nsectors = chunk/SECTORSIZE;
        break;

      default:
	msg("fdvol: First parameter must be one of: 360, 720, 1200, or 1440");
  }

#if !TEST
  biosdisk(RESET, drive, 0, 0, 0, 0, testbuf);
#endif

/*
 * Start writing data to diskette until there is no more data to write.
 * Optionally read and write in units of 3 sectors.  Folk tradition says
 * that this makes fewer buggy BIOSes unhappy than doing a whole track at a
 * time.
 */
  offset = 0;
  i = 0;
  fdin = -1;

  while(1) {
	if (done > 0) {
		if (done == 1) msg("User abort");
#if !TEST
		biosdisk(READ, drive, 0, 0, 1, 1, testbuf); /* Retract head */
#endif
                fprintf(stderr, "Done.                                                      \n");
		exit(done == 1 ? 1 : 0);
	}

	/* Until a chunk is read. */
	count = 0;
	while (count < chunk) {
		if (fdin == -1) {			/* open next file */
#if !TEST
			_fmode = O_BINARY;
#endif
			fdin = open(files[i], O_RDONLY);
			if (fdin < 0) {
				perror(files[i]);
				exit(1);
			}
		}
							/* read from file */
		ct = read(fdin, buffer + count, chunk - count);
		if (ct < 0) {
			perror(files[i]);
			exit(1);
		}
		if (ct == 0) {				/* end of file */
			close(fdin);
			fdin = -1;

			/* choose next file */
			if (++i >= nfiles) break;	/* no more files */
		}
		count += ct;
	}

	if (count == 0) {				/* absolute EOF */
		done = 2;
		continue;
	}

	if (count < chunk) {				/* pad last track */
		/* Pad out buffer with zeroes. */
		p = &buffer[count];
		while (p < &buffer[chunk]) *p++ = 0;
		done = 2;
	}

	r = offset % drive_size;
	if (r == 0) {
		/* An integral number of diskettes have been filled. Prompt. */
                fprintf(stderr, "Please insert formatted diskette #%d in drive %c, then hit Enter%c\n", disknr, c, 7);
		disknr++;
#if !TEST
		while(bioskey(1) == 0) ;	/* wait for input */
		if ((bioskey(0) & 0x7F)	== 3) exit(1);	/* CTRL-C */
                biosdisk(READ,  drive, 0, 0, 1, 1, testbuf); /* get it going */
#endif

	}
	/* Compute cyl, head, sector. */
	cyl = r/cyl_size;
	r -= cyl * cyl_size;
	head = (r < cyl_size/2 ? 0 : 1);
	r -= head * cyl_size/2;
	sector = r/SECTORSIZE;
	
        fprintf(stderr, "Track: %2d  Head: %d  Sector: %2d    File offset: %ld\r",
                cyl, head, sector+1,offset);
#if !TEST
        status = biosdisk(WRITE, drive, head, cyl, sector+1, nsectors, buffer);
        if (status != 0) Error(status, cyl, head, sector);
#else
	write(1, buffer, chunk);
#endif
        offset += chunk;
   }
}
