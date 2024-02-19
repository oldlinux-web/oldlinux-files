/*			_ f o p e n
 *
 * Open a file
 *
 * This function scans the mode with which a channel should be
 * opened. It then opens the channel using that mode and returns
 * the channel number to the caller. On error, the value -1 will
 * be returned.
 *
 * If the function succeeds, the flags argument will be set to the
 * mode with which the channel was opened. If the fd argument is
 * -1, the channel will be allocated, otherwise the specified channel
 * will be used.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

#define CREATMODE 0666			/* mode to creat file */

int __fopen F4(CONST char *, name,
              CONST char *, mode,
              int, fd,
	      register short *, flags)

{
  register int openmode;		/* mode for open */
  int readwrite;			/* basic mode */
  int update;				/* read and write required */

  readwrite = *mode++;

  if (*mode == 'b')
    mode++;
  if ((update = *mode == '+') != 0) {
    *flags = _IORW;
    openmode = O_RDWR;
  }

  if (readwrite == 'r') {

    if (! update) {
      *flags = _IOREAD;
      openmode = O_RDONLY;
    }

    if (fd < 0)
      fd = open(name, openmode, CREATMODE);
  }

  else {
    if (! update) {
      *flags = _IOWRITE;
      openmode = O_WRONLY;
    }

    openmode |= O_CREAT;
    if (readwrite != 'a')
      openmode |= O_TRUNC;
    else {
      openmode |= O_APPEND;
#ifndef		OPEN3
      *flags |= _IOAPPEND;
#endif
    }

    if (fd < 0)
      fd = open(name, openmode, CREATMODE);
  }

  return fd;
}
