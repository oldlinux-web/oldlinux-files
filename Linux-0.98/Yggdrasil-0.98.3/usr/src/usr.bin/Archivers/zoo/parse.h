/* @(#) parse.h 2.1 87/12/25 12:24:15 */

/*
The contents of this file are hereby released to the public domain.

                           -- Rahul Dhesi 1986/11/14
*/

/*
defines structure used in call to parse()
*/
#define XTRA  2         /* extra space to avoid off-by-one errors */


struct path_st {
   char drive[2+1+XTRA];            /* drive name            */
   char dir[PATHSIZE+1+XTRA];       /* path prefix           */
   char fname[8+1+XTRA];            /* root name of filename */
   char lfname[LFNAMESIZE+1+XTRA];  /* long filename    */
   char ext[EXTLEN+1+XTRA];         /* extension        */
};

#ifdef LINT_ARGS
void parse (struct path_st *, char *);
#else
void parse();
#endif
