/*
 * Personal Cron 1.0 - doesn't need root access to install or run!
 * 
 * This cron will only run stuff that's in $CRONTAB.  It runs as the invoker
 * (unless you set the uid to someone else and set the uid bit).
 * 
 * Before you run this, (cc [-DBSD] -s -o cron cron.c) "setenv CRONTAB
 * $HOME/crontab", and stick something appropriate in it.
 * 
 * cron is Copyright 1992 by Edwin R. Carp (erc@apple.com).  Permission is
 * granted to redistribute as part of the linux operating system.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#ifdef BSD
#define index strchr
#define getcwd(a,b) getwd(a)
#endif
char           *getenv();
#ifdef NULL
#undef NULL
#endif
#define NULL (0)
#define WDLEN 64		/* length of working directory */
#define CHILD 0
char           *getenv();
int             cpid, pidstat, ret;
main()
{
#ifndef MSDOS
   if (fork() != CHILD)
      exit(0);			/* BG the process */
   /* signal(SIG_IGN, SIGCLD); */
   signal(SIG_IGN, SIGHUP);
#endif
   while (1)
      check();
}

int
check()
{
   long            time(), secs;
   struct tm      *tm, *localtime();

   time(&secs);
   tm = localtime(&secs);
   if (tm->tm_sec != 0)
      sleep(60 - tm->tm_sec);
   cron();
   sleep(1);
}
cron()
{

   /* simulate cron(1) running on a PC */
   /* should be run once a minute */
   /* crontab file is "set CRONTAB=file" */
   /* returns EOF if we can't read the crontab file */

   FILE           *ctab;
#ifdef MSDOS
#define CRONDEFAULT "c:/usr/lib/crontab"
#else
#define CRONDEFAULT "/usr/lib/crontab"
#endif
   char           *ctabfile, line[512], *lineptr, *strtok(), *ptr, *savptr,
                  *getenv();
   char           *strchr(), *savcwd(), *savewd;
   int             comp[5], match, i;
   long            secs;
   struct tm      *tm, *localtime();

   if ((ctabfile = getenv("CRONTAB")) == (char *) NULL)
      ctabfile = CRONDEFAULT;
   if ((char *) NULL != getenv("DEBUG"))
      printf("DEBUG: crontab filG: crontab file=%s\n", ctabfile);

   if ((ctab = fopen(ctabfile, "r")) == (FILE *) NULL)
      return (EOF);
   if ((char *) NULL != getenv("DEBUG"))
      printf("DEBUG: crontab file open=%s\n", ctabfile);

   time(&secs);
   tm = localtime(&secs);
   comp[0] = tm->tm_min;
   comp[1] = tm->tm_hour;
   comp[2] = tm->tm_mday;
   comp[3] = tm->tm_mon;
   comp[4] = tm->tm_wday;
   while ((char *) NULL != fgets(line, 510, ctab))
   {
      if ((char *) NULL != getenv("DEBUG"))
	 printf("DEBUG: line=%s", line);

      if (strlen(line) == 0)
	 continue;
      line[strlen(line) - 1] = NULL;	/* get rid of \n */
      if ((char *) NULL != getenv("DEBUG"))
	 printf("DEBUG: valid line=%s\n", line);

      lineptr = line;
      match = 0;
      for (i = 0; i < 5; i++)
      {
	 ptr = strtok(lineptr, " \t");
	 lineptr = NULL;
	 if (ptr == (char *) NULL)
	    continue;
	 savptr = ptr;
	 if ((char *) NULL != getenv("DEBUG"))
	    printf("DEBUG: ptr=%s, comp[%d]=%d, comparing\n", ptr, i, comp[i]);

	 if (1 == croncmp(ptr, comp[i]))
	 {
	    if ((char *) NULL != getenv("DEBUG"))
	       printf("DEBUG: ptr=%s, comp[%d]=%d, match!\n", ptr, i, comp[i]);

	    match++;
	 }
	 else if ((char *) NULL != getenv("DEBUG"))
	    printf("DEBUG: ptr=%s, comp[%d]=%d, no match\n", ptr, i, comp[i]);

      }
      ptr = strtok(lineptr, "\0");
      /* ptr should now point to the command to be executed */
      if (match == 5)
      {
	 if ((char *) NULL != getenv("DEBUG"))
	    printf("DEBUG: xqt: %s\n", ptr);
	 savewd = savcwd();
#ifdef WINDOW
	 if (nflag == 1)
	 {
	    curno();
	    systemparse(ptr);
	 }
	 else
	 {
	    curno();
	    nflag = 1;
	    systemparse(ptr);
	    curyes();
	    nflag = 0;
	 }
#else
	 systemparse(ptr);
#endif
      }
      else if ((char *) NULL != getenv("DEBUG"))
	 printf("DEBUG: NO xqt: %s, match=%d\n", ptr, match);

   }
   chdir(savewd);
   fclose(ctab);
   return (0);
}

croncmp(ptr, i)
    char           *ptr;
    int             i;
{

   /* parse ptr and compare to i.  If we find a match, return 1 */
   /* else return 0 */

   int             low, high, current, minusflag = 0;
   char            s[512], *st;

   if (*ptr == '*')
      return (1);		/* always a match */
   if ((char *) NULL != getenv("DEBUG"))
      printf("DEBUG: no star in %s\n", ptr);

   if ((char *) NULL == strchr(ptr, ',') && (char *) NULL == strchr(ptr, '-'))
   {
      if ((char *) NULL != getenv("DEBUG"))
	 printf("DEBUG: looking for exact match: %d = % match: %d = %d\n", atoi(ptr), i);

      if (atoi(ptr) == i)
	 return (1);
      else
	 return (0);
   }
   while (1)
   {
      st = s;
      while (*ptr != ',' && *ptr != '-' && *ptr != NULL)
	 *st++ = *ptr++;
      *st = NULL;
      if ((char *) NULL != getenv("DEBUG"))
	 printf("DEBUG: s=%s\n", s);

      current = atoi(s);
      if (*ptr == ',')
      {
	 if ((char *) NULL != getenv("DEBUG"))
	    printf("DEBUG: found a comma, current=%d, i=%d\n", current, i);

	 if (current == i)
	    return (1);
	 ptr++;
	 continue;		/* match not found yet */
      }
      if (*ptr == '-')
      {
	 if ((char *) NULL != getenv("DEBUG"))
	    printf("DEBUG: found a minus, current=%d\n", current);

	 low = current;
	 minusflag = 1;
	 ptr++;
	 continue;
      }
      if (*ptr == NULL)
      {
	 if ((char *) NULL != getenv("DEBUG"))
	    printf("DEBUG: found a NULL, current=%d, low=%d\n", current, low);

	 if (minusflag == 0)
	 {
	    if ((char *) NULL != getenv("DEBUG"))
	       printf("DEBUG: ...but does %d = %d?\n", current, i);

	    if (current == i)
	       return (1);
	    else
	       return (0);	/* no match */
	 }
	 high = current;
	 if (low <= i && high >= i)
	    return (1);
	 else
	    return (0);
      }
   }
}

int
systemparse(ptr)
    char           *ptr;
{
   /* parse and execute commands separated by a ";" */
   char            line[512], cmd[512], *lineptr, *p;

   strcpy(line, ptr);
#ifndef MSDOS
   sprintf(line, "(nohup %s) > /dev/null&", ptr);
   system(line);
   sleep(2);
#else
   lineptr = line;
   while ((char *) NULL != (p = strtok(lineptr, ";")))
   {
      lineptr = NULL;
      strcpy(cmd, p);
      system(cmd);
   }
#endif
}

char           *
savcwd()
{
   static char     curd[128];

   getcwd(curd, 120);
   return (curd);
}
