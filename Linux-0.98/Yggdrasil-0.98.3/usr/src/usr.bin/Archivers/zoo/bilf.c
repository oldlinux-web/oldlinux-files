/* $Source: /usr/home/dhesi/zoo/RCS/bilf.c,v $ */
/* $Id: bilf.c,v 1.2 91/07/07 14:57:06 dhesi Exp $ */

/*
This program performs conversion of files between stream-LF format
(as used by zoo) and fixed-length record binary format (used for Kermit
transfers of zoo archives).

This program is:
   (C) Copyright 1987 Rahul Dhesi.
   All Rights Reserved.

Permission is hereby granted to copy and modify this for any purpose,
whether commercial or noncommercial, provided only that the above
copyright notice and this paragraph be preserved and included
in all copies.

                                 -- Rahul Dhesi 1987/07/25
*/

#include <stdio.h>
#include <ssdef.h>
#define STAT_NORM SS$_NORMAL
#define STAT_ABORT SS$_ABORT

char *strrchr();
char *strdup ();

main (argc, argv)
int argc;
char *argv[];
{
   char *inname;
   char *outname;
   char *option;
   int status;

   if (argc < 3 || argc > 4) {
      printf ("BILF version 1.00 for VAX/VMS by Rahul Dhesi (1987/07/25)\n\n");
      printf ("(C) Copyright 1987 Rahul Dhesi,  All Rights Reserved\n");
      printf ("Permission to use and distribute is granted provided this copyright\n"); 
      printf ("notice is preserved and included in all copies.\n\n");
      printf ("Usage:  BILF {lb} infile [ outfile ]\n\n");
      printf ("Choose one character from within braces.  If outfile is not supplied\n");
      printf ("it has the same name as infile but a higher version number.\n");
      printf ("Options are:\n\n");
      printf ("l:  Write output file in stream-LF format.  This is the format that\n");
      printf ("    zoo expects all zoo archives to be in.  If a zoo archive was\n");
      printf ("    uploaded to a VAX/VMS system, it will need to be converted to\n");
      printf ("    stream-LF format before manipulating with zoo.\n\n");
      printf ("b:  Write output file in fixed-length 512-byte binary record format.  Before\n");
      printf ("    a zoo archive can be downloaded from a VAX/VMS system to a\n");
      printf ("    microcomputer using VAX/VMS Kermit, it must be converted to\n");
      printf ("    this binary format.  Failure to do so will result in a corrupted\n");
      printf ("    download.\n");
      exit (STAT_NORM);
   }

   inname = argv[2];
   option = argv[1];

   if (argc == 3) {                    /* use same filename for output */
      char *p;
      outname = strdup (inname);
      p = strrchr (outname, ';');      /* strip trailing version field */
      if (p != NULL)
         *p = '\0';
   } else
      outname = argv[3];

   if (*option == 'l')
      status = cvtstream (outname, inname);
   else if (*option == 'b')
      status = cvtbin (outname, inname);
   else
      prterror ('f', "Option %s is invalid\n", option);
   if (status == -1)
      prterror ('w', "An error occurred -- output file may be corrupted\n");
   exit (STAT_NORM);
}

#define  MYBUFSIZ    8192

/* writes input file to output file in stream format */
int cvtstream (outname, inname)
char *outname, *inname;
{
   FILE *infile, *outfile;
   char buffer[MYBUFSIZ];
   int count;

   infile = fopen (inname, "r");
   if (infile == NULL)
      prterror ('f', "Could not open input file %s\n", inname);
   outfile = fopen (outname, "w");
   if (outfile == NULL)
      prterror ('f', "Could not open output file %s\n", outname);

   while ((count = fread (buffer, 1, sizeof (buffer), infile)) > 0)
      count = fwrite (buffer, 1, count, outfile);

   close (infile); close (outfile);
   if (count == -1)
      return (-1);
   else
      return (0);
}

/*
VMS C doesn't have strdup().
*/
char *strdup (str)
char *str;
{
   char *malloc();
   char *newstr = malloc (strlen (str) + 1);
   if (newstr != NULL) {
      strcpy (newstr, str);
      return (newstr);
   } else
      return ((char *) NULL);
}

/* BLKSIZ must correspond to block size specified below in creat() */
#define BLKSIZ 512

/*
Writes input file to output in fixed-length BLKSIZ-byte record format.
*/

#if 1
#include <file.h>
#else
#include <fcntl.h>
#endif

int convert ();

int cvtbin (outname, inname)
char *outname, *inname;
{
   int status, inhan, outhan;
   inhan = open (inname, O_RDONLY);
   if (inhan == -1)
      prterror ('f', "Could not open input file %s\n", inname);
   outhan = creat (outname, 0, "rfm=fix", "mrs=512");
   if (outhan == -1)
      prterror ('f', "Could not open output file %s\n", outname);
   status = convert (outhan, inhan);
   close (inhan);
   close (outhan);
   return (status);
}

/*
Function convert() reads from inhan and writes to outhan, always
writing in BLKSIZ-byte blocks, padding with nulls if necessary
*/

int convert (outhan, inhan)
int inhan, outhan;
{
   char junk[BLKSIZ];
   int count;
   int done = 0;
   do {
      count = vmsread (inhan, junk, BLKSIZ);
      if (count <= 0)
         break;
      if (count < BLKSIZ) {
         int i;
         for (i = count; i < BLKSIZ; i++)
            junk[i] = 0;
         done++;
      }
      count = write (outhan, junk, BLKSIZ);
      if (count == -1)
         break;
   } while (!done);
   if (count == -1)
      return (-1);
   else
      return (0);
}

/**** Function vmsread() does a standard read() but gets around bugs
in the read() function of VAX/VMS C which make it unable to always
read the entire amount requested in a single read() call.
*/
int vmsread (han, buf, amount)
int han;
char *buf;
int amount;
{
   int count;
   int thiscount;
   count = 0;
   while (count != -1 && count < amount) {
      thiscount = read (han, &buf[count], amount - count);
      if (thiscount == 0)
         thiscount = read (han, &buf[count], amount - count);
      if (thiscount == 0)
          break;
      if (thiscount == -1)
         count = -1;
      else
         count += thiscount;
   }
   return (count);
}

prterror (level, msg1, msg2)
char level;
char *msg1, *msg2;
{
   if (level == 'e' || level == 'w' || level == 'f')
      printf ("BILF: ");

   switch (level) {
      case 'e': printf ("ERROR: ");    break;
      case 'w': printf ("WARNING: ");  break;
      case 'f': printf ("FATAL: ");    break;
      default:  prterror ('f', "Internal error in prterror()\n");
   }

   printf (msg1, msg2);
   if (level == 'f')
      exit (STAT_ABORT);
}
