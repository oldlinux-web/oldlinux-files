#include "stdioprivate.h"

static inline int __macro__getc(FILE *stream)
{
  return getc(stream);
}

static inline int __macro__getchar (void)
{
  return getchar();
}

static inline int __macro__putc(int c, FILE *stream)
{
  return putc(c, stream);
}

static inline int __macro__putchar(int c)
{
  return putchar(c);
}

#undef getc
#undef getchar
#undef putc
#undef putchar

int getc(FILE *stream)
{
  return __macro__getc (stream);
}

int getchar (void)
{
  return __macro__getchar ();
}

int putc(int c, FILE *stream)
{
  return __macro__putc(c, stream);
}

int putchar(int c)
{
  return __macro__putchar(c);
}
