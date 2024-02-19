#ifndef lint
static char rcsid[] = "$Header: /usr/people/sam/tiff/libtiff/RCS/tif_compat.c,v 1.10 91/09/27 09:40:42 sam Exp $";
#endif

/*
 * Copyright (c) 1988, 1989, 1990, 1991 Sam Leffler
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * TIFF Library Compatibility Routines.
 */
#include "tiffcompat.h"

#if defined(unix) || defined(MSDOS)
#include <sys/stat.h>

long
TIFFGetFileSize(fd)
	int fd;
{
	struct stat sb;

	return (fstat(fd, &sb) == -1 ? 0 : sb.st_size);
}
#endif

#if defined(unix) && defined(MMAP_SUPPORT)
#include <sys/mman.h>

int
TIFFMapFileContents(fd, pbase, psize)
	int fd;
	char **pbase;
	long *psize;
{
	long size = TIFFGetFileSize(fd);
	if (size != -1) {
		*pbase = (char *) mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
		if (*pbase != (char *)-1) {
			*psize = size;
			return (1);
		}
	}
	return (0);
}

void
TIFFUnmapFileContents(base, size)
	char *base;
	long size;
{
	(void) munmap(base, size);
}
#endif /* defined(unix) && defined(MMAP_SUPPORT) */

#if defined(THINK_C) || defined(applec)
long
TIFFGetFileSize(int fd)
{
	long pos, eof;
	
	pos = lseek(fd, 0, SEEK_CUR);
	eof = lseek(fd, 0, SEEK_END);
	lseek(fd, pos, SEEK_SET);
	return(eof);
}
#endif /* THINK_C || applec */

#if defined(applec)
#include <ioctl.h>
#include <Files.h>
#undef lseek

long
mpw_lseek(int fd, long offset, int whence)
{
	long filepos, filesize, newpos;
	short macfd;
	
	if ((filepos = lseek(fd, 0, SEEK_CUR)) < 0 ||
	    (filesize = lseek(fd, 0, SEEK_END)) < 0)
		return (EOF);
	newpos = offset + (whence == SEEK_SET ? 0 : 
			   whence == SEEK_CUR ? filepos :
						filesize);
	if (newpos > filesize)
		if (ioctl(fd, FIOREFNUM, &macfd) == -1 ||
		    SetEOF(macfd, newpos) != 0)
			return (EOF);
	return (lseek(fd, newpos, SEEK_SET));
}
#endif /* applec */
