#ifndef LINT
static char sccsid[]="@(#) fiz.c 2.6 88/01/31 23:23:50";
#endif /* LINT */

/*
The contents of this file are hereby released to the public domain.

                                   -- Rahul Dhesi 1987/02/06
*/

/*
Searches for all directory entries in an archive and prints their
offsets.  Zoo 1.41 and later may then be asked to extract a specific
file by supplying the offset of the file.
*/

#include "options.h"
#include "zooio.h"
#include "various.h"
#include "zoofns.h"
#include "portable.h"         /* I/O definitions */
#include "zoo.h"

void prtctrl ();
void prtch ();

main(argc,argv)
register int argc;
register char **argv;
{
   char *zooname;          /* name of archive to be read */
   ZOOFILE zoo_file;       /* the archive being examined opened for read */
   int state;              /* to keep track of how much of tag seen */
   int inch;               /* char just read from archive */

   static char usage1[] = "Fiz 2.0 (1987/02/01) public domain Zoo archive repair utility by Rahul Dhesi\n";
   static char usage2[] = "Usage:  fiz archive[.zoo]  (\"fiz -h\" for help)\n";

#ifdef SETBUF
/* set stdout to unbuffered */
setbuf (stdout, (char *) NULL);
#endif

   if (argc < 2) {
      printf("%s%s", usage1, usage2);
      exit (1);
   }

   if (strcmp(argv[1],"-h") == 0)
      goto givehelp;

   zooname = argv[1];

   /* Add default extension if none supplied */
   {
      char *p, *q;
      p = zooname + strlen(zooname);         /* point to last char */
      while (p != zooname && *p != EXT_CH)
         --p;
      /* either found EXT_CH or reached beginning of zooname */
      if (*p != EXT_CH) {
         q = malloc(strlen(zooname) + strlen(EXT_DFLT) + 2);
         if (q == NULL) {
            printf("Fiz:  Ran out of memory.\n");
            exit(1);
         }
         strcpy(q, zooname);
         strcat(q, EXT_DFLT);
         zooname = q;
      }
   }

   zoo_file = zooopen (zooname, Z_READ);
   if (zoo_file == NOFILE) {
      printf("Fiz:  FATAL:  Could not open %s.\n", zooname);
      exit(1);
   }

#ifdef DOUBLE_SECRET
	{ void oh_well(void); oh_well(); }
#endif

#define  NOSTATE  1
#define  HDR_1   0xdc
#define  HDR_2   0xa7
#define  HDR_3   0xc4
#define  HDR_4   0xfd

#define	DAT_1   '@'
#define	DAT_2   ')'
#define	DAT_3   '#'
#define	DAT_4   '('

/* finite state machine implemented here by hand */

   state = NOSTATE;
   while ((inch = zgetc(zoo_file)) != EOF) {
      inch = inch & 0xff;
      if (state == NOSTATE) {
			if (inch == HDR_1)
				state = HDR_1;
			else if (inch == DAT_1)
				state = DAT_1;
		} else if (state == HDR_1 && inch == HDR_2)
         state = HDR_2;
      else if (state == HDR_2 && inch == HDR_3)
         state = HDR_3;
      else if (state == HDR_3 && inch == HDR_4)
         state = HDR_4;
		else if (state == DAT_1 && inch == DAT_2)
			state = DAT_2;
		else if (state == DAT_2 && inch == DAT_3)
			state = DAT_3;
		else if (state == DAT_3 && inch == DAT_4)
			state = DAT_4;
      else
         state = NOSTATE;

      if (state == HDR_4) {           						/* found archive tag */
         long save_pos;
         struct direntry direntry;
         save_pos = zootell(zoo_file);
         zooseek(zoo_file, save_pos-4L, 0);				/* back to tag pos */
         frd_dir(&direntry, zoo_file);						/* read dir entry */
         printf("****************\n");

         printf ("%8lu: DIR ", save_pos-4L);

         if (direntry.dirlen > 0) {
            printf ("[");
            prtctrl (direntry.dirname);
            printf ("]");
         }

         printf(" [");
         prtctrl (direntry.fname);
         printf ("]");

         if (direntry.namlen > 0) {
            printf (" [");
            prtctrl (direntry.lfname);
            printf ("]");
         }
			printf (" ==> %4lu", direntry.offset);
         if (direntry.dir_crc != 0)
            printf (" [*bad CRC*]");
         printf ("\n");
         fseek (zoo_file, save_pos, 0);         /* try again from there */
      } else if (state == DAT_4) {				/* file data */
         printf ("%8lu: DATA\n", zootell(zoo_file) + 1);
		}
   }
exit (0);      /* don't fall through */

givehelp:

/*
vi macros:
to add printf:
:s/^.*$/printf("&\\n");/
To remove printf:
:s/^printf("\(.*\)\\n");/\1/
*/
printf("Fiz is used to help you recover data from a damaged archive.  Fiz searches\n");
printf("the specified archive for directory entries and stored files, and prints the\n");
printf("position of each one found.  Each directory entry contains a number that\n");
printf("represents the location in the archive where the file is stored;  fiz also\n");
printf("prints this position.  All numbers printed are decimal numbers.\n\n");

printf("Use Zoo version 2.00 or higher to list or extract files in the damaged\n");
printf("archive starting at a position identified by fiz.  For example, you can\n");
printf("start extracting files from archive \"badarc.zoo\" at position 1098 with the\n");
printf("command:\n\n");

printf("     zoo x@1098 badarc\n\n");

printf("Zoo will ignore the first 1098 bytes of the damaged archive and you should be\n");
printf("able to recover the undamaged files from the rest of the archive.  You can\n");
printf("also manually specify where to look for the file data with a command like\n\n");

printf("     zoo x@1098,1153\n\n");

printf("which tells zoo to use the directory entry at position 1098, but to get the\n");
printf("actual file data from position 1153 (and not from where the directory entry\n");
printf("says the data ought to be).  See the manuals for fiz and zoo for more details.\n");

exit (0);
}

/*
prtctrl() prints a string with all unprintable characters converted
to printable form.  To avoid the program running astray trying to
print damaged data, no more than MAXPRT characters are printed.
Characters with the 8th bit set are printed preceded with ~.  Control
characters are printed preceded with ^.  Both ~ and ^ may preced
the character if a control character has the 8th bit set.
*/
#define  MAXPRT      50

void prtctrl (str)
char *str;
{
   unsigned int ch;
   int count;
   count = 0;

   while (count < MAXPRT && *str != '\0') {
      ch = (unsigned) *str;
      prtch(ch);
      str++;
      count++;
   }
}

/*
Does the actual character printing for prtctrl()
*/
void prtch(ch)
unsigned int ch;
{
   /* assumes ASCII character set */
   if (ch < ' ') {                        /* ^@ through ^_ */
      printf("^%c", ch + 0x40);
   } else if (ch == 0x7f) {               /* DEL */
      printf("^?");
   } else if (ch > 0x7f) {                /* 8th bit set */
      printf("~");                        /* .. so precede with ~ */
      prtch(ch & 0x7f);                   /* slick recursive call */
   } else
      printf("%c", ch);                   /* plain char */
}
