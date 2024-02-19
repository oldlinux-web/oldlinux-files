/*				_ f g e t l x
 *
 * Read a line from stream
 *
 * The line is read from the specified stream into the named
 * buffer. At most n-1 characters are read. Reading will stop
 * when the buffer is filled, or when a newline character is
 * read. The buffer is null terminated. The newline is included
 * in the string.
 *
 * The function returns a pointer to the null terminator at the
 * end of the buffer. If no characters were read because of EOF,
 * a null pointer is returned.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

char *__fgetlx F3(char *, buf, __stdiosize_t, n, register FILE *, fp)

{
  register __stdiobuf_t *s;		/* output buffer */
  __stdiobuf_t *r;			/* output buffer */
  __stdiobuf_t *q;			/* input buffer pointer */
  int ch;				/* character read */
  __stdiosize_t bytesleft;		/* bytes left in current load */
  __stdiosize_t bytesread;		/* bytes read */

  if (n <= 1)
    return n > 0 ? (buf[0] = 0, buf) : (char *) (NULL);

  if (GETFLAG(fp, _IOSTDX) == _IOSTDIN && __Zout != NULL &&
      (ALLFLAGS(fp) & ALLFLAGS(__Zout) & _IOLBF) != 0)
    (void) FFLUSH(__Zout);

  for (s = (__stdiobuf_t *) buf, --n; ; ) {
    if ((bytesleft = BYTESINREADBUFFER(fp)) != 0) {
      if (bytesleft > n)
	bytesleft = n;
      n -= bytesleft;
      q = GETREADPTR(fp);

      if ((r = (__stdiobuf_t *) MEMCHR(q, '\n', (size_t) bytesleft)) != NULL)
	bytesread = r - q + 1;

      else {
	bytesread = bytesleft;
	bytesleft = 0;
      }

      MEMCPY(s, q, (size_t) bytesread);

      s += bytesread;

      SETREADPTR(fp, q + bytesread);
    }
    *s = 0;
    if (bytesleft != 0 || n == 0)
      return (char *) (s);
    if ((ch = getc(fp)) == EOF)
      return s == (__stdiobuf_t *) buf ? (char *) (NULL) : (char *) (s);
    if ((*s++ = ch) == '\n' || --n == 0) {
      *s = 0;
      return (char *) (s);
    }
  }
}
