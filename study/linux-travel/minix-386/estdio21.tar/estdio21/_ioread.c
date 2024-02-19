/*                              _ i o r e a d                              */

/* Call read(2) repeatedly to read data. Return the number of bytes read.
 * Unlike _iowrite(), this will not retry the read until the required
 * number of bytes have been transferred. This is so that streams
 * that attach to things like terminals will be able to return their
 * natural record lengths.
 */

#include <stdiolib.h>

/*LINTLIBRARY*/

__stdiosize_t __ioread F3(int, fd, char *, p, __stdiosize_t, n)

{
  register int r;			/* bytes read by read call */

  if (n > INT_MAX)
    n = INT_MAX;
  do
    r = read(fd, p, n);
  while (r == -1 && (errno == EINTR || errno == EAGAIN));

  return r < 0 ? 0 : r;
}
