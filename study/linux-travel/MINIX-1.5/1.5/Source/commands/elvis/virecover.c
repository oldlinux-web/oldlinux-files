/* This file contains the file recovery program */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "vi.h"

struct stat	stbuf;
BLK		hdr;
BLK		text;

main(argc, argv)
	int	argc;
	char	**argv;
{
	if (argc < 2)
	{
		/* maybe stdin comes from a file? */
		if (fstat(0, &stbuf) < 0 || (S_IFMT & stbuf.st_mode) != S_IFREG)
		{
			fprintf(stderr, "usage: %s lostfile...\n", argv[0]);
		}
		else if (read(0, &hdr, BLKSIZE) != BLKSIZE)
		{
			fprintf(stderr, "couldn't get header\n");
		}
		else
		{
			copytext(0, stdout);
		}
	}
	else
	{
		while (--argc > 0)
		{
			recover(*++argv);
		}
	}
	exit(0);
}


/* This function recovers a single file */
recover(filename)
	char	*filename;
{
	char		tmpname[100];
	int		tmpfd;
	FILE		*fp;
	long		mtime;
	int		i, j;

	/* get the file's status info */
	if (stat(filename, &stbuf) < 0)
	{
		/* if serious error, give up on this file */
		if (errno != ENOENT)
		{
			perror(filename);
			return;
		}

		/* else fake it for a new file */
		stat(".", &stbuf);
		stbuf.st_mode = S_IFREG;
		stbuf.st_mtime = 0L;
	}

	/* find the tmp file */
	sprintf(tmpname, TMPNAME, stbuf.st_ino, stbuf.st_dev);
	tmpfd = open(tmpname, O_RDONLY);
	if (tmpfd < 0)
	{
		perror(tmpname);
		return;
	}

	/* make sure the file hasn't been modified more recently */
	mtime = stbuf.st_mtime;
	fstat(tmpfd, &stbuf);
	if (stbuf.st_mtime < mtime)
	{
		printf("\"%s\" has been modified more recently than its recoverable version\n", filename);
		puts("Do you still want to recover it?\n");
		puts("\ty - Yes, discard the current version and recover it.\n");
		puts("\tn - No, discard the recoverable version and keep the current version\n");
		puts("\tq - Quit without doing anything for this file.\n");
		puts("Enter y, n, or q --> ");
		fflush(stdout);
		for (;;)
		{
			switch (getchar())
			{
			  case 'y':
			  case 'Y':
				goto BreakBreak;

			  case 'n':
			  case 'N':
				close(tmpfd);
				unlink(tmpname);
				return;

			  case 'q':
			  case 'Q':
				close(tmpfd);
				return;
			}
		}
BreakBreak:;
	}

	/* make sure this tmp file is intact */
	if (read(tmpfd, &hdr, BLKSIZE) != BLKSIZE)
	{
		fprintf(stderr, "%s: bad header in tmp file\n", filename);
		close(tmpfd);
		unlink(tmpname);
		return;
	}
	for (i = j = 1; i < MAXBLKS && hdr.n[i]; i++)
	{
		if (hdr.n[i] > j)
		{
			j = hdr.n[i];
		}
	}
	lseek(tmpfd, (long)j * (long)BLKSIZE, 0);
	if (read(tmpfd, &text, BLKSIZE) != BLKSIZE)
	{
		fprintf(stderr, "%s: bad data block in tmp file\n", filename);
		close(tmpfd);
		unlink(tmpname);
		return;
	}

	/* open the normal text file for writing */
	fp = fopen(filename, "w");
	if (!fp)
	{
		perror(filename);
		close(tmpfd);
		return;
	}

	/* copy the text */
	copytext(tmpfd, fp);

	/* cleanup */
	close(tmpfd);
	fclose(fp);
	unlink(tmpname);
}


/* This function moves text from the tmp file to the normal file */
copytext(tmpfd, fp)
	int	tmpfd;	/* fd of the tmp file */
	FILE	*fp;	/* the stream to write it to */
{
	int	i;

	/* write the data blocks to the normal text file */
	for (i = 1; i < MAXBLKS && hdr.n[i]; i++)
	{
		lseek(tmpfd, (long)hdr.n[i] * (long)BLKSIZE, 0);
		read(tmpfd, &text, BLKSIZE);
		fputs(text.c, fp);
	}
}
