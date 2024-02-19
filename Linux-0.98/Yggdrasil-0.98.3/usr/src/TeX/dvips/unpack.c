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
 *   unpack.c of dvisw software package.  This code is copyright (C) 1986
 *   by Radical Eye Software.
 *
 *   unpacks the raster data from the packed buffer.  This code was 
 *   translated from pktopx.web using an automatic translator, then
 *   converted for this purpose.  This little routine can be very useful
 *   in other drivers as well.
 */
#include "structures.h"

/*
 * external procedures
 */
extern void error();

/*
 *   Some statics for use here.
 */
static halfword bitweight ; 
static halfword dynf ;
static halfword gpower[17] = { 0 , 1 , 3 , 7 , 15 , 31 , 63 , 127 ,
     255 , 511 , 1023 , 2047 , 4095 , 8191 , 16383 , 32767 , 65535 } ; 
static halfword repeatcount ;
static quarterword *p ;

/*
 *   We need procedures to get a nybble, bit, and packed word from the
 *   packed data structure.
 */

shalfword
getnyb ()
{
    if ( bitweight == 0 ) 
    { bitweight = 16 ; 
      return(*p++ & 15) ;
    } else {
      bitweight = 0 ;
      return(*p >> 4) ;
    }
} 

Boolean
getbit ()
{
    bitweight >>= 1 ; 
    if ( bitweight == 0 ) 
    { p++ ;
      bitweight = 128 ;
    } 
    return(*p & bitweight) ;
} 

halfword
pkpackednum ()
{
    register halfword i, j ; 
    i = getnyb () ; 
    if ( i == 0 ) 
    { do { j = getnyb () ; 
        i++ ; 
        } while ( ! ( j != 0 ) ) ; 
      while ( i != 0 ) 
        { j = j * 16 + getnyb () ; 
          i-- ; 
          } 
        return ( j - 15 + ( 13 - dynf ) * 16 + dynf ) ; 
      } 
    else if ( i <= dynf ) return ( i ) ; 
    else if ( i < 14 ) return ( ( i - dynf - 1 ) * 16 + getnyb () + dynf + 1 
    ) ; 
    else 
    { if ( i == 14 ) repeatcount = pkpackednum () ; 
      else repeatcount = 1 ; 
      return ( pkpackednum () ) ; 
      } 
}

/*
 *   And now we have our main routine.
 */
void
unpack(pack, raster, cwidth, cheight, cmd)
	quarterword *pack ;
	halfword *raster ;
	halfword cwidth, cheight, cmd ;
{ 
  register integer i, j ; 
  shalfword wordwidth ; 
  register halfword word, wordweight ;
  shalfword rowsleft ; 
  Boolean turnon ; 
  shalfword hbit ; 
  halfword count ; 

      p = pack ;
      dynf = cmd / 16 ; 
      turnon = cmd & 8 ; 
      wordwidth = (cwidth + 15)/16 ;
      if ( dynf == 14 ) 
      { bitweight = 256 ; 
        for ( i = 1 ; i <= cheight ; i ++ ) 
          { word = 0 ; 
            wordweight = 32768 ; 
            for ( j = 1 ; j <= cwidth ; j ++ ) 
              { if ( getbit () ) word += wordweight ; 
                wordweight >>= 1 ;
                if ( wordweight == 0 ) 
                { *raster++ = word ; 
                  word = 0 ;
                  wordweight = 32768 ; 
                  } 
                } 
              if ( wordweight != 32768 ) 
                 *raster++ = word ; 
            } 
      } else {
        rowsleft = cheight ; 
        hbit = cwidth ; 
        repeatcount = 0 ; 
        wordweight = 16 ; 
        word = 0 ; 
        bitweight = 16 ;
        while ( rowsleft > 0 ) 
          { count = pkpackednum () ; 
            while ( count != 0 ) 
              { if ( ( count < wordweight ) && ( count < hbit ) ) 
                { if ( turnon ) word += gpower [ wordweight ] - gpower 
                  [ wordweight - count ] ; 
                  hbit -= count ; 
                  wordweight -= count ; 
                  count = 0 ; 
                  } 
                else if ( ( count >= hbit ) && ( hbit <= wordweight ) ) 
                { if ( turnon )
                     word += gpower [ wordweight ] - gpower 
                  [ wordweight - hbit ] ; 
                  *raster++ = word ; 
                  for ( i = 1 ; i <= repeatcount ; i ++ ) {
                    for ( j = 1 ; j <= wordwidth ; j ++ ) {
                      *raster = *(raster - wordwidth) ;
                      raster++ ;
                    }
                  }
                  rowsleft -= repeatcount + 1 ; 
                  repeatcount = 0 ; 
                  word = 0 ; 
                  wordweight = 16 ; 
                  count -= hbit ; 
                  hbit = cwidth ; 
                  } 
                else 
                { if ( turnon ) word += gpower [ wordweight ] ; 
                  *raster++ = word ;
                  word = 0 ; 
                  count -= wordweight ; 
                  hbit -= wordweight ; 
                  wordweight = 16 ; 
                  } 
                } 
              turnon = ! turnon ; 
            } 
          if ( ( rowsleft != 0 ) || ( hbit != cwidth ) ) 
          error ( "! error while unpacking; more bits than required" ) ; 
        } 
}
