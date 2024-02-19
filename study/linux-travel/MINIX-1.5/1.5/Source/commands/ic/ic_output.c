/****************************************************************/
/*								*/
/*	ic_output.c						*/
/*								*/
/*		Output routines for the "Integer Calculator".	*/
/*								*/
/****************************************************************/
/*   origination        1988-Apr-6          Terrence W. Holm	*/
/****************************************************************/




#include <stdio.h>

#include "ic.h"






/****************************************************************/
/*   		Code for handling termcap			*/
/****************************************************************/


#define  TC_BUFFER  1024	/* Size of termcap(3) buffer	*/
#define  TC_STRINGS  200	/* Enough room for cm,cl,so,se	*/




static  char  *Tmove;		/* (cm) - Format for tgoto	 */
static  char  *Tclr_all;	/* (cl) - String to clear screen */
static  char  *Treverse;	/* (so) - String to reverse mode */
static  char  *Tnormal;		/* (se) - String to undo reverse */







/****************************************************************/
/*								*/
/*	Init_Termcap()						*/
/*								*/
/*		Initializes the external variables for the	*/
/*		current terminal.				*/
/*								*/
/*		NULL is returned on error conditions.		*/
/*								*/
/****************************************************************/


Init_Termcap()
  {
  char  *term;
  char   buffer[ TC_BUFFER ];
  static char strings[ TC_STRINGS ];
  char  *s = &strings[0];

  term = getenv( "TERM" );

  if ( term == NULL )
    return( 0 );

  if ( tgetent( buffer, term ) != 1 )
    return( 0 );



  if ( (Tmove=tgetstr( "cm", &s )) == NULL )
    return( 0 );

  if ( (Tclr_all=tgetstr( "cl", &s )) == NULL )
    return( 0 );

  if ( (Treverse=tgetstr( "so", &s )) == NULL )
    {
    Treverse = s;
    *s = '\0';
    ++s;
    }

  if ( (Tnormal=tgetstr( "se", &s )) == NULL )
    {
    Tnormal = s;
    *s = '\0';
    ++s;
    }

  return( EOF );
  }








/****************************************************************/
/*								*/
/*	Move( column, line )					*/
/*								*/
/*		Use the termcap string to move the cursor.	*/
/*								*/
/****************************************************************/


Move( column, line )
  int  column;
  int  line;

  {
  Puts( tgoto( Tmove, column, line ) );
  }








/****************************************************************/
/*								*/
/*	Puts( string )						*/
/*								*/
/*		Write the given termcap string to the standard	*/
/*		output device.					*/
/*								*/
/****************************************************************/


Puts( string )
  char  *string;

  {
  int Putchar();

  tputs( string, 1, Putchar );
  }


Putchar( c )
  char  c;

  {
  putchar( c );
  }








/****************************************************************/
/*   		       Output routines				*/
/****************************************************************/




/****************************************************************/
/*								*/
/*	Draw_Help_Screen()					*/
/*								*/
/****************************************************************/


Draw_Help_Screen()

  {
  Puts( Tclr_all );		/*  Clear the screen  */

  printf( "\n\n                             " );
  Puts( Treverse );
  printf( "IC  COMMANDS" );
  Puts( Tnormal );
  printf( "\n\n\n" );

  printf("         h   Help                            ENTER Push stack\n");
  printf("         i   Input base  (h d o b)            DEL  Clear entry\n" );
  printf("  PGDN   l   Last top of stack\n" );
  printf("         m   Minix shell                       .   Change sign\n" );
  printf("         o   Output base (h d o b a)           +   Add\n" );
  printf("  DOWN   p   Pop stack                         -   Subtract\n" );
  printf("  END    q   Quit                              *   Multiply\n" );
  printf("  LEFT   r   Recall (0-9)                      /   Divide\n" );
  printf("  RIGHT  s   Store [+] (0-9)                   %%   Remainder\n");
  printf("         t   Translate (char)                  ~   Not\n" );
  printf("  PGUP   w   Write top to scratch pad          &   And\n" );
  printf("  UP     x   Exchange top of stack             |   Or\n" );
  printf("  HOME   z   Zero all state                    ^   Exclusive-or\n\n\n" );

  printf( "\n\nPress a key to continue..." );
  }








/****************************************************************/
/*								*/
/*	Draw_Prompt( string )					*/
/*								*/
/*		Write a message in the "wait" area.		*/
/*								*/
/****************************************************************/


Draw_Prompt( string )
  char  *string;

  {
  Move( WAIT_COLUMN, WAIT_LINE );

  Puts( Treverse );
  printf( string );
  Puts( Tnormal );
  }








/****************************************************************/
/*								*/
/*	Erase_Prompt()						*/
/*								*/
/*		Erase the message in the "wait" area.		*/
/*								*/
/****************************************************************/

Erase_Prompt()

  {
  Move( WAIT_COLUMN, WAIT_LINE );

  printf("           " );

  Move( WAIT_COLUMN, WAIT_LINE );
  }








/****************************************************************/
/*								*/
/*	Draw_Screen( state )					*/
/*								*/
/*		Redraw everything.				*/
/*								*/
/****************************************************************/


Draw_Screen( s )
  ic_state *s;

  {
  Puts( Tclr_all );		/*  Clear the screen  */

  Draw_Stack( s );
  Draw_Registers( s );


  Move( STATUS_COLUMN, STATUS_LINE );

  printf( "Input base = %2d    ", s->input_base );

  if ( s->output_base == ASCII )
    printf( "Output is ASCII     " );
  else
    printf( "Output base = %2d    ", s->output_base );

  if ( (int) s->scratch_pad != 0 )
    printf( "Scratch file = %s", s->file_name );

  Move( WAIT_COLUMN, WAIT_LINE );
  }








/****************************************************************/
/*								*/
/*	Draw_Stack( state )					*/
/*								*/
/*		Redraw the stack.				*/
/*								*/
/****************************************************************/


Draw_Stack( s )
  ic_state *s;

  {
  int i;

  for ( i = STACK_SIZE-1;  i >= 0;  --i )
    {
    Move( STACK_COLUMN, STACK_LINE + STACK_SIZE - 1 - i );

    if ( i >= s->stack_size )
      printf( "%*c", s->output_base == BINARY ? 32 : 17, ' ' );
    else
      Print_Number( stdout, s->stack[i], s->output_base );
    }

  Move( WAIT_COLUMN, WAIT_LINE );
  }








/****************************************************************/
/*								*/
/*	Draw_Registers( state )					*/
/*								*/
/*		Redraw the registers. Note that only registers	*/
/*		in use are displayed. A register only drops	*/
/*		out of use after a 'z' command, which will	*/
/*		explicitly clear the display, thus we never	*/
/*		have to "wipe off" a value, as the		*/
/*		Draw_Stack() routine must.			*/
/*								*/
/****************************************************************/


Draw_Registers( s )
  ic_state *s;

  {
  int i;

  for ( i = 0;  i < REGISTERS;  ++i )
    {
    if ( (1 << i) & s->register_mask  )
      {
      Move( REG_COLUMN, REG_LINE + i );
      Print_Number( stdout, s->registers[i], s->output_base );
      printf( " (r%1d)", i );
      }
    }

  Move( WAIT_COLUMN, WAIT_LINE );
  }








/****************************************************************/
/*								*/
/*	Draw_Top_of_Stack( state )				*/
/*								*/
/*		Redraw only the entry on the top of the stack.	*/
/*								*/
/****************************************************************/


Draw_Top_of_Stack( s )
  ic_state *s;

  {
  Move( STACK_COLUMN, STACK_LINE + STACK_SIZE - 1 );

  Print_Number( stdout, s->stack[0], s->output_base );

  Move( WAIT_COLUMN, WAIT_LINE );
  }








/****************************************************************/
/*								*/
/*	Print_Number( stream, number, output_base )		*/
/*								*/
/*		Output the "number" to "stream" in the		*/
/*		specified "output_base".			*/
/*								*/
/****************************************************************/


Print_Number( stream, number, output_base )
  FILE    *stream;
  long int number;
  int      output_base;

  {
  switch ( output_base )
    {
    case HEXADECIMAL :	fprintf( stream, "%12lx", number );
			break;


    case DECIMAL :	fprintf( stream, "%12ld", number );
			break;


    case OCTAL :	fprintf( stream, "%12lo", number );
			break;


    case BINARY :	{
      			unsigned long int mask;
      			char pad = ' ';

      			for ( mask = 0x80000000;  mask > 1;  mask >>= 1 )
			  putc( ( mask & number ) ? (pad = '0', '1') : pad, stream );

      			putc( ( 0x01 & number ) ? '1' : '0', stream );

			break;
      			}


    case ASCII :	{
			char c = number & 0x7f;

			if ( (number & ~ 0x7fL) == 0 )
			  fprintf( stream, "%15c", ' ' );
			else
    			  fprintf( stream, "%12lx + ", number & ~ 0x7fL );

			if ( c < ' ' )
			  fprintf( stream, "^%c", c + '@' );
			else if ( c == ' ' )
			  fprintf( stream, "sp" );
			else if ( c < 0x7f )
			  fprintf( stream, " %c", c );
			else
			  fprintf( stream, "^?" );

			break;
			}

    default :		fprintf( stderr, "Internal failure (output base)\n" );
			Sigint();
    }
  }
