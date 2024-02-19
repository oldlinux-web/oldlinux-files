/*
 *   This is dvips, a freely redistributable PostScript driver
 *   for dvi files.  It is (C) Copyright 1987 by Tomas Rokicki.
 *   You may modify and use this program to your heart's content,
 *   so long as you send modifications to Tomas Rokicki.  It can
 *   be included in any distribution, commercial or otherwise, so
 *   long as the banner string defined in structures.h is not
 *   modified (except for the version number) and this banner is
 *   printed on program invocation, or can be printed on program
 *   invocation with the -? option.
 */
/*
 *   dviinput.c of dvisw software package.  This code is copyright (C) 1986
 *   by Radical Eye Software.
 *
 *   Input files for the dvi file.  These routines could probably be
 *   sped up significantly; they are very machine dependent, though, so
 *   I will leave such tuning to the installer.   They simply get and
 *   return bytes of the dvifile in batches of one, two, three, and four,
 *   updateing the current position as necessary.
 */
#include "structures.h"
void error() ;
extern integer curpos ;
extern FILE *dvifile ;

void
abortpage()
{
   error("! unexpected eof on DVI file") ;
}

shalfword
dvibyte()
{
  register shalfword i ;
  curpos++ ;
  if ((i=getc(dvifile))==EOF)
    abortpage() ;
  return(i) ;
}

halfword
twobytes()
{
  register halfword i ;
  i = dvibyte() ;
  return(i*256+dvibyte()) ; }

integer
threebytes()
{
  register integer i ;
  i = twobytes() ;
  return(i*256+dvibyte()) ; }

shalfword
signedbyte()
{
  register shalfword i ;
  curpos++ ;
  if ((i=getc(dvifile))==EOF)
    abortpage() ;
  if (i<128) return(i) ;
  else return(i-256) ;
}

shalfword
signedpair()
{
  register shalfword i ;
  i = signedbyte() ;
  return(i*256+dvibyte()) ;
}

integer
signedtrio()
{
  register integer i ;
  i = signedpair() ;
  return(i*256+dvibyte()) ;
}

integer
signedquad()
{
  register integer i ;
  i = signedpair() ;
  return(i*65536+twobytes()) ;
}

void
skipover(i)
	int i ;
{
  while (i-->0) (void)dvibyte() ;
}
