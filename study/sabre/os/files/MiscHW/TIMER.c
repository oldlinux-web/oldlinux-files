Ok, here is some code I wrote a while ago, and touched up a little
recently as someone else needed it.  As you may have noticed by the
line count, it is VERY wordy...hopefully some of it is useful.

This code returns a time value in units of 838 nanoseconds,
and I have found it quite useful.  Even though the examples I
provide are for fixed-frame-rate games, I have used this in
a crude ray-casting (yuk) demo I wrote a while back which will
run as fast as your CPU (or monitor/video) will allow. 

Lemme know if you create something cool with it!

Ethan -- rer@wlv.iipo.gtegsc.com
(following are timer.c and timer.h)

SNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIP

/***************************************************************************\
**  TIMING CODE MODULE (80x86 specific code!)                              **
**=========================================================================**
**  Written by Ethan Rohrer, (c) Nuthing Software, December 4, 1994        **
**                                                                         **
**  Revision History                                                       **
**  ----------------                                                       **
**  Date        Description                                                **
**  ---------   ---------------------------------------------------------  **
**  13 May 94   Initial Release                                            **
**  04 Dec 94   Updated code to conform to current coding standards/style  **
**              Allowed appl. to specify # of timers as param. to TM_Init  **
**              Standardized error handling with function _TM_Error        **
**=========================================================================**
**  This file contains code which makes use of system timer 0.  This timer **
**  normally operates at a frequency of 1.1932MHz, regardless of the speed **
**  of the CPU.  Normally, this timer is operating in mode 3 (square wave  **
**  mode), and it completes a "cycle" in 0.054926 seconds (~1/18.207       **
**  seconds).  The reason I say "normally" is because it is possible to    **
**  change the length of the cycle, which changes the timer's frequency.   **
**  This is NOT what the following code does.                              **
**                                                                         **
**  System timer 0 has its own 16 bit counter.  Here is some simplified    **
**  pseudo-code of what system timer 0 does with its counter when it is    **
**  operating in mode 3, and its frequency has not been tampered with:     **
**                                                                         **
**      counter = 65536                                                    **
**      while (counter != 0)                                               **
**          counter -= 2;                                                  **
**      counter = 65536                                                    **
**      while (counter != 0)                                               **
**          counter -= 2;                                                  **
**                                                                         **
**  You read that right.  The counter is decremented from 65536 to 0       **
**  TWICE.  This is somewhat unfortunate, because we want to read that     **
**  counter to use for our timing operations because it is very accurate.  **
**  The counter is decremented 65536 times in one timer cycle (32768       **
**  times in each while loop above).  So, the time between decrements is:  **
**                                                                         **
**      (0.054926 seconds/cycle) / (65536 decrements/cycle) =              **
**                      0.000000838 seconds/decrement = 838ns/decrement    **
**                                                                         **
**  Since the counter is decremented to 0 twice in each timer cycle, we    **
**  would only be able to time events that take no longer than 0.027463    **
**  seconds (one half of the timer cycle, the duration of one of the       **
**  while loops above).                                                    **
**                                                                         **
**  The solution used by the code in this file is to change the timer's    **
**  operation mode to mode 2.  Here is some simplified pseudo-code of what **
**  system timer 0 does with its counter when it is operating in mode 2,   **
**  and its frequency has not been tampered with:                          **
**                                                                         **
**      counter = 65536                                                    **
**      while (counter != 0)                                               **
**          counter -= 1;                                                  **
**                                                                         **
**  This solves any problems concerning the ambiguity of determining       **
**  which while loop is executing when we read the counter from the timer. **
**  But now, we have a 16 bit value which can only be used to time events  **
**  which take no longer than 0.054926 seconds (duration of one cycle).    **
**                                                                         **
**  The solution used by this code is to make use of another timer:        **
**  the "large timer", which is a 32-bit value at memory location          **
**  0x40:0x6C.  Conveniently, this "large timer" is incremented each time  **
**  system timer 0 completes a cycle (once each 0.054926 seconds).         **
**  The code in this file generates 32-bit values to represent the         **
**  time.  Obviously, we can't pack in the 16 bit counter and the 32 bit   **
**  large timer into the 32 bit time type, so we cut off the high order    **
**  16 bits of the large timer.  The following picture describes how the   **
**  time value is generated using the timers:                              **
**                                                                         **
**    31                                        0 15                   0   **
**    ------------------------------------------- ----------------------   **
**    |         L a r g e   T i m e r           | |  65535 - Counter   |   **
**    ------------------------------------------- ----------------------   **
**                         |                     |                     |   **
**                        \|/                   \|/                   \|/  **
**                         V                     V                     V   **
**                         31                  16 15                   0   **
**                         ---------------------- ----------------------   **
**                         |                 t T I M E                 |   **
**                         ---------------------- ----------------------   **
**                                                                         **
**  (Note that we have to use (65535-Counter) because Counter is being     **
**  decremented by the timer, but time is increasing)                      **
**                                                                         **
**=========================================================================**
**  USING THIS MODULE                                                      **
**      Before calling any other timing routine, you must call TM_Init(n), **
**      where  n  specifies the number of timers your application needs.   **
**      It may be a good idea to call TM_Init() in the initialization      **
**      portion of your application.                                       **
**                                                                         **
**      To begin timing an event, make a call to TM_StartTimer(tid),       **
**      where tid is an integer in the range [0..(n-1)] which specifies    **
**      which of the n timers you are starting.                            **
**                                                                         **
**      To compute the duration of the event, just call                    **
**      TM_ElapsedTime(tid), where tid is the same integer used in the     **
**      call to TM_StartTimer().                                           **
**                                                                         **
**      When you are finished with the timing routines, call TM_Close().   **
**      This should fit in nicely with the cleanup section of your         **
**      application.                                                       **
**                                                                         **
**      If your application NEEDS to handle the time computations itself,  **
**      the function TM_ReadTimer(tid) is also available.  This function   **
**      will return the current time (MOD 1 hour, approximately).  I       **
**      discourage use of this function, but discovered that I need it     **
**      for another module/library...                                      **
**                                                                         **
**      EXAMPLES                                                           **
**          A simple delaying routine:                                     **
**              void delay( tTIME duration )                               **
**              {                                                          **
**                  TM_StartTimer(0);                                      **
**                  while (TM_ElapsedTime(0) < duration)                   **
**                      ;                                                  **
**              }                                                          **
**                                                                         **
**          A fixed frame-rate game:                                       **
**              TM_Init(1);                                                **
**              while (player_not_dead)                                    **
**              {                                                          **
**                  TM_StartTimer(0);                                      **
**                  MoveMonsters();                                        **
**                  MovePlayers();                                         **
**                  UpdateDisplay();                                       **
**                  while (TM_ElapsedTime(0) < frame_duration)             **
**                      ;                                                  **
**              }                                                          **
**              TM_Close();                                                **
\***************************************************************************/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

/*-------------------------------- MACROS ---------------------------------*/

/* macro to return the current value of the large timer */

#define  TM_LARGE_TIMER   (*((unsigned long far *)MK_FP(0x40, 0x6C)))

/*--------------------------- GLOBAL VARIABLES ----------------------------*/

/* starting times for all of the timers this code will support */

tTIME  *gpTM_start_time = NULL;

unsigned int gTM_max_timers = 0;

/* flag to let us know if it is ok to run the timing routines */
/*     (1 = NOT safe, 0 = safe)                               */

unsigned char  gTM_module_not_initialized = 1;

/*------------------------- Error Message Strings -------------------------*/

#define TM_ERR_STR_GENERAL                                                 \
	"General (unspecified) error."
#define TM_ERR_STR_UNINITIALIZED                                           \
	"Timing routines not yet initialized.\n"                           \
	"(TM_Init() has not been called)"
#define TM_ERR_STR_BAD_TIMER_ID                                            \
	"Application specified an invalid timer ID."
#define TM_ERR_STR_ALLOC                                                   \
	"Unable to allocate dynamic memory."
#define TM_ERR_STR_ZERO_TIMERS                                             \
	"Application requested 0 timers.\n"                                \
	"(must request 1 or more timers to use this module)"

/*------------------------- Error Message Indices -------------------------*/
/* (Make sure these indices are accurate according to gpTM_error_text[]     */
/* declared below !!)                                                      */

#define TM_ERR_GENERAL           0
#define TM_ERR_UNINITIALIZED     1
#define TM_ERR_BAD_TIMER_ID      2
#define TM_ERR_ALLOC             3
#define TM_ERR_ZERO_TIMERS       4

/*------------------------- Error Message Strings -------------------------*/
/* (Make sure the positions of the error messages in this array are        */
/* accurately represented by the error messages indices listed above !!)   */

char *gpTM_error_text[] =
{
    TM_ERR_STR_GENERAL,
    TM_ERR_STR_UNINITIALIZED,
    TM_ERR_STR_BAD_TIMER_ID,
    TM_ERR_STR_ALLOC,
    TM_ERR_STR_ZERO_TIMERS
};

/***************************************************************************\
**  void _TM_Error( )                                                      **
*****************************************************************************
**  ARGUMENTS                                                              **
**      const char *pCalling_function_name                                 **
**          (I) name of the calling function                               **
**      int error_number                                                   **
**          (I) integer identifier of the error that occurred              **
**      const char *pCustom_message                                        **
**          (I) additional message text to be displayed                    **
**-------------------------------------------------------------------------**
**  RETURNS                                                                **
**      void                                                               **
**-------------------------------------------------------------------------**
**  EXAMPLE USAGE (NOT INTENDED FOR EXTERNAL USE)                          **
**      if ( gTM_module_not_initialized )                                  **
**      {                                                                  **
**          _TM_Error ( pFunction_name, TM_ERR_UNINITIALIZED, NULL ); <-<< **
**          return;                                                        **
**      }                                                                  **
**-------------------------------------------------------------------------**
**  DETECTABLE ERROR CONDITIONS                                            **
**      None                                                               **
**-------------------------------------------------------------------------**
**  DESCRIPTION                                                            **
**      This function will generate a message which will be sent to stderr **
**      to inform the user of an error.  This message will include the     **
**      name of the function the error occurred in (if supplied), a canned **
**      error string for the error indicated, and a custom string (if      **
**      supplied) which may provide more details about th error.           **
**-------------------------------------------------------------------------**
**  LIMITATIONS                                                            **
**      The message text must not exceed 1024 bytes in size, which can     **
**      store over 12 80-character lines of text.                          **
\***************************************************************************/

void _TM_Error ( pCalling_function_name, error_number, pCustom_message )
    const char  *pCalling_function_name;
    int          error_number;
    const char  *pCustom_message;
{
    char error_message[1024];          /* buffer for message text */

    /*---------------------------------------------------------------------*\
    **  Insert the "ERROR IN MODULE "TIMER"" header string into our        **
    **  message.                                                           **
    \*---------------------------------------------------------------------*/

    sprintf ( error_message, 
	      "\n******** ERROR IN MODULE \"TIMER\" *********\n" );

    /*---------------------------------------------------------------------*\
    **  Insert the name of the function in which the error was discovered. **
    **  This should always be provided, but check for NULL to be safe.     **
    \*---------------------------------------------------------------------*/

    strcat ( error_message, "FUNCTION: " );

    if ( pCalling_function_name != (char *)NULL )
	strcat ( error_message, pCalling_function_name );
    else
	strcat ( error_message, "<not specified - kill the programmer>" );

    strcat ( error_message, "\n" );

    /*---------------------------------------------------------------------*\
    **  Insert the canned error message text for the specified error       **
    **  number.                                                            **
    \*---------------------------------------------------------------------*/

    strcat ( error_message, gpTM_error_text[error_number] );
    strcat ( error_message, "\n" );

    /*---------------------------------------------------------------------*\
    **  Insert the custom_message, if it is supplied.  This custom message **
    **  should provide more detailed information than the generic error    **
    **  strings.                                                           **
    \*---------------------------------------------------------------------*/

    if ( pCustom_message != (char *)NULL )
    {
	strcat ( error_message, pCustom_message );
    } /* end if ( custom message was supplied ) */

    strcat ( error_message, "\n" );

    /*---------------------------------------------------------------------*\
    **  Send the message off to stderr.                                    **
    \*---------------------------------------------------------------------*/

    fprintf ( stderr, "%s", error_message );

} /* end _TM_Error ( ) */

/***************************************************************************\
**  tTIME TM_ReadTime( )                                                   **
*****************************************************************************
**  ARGUMENTS                                                              **
**      void                                                               **
**-------------------------------------------------------------------------**
**  RETURNS                                                                **
**      tTIME - the current time measured in units of 838ns                **
**-------------------------------------------------------------------------**
**  EXAMPLE USAGE (NOT INTENDED FOR EXTERNAL USE)                          **
**      InitializeApplication();                                           **
**      TM_Init(1);                                                        **
**        . . .                                                            **
**      current_time = TM_ReadTime( );        <-----<<                     **
**        . . .                                                            **
**      TM_Close();                                                        **
**      ShutDownApplication();                                             **
**-------------------------------------------------------------------------**
**  DETECTABLE ERROR CONDITIONS                                            **
**      *  Module has not been initialized (TM_Init() not called)          **
**-------------------------------------------------------------------------**
**  DESCRIPTION                                                            **
**      This function generates a 32-bit value in units of 838ns.  This    **
**      value spans a time period of about one hour.  The high 16 bits of  **
**      this value come from the large timer (the 32-bit value at          **
**      0x40:0x6C, the number of timer cycles since midnight), and the     **
**      low 16 bits come from system timer 0's counter.                    **
**                                                                         **
**      NOTE: System timer 0's counter repeatedly cycles from 65535 down   **
**            to 0 (decremented), but time is steadily increasing.  To     **
**            get a steadily increasing value from this timer, we subtract **
**            the actual value of the counter from 65535.                  **
**-------------------------------------------------------------------------**
**  LIMITATIONS                                                            **
**      The value returned by this function is the current time MOD        **
**      (approximately) one hour, not the time of day.  It is only useful  **
**      useful for timing events that finish within an hour.               **
\***************************************************************************/

tTIME TM_ReadTime ( void )
{
    register unsigned char LSB;        /* least significant byte           */
    register unsigned char MSB;        /* most significant byte            */
    char *pFunction_name =             /* This function's name, used for   */
	     "TM_ReadTime()";          /*     error reporting              */

    /*---------------------------------------------------------------------*\
    **  Handle uninitialized module error.                                 **
    \*---------------------------------------------------------------------*/

    if ( gTM_module_not_initialized )
    {
	_TM_Error ( pFunction_name, TM_ERR_UNINITIALIZED, NULL );
	return( (tTIME) (-1) );
    } /* end if */

    /*---------------------------------------------------------------------*\
    **  By writing 0x00 to port 0x43, we are specifying the following      **
    **  command:                                                           **
    **      bits 76  = 00  --> timer 0                                     **
    **      bits 54  = 00  --> reading 16-bit value: lsb followed by msb   **
    **      bits 321 = 000 --> ignored                                     **
    **      bit  0   = 0   --> ignored                                     **
    \*---------------------------------------------------------------------*/

    outportb ( 0x43, 0x00 );

    /*---------------------------------------------------------------------*\
    **  The following two statements reading from port 0x40 are reading    **
    **  system timer 0's 16-bit counter: MSB after LSB.                    **
    \*---------------------------------------------------------------------*/

    LSB = inportb ( 0x40 );                         /* get low order byte  */
    MSB = inportb ( 0x40 );                         /* get high order byte */

    /*---------------------------------------------------------------------*\
    **  Pack the time into the 32-bit tTIME return value.                  **
    **  (see the preamble at the top of this file for more details)        **
    \*---------------------------------------------------------------------*/

    return( (unsigned long int) (TM_LARGE_TIMER << 16)
	  | (unsigned int) (0xFFFF-((((unsigned int)MSB)<<8)|LSB)));

} /* end TM_ReadTime() */

/***************************************************************************\
**  void TM_StartTimer( tid )                                              **
*****************************************************************************
**  ARGUMENTS                                                              **
**      unsigned int tid                                                   **
**          (I) integer label, which is used as an index into the array    **
**              of timers (gpTM_start_time[])                              **
**-------------------------------------------------------------------------**
**  RETURNS                                                                **
**      void                                                               **
**-------------------------------------------------------------------------**
**  EXAMPLE USAGE                                                          **
**      #define THIRTYITH_OF_A_SECOND  39759                               **
**      #define DELAY_TIMER            0                                   **
**      InitializeApplication();                                           **
**      TM_Init(1);                                                        **
**        . . .                                                            **
**      TM_StartTimer(DELAY_TIMER);           <-----<<                     **
**      MoveMonsters();                                                    **
**      MovePlayers();                                                     **
**      UpdateDisplay();                                                   **
**      while (TM_ElapsedTime(DELAY_TIMER) < THIRTYITH_OF_A_SECOND)        **
**          ;                                                              **
**        . . .                                                            **
**      TM_Close();                                                        **
**      ShutDownApplication();                                             **
**-------------------------------------------------------------------------**
**  DETECTABLE ERROR CONDITIONS                                            **
**      *  Module has not been initialized (TM_Init() not called)          **
**      *  tid is out of range (range = [0..(gTM_max_timers-1)])           **
**-------------------------------------------------------------------------**
**  DESCRIPTION                                                            **
**      This procedure starts a pseudo-timer by reading the current time   **
**      and storing it in the global array gpTM_start_time[].              **
**-------------------------------------------------------------------------**
**  LIMITATIONS                                                            **
**      None                                                               **
\***************************************************************************/

void TM_StartTimer ( tid )
    unsigned int tid;
{
    char custom_message[256];          /* Buffer for custom error message  */
    char *pFunction_name =             /* This function's name, used for   */
	     "TM_StartTimer()";        /*     error reporting              */

    /*---------------------------------------------------------------------*\
    **  Handle uninitialized module error.                                 **
    \*---------------------------------------------------------------------*/

    if (gTM_module_not_initialized)
    {
	_TM_Error ( pFunction_name, TM_ERR_UNINITIALIZED, NULL );
	return;
    } /* end if */

    if ( tid < gTM_max_timers ) /* then tid is valid */
    {
	/*-----------------------------------------------------------------*\
	**  The timer ID is valid, so mark the starting time for this      **
	**  timer (tid)                                                    **
	\*-----------------------------------------------------------------*/

	gpTM_start_time[tid] = TM_ReadTime ( );
    }
    else /* tid is out of range */
    {
	/*-----------------------------------------------------------------*\
	**  Handle the bad timer ID error.                                 **
	\*-----------------------------------------------------------------*/

	sprintf ( custom_message, "Request received for timer %u, but the "
				  "last valid timer is timer %u.\n",
		  tid, gTM_max_timers-1 );
	_TM_Error ( pFunction_name, TM_ERR_BAD_TIMER_ID, custom_message );
    } /* end if */
} /* end TM_StartTimer() */

/***************************************************************************\
**  tTIME TM_ElapsedTime( tid )                                            **
*****************************************************************************
**  ARGUMENTS                                                              **
**      unsigned int tid                                                   **
**          (I) integer label, which is used as an index into the array of **
**              timers (gpTM_start_time[])                                 **
**-------------------------------------------------------------------------**
**  RETURNS                                                                **
**      tTIME - the amount of time that has passed since the last call     **
**              to TM_StartTimer(tid), measured in units of 838ns          **
**-------------------------------------------------------------------------**
**  EXAMPLE USAGE:                                                         **
**      #define THIRTYITH_OF_A_SECOND  39759                               **
**      #define DELAY_TIMER            0                                   **
**      InitializeApplication();                                           **
**      TM_Init(1);                                                        **
**        . . .                                                            **
**      TM_StartTimer(DELAY_TIMER);                                        **
**      MoveMonsters();                                                    **
**      MovePlayers();                                                     **
**      UpdateDisplay();                                                   **
**      while (TM_ElapsedTime(DELAY_TIMER) < THIRTYITH_OF_A_SECOND) <---<< **
**          ;                                                              **
**        . . .                                                            **
**      TM_Close();                                                        **
**      ShutDownApplication();                                             **
**-------------------------------------------------------------------------**
**  DETECTABLE ERROR CONDITIONS                                            **
**      *  Module has not been initialized (TM_Init() not called)          **
**      *  tid is out of range (range = [0..(gTM_max_timers-1)])           **
**-------------------------------------------------------------------------**
**  DESCRIPTION                                                            **
**      This function calculates the time that has elapsed for timer slot  **
**      tid since the last call to TM_StartTimer(tid) by getting the       **
**      current time and subtracting the starting time (stored in global   **
**      array gpTM_start_time[].)                                          **
**-------------------------------------------------------------------------**
**  LIMITATIONS                                                            **
**      The largest elapsed time that can accurately be measured is        **
**      approximately one hour:                                            ** 
**          (tTIME = unsigned long int = 32-bits)                          **
**          (65536(ticks/cycle)*18.2(cycles/sec)*3600 ~= 2^32)             **
\***************************************************************************/

tTIME TM_ElapsedTime ( tid )
    unsigned int tid;
{
    char custom_message[256];          /* Buffer for custom error message  */
    char *pFunction_name =             /* This function's name, used for   */
	     "TM_ElapsedTime()";       /*     error reporting              */

    /*---------------------------------------------------------------------*\
    **  Handle uninitialized module error.                                 **
    \*---------------------------------------------------------------------*/

    if ( gTM_module_not_initialized )
    {
	_TM_Error( pFunction_name, TM_ERR_UNINITIALIZED, NULL );
	return( (tTIME) (-1) );
    } /* end if */
	
    if ( tid < gTM_max_timers ) /* then tid is valid */
    {
	/*-----------------------------------------------------------------*\
	**  The timer ID is valid, so compute the elapsed time.            **
	\*-----------------------------------------------------------------*/

	return( TM_ReadTime ( ) - gpTM_start_time[tid] );
    }
    else /* tid is out of range */
    {
	/*-----------------------------------------------------------------*\
	**  Handle the bad timer ID error.                                 **
	\*-----------------------------------------------------------------*/

	sprintf ( custom_message, "Request received for timer %u, but the "
				  "last valid timer is timer %u.\n",
		  tid, gTM_max_timers-1 );
	_TM_Error ( pFunction_name, TM_ERR_BAD_TIMER_ID, custom_message );
	return( (tTIME) (-1) );
    } /* end if */
} /* end TM_ElapsedTime() */

/***************************************************************************\
**  int TM_Init( )                                                         **
*****************************************************************************
**  ARGUMENTS                                                              **
**      unsigned int num_timers                                            **
**          (I) number of timers the application is requesting             **
**-------------------------------------------------------------------------**
**  RETURNS                                                                **
**      int - non-zero means an error occurred                             **
**            0 if the timer module was successfully initialized           **
**-------------------------------------------------------------------------**
**  EXAMPLE USAGE                                                          **
**      InitializeApplication();                                           **
**      TM_Init(1);                     <-----<<                           **
**        . . .                                                            **
**      ApplicationBody();                                                 **
**        . . .                                                            **
**      TM_Close();                                                        **
**      ShutDownApplication();                                             **
**-------------------------------------------------------------------------**
**  DETECTABLE ERROR CONDITIONS                                            **
**      * Invalid number of timers requested (0)                           **
**      * Failed to allocated memory for array of pseudo-timers            **
**-------------------------------------------------------------------------**
**  DESCRIPTION                                                            **
**      This procedure sets timer 0 to operation mode 2, and allocates     **
**      memory to store starting times for the specified number of         **
**      (pseudo)timers.                                                    **
**                                                                         **
**      The gTM_module_not_initialized is set to 0 to express that the     **
**      module has been initialized, so the code can be used correctly.    **
**-------------------------------------------------------------------------**
**  LIMITATIONS                                                            **
**      None                                                               **
\***************************************************************************/

int TM_Init ( num_timers )
    unsigned int num_timers;
{
    char custom_message[256];          /* Buffer for custom error message  */
    char *pFunction_name =             /* This function's name, used for   */
	     "TM_Init()";              /*     error reporting              */

    /*---------------------------------------------------------------------*\
    **  Make sure this code doesn't get executed more than once before     **
    **  TM_Close().  We would allocate a new array of timers, and lose     **
    **  the previously allocated array (memory leak).                      **
    \*---------------------------------------------------------------------*/

    if ( !gTM_module_not_initialized )
    {
	return ( 0 );                /* return success                     */
				     /* (this isn't an error, just stupid) */
    }

    /*---------------------------------------------------------------------*\
    **  Allocate an array of qseudo-timers                                 **
    \*---------------------------------------------------------------------*/

    if ( num_timers > 0 )
    {
	gpTM_start_time = (tTIME *) calloc ( num_timers, sizeof ( tTIME ) );

	if ( gpTM_start_time == NULL )
	{
	    sprintf ( custom_message,
		      "Failed to allocate %u timers of size %u.\n",
		      num_timers, (unsigned int) sizeof ( tTIME ) );
	    _TM_Error ( pFunction_name, TM_ERR_ALLOC, custom_message );
	    return ( -1 );
	} /* end if */

	gTM_max_timers = num_timers;
    }
    else /* num_timers == 0, since num_timers is an unsigned int */
    {
	/*-----------------------------------------------------------------*\
	**  Application requested 0 timers, which is pointless.            **
	\*-----------------------------------------------------------------*/

	_TM_Error ( pFunction_name, TM_ERR_ZERO_TIMERS, NULL );
	return ( -1 );
    } /* end if-else */

    /*---------------------------------------------------------------------*\
    **  Set timer to operation mode 2                                      **
    **                                                                     **
    **  By writing 0x34 to port 0x43, we are specifying:                   **
    **     (0x34 = 00110100)                                               **
    **     bits 76  = 00  --> timer 0                                      **
    **     bits 54  = 11  --> writing 16-bit value: lsb followed by msb    **
    **     bits 321 = 010 --> operation mode 2                             **
    **     bit  0   = 0   --> binary counter operation                     **
    \*---------------------------------------------------------------------*/

    outportb ( 0x43, 0x34 );

    /*---------------------------------------------------------------------*\
    **  The following two statements writing 0x00 to port 0x40 are writing **
    **  a 16-bit value of 0x0000 to the timer's counter, which specifies   **
    **  that the counter is to begin its cycle with a value of 65536       **
    **  (0x10000), which specifies a maximum length cycle (54.926ms).      **
    \*---------------------------------------------------------------------*/

    outportb ( 0x40, 0x00 );
    outportb ( 0x40, 0x00 );

    /*---------------------------------------------------------------------*\
    **  Unlock this module                                                 **
    \*---------------------------------------------------------------------*/

    gTM_module_not_initialized = 0;

    return ( 0 );                      /* initialization successful */

} /* TM_Init() */

/***************************************************************************\
**  void TM_Close( )                                                       **
*****************************************************************************
**  ARGUMENTS                                                              **
**      void                                                               **
**-------------------------------------------------------------------------**
**  RETURNS                                                                **
**      void                                                               **
**-------------------------------------------------------------------------**
**  EXAMPLE USAGE                                                          **
**      InitializeApplication();                                           **
**      TM_Init(1);                                                        **
**        . . .                                                            **
**      ApplicationBody();                                                 **
**       . . .                                                             **
**      TM_Close();                     <-----<<                           **
**      ShutDownApplication();                                             **
**-------------------------------------------------------------------------**
**  DETECTABLE ERROR CONDITIONS                                            **
**      *  Module has not been initialized (TM_Init() not called)          **
**-------------------------------------------------------------------------**
**  DESCRIPTION                                                            **
**     This procedure returns system timer 0 to operation mode 3, and      **
**     deallocates the memory used to store the starting times for each    **
**     of the (pseudo)timers.                                              **
**                                                                         **
**     The gTM_module_not_initialized is set to 1 to prevent further use   **
**     of this module.  This module will remain locked until TM_Init()     **
**     is called.                                                          **
**-------------------------------------------------------------------------**
**  LIMITATIONS                                                            **
**      None                                                               **
\***************************************************************************/

void TM_Close ( void )
{
    char *pFunction_name =             /* This function's name, used for   */
	     "TM_Close()";             /*     error reporting              */

    /*---------------------------------------------------------------------*\
    **  Handle uninitialized module error.                                 **
    \*---------------------------------------------------------------------*/

    if ( gTM_module_not_initialized )
    {
	_TM_Error ( pFunction_name, TM_ERR_UNINITIALIZED, NULL );
	return;
    } /* end if */

    /*---------------------------------------------------------------------*\
    **  Deallocate the array of pseudo-timers                              **
    \*---------------------------------------------------------------------*/

    if ( gpTM_start_time != NULL )
    {
	free ( gpTM_start_time );
	gpTM_start_time = NULL;
    } /* end if */

    /*---------------------------------------------------------------------*\
    **  Set timer to operation mode 3                                      **
    **                                                                     **
    **  By writing 0x36 to port 0x43, we are specifying:                   **
    **     (0x36 = 00110110)                                               **
    **     bits 76  = 00  --> timer 0                                      **
    **     bits 54  = 11  --> writing 16-bit value: lsb followed by msb    **
    **     bits 321 = 011 --> operation mode 3                             **
    **     bit  0   = 0   --> binary counter operation                     **
    \*---------------------------------------------------------------------*/

    outportb ( 0x43, 0x36 );

    /*---------------------------------------------------------------------*\
    **  The following two statements writing 0x00 to port 0x40 are writing **
    **  a 16-bit value of 0x0000 to the timer's counter, which specifies   **
    **  that the counter is to begin its cycle with a value of 65536       **
    **  (0x10000), which specifies a maximum length cycle (54.926ms).      **
    \*---------------------------------------------------------------------*/

    outportb ( 0x40, 0x00 );
    outportb ( 0x40, 0x00 );

    /*---------------------------------------------------------------------*\
    **  Lock this module                                                   **
    \*---------------------------------------------------------------------*/

    gTM_module_not_initialized = 1;

} /* end TM_Close ( ) */

SNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIPSNIP

/***************************************************************************\
** TIMING CODE MODULE                                                      **
**=========================================================================**
**  Written by Ethan Rohrer, (c) Nuthing Software, December 4, 1994        **
**                                                                         **
**  Revision History                                                       **
**  ----------------                                                       **
**  Date        Description                                                **
**  ---------   ---------------------------------------------------------  **
**  13 May 94   Initial Release                                            **
**  04 Dec 94   Updated code to conform to current coding standards/style  **
**              Allowed appl. to specify # of timers as param. to TM_Init  **
**              Standardized error handling with function _TM_Error   	   **
**=========================================================================**
**  This file contains the declarations for publicly available types and   **
**  functions.                                                             **
**=========================================================================**
**  USING THIS MODULE                                                      **
**      Before calling any other timing routine, you must call TM_Init(n), **
**      where  n  specifies the number of timers your application needs.   **
**      It may be a good idea to call TM_Init() in the initialization      **
**      portion of your application.                                       **
**                                                                         **
**      To begin timing an event, make a call to TM_StartTimer(tid),       **
**      where tid is an integer in the range [0..(n-1)] which specifies    **
**      which of the n timers you are starting.                            **
**                                                                         **
**      To compute the duration of the event, just call                    **
**      TM_ElapsedTime(tid), where tid is the same integer used in the     **
**      call to TM_StartTimer().                                           **
**                                                                         **
**      When you are finished with the timing routines, call TM_Close().   **
**      This should fit in nicely with the cleanup section of your         **
**      application.                                                       **
**                                                                         **
**      If your application NEEDS to handle the time computations itself,  **
**      the function TM_ReadTimer(tid) is also available.  This function   **
**      will return the current time (MOD 1 hour, approximately).  I       **
**      discourage use of this function, but discovered that I need it     **
**      for another module/library...                                      **
**                                                                         **
**      EXAMPLES                                                           **
**          A simple delaying routine:                                     **
**          (TM_Init() must be called before this routine!)                **
**              void delay( tTIME duration )                               **
**              {                                                          **
**                  TM_StartTimer(0);                                      **
**                  while (TM_ElapsedTime(0) < duration)                   **
**                      ;                                                  **
**              }                                                          **
**                                                                         **
**          A fixed frame-rate game:                                       **
**              TM_Init(1);                                                **
**              while (player_not_dead)                                    **
**              {                                                          **
**                  TM_StartTimer(0);                                      **
**                  MoveMonsters();                                        **
**                  MovePlayers();                                         **
**                  UpdateDisplay();                                       **
**                  while (TM_ElapsedTime(0) < frame_duration)             **
**                      ;                                                  **
**              }                                                          **
**              TM_Close();                                                **
\***************************************************************************/

#ifndef __TIMER_H__
#define __TIMER_H__

/*------------------------------ Types ------------------------------------*/

/* type used to store time: should be 32-bits */

typedef  unsigned long int  tTIME;

/*----------------------------- Defines -----------------------------------*/

/* maximum number of timers to allow */

#define  TM_MAX_TIMERS  1

/* just a handy constant */

#define  ONE_SECOND  1193200    /* 65536(ticks/cycle) * 18.207(cycles/sec) */

/*------------------------- Public Prototypes -----------------------------*/

extern  void   TM_Close( );
extern  tTIME  TM_ElapsedTime( );
extern  int    TM_Init( );
extern  tTIME  TM_ReadTime( );
extern  void   TM_StartTimer( );

#endif /* __TIMER_H__ */

