/*                             _ v f s c a n f                             */

/* This funcion forms the basis for all the scanf family of functions.
 * The function returns EOF on end of input, and a short count for
 * missing or illegal data items. Hooks are provided for the floating
 * point input routines.
 */

#include <ctype.h>
#include "stdiolib.h"
#include "bitset.h"

#define CHARSET		(1 << CHAR_BIT)

#define BLANK(c)	isspace(c)	/* was ((c) == ' ' || (c) == '\t') */
#define NEXTCH()	(nbytes++, ch = getc(fp))
#define ENDFIELD() \
  if (fieldok) { \
    if (! noassign) items++; \
  } \
  else \
    goto Putback;

/* Conversion of each digit to the internal number representation
 * relies on the fact that the character codes for '0-'9', the
 * codes for 'a'-'f' and the codes for 'A'-'F' are each ordered
 * consecutively.
 */

#if '0'+1 != '1' || '1'+1 != '2' || '2'+1 != '3' || '3'+1 != '4' || \
    '4'+1 != '5' || '5'+1 != '6' || '6'+1 != '7' || '7'+1 != '8' || \
    '8'+1 != '9' || \
    'a'+1 != 'b' || 'b'+1 != 'c' || 'c'+1 != 'd' || 'd'+1 != 'e' || \
    'e'+1 != 'f' || \
    'A'+1 != 'B' || 'B'+1 != 'C' || 'C'+1 != 'D' || 'D'+1 != 'E' || \
    'E'+1 != 'F'
  << Violation of collating sequence assumption >>
#endif

int __vfscanf F3(register FILE *, fp, register CONST char *, fmt, VA_LIST, args)

{
  int items;			/* number of items done */
  int ch;			/* next character */
  int lch;			/* lowercase version of character */
  int i;			/* general index */
  int nbytes;			/* bytes read */
  char *p;			/* string pointer */
  char skipspace;		/* force white space skip */
  char noassign;		/* do not do assignment */
  char longdoubleflag;		/* pointer is longdouble */
  char longflag;		/* pointer is long */
  char shortflag;		/* pointer is short */
  char fieldok;			/* this field parsed ok */
  char invertedset;		/* inverted set */
  char sign;			/* conversion is signed */
  char negative;		/* number is negative */
  int fieldwidth;		/* width of field */
  unsigned radix;		/* radix for conversion */
  unsigned lastdigit;		/* last digit (0-9) in radix */
  long longv;			/* long value for conversion */
  bitstring(cset, CHARSET);	/* set for %[] */

/* Prime the look ahead character */
  if ((ch = getc(fp)) == EOF)
    return EOF;

  for (nbytes = items = 0; ; fmt++) {

/* Skip whitespace in format */
    for (skipspace = 0; BLANK(*fmt); fmt++)
      skipspace = 1;

/* Check for end of format or end of input */
    if (*fmt == 0 || ch == EOF)
      goto Putback;

/* Check for verbatim character */
    if (*fmt != '%') {
      while (BLANK(ch))
        NEXTCH();

      if (ch != *fmt)
        goto Putback;

      NEXTCH();
      continue;
    }

/* Format precursor seen --- see if assignment required */
    if ((noassign = *++fmt == '*') != 0)
      fmt++;

/* Check for width specification */
    fieldwidth = -1;
    if (*fmt >= '0' && *fmt <= '9') {
      for (fieldwidth = 0; *fmt >= '0' && *fmt <= '9'; )
        fieldwidth = fieldwidth * 10 + *fmt++ - '0';
    }

/* Check for long, short or longdouble pointers */
    if ((longflag = *fmt == 'l')       != 0 ||
	(shortflag = *fmt == 'h')      != 0 ||
	(longdoubleflag = *fmt == 'L') != 0)
      fmt++;

/* Skip whitespace in the input stream */
    if (skipspace || (*fmt != 'c' && *fmt != '[')) {
      while (isspace(ch))
        NEXTCH();
    }

/* Assume that field is not parsed */
    fieldok = 0;

/* Initialise number conversion --- assume unsigned decimal */
    longv     = 0;
    negative  = 0;
    sign      = 0;
    radix     = 10;
    lastdigit = '9';

    switch (*fmt) {

/* Bytes read */
    case 'n':
      *(VA_ARG(args, int *)) = nbytes;
      break;

/* Pointer */
    case 'p':
      longflag  = sizeof(void *) > sizeof(unsigned int);
      radix     = 16;
      goto oxud;

/* General integer */
    case 'i':

/* Octal or hexadecimal indication */
      if (fieldwidth != 0 && ch == '0') {
	NEXTCH();
	if (--fieldwidth != 0) {
	  if (ch == 'x' || ch == 'X') {
	    NEXTCH();
	    radix = 16;
	  }
	  else {
	    radix = 8;
	    lastdigit = '7';
	  }
	}
	goto atoi;
      }
/* Fall through to signed decimal */

    case 'd': sign  = 1;                  goto oxud;
    case 'o': radix = 8; lastdigit = '7'; goto oxud;
    case 'x': radix = 16;
    case 'u':

oxud:

/* Look for sign if number is signed */
      if (fieldwidth != 0) {
	if (sign) {
	  if (ch == '+') {
	    NEXTCH();
	    fieldwidth--;
	  }
	  else if (ch == '-') {
	    negative = 1;
	    NEXTCH();
	    fieldwidth--;
	  }
	}

/* Look for leading 0x for hexadecimal */
	else {
	  if (radix == 16 && ch == '0') {
	    NEXTCH();
	    if (--fieldwidth != 0) {
	      if (ch != 'x' && ch != 'X')
		fieldok = 1;
	      else {
		NEXTCH();
		fieldwidth--;
	      }
	    }
	  }
	}
      }

atoi:

/* Scan and convert */
      while (fieldwidth < 0 || fieldwidth--) {
	if (ch >= '0' && ch <= lastdigit)
	  ch -= '0';
	else if (ch == EOF)
	  break;
	else {
	  lch = TOLOWER(ch);
	  if (lch >= 'a' && (lch += 10 - 'a') < radix)
	    ch = lch;
	  else
	    break;
	}
	longv = longv * radix + ch;
	NEXTCH();
	fieldok = 1;
      }

/* Complete the conversion */      
      if (! noassign) {
	if (negative)
	  longv = -longv;
	if (longflag)
	  if (sign) *VA_ARG(args, long *)           = longv;
	  else      *VA_ARG(args, unsigned long *)  = longv;
	else if (shortflag)
	  if (sign) *VA_ARG(args, short *)          = (short) longv;
	  else      *VA_ARG(args, unsigned short *) = (unsigned short) longv;
	else
	  if (sign) *VA_ARG(args, int *)            = (int) longv;
	  else      *VA_ARG(args, unsigned int *)   = (unsigned int) longv;
      }
      ENDFIELD();
      break;

/* Floating point */
    case 'e':
    case 'f':
    case 'g':
      (void) ungetc(ch, fp);
      if (noassign)
	i   = 0;
      else {
	if (longdoubleflag)
	  i   = 1;
	else if (longflag)
	  i   = 2;
	else
	  i   = 3;
      }
      nbytes += __tvc(fp, fieldwidth, &args, i, &fieldok);
      ch = getc(fp);
      ENDFIELD();
      break;

/* Single character */
    case 'c':
      if (fieldwidth == -1)
	fieldwidth = 1;

/* Initialise the string pointer */	
      if (! noassign)
        p = VA_ARG(args, char *);
	
      while (fieldwidth-- && ch >= 0) {
	if (! noassign)
	  *p++ = ch;
	NEXTCH();
	fieldok = 1;
      }
      ENDFIELD();
      break;

    case 's':

/* Initialise the string pointer */    
      if (! noassign)
	p = VA_ARG(args, char *);
	
      while (fieldwidth < 0 || fieldwidth--) {
        if (ch <= 0 || BLANK(ch))
	  break;
	if (! noassign)
	  *p++ = ch;
	NEXTCH();
	fieldok = 1;
      }

/* Terminate the string with a null */
      if (! noassign)
	*p++ = 0;
      ENDFIELD();
      break;

    case '[':

 /* Clear the bit set */
      bitempty(cset, CHARSET);

/* Check for inverted set */
      if ((invertedset = *++fmt == '^') != 0)
	fmt++;

/* Check for right bracket in set */
      if (*fmt == ']')
        bitset(cset, *fmt++);

/* Scan	search set, setting bits */
      while (*fmt != 0 && *fmt != ']') {
	if (fmt[1] != '-' || fmt[2] == ']' || fmt[2] == 0 || fmt[0] > fmt[2])
	  bitset(cset, *fmt++);
	else {
	  for (i = fmt[0]; i <= fmt[2]; i++)
	    bitset(cset, i);
	  fmt += 3;
	}
      }

/* Check for unsatisfactory set construction */
      if (*fmt != ']')
	goto Putback;

/* Initialise string pointer */
      if (! noassign)
	p = VA_ARG(args, char *);

/* Scan input for satisfactory characters */
      while (fieldwidth < 0 || fieldwidth--) {
        if (BLANK(ch) || ch <= 0 || ! (bittest(cset, ch) ^ invertedset))
	  break;
	if (! noassign)
	  *p++ = ch;
	NEXTCH();
	fieldok = 1;
      }

/* Terminate string with null */
      if (! noassign)
	*p++ = 0;
      ENDFIELD();
      break;
    }
  }

/* Restore the look ahead character */
Putback:
  (void) ungetc(ch, fp);
  return items;
}
