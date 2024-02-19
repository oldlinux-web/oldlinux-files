/*
$Header: /usr/src/linux/lp/lpjob.h,v 1.1 1992/01/12 18:33:46 james_r_wiegand Exp $

  linux/lp/lpjob.h  printer job setup data definitions for lp
  c. 1992 James Wiegand

*/
#include <stdio.h>
#include <time.h>
#include "pcap.h"

#ifdef __LP_JOB__
  #define EXTERN
#else
  #define EXTERN extern
#endif

extern char* char2esc( char * );

#define FAILURE -1
#define SSV( index ) string_set [ index ].svalue
#define NSV( index ) numeric_set[ index ].nvalue
#define IFIS( string )\
        (( string ) ? ( string ) : (char *) ' ')
        
/* ok, here it is folks... */
struct lp_numeric
  {
  int  nvalue;
  char *npcapname;
  };

struct lp_string
  {
  char *svalue;
  char *spcapname;
  };

enum { DU, UU,
       NC, NF, SB, WR, SH,
       MC, PW, PL, PX, PY, HL, FL, EX };

enum { LO,
      FF, TR, PR, LS, HT, FT, SD, LF };
             
#ifdef __LP_JOB__
struct lp_numeric numeric_set[] =
  {
    { 0,  "du" }, /* daemon uid */
    { 0,  "uu" }, /* user uid */
    /* **WARNING** the loops in numericread are hardcoded to these offsets */
    /* start with flags  2 - 5 */
    { 0,  "nc" }, /* no copies flag */
    { 0,  "nf" }, /* no formfeed flag */
    { 0,  "sb" }, /* short banner flag */
    { 0,  "wr" }, /* wrap lines */
    { 0,  "sh" }, /* skip header */
    /* now read numeric values 6 - 13 */
    { 1,  "mc" }, /* maximum number of copies */
    { 80, "pw" }, /* characters/line */
    { 66, "pl" }, /* lines/page */
    { 0,  "px" }, /* page width in pixels ???? */
    { 0,  "py" }, /* page height in pixels ???? */
    { 0,  "hl" }, /* header lines */
    { 0,  "fl" }, /* footer lines */
    { 0,  "ex" }, /* expand tabs 0 = hard tabs, nonzero = tab value */
  };

struct lp_string string_set[] =
  {
    { ".daemon",  "lo" }, /* lock file name */
    /* start reading here 1 - 8 */
    {  "\xC",     "ff" }, /* form feed char */
    {  0,         "tr" }, /* trailer string */
    {  0,         "pr" }, /* page prolog- page setup string */
    {  0,         "ls" }, /* line prolog- line setup string */
    {  0,         "ht" }, /* header text */
    {  0,         "ft" }, /* footer text */
    {  0,         "sd" }, /* spool dir */
    {  0,         "lf" }  /* log dir */
  };

char *defaultjob = "lp";

#else
  extern struct lp_numeric numeric_set[];
  extern struct lp_string  string_set [];
  extern char *defaultjob;
#endif
  
/* job data */

EXTERN char pcapbuffer[ 1024 ];
/* EXTERN char *strdup( char * );  */
EXTERN char pusername[ 14 ];
EXTERN char pgroup   [ 14 ];
EXTERN char pfilename[ 14 ];
EXTERN char pdate    [ 14 ];
EXTERN time_t ptime;
EXTERN int  pageno;

/* functions in lpjob.c */
void numericread( void );
void stringread( void );
void lpjobinit( void );
void lpbanner( int );

/* functions in lptext.c */
int processtext( int in, int out );

/* functions in lp.c */
void message( char *, ... );
