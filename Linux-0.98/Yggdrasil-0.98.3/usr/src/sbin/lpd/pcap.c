/*
$Header: /usr/src/linux/lp/pcap.c,v 1.2 1992/01/12 18:24:17 james_r_wiegand Exp $
 *  termcap.c V1.1  20/7/87   agc Joypace Ltd
 *
 *  Copyright Joypace Ltd, London, UK, 1987. All rights reserved.
 *  This file may be freely distributed provided that this notice
 *  remains attached.
 *
 *  A public domain implementation of the termcap(3) routines.
 *
 *
 *
 *   Klamer Schutte       V1.2    Nov. 1988
 *
 *   - Can match multiple terminal names     [tgetent]
 *   - Removal of **area assignments       [tgetstr]
 *
 *   Terrence W. Holm     V1.3    May, Sep, Oct.  1988
 *
 *   - Correct when TERM != name and TERMCAP is defined  [tgetent]
 *   - Correct the comparison for the terminal name    [tgetent]
 *   - Correct the value of ^x escapes                 [tgetstr]
 *   - Added %r to reverse row/column      [tgoto]
 *   - Fixed end of definition test      [tgetnum/flag/str]
 *
 *   Terrence W. Holm     V1.4    Jan. 1989
 *
 *   - Incorporated Klamer's V1.2 fixes into V1.3
 *   - Added %d, (old %d is now %2)      [tgoto]
 *   - Allow '#' comments in definition file     [tgetent]
 *
 *   James R. Wiegand     V1.0  Jan. 1991
 *   - Now adapted for reading printcap files
 */

#include "pcap.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *capab = (char *)NULL;   /* the capability itself */

extern short ospeed;    /* output speed */
extern char PC;     /* padding character */
extern char *BC;    /* back cursor movement */
extern char *UP;    /* up cursor movement */

/*
 *  pgetent - get the printcap entry for terminal name, and put it
 *  in bp (which must be an array of 1024 chars). Returns 1 if
 *  printcap entry found, 0 if not found, and -1 if file not found.
 */

int pgetent(char * bp, char * name)
{
  FILE *fp;
  char *file;
  char *term;
  short len = strlen(name);

  capab = bp;

  /* If TERMCAP begins with a '/' then use TERMCAP as the path   */
  /* Name of the printcap definitions file. If TERMCAP is a  */
  /* Definition and TERM equals "name" then use TERMCAP as the   */
  /* Definition. Otherwise use "/etc/printcap" as the path name.   */

  if( !( file = getenv( "PRINTCAP" )))
    file = "/etc/printcap";
  else
    if( *file != '/' )
      if(( term = getenv( "PRINTER" )) != (char *) NULL
           && strcmp( term, name ) == 0 )
        {
        *bp = '\0';
        strncat(bp, file, 1023);
        return(1);
        }
      else
        file = "/etc/printcap";

  if ( !( fp = fopen( file, "r" )))
    {
    capab = (char *)NULL;   /* no valid printcap  */
    return(-1);
    }

  for (;;)
    {
    /* Read in each definition */
    int def_len = 0;
    char *cp = bp;

    do
      {
      if( fgets( &bp[def_len], (unsigned int)( 1024 - def_len ), fp )
                == (char *) NULL )
        {
        fclose( fp );
        capab = (char *) NULL;  /* no valid printcap */
        return( 0 );
        }
      def_len = strlen(bp) - 2;
      }
    while ( bp[ def_len ] == '\\' );

  while ( isspace( *cp ))
    cp++;

  /* Comment lines start with a '#'  */
  if ( *cp == '#' )
    continue;

  /* See if any of the terminal names in this definition */
  /* Match "name".             */

  do
    {
    if( strncmp( name, cp, len ) == 0 &&
      ( cp[ len ] == '|' || cp[ len ] == ':' ))
      {
      fclose( fp );
      return( 1 );
      }
    while (( *cp ) && ( *cp != '|' ) && ( *cp != ':' )) cp++;
    }
  while ( *cp++ == '|' );
  }
}


/*
 *  pgetnum - get the numeric terminal capability corresponding
 *  to id. Returns the value, -1 if invalid.
 */

int pgetnum(char * id)
{
register char *cp = capab;

  if( cp == (char *)NULL || id == (char *)NULL)
    return(-1);

  for (;;)
    {
    while( *cp++ != ':' )
      if( cp[ -1 ] == '\0' )
        return( 0 );

  while ( isspace( *cp ))
    cp++;

  if( strncmp( cp, id, 2 ) == 0 && cp[ 2 ] == '#' )
    return( atoi( cp + 3 ));
  }
}

/*
 *  pgetflag - get the boolean flag corresponding to id. Returns -1
 *  if invalid, 0 if the flag is not in printcap entry, or 1 if it is
 *  present.
 */

int pgetflag(char *id)
{
register char *cp = capab;

  if( cp == (char *) NULL || id == (char *) NULL )
    return(-1);

  for (;;)
    {
    while( *cp++ != ':' )
      if( cp[ -1 ] == '\0' )
        return(0);

    while( isspace( *cp ))
      cp++;

    if( strncmp( cp, id, 2 ) == 0 )
      return(1);
    }
}

/*
 *  pgetstr - get the string capability corresponding to id and place
 *  it in area (advancing area at same time). Expand escape sequences
 *  etc. Returns the string, or NULL if it can't do it.
 */

char *pgetstr(char *id, char **area)
{
register char *cp = capab;
register char *wsp = *area; /* workspace pointer  */

  if( cp == (char *)NULL || id == (char *)NULL)
    return((char *)NULL);

  for(;;)
    {
    while( *cp++ != ':' )
      if( cp[ -1 ] == '\0' )
        return((char *)NULL);

  while( isspace( *cp ))
    cp++;

  if( strncmp( cp, id, 2 ) == 0 && cp[2] == '=' )
    {
    for( cp += 3; *cp && *cp != ':'; wsp++, cp++ )
    switch (*cp)
      {
      case '^':
                *wsp = *++cp - '@'; 
                break;
      case '\\':
                switch( *++cp )
                  {
                  case 'E':
                          *wsp = '\033';
                          break;
                  case 'n':
                          *wsp = '\n';
                          break;
                  case 'r':
                          *wsp = '\r';
                          break;
                  case 't':
                          *wsp = '\t';
                          break;
                          case 'b':
                          *wsp = '\b';
                          break;
                  case 'f':
                          *wsp = '\f';
                          break;
                  case '0':
                  case '1':
                  case '2':
                  case '3':
                          {
                          int i;
                          int t = 0;
                          for( i = 0; i < 3 && isdigit( *cp ); ++i, ++cp )
                            t = t * 8 + *cp - '0';
                          *wsp = t;
                          cp--;
                          break;
                          }
                  default:
                          *wsp = *cp;
                  }
                break;

        default:
                *wsp = *cp;
        }

    *wsp++ = '\0';

      {
      char *ret = *area;
      *area = wsp;
      return( ret );
      }
    }
  }       /* end for(;;) */
}
