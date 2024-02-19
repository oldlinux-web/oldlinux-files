#ifndef LINT
static char sccsid[]="@(#) getfile.c 2.7 88/01/24 12:44:23";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
*/

#include "options.h"
/*
This function copies n characters from the source file to the destination

Input:   out_f:    		destination file
         in_f:     		source file
         count:         count of characters to copy
         docrc:         0 if crc not wanted

If count is -1, copying is done until eof is encountered.

The source file is transferred to the current file pointer position in the
destination file, using the handles provided.  Function return value is 0
if no error, 2 if write error, and 3 if read error.

If docrc is not 0, the global variable crccode is updated via addbfcrc().
This is done even if the output is to the null device.

If UNBUF_IO is defined, and if more than UNBUF_LIMIT bytes are 
being transferred or copying is to end of file, the data transfer 
is done using low-level read() and write() functions, which must 
be defined elsewhere.  File descriptors are obtained for this 
purpose using the fileno() macro, which must be provided elsewhere 
too.  This is meant to provide greater efficiency on some systems.
The files of type ZOOFILE are synchronized with their file 
descriptors by doing a reasonable number of seeks and other
miscellaneous operations before and after the transfer.  Such 
simultaneous use of buffered and unbuffered files is not
portable and should not be used without extensive testing.
*/

#ifdef UNBUF_IO
int read PARMS ((int, VOIDPTR, unsigned));
int write PARMS ((int, VOIDPTR, unsigned));
long lseek PARMS ((int, long, int));
long tell PARMS ((int));
#endif /* UNBUF_IO */

#include "zoo.h"		/* satisfy declarations in zooio.h */
#include "zooio.h"
#include "various.h"
#include "zoofns.h"
#include "zoomem.h"

int getfile (in_f, out_f, count, docrc)
ZOOFILE in_f, out_f;
long count;
int docrc;
{
   register int how_much;
#ifdef UNBUF_IO
	int in_d, out_d;	/* file descriptors for unbuffered I/O */
#endif /* UNBUF_IO */

#ifdef UNBUF_IO
	if (out_f != NULLFILE && (count == -1 || count > UNBUF_LIMIT)) {
		in_d = fileno (in_f);		/* get ..						*/
		out_d = fileno (out_f);		/* ... file descriptors		*/

		/* Synchronize buffered and unbuffered files */
		zooseek (in_f, zootell (in_f), 0);
		zooseek (out_f, zootell (out_f), 0);

#if 0
		lseek (in_d, zootell (in_f), 0);
		lseek (out_d, zootell (out_f), 0);
#endif

		if (count == -1) {
			while ((how_much = read (in_d, out_buf_adr, MEM_BLOCK_SIZE)) > 0) {
				if (how_much == -1 ||
						write (out_d, out_buf_adr, how_much) != how_much)
					return (2);
				if (docrc)
					addbfcrc (out_buf_adr,how_much);
			}
			zooseek (in_f, tell (in_d), 0);		/* resynch	*/
			zooseek (out_f, tell (out_d), 0);	/* resynch	*/

#ifndef NDEBUG
			if (ftell (in_f) != tell (in_d) || ftell (out_f) != tell (out_d)) {
				prterror ('w', "seek mismatch in copy to EOF\n");
				printf ("in_f =%6ld, in_d =%6ld\n", ftell (in_f),  tell (in_d));
				printf ("out_f=%6ld, out_d=%6ld\n", ftell (out_f), tell (out_d));
			}
#endif /* NDEBUG */

			return (0);
		}

		while (count > 0) {
			if (count > MEM_BLOCK_SIZE)
				how_much = MEM_BLOCK_SIZE;
			else
				how_much = (int) count;
			count -= how_much;
			if (read (in_d, out_buf_adr, how_much) != how_much)
				return (3);
			if (docrc)
				addbfcrc (out_buf_adr, how_much);
			if (write (out_d, out_buf_adr, how_much) != how_much)
				return (2);
		}
		zooseek (in_f, tell (in_d), 0);		/* resynch	*/
		zooseek (out_f, tell (out_d), 0);	/* resynch	*/
#ifndef NDEBUG
		if (ftell (in_f) != tell (in_d) || ftell (out_f) != tell (out_d))
			 prterror ('w', "seek mismatch in fixed length copy\n");
#endif /* NDEBUG */
		return (0);
	}
#endif /* UNBUF_IO */

   if (count == -1) {
      while ((how_much = zooread (in_f, out_buf_adr, MEM_BLOCK_SIZE)) > 0) {
         if (how_much == -1 ||
               zoowrite (out_f, out_buf_adr, how_much) != how_much)
            return (2);
         if (docrc)
            addbfcrc (out_buf_adr,how_much);
      }
      return (0);
   }

   while (count > 0) {
      if (count > MEM_BLOCK_SIZE)
         how_much = MEM_BLOCK_SIZE;
      else
         how_much = (int) count;
      count -= how_much;
      if (zooread (in_f, out_buf_adr, how_much) != how_much)
         return (3);
      if (docrc)
         addbfcrc (out_buf_adr, how_much);
      if (zoowrite (out_f, out_buf_adr, how_much) != how_much)
         return (2);
   }
   return (0);
}
