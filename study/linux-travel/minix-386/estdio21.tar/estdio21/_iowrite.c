/*                             _ i o w r i t e                             */

/* Call write(2) repeatedly until all data written or an error occurs. Returns
 * the number of bytes written.
 */

#include <stdiolib.h>

/*LINTLIBRARY*/

__stdiosize_t __iowrite F3(int, fd, char *, p, __stdiosize_t, n)

{
  register int wrote;			/* bytes written by write call */
  register __stdiosize_t wb;		/* byte to write each call */
  __stdiosize_t w;			/* total bytes written */

  for (w = 0; ; p += wrote) {
    wb = n;
    if (wb > INT_MAX)
      wb = INT_MAX;
    do
      wrote = write(fd, p, (unsigned int) wb);
    while (wrote == -1 && (errno == EINTR || errno == EAGAIN));
    if (wrote == -1)
      break;
    w += wrote;
    if ((n -= wrote) == 0)
      break;
  }
  return w;
}
