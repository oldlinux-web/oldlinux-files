/****************************************************************/
/*								*/
/*	ic.c							*/
/*								*/
/*		The main loop of the "Integer Calculator".	*/
/*								*/
/****************************************************************/
/*  origination          1988-Apr-6   	    Terrence W. Holm	*/
/*  added Exec_Shell()   1988-Apr-11	    Terrence W. Holm	*/
/*  added "s+"		 1988-Apr-18	    Terrence W. Holm	*/
/*  added cmd line args  1988-May-13	    Terrence W. Holm	*/
/*  'i' also does 'o'	 1988-May-28	    Terrence W. Holm	*/
/*  if ~dec:unsigned *%/ 1988-Jul-10	    Terrence W. Holm	*/
/****************************************************************/






#include <sys/types.h>
#include <signal.h>
#include <stdio.h>

#include "ic.h"




static char copyright[] = { "ic    (c) Terrence W. Holm 1988" };



/****************************************************************/
/*								*/
/*	main()							*/
/*								*/
/*		Initialize. Enter the main processing loop.	*/
/*								*/
/****************************************************************/


main( argc, argv )
  int   argc;
  char *argv[];

  {
  ic_state  state;		/*  This state record is passed	*/
				/*  to most subroutines		*/

  Init_State( &state );


  state.scratch_pad = (FILE *) NULL;	/*  No 'w' command yet	*/



  Init_Getc( argc, argv );	/*  Refs to command line args	*/



  if ( Init_Termcap() == 0 )
    {
    fprintf( stderr, "ic requires a termcap entry\n" );
    exit( 1 );
    }


  Save_Term();		/*  Save terminal characteristics	*/


  if ( signal( SIGINT, SIG_IGN ) != SIG_IGN )
    {
    signal( SIGINT,  Sigint );
    signal( SIGQUIT, Sigint );
    }


  Set_Term(); 		/*  Setup terminal characteristics	*/



  Draw_Screen( &state );

  while (1)
    {
    int  rc = Process( &state, Get_Char() );

    if ( rc == EOF )
      break;

    if ( rc == ERROR )
      putchar( BELL );
    }


  Reset_Term();		/*  Restore terminal characteristics	*/

  exit( OK );
  }








/****************************************************************/
/*								*/
/*	Init_State()						*/
/*								*/
/*		Initialize the state record.			*/
/*								*/
/****************************************************************/


Init_State( s )
  ic_state  *s;

  {
  s->stack[0]      = 0;
  s->stack_size    = 1;
  s->register_mask = 0x000;
  s->last_tos      = 0;
  s->mode          = LAST_WAS_ENTER;
  s->input_base    = DECIMAL;
  s->output_base   = DECIMAL;
  }








/****************************************************************/
/*								*/
/*	Sigint()						*/
/*								*/
/*		Terminate the program on an interrupt (^C)	*/
/*		or quit (^\) signal.				*/
/*								*/
/****************************************************************/


void Sigint(sig)
int sig;
  {
  Reset_Term();		/*  Restore terminal characteristics	*/

  exit( 1 );
  }








/****************************************************************/
/*								*/
/*	Process( state, input_char )				*/
/*								*/
/*		Determine the function requested by the 	*/
/*		input character. Returns OK, EOF or ERROR.	*/
/*								*/
/****************************************************************/


int Process( s, c )
  ic_state  *s;
  int    c;

  {
  switch ( c )
    {
    case '0' :
    case '1' :
    case '2' :
    case '3' :
    case '4' :
    case '5' :
    case '6' :
    case '7' :
    case '8' :
    case '9' :

		return( Enter_Numeric( s, (int) c - '0' ) );


    case 'a' :
    case 'b' :
    case 'c' :
    case 'd' :
    case 'e' :
    case 'f' :

		return( Enter_Numeric( s, (int) c - 'a' + 10 ) );


    case 'h' :  case '?' :		/*  Help		*/

		Draw_Help_Screen();

		Get_Char();

		Draw_Screen( s );
		return( OK );


    case 'i' :				/* Set i/p and o/p base	*/

		{
		int numeral;

		Draw_Prompt( "Base?" );

		numeral = Get_Base( Get_Char() );

		Erase_Prompt();

		if ( numeral == ERROR  ||  numeral == ASCII )
		  return( ERROR );

		s->input_base  = numeral;
		s->output_base = numeral;
		s->mode        = LAST_WAS_FUNCTION;

		Draw_Screen( s );
		return( OK );
		}


    case 'l' :  case ESC_PGDN :		/*  Get last tos value	 */

  		if ( s->mode != LAST_WAS_ENTER )
		  Push( s );

		s->stack[0] = s->last_tos;

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );


    case 'm' :				/*  Invoke a Minix shell */

		Reset_Term();

		Exec_Shell();

		Set_Term();

		Draw_Screen( s );
		return( OK );


    case 'o' :				/*  Set output base	*/

		{
		int numeral;

		Draw_Prompt( "Base?" );

		numeral = Get_Base( Get_Char() );

		Erase_Prompt();

		if ( numeral == ERROR )
		  return( ERROR );

		s->output_base = numeral;
		s->mode        = LAST_WAS_FUNCTION;

		Draw_Screen( s );
		return( OK );
		}


    case 'p' :  case ESC_DOWN :		/*  Pop: Roll down stack */

		{
		long int temp = s->stack[0];
		int  i;

  		for ( i = 0;  i < s->stack_size-1;  ++i )
    		  s->stack[i] = s->stack[i+1];

		s->stack[ s->stack_size-1 ] = temp;

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );
		}


    case 'q' :  case ESC_END :  	/*  Quit		 */
    case EOF :  case CTRL_D  :

		return( EOF );


    case 'r' :  case ESC_LEFT :		/*  Recall from register */

		{
		int numeral;

		Draw_Prompt( "Register?" );

		numeral = Get_Char() - '0';

		Erase_Prompt();

		if (  numeral < 0  ||  numeral >= REGISTERS  ||
		    ((1 << numeral) & s->register_mask) == 0   )
		  return( ERROR );
	
  		if ( s->mode != LAST_WAS_ENTER )
		  Push( s );

		s->stack[0] = s->registers[numeral];

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );
		}


    case 's' :  case ESC_RIGHT :	/*  Store in register,	*/
					/*  or accumulate if	*/
					/*  "s+" is typed.	*/

		{
		int c;
		int numeral;


		Draw_Prompt( "Register?" );

		c = Get_Char();

		if ( c == ESC_PLUS )
		  c = '+';		/*  Allow keypad '+' 	*/


		if ( c == '+' )
		  {
		  Draw_Prompt( "Accumulator?" );
		  numeral = Get_Char() - '0';
		  }
		else
		  numeral = c - '0';

		Erase_Prompt();


		if (  numeral < 0  ||  numeral >= REGISTERS  )
		  return( ERROR );
	

		if (  c != '+'  ||  (s->register_mask & (1 << numeral)) == 0  )
		  {
		  s->register_mask |= 1 << numeral;
		  s->registers[numeral] = s->stack[0];
		  }
		else
		  s->registers[numeral] += s->stack[0];


		s->mode = LAST_WAS_FUNCTION;

		Draw_Registers( s );
		return( OK );
		}


    case 't' :				/*  Translate from ASCII */

		{
		long int numeral;

		Draw_Prompt( "Character?" );

		numeral = (long int) Getc();

		Erase_Prompt();

  		if ( s->mode != LAST_WAS_ENTER )
		  Push( s );

		s->stack[0] = numeral;

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );
		}


    case 'w' :  case ESC_PGUP :		/*  Write tos to a file	*/

		{
		if ( (int) s->scratch_pad == (int) NULL )
		  {
		  /*  Try to open a scratch pad file  */

		  strcpy( s->file_name, "./pad" );

		  if ( (s->scratch_pad=fopen(s->file_name,"w")) == NULL )
		    {
		    /*  Unsuccessful, try in /tmp  */
		    char *id;

		    strcpy( s->file_name, "/tmp/pad_" );

		    if ( (id=cuserid(NULL)) == NULL )
		      return( ERROR );

		    strcat( s->file_name, id );

		    if ( (s->scratch_pad=fopen(s->file_name,"w")) == NULL )
		      return( ERROR );
		    }

		  Draw_Screen( s );
		  }


		/*  We have a successfully opened file  */

		Print_Number( s->scratch_pad, s->stack[0], s->output_base );
		putc( '\n', s->scratch_pad );
		fflush( s->scratch_pad );

		s->mode = LAST_WAS_FUNCTION;

		return( OK );
		}


    case 'x' :  case ESC_UP :		/*  Exchange top of stack */

		{
		long int temp = s->stack[0];

		if ( s->stack_size < 2 )
		  return( ERROR );

		s->stack[0] = s->stack[1];
		s->stack[1] = temp;

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );
		}


    case 'z' :  case ESC_HOME:		/*  Clear all		*/

		Init_State( s );

		Draw_Screen( s );
		return( OK );


    case BS  :  case DEL :		/*  Clear top of stack  */

		s->stack[0] = 0;

		s->mode = LAST_WAS_ENTER;

		Draw_Top_of_Stack( s );
		return( OK );


    case '\n' :				/*  Enter		*/

		Push( s );

		s->mode = LAST_WAS_ENTER;

		Draw_Stack( s );
		return( OK );


    case '.' :				/*  Change sign		*/

		s->last_tos = s->stack[0];

		s->stack[0] = - s->stack[0];

		s->mode = LAST_WAS_FUNCTION;

		Draw_Top_of_Stack( s );
		return( OK );


    case '+' :  case ESC_PLUS :		/*  Add			*/

		if ( s->stack_size < 2 )
		  return( ERROR );

		s->last_tos = s->stack[0];

		Pop( s );

		s->stack[0] += s->last_tos;

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );


    case '-' :  case ESC_MINUS :	/*  Subtract		*/

		if ( s->stack_size < 2 )
		  return( ERROR );

		s->last_tos = s->stack[0];

		Pop( s );

		s->stack[0] -= s->last_tos;

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );


    case '*' :				/*  Multiply		*/

		if ( s->stack_size < 2 )
		  return( ERROR );

		s->last_tos = s->stack[0];

		Pop( s );

		if ( s->input_base == DECIMAL )
		  s->stack[0] *= s->last_tos;
		else
		  s->stack[0] = (long int)
			 ( UNS(s->stack[0]) * UNS(s->last_tos) );

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );


    case '/' :				/*  Divide		*/

		if ( s->stack_size < 2  ||  s->stack[0] == 0 )
		  return( ERROR );

		s->last_tos = s->stack[0];

		Pop( s );

		if ( s->input_base == DECIMAL )
		  s->stack[0] /= s->last_tos;
		else
		  s->stack[0] = (long int)
			 ( UNS(s->stack[0]) / UNS(s->last_tos) );

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );


    case '%' :  case ESC_5 :		/*  Remainder		*/

		if ( s->stack_size < 2  ||  s->stack[0] == 0 )
		  return( ERROR );

		s->last_tos = s->stack[0];

		Pop( s );

		if ( s->input_base == DECIMAL )
		  s->stack[0] %= s->last_tos;
		else
		  s->stack[0] = (long int)
			 ( UNS(s->stack[0]) % UNS(s->last_tos) );

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );


    case '~' :				/*  Not			*/

		s->last_tos = s->stack[0];

		s->stack[0] = ~ s->stack[0];

		s->mode = LAST_WAS_FUNCTION;

		Draw_Top_of_Stack( s );
		return( OK );


    case '&' :				/*  And			*/

		if ( s->stack_size < 2 )
		  return( ERROR );

		s->last_tos = s->stack[0];

		Pop( s );

		s->stack[0] &= s->last_tos;

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );


    case '|' :				/*  Or			*/

		if ( s->stack_size < 2 )
		  return( ERROR );

		s->last_tos = s->stack[0];

		Pop( s );

		s->stack[0] |= s->last_tos;

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );


    case '^' :				/*  Exclusive-or	*/

		if ( s->stack_size < 2 )
		  return( ERROR );

		s->last_tos = s->stack[0];

		Pop( s );

		s->stack[0] ^= s->last_tos;

		s->mode = LAST_WAS_FUNCTION;

		Draw_Stack( s );
		return( OK );


    default:
		return( ERROR );
    }
  }








/****************************************************************/
/*								*/
/*	Enter_Numeric( state, numeral )				*/
/*								*/
/*		A numeral (0 to 15) has been typed.		*/
/*		If a number is currently being entered		*/
/*		then shift it over and add this to the		*/
/*		display. If the last operation was a function	*/
/*		then push up the stack first. If the last	*/
/*		key was "ENTER", then clear out the top of	*/
/*		the stack and put the numeral there.		*/
/*								*/
/*		Returns OK or ERROR.				*/
/*								*/
/****************************************************************/


int Enter_Numeric( s, numeral )
  ic_state *s;
  int   numeral;

  {
  if ( numeral >= s->input_base )
    return( ERROR );


  switch ( s->mode )
    {
    case LAST_WAS_FUNCTION :
		Push( s );
		s->stack[0] = numeral;
		Draw_Stack( s );
		break;

    case LAST_WAS_NUMERIC :
		s->stack[0] = s->stack[0] * s->input_base + numeral;
		Draw_Top_of_Stack( s );
		break;

    case LAST_WAS_ENTER :
		s->stack[0] = numeral;
		Draw_Top_of_Stack( s );
		break;

    default:
		fprintf( stderr, "Internal failure (mode)\n" );
		Sigint();
    }

  s->mode = LAST_WAS_NUMERIC;

  return( OK );
  }








/****************************************************************/
/*								*/
/*	Push( state )						*/
/*								*/
/*		Push up the stack one level.			*/
/*								*/
/****************************************************************/


Push( s )
  ic_state *s;

  {
  int i;

  if ( s->stack_size == STACK_SIZE ) 
    --s->stack_size;

  for ( i = s->stack_size;  i > 0;  --i )
    s->stack[i] = s->stack[i-1];

  ++s->stack_size;
  }








/****************************************************************/
/*								*/
/*	Pop( state )						*/
/*								*/
/*		Pop the stack down one level.			*/
/*		This routine is only called with 		*/
/*		the stack size > 1.				*/
/*								*/
/****************************************************************/


Pop( s )
  ic_state *s;

  {
  int i;

  for ( i = 0;  i < s->stack_size-1;  ++i )
    s->stack[i] = s->stack[i+1];

  --s->stack_size;
  }








/****************************************************************/
/*								*/
/*	Exec_Shell()						*/
/*								*/
/*		Fork off a sub-process to exec() the shell.	*/
/*								*/
/****************************************************************/


Exec_Shell()

  {
  int pid = fork();

  if ( pid == -1 )
    return;


  if ( pid == 0 )
    {
    /*  The child process  */

    extern char **environ;
    char *shell  =  getenv( "SHELL" );

    if ( shell == NULL )
      shell = "/bin/sh";

    execle( shell, shell, (char *) 0, environ );

    perror( shell );
    exit( 127 );
    }


  /*  The parent process: ignore signals, wait for sub-process	*/

  signal( SIGINT,  SIG_IGN );
  signal( SIGQUIT, SIG_IGN );

  {
  int  status;
  int  w;

  while ( (w=wait(&status)) != pid  &&  w != -1 );
  }

  signal( SIGINT,  Sigint );
  signal( SIGQUIT, Sigint );

  return;
  }
