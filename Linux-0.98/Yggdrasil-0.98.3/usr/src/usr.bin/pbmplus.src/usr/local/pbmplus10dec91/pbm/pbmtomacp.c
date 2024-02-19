/* pbmtomacp.c - read a portable bitmap and produce a MacPaint bitmap file
**
** Copyright (C) 1988 by Douwe vand der Schaaf.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pbm.h"
#include "macp.h"

#define TRUE		1
#define FALSE		0
#define EQUAL		1
#define UNEQUAL		0

static void fillbits ARGS(( bit **bits, bit **bitsr, int top, int left, int bottom, int right ));
static void writemacp ARGS(( bit **bits ));
static int packit ARGS(( bit *pb, bit *bits ));
static void filltemp ARGS(( bit *dest, bit *src ));
static void sendbytes ARGS(( bit *pb, register int npb ));
static void header ARGS(( void ));

static FILE *fdout;

void main(argc, argv)
int argc;
char *argv[];
{ FILE *ifp;
  register bit **bits, **bitsr;
  int argn, rows, cols;
  int left,bottom,right,top;
  int lflg, rflg, tflg, bflg;
  char name[100];
  char *usage = "[-l left] [-r right] [-b bottom] [-t top] [pbmfile]";

  pbm_init( &argc, argv );

  argn = 1;
  fdout = stdout;
  lflg = rflg = tflg = bflg = 0;

  while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
  { switch ( argv[argn][1] )
    { case 'l':
      lflg++;
      argn++;
      left = atoi( argv[argn] );
      break;

      case 'r':
      rflg++;
      argn++;
      right = atoi( argv[argn] );
      break;

      case 't':
      tflg++;
      argn++;
      top = atoi( argv[argn] );
      break;

      case 'b':
      bflg++;
      argn++;
      bottom = atoi( argv[argn] );
      break;

      case '?':
      default:
      pm_usage( usage );
    }
    ++argn;
  }

  if ( argn == argc )
  { ifp = stdin;
    strcpy( name, "noname" );
  }
  else
  { ifp = pm_openr( argv[argn] );
    strcpy( name, argv[argn] );
    ++argn;
  }

  if ( argn != argc )
    pm_usage( usage );

  bitsr = pbm_readpbm( ifp, &cols, &rows );

  pm_close( ifp );

  bits = pbm_allocarray( MAX_COLS, MAX_LINES );

  if( !lflg )
    left = 0;

  if( rflg )
  { if( right - left >= MAX_COLS )
      right = left + MAX_COLS - 1;
  }
  else
    right = ( left + MAX_COLS > cols ) ? ( cols - 1 ) : ( left + MAX_COLS - 1 );

  if( !tflg )
    top = 0;

  if( bflg )
  { if( bottom - top >= MAX_LINES )
      bottom = top + MAX_LINES - 1;
  }
  else
    bottom = ( top + MAX_LINES > rows ) ?
		   ( rows - 1 ) : ( top + MAX_LINES - 1 );
  
    if( right <= left || left < 0 || right - left + 1 > MAX_COLS )
      pm_error("error in right (= %d) and/or left (=%d)",right,left );
    if( bottom <= top || top < 0 || bottom - top + 1 > MAX_LINES )
      pm_error("error in bottom (= %d) and/or top (=%d)",bottom,top );

  fillbits( bits, bitsr, top, left, bottom, right );

  writemacp( bits );

  exit( 0 );

}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* centreer het over te zenden plaatje in het MacPaint document
 *
 * Het plaatje wordt vanaf al of niet opgegeven (left, bottom)
 * in een pbm bitmap van de juist macpaint afmetingen gezet,
 * en eventueel afgekapt.
 */
static void
fillbits( bits, bitsr, top, left, bottom, right )
bit **bits, **bitsr;
int top, left, bottom, right;
{ register bit *bi, *bir;
  register int i, j;
  register int bottomr, leftr, topr, rightr;
  int width, height;

  width = right - left + 1;
  leftr = (MAX_COLS - width) / 2;
  rightr = leftr + width - 1;

  height = bottom - top + 1;
  topr = ( MAX_LINES - height ) / 2;
  bottomr = topr + height - 1;

  for( i = 0; i < topr; i++ )
  { bi = bits[i];
    for( j = 0; j < MAX_COLS; j++ )
      *bi++ = 0;
  }

  for( i = topr; i <= bottomr; i++ )
  { bi = bits[i];
    { for( j = 0; j < leftr; j++ )
	*bi++ = 0;
      bir = bitsr[ i - topr + top ];
      for( j = leftr; j <= rightr; j++ )
	*bi++ = bir[j - leftr + left];
      for( j = rightr + 1; j < MAX_COLS; j++ )
	*bi++ = 0;
  } }

  for( i = bottomr + 1; i < MAX_LINES; i++ )
  { bi = bits[i];
    for( j = 0; j < MAX_COLS; j++ )
      *bi++ = 0;
  }
} /* fillbits */
      
/* - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void
writemacp( bits )
bit **bits;
{ register int i;
  bit pb[MAX_COLS * 2];
  int npb;

  header();
  for( i=0; i < MAX_LINES; i++ )
  { npb = packit( pb, bits[i] );
    sendbytes( pb, npb );
  }
} /* writemacp */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* pack regel van MacPaint doc in Apple's format
 * return value = # of bytes in pb 
 */
static int
packit( pb, bits )
bit *pb, *bits;
{ register int charcount, npb, newcount, flg;
  bit temp[72];
  bit *count, *srcb, *destb, save;

  srcb = bits; destb = temp;
  filltemp( destb, srcb );
  srcb = temp;
  destb = pb;
  npb = 0;
  charcount = BYTES_WIDE;
  flg = EQUAL;
  while( charcount )
  { save = *srcb++;
    charcount--;
    newcount = 1;
    while( (*srcb == save) && charcount )
    { srcb++;
      newcount++;
      charcount--;
    }
    if( newcount > 2 )
    { count = destb++;
      *count = 257 - newcount;
      *destb++ = save;
      npb += 2;
      flg = EQUAL;
    }
    else
    { if( flg == EQUAL )
      { count = destb++;
	*count = newcount - 1;
	npb++;
      }
      else
	*count += newcount;
      while( newcount-- )
      { *destb++ = save;
        npb++;
      }
      flg = UNEQUAL;
  } }
  return npb;
} /* packit */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void
filltemp( dest, src )
bit *dest, *src;
{ register unsigned char ch, zero, acht;
  register int i, j;

  zero = '\0';
  acht = 8;
  i = BYTES_WIDE;
  while( i-- )
  { ch = zero; 
    j = acht;
    while( j-- )
    { ch <<= 1;
      if( *src++ )
	ch++;
    }
    *dest++ = ch;
  }
} /* filltemp */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void
sendbytes( pb, npb )
bit *pb;
register int npb;
{ register bit *b;

  b = pb;
  while( npb-- )
    (void) putc( *b++, fdout );
} /* sendbytes */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void
header()
{ register int i;
  register char ch;

  /* header contains nothing ... */
  ch = '\0';
  for(i = 0; i < HEADER_LENGTH; i++ )
    (void) putc( ch, fdout );
} /* header */
