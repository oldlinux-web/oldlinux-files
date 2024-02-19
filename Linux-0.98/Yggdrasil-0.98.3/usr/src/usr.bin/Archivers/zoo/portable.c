#ifndef LINT
/* @(#) portable.c 2.24 88/08/24 01:22:06 */
static char sccsid[]="@(#) portable.c 2.24 88/08/24 01:22:06";
#endif /* LINT */

#include "options.h"
/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
*/
/**********************
portable.c contains functions needed to make Zoo portable to various
implementations of C.

Note:  Provided a 2's complement machine is used, all functions in
this file are themselves machine-independent and need not be changed
when implementing Zoo on a different machine.  Some code will choke
on 1's complement machines--I think.  

For machine-dependent declarations see files "machine.h" and "options.h". 

For machine-dependent functions see file "machine.c"
*/

#include "zoo.h"
#include "zooio.h"

#include "various.h"
#include "zoofns.h"

#include "machine.h"
#include "debug.h"
#include "assert.h"

#ifdef NEEDCTYP
#include <ctype.h>              /* for tolower() */
#endif

#include "portable.h"

#ifdef TRACE_IO
extern int verbose;
#endif

/* Functions defined for use within this file only.  */
long to_long PARMS((BYTE[]));
int to_int PARMS((BYTE[]));
void b_to_zooh PARMS((struct zoo_header *, BYTE[]));
void b_to_dir PARMS((struct direntry *, BYTE[]));
int dir_to_b PARMS((BYTE[], struct direntry *));
void zooh_to_b PARMS((BYTE[], struct zoo_header *));
void splitlong PARMS((BYTE[], long));
void splitint PARMS((BYTE[], int));

#ifdef TRACE_IO
void show_h PARMS ((struct zoo_header *));
void show_dir PARMS ((struct direntry *));
#endif /* TRACE_IO */

extern unsigned int crccode;

/************************************************************************/
/* I/O functions */
/************************************************************************/

/* some functions get defined only if they aren't already macros */

#ifndef zooread
int zooread (file, buffer, count)
ZOOFILE file; char *buffer; int count;
{ return (fread (buffer, 1, count, file)); }
#endif /* zooread */

#ifndef FIZ
#ifndef zoowrite
int zoowrite (file, buffer, count)
ZOOFILE file; char *buffer; int count;
{ 
	if (file == NULLFILE)
	   return (count);
	else
		return (fwrite (buffer, 1, count, file)); 
}
#endif /* zoowrite */

ZOOFILE zoocreate (fname)
char *fname;
{ return ((ZOOFILE) fopen (fname, Z_NEW)); }

#endif /* FIZ */

#ifndef zooseek
long zooseek (file, offset, whence)
ZOOFILE file; long offset; int whence;
{ return (fseek (file, offset, whence)); }
#endif /* zooseek */

ZOOFILE zooopen (fname, option)
char *fname; char *option;
{ return ((ZOOFILE) fopen (fname, option)); }

#ifndef zootell
long zootell (file)
ZOOFILE file;
{ return ftell (file); }
#endif /* zootell */

int zooclose (file)
ZOOFILE file;
{ return fclose (file); }

/**********************
low_ch() is a macro that returns a lowercased char; it may be
used with any char, whether or not it is uppercase.   It will
be used below by one or two functions.
*/

#define low_ch(c)		(isupper(c) ? tolower(c) : c)

/************************************************************************/
/*** Following are functions that make up for various implementations ***/
/*** of C not having certain library routines.                        ***/
/************************************************************************/

#ifndef FIZ
/**********************
str_lwr() converts a string to lowercase and returns a pointer to the string
*/
char *str_lwr (str)
char *str;
{
   register char *s;
   s = str;
   while (*s != '\0') {
      *s = toascii(*s);
		*s = low_ch(*s);
      s++;
   }
   return (str);
}

/**********************
str_icmp() compares strings just like strcmp() but it does it without regard to
case.
*/
int str_icmp (s1, s2)
register char *s1, *s2;
{
   for ( ; low_ch(*s1) == low_ch(*s2);  s1++, s2++)
      if (*s1 == '\0')
         return(0);
   return(low_ch(*s1) - low_ch(*s2));
}

#ifdef NEED_MEMSET
/**********************
memset() it sets the first "count" bytes of "dest" to the character
"c" and returns a pointer to "dest".
*/
VOIDPTR memset (dest, c, count)
register VOIDPTR dest;
int c;
unsigned count;
{
   register unsigned i;
   for (i = 0; i < count; i++) {
      *((char *) (dest + i)) = c;
   }
   return dest;
}
#endif /* NEED_MEMSET */

#ifdef NEED_MEMCPY
/**********************
memcpy() copies "count" bytes from "src" to "dest" and returns 
a pointer to "dest".  Not necessarily safe for overlapping moves. */

VOIDPTR memcpy(dest, src, count)
register VOIDPTR dest;
register VOIDPTR src;
unsigned count;
{
	VOIDPTR savedest = dest;
	while (count > 0) {
		*((char *) dest++) = *((char *) src++);
		count--;
	}
}
#endif /* NEED_MEMCPY */

#ifndef FPUTCHAR
/**********************
fputchar() writes a character to stdout.  It is identical to putchar
but is a function, not a macro.
*/
int fputchar (c)
int c;
{
   return (fputc(c, stdout));
}
#endif /* FPUTCHAR */
#endif /* FIZ */

/***********************************************************************/
/*** Following are declarations and functions that are written in a  ***/
/*** machine-independent way but they implement machine-dependent    ***/
/*** activities                                                      ***/
/***********************************************************************/

#ifndef DIRECT_CONVERT
/**********************
to_long() converts four consecutive bytes, in order of increasing
significance, to a long integer.  It is used to make Zoo independent of the
byte order of the system.  
*/
long to_long(data)
BYTE data[];
{
   return (long) ((unsigned long) data[0] | ((unsigned long) data[1] << 8) |
         ((unsigned long) data[2] << 16) | ((unsigned long) data[3] << 24));
}

#ifndef FIZ
/********************
splitlong() converts a long integer to four consecutive BYTEs in order
of increasing significance.
*/
void splitlong(bytes, bigword)
BYTE bytes[];
long bigword;
{
   int i;
   for (i = 0; i < 4; i++) {
      bytes[i] = bigword & 0xff;
      bigword = (unsigned long) bigword >> 8;
   }
}     
#endif /* FIZ */

/*******************
splitint() converts an integer to two consecutive BYTEs in order
of increasing significance.
*/
void splitint(bytes, word)
BYTE bytes[];
int word;
{
   bytes[0] = word & 0xff;
   word = (unsigned int) word >> 8;
   bytes[1] = word & 0xff;
}

/**********************
to_int() converts two consecutive bytes, in order of increasing
significance, to an integer, in a machine-independent manner
*/
int to_int(data)
BYTE data[];
{
   return (int) ((unsigned int) data[0] | ((unsigned int) data[1] << 8));
}

#else /* else of ifndef DIRECT_CONVERT */

long to_long(data)
BYTE data[];
{
   return ( * (long *) data );
}

#ifndef FIZ
/********************
splitlong() converts a long integer to four consecutive BYTEs in order
of increasing significance.
*/
void splitlong(bytes, bigword)
BYTE bytes[];
long bigword;
{
   * (long *) bytes = bigword;
}
#endif /* FIZ */

/*******************
splitint() converts an integer to two consecutive BYTEs in order
of increasing significance.
*/
void splitint(bytes, word)
BYTE bytes[];
int word;
{
   * (int *) bytes = word;
}

/**********************
to_int() converts two consecutive bytes, in order of increasing
significance, to an integer.
*/
int to_int(data)
BYTE data[];
{
   return (*(int *) data);
}

#endif /* ifndef DIRECT_CONVERT .. else ... */

#ifndef FIZ
/**********************
Function frd_zooh() reads the header of a Zoo archive in a machine-
independent manner, from a ZOOFILE.
*/
int frd_zooh(zoo_header, zoo_file)
struct zoo_header *zoo_header;
ZOOFILE zoo_file;
{
   int status;
   BYTE bytes[SIZ_ZOOH];         /* canonical header representation */
#ifdef TRACE_IO
   if (verbose) {
      printf("At file position [%8lx] ", ftell(zoo_file));
   }
#endif
   status = zooread (zoo_file, (char *) bytes, SIZ_ZOOH);
   b_to_zooh (zoo_header, bytes);   /* convert array to structure */
#ifdef TRACE_IO
   if (verbose) {
      printf("frd_zooh: reading\n");
      show_h(zoo_header);
   }
#endif
   if (status < MINZOOHSIZ)
      return (-1);
   else
      return (0);
}
#endif /* FIZ */

/**********************
Function frd_dir() reads a directory entry in a machine-independent manner,
from a ZOOFILE.
*/
int frd_dir(direntry, zoo_file) 
struct direntry *direntry; 
ZOOFILE zoo_file;
{
   int status;
   BYTE bytes[MAXDIRSIZE];    /* big enough to hold variable part too */

   /* To simplify things, we read the maximum possible size of the
   directory entry including the variable size and discard what is not
   needed */
#ifdef TRACE_IO
   if (verbose) {
      printf("At file position [%8lx] ", ftell(zoo_file));
   }
#endif
   status = zooread (zoo_file, (char *) bytes, MAXDIRSIZE);
   if (status < SIZ_DIR)
      return (-1);
   b_to_dir (direntry, bytes);
#ifdef TRACE_IO
   if (verbose) {
      printf("frd_dir: reading\n");
      show_dir(direntry);
   }
#endif
   return (0);
}

#ifndef FIZ
/***********************
Function fwr_dir() writes a directory entry in a machine-independent manner
to a ZOOFILE.  Return value is -1 on error, else 0.
*/
int fwr_dir(direntry, zoo_file)
struct direntry *direntry;
ZOOFILE zoo_file;
{
   int size;
   BYTE bytes[MAXDIRSIZE];
   assert (direntry->type <= 2);
   size = dir_to_b (bytes, direntry);
#ifdef TRACE_IO
   if (verbose) {
      printf("At file position [%8lx] ", ftell(zoo_file));
      printf("fwr_dir: writing\n");
      show_dir(direntry);
   }
#endif

   if (zoowrite (zoo_file, (char *) bytes, size) != size)
      return (-1);
   else
      return (0);
}

/***********************
Function fwr_zooh() writes an archive header in a machine-independent manner
to a ZOOFILE.  Return value is -1 if error else 0.
*/
int fwr_zooh(zoo_header, zoo_file)
struct zoo_header *zoo_header;
ZOOFILE zoo_file;
{
   BYTE bytes[SIZ_ZOOH];	/* was SIZ_DIR -- probably a typo */
	int hsize;					/* how much to write -- depends on header type */
	hsize = MINZOOHSIZ;				/* in case it's an old type 0 header */
	if (zoo_header->type > 0)		/* but if it's a newer header... */
		hsize = SIZ_ZOOH;				/* ...size of new type 1 header */
   zooh_to_b (bytes, zoo_header);
   if (zoowrite (zoo_file, (char *) bytes, hsize) != hsize)
      return (-1);
   else
      return (0);
}

/***********************
b_to_zooh() converts an array of BYTE to a zoo_header structure.
*/
void b_to_zooh (zoo_header, bytes)
struct zoo_header *zoo_header;
BYTE bytes[];
{
   int i;
   for (i = 0; i < SIZ_TEXT; i++)                     /* copy text */
      zoo_header->text[i] = bytes[TEXT_I + i];
   zoo_header->zoo_tag = to_long(&bytes[ZTAG_I]);     /* copy zoo_tag */
   zoo_header->zoo_start = to_long(&bytes[ZST_I]);    /* copy zoo_start */
   zoo_header->zoo_minus = to_long(&bytes[ZSTM_I]);
   zoo_header->major_ver = bytes[MAJV_I];          /* copy versions */
   zoo_header->minor_ver = bytes[MINV_I];
	/* default is no archive comment and a header type of 0 */
	zoo_header->type = 0;
	zoo_header->acmt_pos = 0L;
	zoo_header->acmt_len = 0;
	zoo_header->vdata		= 0;
	if (zoo_header->zoo_start != FIXED_OFFSET) {			/* if newer header */
		zoo_header->type = bytes[HTYPE_I];
		zoo_header->acmt_pos = to_long(&bytes[ACMTPOS_I]);
		zoo_header->acmt_len = to_int(&bytes[ACMTLEN_I]);
		zoo_header->vdata		= bytes[HVDATA_I];
	}
}

/***********************
zooh_to_b() converts a zoo_header structure to an array of BYTE.
*/
void zooh_to_b (bytes, zoo_header)
struct zoo_header *zoo_header;
BYTE bytes[];
{
   int i;
   for (i = 0; i < SIZ_TEXT; i++)                     /* copy text */
      bytes[TEXT_I + i] = zoo_header->text[i];
   splitlong (&bytes[ZTAG_I], zoo_header->zoo_tag);
   splitlong (&bytes[ZST_I], zoo_header->zoo_start);
   splitlong (&bytes[ZSTM_I], zoo_header->zoo_minus);
   bytes[MAJV_I] =   zoo_header->major_ver;           /* copy versions */ 
   bytes[MINV_I] =   zoo_header->minor_ver;
	bytes[HTYPE_I] =	zoo_header->type;						/* header type */
	if (zoo_header->type > 0) {
		splitlong (&bytes[ACMTPOS_I], zoo_header->acmt_pos);	/* comment posn */
		splitint (&bytes[ACMTLEN_I], zoo_header->acmt_len);	/* comment len */
		bytes[HVDATA_I] = zoo_header->vdata;					/* version data */
	}
} /* zooh_to_b() */

/************************
dir_to_b() converts a directory entry structure to an array of BYTE.
*/
int dir_to_b (bytes, direntry)
struct direntry *direntry;
BYTE bytes[];
{
   int i;
   int cursize;
   int fixsize;
	int totalsize;
   splitlong(&bytes[DTAG_I], direntry->zoo_tag);
   bytes[DTYP_I] = direntry->type ;
   bytes[PKM_I] = direntry->packing_method ;
   splitlong(&bytes[NXT_I], direntry->next);
   splitlong(&bytes[OFS_I], direntry->offset);
   splitint(&bytes[DAT_I], direntry->date);
   splitint(&bytes[TIM_I], direntry->time);
   splitint(&bytes[CRC_I], direntry->file_crc);
   splitlong(&bytes[ORGS_I], direntry->org_size);
   splitlong(&bytes[SIZNOW_I], direntry->size_now);
   bytes[DMAJ_I] = direntry->major_ver;
   bytes[DMIN_I] = direntry->minor_ver;
   bytes[DEL_I] = direntry->deleted;
   bytes[STRUC_I] = direntry->struc;
   splitlong(&bytes[CMT_I], direntry->comment);
   splitint(&bytes[CMTSIZ_I], direntry->cmt_size);
   for (i = 0; i < FNM_SIZ; i++)
      bytes[FNAME_I + i] = direntry->fname[i];
   bytes[TZ_I] = NO_TZ;       /* assume unknown */
   bytes[NAMLEN_I] = 0;
   bytes[DIRLEN_I] = 0;

   cursize = SIZ_DIR;         /* to count size of directory */
   fixsize = SIZ_DIR;         /* size of fixed part */
   assert (direntry->type <= 2);
   if (direntry->type == 2) { /* handle stuff relevant to type 2 */
      cursize = SIZ_DIRL;
      fixsize = SIZ_DIRL;
      bytes[TZ_I] = direntry->tz;
      assert(direntry->namlen < 256 && direntry->namlen >= 0);
		cursize += 2;        /* space for namlen and dirlen */
      if (direntry->namlen != 0) {
         bytes[NAMLEN_I] = direntry->namlen;
         for (i = 0; i < direntry->namlen; i++)
            bytes[LFNAME_I+i] = direntry->lfname[i];
         cursize += direntry->namlen;
      }
      assert(direntry->dirlen < 256 && direntry->dirlen >= 0);
      if (direntry->dirlen != 0) {
         bytes[DIRLEN_I] = direntry->dirlen;
         for (i = 0; i < direntry->dirlen; i++)
            bytes[cursize+i] = direntry->dirname[i];
         cursize += direntry->dirlen;
      }
		/* Can't store system id if no namlen & dirlen...BUG!...now fixed.
			Fortunately, system_id was always 0 so far so it probably
			got interpreted as namlen=0 and dirlen=0 (2 bytes) */
      splitint(&bytes[cursize], direntry->system_id);
		cursize += 2;
		bytes[cursize] = direntry->fattr & 0xff;		  					/* byte 0 */
		splitint(&bytes[cursize+1], (int) (direntry->fattr >> 8));  /* 1 & 2 */
		cursize += 3;
		bytes[cursize] = (direntry->vflag & 0xff);			/* version flag */
		splitint(&bytes[cursize+1], direntry->version_no);	/* version number */
		cursize += 3;
   }

   splitint(&bytes[VARDIRLEN_I], direntry->var_dir_len);
   assert(cursize == 
            ((bytes[DIRLEN_I] > 0 || bytes[NAMLEN_I] > 0) ? 2 : 0) +
            fixsize + bytes[DIRLEN_I] + bytes[NAMLEN_I]
         );

	/* total size of dir entry is size of fixed part + size of var. part */
	totalsize = fixsize + direntry->var_dir_len;

   /* Do CRC assuming CRC field is zero, and stuff CRC into field. */
   splitint(&bytes[DCRC_I], 0);           /* fill with zeroes */
   crccode = 0;
   /* avoid mixing pointers to signed and unsigned char */
   addbfcrc((char *) bytes, totalsize); 		/* update CRC */
   splitint(&bytes[DCRC_I], crccode);

   /* return total length of directory entry */
   return (totalsize);

} /* dir_to_b() */
#endif /* FIZ */

/* b_to_dir() converts bytes to directory entry structure.  The CRC of the
directory bytes, if any, is checked and a zero or nonzero value is returned
in direntry->dir_crc according as the check is good or bad */

void b_to_dir(direntry, bytes)
struct direntry *direntry;
BYTE bytes[];
{
   int i;
	int sysid_offs;			/* temp variable */
   unsigned int savecrc;
   direntry->zoo_tag = to_long(&bytes[DTAG_I]);
   direntry->type = bytes[DTYP_I];
   direntry->packing_method = bytes[PKM_I];
   direntry->next = to_long(&bytes[NXT_I]);
   direntry->offset = to_long(&bytes[OFS_I]);
   direntry->date = to_int(&bytes[DAT_I]);
   direntry->time = to_int(&bytes[TIM_I]);
   direntry->file_crc = to_int(&bytes[CRC_I]);
   direntry->org_size = to_long(&bytes[ORGS_I]);
   direntry->size_now = to_long(&bytes[SIZNOW_I]);
   direntry->major_ver = bytes[DMAJ_I];
   direntry->minor_ver = bytes[DMIN_I];
   direntry->deleted = bytes[DEL_I];
   direntry->struc = bytes[STRUC_I];
   direntry->comment = to_long(&bytes[CMT_I]);
   direntry->cmt_size = to_int(&bytes[CMTSIZ_I]);
	/* for now, versions not implemented */
	direntry->vflag = 0;
	direntry->version_no = 0;
   for (i = 0; i < FNM_SIZ; i++)
      direntry->fname[i] = bytes[FNAME_I + i];

   /* start by assuming variable part is zero bytes */
   direntry->var_dir_len = direntry->dir_crc    = 0;
   direntry->namlen      = direntry->dirlen     = 0;
   direntry->lfname[0]   = direntry->dirname[0] = '\0';
   direntry->tz = NO_TZ;               /* assume unknown */
   direntry->system_id = SYSID_NIX;    /* default system_id if not present */
	direntry->fattr = NO_FATTR;			/* assume none */

   assert (direntry->type <= 2);
   if (direntry->type == 2) {
      direntry->var_dir_len = to_int(&bytes[VARDIRLEN_I]);
      assert(direntry->var_dir_len <= MAXDIRSIZE);
      if (direntry->var_dir_len > MAXDIRSIZE)
         direntry->var_dir_len = MAXDIRSIZE;
      direntry->tz = bytes[TZ_I];   
      if (direntry->var_dir_len > 0)
         direntry->namlen = bytes[NAMLEN_I];
      if (direntry->var_dir_len > 1)
         direntry->dirlen = bytes[DIRLEN_I];
      for (i = 0; i < direntry->namlen; i++)
         direntry->lfname[i] = bytes[LFNAME_I + i];
      for (i = 0; i < direntry->dirlen; i++)
         direntry->dirname[i] = bytes[DIRNAME_I + direntry->namlen + i];
		sysid_offs = DIRNAME_I + direntry->namlen + i;	/* offset of system id */
		if (direntry->var_dir_len > direntry->namlen + direntry->dirlen + 2) {
			direntry->system_id = to_int(&bytes[sysid_offs]);
		}
		if (direntry->var_dir_len > direntry->namlen + direntry->dirlen + 4) {
			direntry->fattr = ((unsigned long) bytes[sysid_offs + 2]) |
									((unsigned long) bytes[sysid_offs + 3] << 8) |
									((unsigned long) bytes[sysid_offs + 4] << 16);
		}
		if (direntry->var_dir_len > direntry->namlen + direntry->dirlen + 7) {
			direntry->vflag = bytes[sysid_offs + 5];
			direntry->version_no = to_int(&bytes[sysid_offs + 6]);
		}
      /* do CRC calculation */
      savecrc = (unsigned int) to_int(&bytes[DCRC_I]);
      crccode = 0;
      splitint(&bytes[DCRC_I], 0);
      addbfcrc((char *) bytes, SIZ_DIRL + direntry->var_dir_len);
      direntry->dir_crc = crccode - savecrc;
   }
}

#ifdef FILTER
#define TWOBYTES	2	/* better than literal 2;  figure out why */

/* rdint() reads two bytes from standard input in archive order */
int rdint (val)
unsigned int *val;
{
	BYTE bytes[TWOBYTES];
	if (zooread (STDIN, bytes, TWOBYTES) == TWOBYTES) {
		*val = to_int(bytes);
		return (0);
	} else
		return (1);
}

/* wrint() writes an unsigned int to standard output in archive order */
int wrint (val)
unsigned int val;
{
	BYTE bytes[TWOBYTES];
	splitint (bytes, val);
	if (zoowrite (STDOUT, bytes, TWOBYTES) == TWOBYTES)
		return (0);
	else
		return (1);
}
#endif /* FILTER */

#ifdef TRACE_IO
/* dump contents of archive header */
void show_h (zoo_header)
struct zoo_header *zoo_header;
{
   int i;
   printf ("Header text:\n");
   for (i = 0; i < SIZ_TEXT;  i++) {      /* ASSUMES ASCII TEXT */
      int c;
      c = zoo_header->text[i];
      if (c >= ' ' && c < 0x7f)
         putchar (c);
      else {
         putchar ('^');
         putchar (i & 0x40);
      }
   }
   putchar('\n');
   printf ("zoo_tag = [%8lx] zoo_start = [%8lx] zoo_minus = [%8lx]\n",
            zoo_header->zoo_tag, zoo_header->zoo_start, 
            zoo_header->zoo_minus);
   printf ("major_ver.minor_ver = [%d.%d]\n",
            zoo_header->major_ver, zoo_header->minor_ver);
	if (zoo_header->zoo_start != FIXED_OFFSET) {
		printf ("type = [%d] ", zoo_header->type);
		printf ("acmt_pos = [%8lx] acmt_len = [%4x] vdata = [%2x]",
					zoo_header->acmt_pos, zoo_header->acmt_len, zoo_header->vdata);
		printf ("\n");
	}
   printf ("---------\n");
}

/* dump contents of directory entry */
void show_dir (direntry)
struct direntry *direntry;
{
   printf ("Directory entry for file [%s][%s]:\n",
            direntry->fname, direntry->lfname);
   printf ("tag = [%8lx] type = [%d] PM = [%d] Next = [%8lx] Offset = [%8lx]\n",
            direntry->zoo_tag, (int) direntry->type, 
            (int) direntry->packing_method, direntry->next, 
            direntry->offset);
   printf ("Orig size = [%ld] Size now = [%ld] dmaj_v.dmin_v = [%d.%d]\n",
         direntry->org_size, direntry->size_now,
         (int) direntry->major_ver, (int) direntry->minor_ver);
   printf ("Struc = [%d] DEL = [%d] comment_offset = [%8lx] cmt_size = [%d]\n",
         (int) direntry->struc, (int) direntry->deleted, direntry->comment,
         direntry->cmt_size);
   printf ("var_dir_len = [%d] TZ = [%d] dir_crc = [%4x]\n",
            direntry->var_dir_len, (int) direntry->tz, direntry->dir_crc);
   printf ("system_id = [%d]  dirlen = [%d]  namlen = [%d] fattr=[%24lx]\n", 
		direntry->system_id, direntry->dirlen, direntry->namlen, direntry->fattr);
	printf ("vflag = [%4x] version_no = [%4x]\n",
				direntry->vflag, direntry->version_no);
   if (direntry->dirlen > 0)
      printf ("dirname = [%s]\n", direntry->dirname);
   printf ("---------\n");
}
#endif   /* TRACE_IO */
