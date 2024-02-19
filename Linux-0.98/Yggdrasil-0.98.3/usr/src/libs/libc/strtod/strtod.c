/*
 * This file is part of the C library for Linux and is
 * covered by the GNU General Public license version 2, or
 * any later version.
 * 
 * Copyright (C) 1992 Hoongjiu Lu
 *
 * This file is borrowed from gas 1.38.1.
 * Copyright (C) 1987 Free Software Foundation, Inc.
 */
#include <ansidecl.h>
#include <stdlib.h>
#include <errno.h>

char EXP_CHARS[] = "eE";

#undef DEBUG

#ifdef DEBUG
#include <stdio.h>
#endif

#include <float.h>
#undef HUGE_VAL
#define HUGE_VAL   DBL_MAX

#include "strtod.h"

/*
 * Build any floating-point literal here.
 * Also build any bignum literal here.
 */

/* LITTLENUM_TYPE	generic_buffer [6];	JF this is a hack */
/* Seems atof_machine can backscan through generic_bignum and hit whatever
   happens to be loaded before it in memory.  And its way too complicated
   for me to fix right.  Thus a hack.  JF:  Just make generic_bignum bigger,
   and never write into the early words, thus they'll always be zero.
   I hate Dean's floating-point code.  Bleh.
 */
LITTLENUM_TYPE	generic_bignum [SIZE_OF_LARGE_NUMBER+6];
FLONUM_TYPE	generic_floating_point_number =
{
  & generic_bignum [6],		/* low (JF: Was 0) */
  & generic_bignum [SIZE_OF_LARGE_NUMBER+6 - 1], /* high JF: (added +6) */
  0,				/* leader */
  0,				/* exponent */
  0				/* sign */
};

static inline int /* Knows about order of bytes in address. */
md_number_to_chars (con, value, nbytes)
     char	con [];	/* Return 'nbytes' of chars here. */
     long int	value;		/* The value of the bits. */
     int	nbytes;		/* Number of bytes in the output. */
{
  register char * p = con;

  switch (nbytes) {
  case 1:
    p[0] = value & 0xff;
    break;
  case 2:
    p[0] = value & 0xff;
    p[1] = (value >> 8) & 0xff;
    break;
  case 4:
    p[0] = value & 0xff;
    p[1] = (value>>8) & 0xff;
    p[2] = (value>>16) & 0xff;
    p[3] = (value>>24) & 0xff;
    break;
  default:
    errno = ERANGE;
    return 1;
  }
  return 0;
}

#define MAX_LITTLENUMS 6

/* Turn the string pointed to by litP into a floating point constant of type
   type, and emit the appropriate bytes.  The number of LITTLENUMS emitted
   is stored in *sizeP .  An error message is returned, or NULL on OK.
 */
double
#ifndef DEBUG
DEFUN(strtod, (nptr, endptr), CONST char *nptr AND char **endptr)
#else
Strtod (nptr, endptr)
char *nptr;
char **endptr;
#endif
{
  double num;
  char *litP = (char *) &num;
  int prec = NUMBER_OF_WORDS_IN_DOUBLE;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  LITTLENUM_TYPE *wordP;
  char *next;

  if ((next = atof_ieee (nptr,words)) && endptr)
    *endptr = next;

  if (!next) {
#ifdef DEBUG
fprintf (stderr, "bad:%c, %d\n", generic_floating_point_number.sign,
generic_floating_point_number.exponent);
#endif
    /* fail to convert */
    switch (errno) {
    case EINVAL:
      if (endptr) *endptr = (char *) nptr;
      num = 0.0;
      break;
    case ERANGE:
      num = (generic_floating_point_number.exponent > 0) ?
		((generic_floating_point_number.sign == '+') ?
		HUGE_VAL : -HUGE_VAL) : 0.0;
      break;
    }
    return num;
  } 

  /* this loops outputs the LITTLENUMs in REVERSE order; in accord with
     the bigendian 386 */
  for(wordP = words + prec - 1;prec--;) {
    md_number_to_chars (litP, (long) (*wordP--), sizeof(LITTLENUM_TYPE));
    litP += sizeof(LITTLENUM_TYPE);
  }
  return num;	/* Someone should teach Dean about null pointers */
}
