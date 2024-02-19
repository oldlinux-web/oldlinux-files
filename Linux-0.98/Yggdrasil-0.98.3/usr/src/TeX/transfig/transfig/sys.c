/*
 * TransFig: Facility for Translating Fig code
 * Copyright (c) 1991 Micah Beck, Cornell University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Cornell University not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Cornell University makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * CORNELL UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL CORNELL UNIVERSITY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include "transfig.h"

#define MAXSYS 10000
static char sysbuf[MAXSYS];

char *sysls()
{
  FILE *ls;
  int i;
  char c;

  ls = popen("/bin/ls *.fig", "r");
  i = 0;
  c = fgetc(ls);
  while (c != EOF & i < MAXSYS-1)
  {
	sysbuf[i] = c;
	i += 1;
	c = fgetc(ls);
  }
  sysbuf[i] = '\0';
  return sysbuf;
}

sysmv(f)
char *f;
{
  sprintf(sysbuf, "%s~", f);
  unlink(sysbuf);
  if (!link(f, sysbuf)) unlink(f);
}

char *strip(str, suf)
char *str, *suf;
{
  char *p1, *p2;

  for (p1 = &str[strlen(str)-1], p2 = &suf[strlen(suf)-1];
	(p1 >= str && p2 >= suf) && (*p1 == *p2);
	--p1, --p2);

  if (p2 < suf)
  {
	*(p1+1) = '\0';
	return str;
  } else
	return NULL;
}

char *mksuff(name, suff)
char *name, *suff;
{
  char *temp;

  temp = (char *)malloc(strlen(name)+strlen(suff)+1);
  strcpy(temp, name);
  strcat(temp, suff);
  return temp;
}
