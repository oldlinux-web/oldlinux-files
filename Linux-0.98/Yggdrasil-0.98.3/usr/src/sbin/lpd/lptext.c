/*
$Header: /usr/src/linux/lp/lptext.c,v 1.1 1992/01/12 18:23:29 james_r_wiegand Exp $

  linux/lp/lptext.c - text handling routines for lp
  c. 1992 by James Wiegand
*/

#include "lpjob.h"
#include <string.h>
#include <unistd.h>

int macro_expansion( char *in, char *out )
{
/*
  perform the desired text formatting for headers and footers-

  $$l   - left-justify following text or macro expansion
  $$c   - center      
  $$r   - right-justify
  $$n   - expands to user name
  $$f   - expands to file name
  $$p   - expands to page number " - nnnn - " 
  $$t   - expands to ctime(3) string
  
  NOTE  - the input line is terminated by a newline or null,
          so it's best NOT to include any control characters in
          the header/footer strings.
*/

char centerbuffer[ 256 ]; /* buffer for centered text */
char rightbuffer [ 256 ]; /* buffer for right-justified text */
char buffer[ 256 ]; /* buffer for expansions */
int count, left, right, center, mode, usemacro;
int mloop;

enum { LEFT, RIGHT, CENTER };

/* pad the printed line to all spaces and zot on a newline */  
  for( count = 0; count < NSV(PW); count ++ )
    out[ count ] = ' ';
  out[ count++ ] = '\n';
  out[ count   ] = 0x0;

/* now everything that is written will be scribbled in this space */
/* nothing to do */
  if( !in || !*in || *in =='\n' )
    return 0;

  /* begin at start of input line */
  count   = 0;
  /* default mode is left-justified */
  mode    = LEFT;
  /* start at left margin */
  left    = 0;
  /* nothing in center buffer */
  center  = 0;
  /* nothing in right buffer */
  right   = 0;
  /* no macro text */
  usemacro = 0;
  
  while( in[ count ] && in[ count ]  != '\n' )
    {
    if( in[ count ] == '$' &&  in[ count + 1 ] == '$' )
      {
      /* skip over macro escape */
      count += 2;
      /* decode action */
      /*
      to center text the expansion buffer is used-
      but we have to be careful not to forget about it when the
      expansion is done
      the emit mode is set every time a macro command is set
      */
      switch( in[ count ])
        {
        /* mode set macros. just change mode & bail out */
        case 'l':
                mode = LEFT;
                count ++;
                continue;

        case 'c':
                mode = CENTER;
                count ++;
                continue;

        case 'r':
                mode = RIGHT;
                count ++;
                continue;

        /* text macros. expand text, switch input to buffer, & set sail */
        /* username */
        case 'n':
                strcpy( buffer, pusername );
                usemacro = 1;
                break;
        /* filename */
        case 'f':
                strcpy( buffer, pfilename );
                usemacro = 1;
                break;
        /* page number */
        case 'p':
                sprintf( buffer, "- %d -", pageno );
                usemacro = 1;
                break;
        /* time */
        case 't':
                strcpy( buffer, ctime( &ptime ));
                buffer[ 24 ] = 0x0;
                usemacro = 1;
                break;
        /* unknown macro */
        default:
                return -1;
        }

      /* go past macro letter */
      count ++;
      }

     /* text emission */
    switch( mode )
      {
      case LEFT:
              /* truncate long lines */
              if( left < NSV(PW) )
                {
                /* emit text at left justified position */
                if( usemacro )
                  {
                  for( mloop = 0; buffer[ mloop ]; mloop ++ )
                    {
                    out[ left ] = buffer[ mloop ];
                    left ++;
                    }
                  usemacro = 0;
                  }
                else
                  {
                  out[ left ] = in[ count ];
                  count ++;
                  left ++;
                  }
                }
              break;
      case CENTER:
              /* emit text into buffer */
              /* truncate long lines */
              if( center < NSV(PW) )
                {
                /* emit text into center buffer */
                if( usemacro )
                  {
                  for( mloop = 0; buffer[ mloop ]; mloop ++ )
                    {
                    centerbuffer[ center ] = buffer[ mloop ];
                    center ++;
                    }
                  usemacro = 0;
                  }
                else
                  {
                  centerbuffer[ center ] = in[ count ];
                  count ++;
                  center ++;
                  }
                }
              break;
                
      case RIGHT:
              /* emit text into buffer */
              /* truncate long lines */
              if( right < NSV(PW) )
                {
                /* emit text into center buffer */
                if( usemacro )
                  {
                  for( mloop = 0; buffer[ mloop ]; mloop ++ )
                    {
                    rightbuffer[ right ] = buffer[ mloop ];
                    right ++;
                    }
                  usemacro = 0;
                  }
                else
                  {
                  rightbuffer[ right ] = in[ count ];
                  count ++;
                  right ++;
                  }
                }
              break;
 
      }
    }
  /* splice in center buffer */
  if( center )
    {
    count = ( NSV(PW) - center ) / 2;
    for( mloop = 0; mloop < center; count ++, mloop ++ )
      out[ count ] = centerbuffer[ mloop ];
    }

  /* splice in right buffer */
  if( right )
    {
    /* text starts at right margin */
    count = NSV(PW) - 1;
    for( mloop = right - 1; mloop >= 0; count --, mloop -- )
      out[ count ] = rightbuffer[ mloop ];
    }
    
  return 0;
}

int readline( int filein, char *out )
{
int result;
int count = 0;
char inchar;

  while( 1 )
    {
    result = read( filein, &inchar, 1 );
    *out = inchar;
    count ++;
    out ++;
    *out = 0x0;
    
    if( result == 0 )
      return FAILURE;
    
    if( inchar == '\n' )
      return count;
    }
  
}

int processtext( int filein, int fileout )
{
/*
  page handling stuff
  - do header lines
  - read in & spit out hunks of text until the page is full
  - do footer lines
  --  repeat until well done, serve with 3-hole punch
*/
char buffer[ 1024 ];
char macrotext[ 512 ];
int result;
int linecount = 0, count;

  /* set date/time of submission */
  time( &ptime );
  pageno = 1;
  
  while( 1 )
    {
    result = readline( filein, buffer );
    if( result == FAILURE )
      {
      /* finish up last page */
      for( ; linecount < ( NSV(PL) - 2 ); linecount ++ )
        write( fileout, "\n", 1 );
      macro_expansion( SSV(FT), macrotext );
      write( fileout, macrotext, strlen( macrotext ));      
      write( fileout, SSV(FF), 1 );
      return 0;
      }

    linecount ++;

    /* do footer processing */
    if( linecount == ( NSV(PL) - NSV(FL) - 1 ))
      {
      for( count = 1; count < NSV(FL); count ++ )
        write( fileout, "\n", 1 );
      macro_expansion( SSV(FT), macrotext );
      write( fileout, macrotext, strlen( macrotext ));
      write( fileout, SSV(FF), 1 );
      linecount = 1;
      pageno ++;
      }

    /* do header processing */
    if( linecount == 1 && NSV(HL) )
      {
      macro_expansion( SSV(HT), macrotext );
      write( fileout, macrotext, strlen( macrotext ));
      for( count = 1; count < NSV(HL); count ++ )
        write( fileout, "\n", 1 );
      linecount += NSV(HL);
      }
 
    result = write( fileout, buffer, strlen( buffer ));
    if( result == FAILURE )
      return -1; 

    }
}

#ifdef TEST
void main( void )
{
char outs[ 128 ];
char *string = "$$n $$t$$c$$p$$r$$f";

  strcpy( pusername, "jim" );
  strcpy( pfilename, "/dev/foo" );
  pageno = 120;
  time( &ptime );
  macro_expansion( string, outs );
  printf( "%s\n\r", string );
  printf( "%s", outs );
}
#endif
