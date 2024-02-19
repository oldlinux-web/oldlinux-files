/* strip certain symbols from a rel file.
   Copyright (C) 1986 Free Software Foundation, Inc.

		       NO WARRANTY

  BECAUSE THIS PROGRAM IS LICENSED FREE OF CHARGE, WE PROVIDE ABSOLUTELY
NO WARRANTY, TO THE EXTENT PERMITTED BY APPLICABLE STATE LAW.  EXCEPT
WHEN OTHERWISE STATED IN WRITING, FREE SOFTWARE FOUNDATION, INC,
RICHARD M. STALLMAN AND/OR OTHER PARTIES PROVIDE THIS PROGRAM "AS IS"
WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY
AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM PROVE
DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR
CORRECTION.

 IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW WILL RICHARD M.
STALLMAN, THE FREE SOFTWARE FOUNDATION, INC., AND/OR ANY OTHER PARTY
WHO MAY MODIFY AND REDISTRIBUTE THIS PROGRAM AS PERMITTED BELOW, BE
LIABLE TO YOU FOR DAMAGES, INCLUDING ANY LOST PROFITS, LOST MONIES, OR
OTHER SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE
USE OR INABILITY TO USE (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR
DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY THIRD PARTIES OR
A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS) THIS
PROGRAM, EVEN IF YOU HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
DAMAGES, OR FOR ANY CLAIM BY ANY OTHER PARTY.

		GENERAL PUBLIC LICENSE TO COPY

  1. You may copy and distribute verbatim copies of this source file
as you receive it, in any medium, provided that you conspicuously
and appropriately publish on each copy a valid copyright notice
"Copyright (C) 1986 Free Software Foundation, Inc.", and include
following the copyright notice a verbatim copy of the above disclaimer
of warranty and of this License.

  2. You may modify your copy or copies of this source file or
any portion of it, and copy and distribute such modifications under
the terms of Paragraph 1 above, provided that you also do the following:

    a) cause the modified files to carry prominent notices stating
    that you changed the files and the date of any change; and

    b) cause the whole of any work that you distribute or publish,
    that in whole or in part contains or is a derivative of this
    program or any part thereof, to be licensed at no charge to all
    third parties on terms identical to those contained in this
    License Agreement (except that you may choose to grant more extensive
    warranty protection to some or all third parties, at your option).

    c) You may charge a distribution fee for the physical act of
    transferring a copy, and you may at your option offer warranty
    protection in exchange for a fee.

Mere aggregation of another unrelated program with this program (or its
derivative) on a volume of a storage or distribution medium does not bring
the other program under the scope of these terms.

  3. You may copy and distribute this program (or a portion or derivative
of it, under Paragraph 2) in object code or executable form under the terms
of Paragraphs 1 and 2 above provided that you also do one of the following:

    a) accompany it with the complete corresponding machine-readable
    source code, which must be distributed under the terms of
    Paragraphs 1 and 2 above; or,

    b) accompany it with a written offer, valid for at least three
    years, to give any third party free (except for a nominal
    shipping charge) a complete machine-readable copy of the
    corresponding source code, to be distributed under the terms of
    Paragraphs 1 and 2 above; or,

    c) accompany it with the information you received as to where the
    corresponding source code may be obtained.  (This alternative is
    allowed only for noncommercial distribution and only if you
    received the program in object code or executable form alone.)

For an executable file, complete source code means all the source code for
all modules it contains; but, as a special exception, it need not include
source code for modules which are standard libraries that accompany the
operating system on which the executable file runs.

  4. You may not copy, sublicense, distribute or transfer this program
except as expressly provided under this License Agreement.  Any attempt
otherwise to copy, sublicense, distribute or transfer this program is void and
your rights to use the program under this License agreement shall be
automatically terminated.  However, parties who have received computer
software programs from you with this License Agreement will not have
their licenses terminated so long as such parties remain in full compliance.

  5. If you wish to incorporate parts of this program into other free
programs whose distribution conditions are different, write to the Free
Software Foundation at 675 Mass Ave, Cambridge, MA 02139.  We have not yet
worked out a simple rule that can be stated here, but we will often permit
this.  We will be guided by the two goals of preserving the free status of
all derivatives of our free software and of promoting the sharing and reuse of
software.

 In other words, you are welcome to use, share and improve this program.
 You are forbidden to forbid anyone else to use, share and improve
 what you give them.   Help stamp out software-hoarding!  */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

#ifdef USG
#include <fcntl.h>
#include <string.h>
#else
#include <strings.h>
#endif

#ifdef COFF_ENCAPSULATE
#include "a.out.encap.h"
#else
/* On native BSD systems, use the system's own a.out.h.  */
#include <a.out.h>
#endif

/* If BSD, we can use `ftruncate'.  */

#ifndef USG
#define HAVE_FTRUNCATE
#endif

/* Struct or union for header of object file.  */

#define HEADER_TYPE struct exec

/* Count the number of nlist entries that are for local symbols. */
int local_sym_count;

/* Count number of nlist entries that are for local symbols
   whose names don't start with L. */
int non_L_local_sym_count;

/* Count the number of nlist entries for debugger info.  */
int debugger_sym_count;

/* Count the number of global symbols referenced or defined.  */
int global_sym_count;

/* Total number of symbols to be preserved in the current file.  */
int nsyms;

/* Number of files specified in the command line. */

int number_of_files;

/* Each specified file has a file_entry structure for it.
   These are contained in the vector which file_table points to.  */

struct file_entry {
  char *filename;
  HEADER_TYPE header;		/* the file's header */
  int ss_size;			/* size, in bytes, of symbols_and_strings data */
  struct nlist *symbols_and_strings;
};

struct file_entry *file_table;

/* Descriptor on which current file is open.  */

int input_desc;

/* Stream for writing that file using stdio.  */

FILE *outstream;

/* 1 => strip all symbols; 2 => strip all debugger symbols */
int strip_symbols;

/* 1 => discard locals starting with L; 2 => discard all locals */
int discard_locals;

void strip_file ();
int file_open ();
void rewrite_file_symbols(), file_close();
int read_header (), read_entry_symbols (), read_file_symbols ();
void count_file_symbols ();
char *concat ();

main (argc, argv)
     char **argv;
     int argc;
{
  int c;
  extern int optind;
  struct file_entry *p;
  int i;

  strip_symbols = 0;   /* default is to strip everything.  */
  discard_locals = 0;

  while ((c = getopt (argc, argv, "sSxX")) != EOF)
    switch (c)
      {
      case 's':
	strip_symbols = 1;
	break;
      case 'S':
	strip_symbols = 2;
	break;
      case 'x':
	discard_locals = 2;
	break;
      case 'X':
	discard_locals = 1;
	break;
      }

  /* Default is to strip all symbols.  */
  if (strip_symbols == 0 && discard_locals == 0)
    strip_symbols = 1;

  number_of_files = argc - optind;

  if (!number_of_files)
    fatal ("no files specified", 0);

  p = file_table
    = (struct file_entry *) xmalloc (number_of_files * sizeof (struct file_entry));

  /* Now fill in file_table */

  for (i = 0; i < number_of_files; i++)
    {
      p->filename = argv[i + optind];
      p->symbols_and_strings = 0;
      p++;
    }

  for (i = 0; i < number_of_files; i++)
    strip_file (&file_table[i]);
}

/* process one input file */

void
strip_file (entry)
     struct file_entry *entry;
{
  int val;

  local_sym_count = 0;
  non_L_local_sym_count = 0;
  debugger_sym_count = 0;
  global_sym_count = 0;

  val = file_open (entry);
  if (val < 0)
    return;

  if (strip_symbols != 1)
    /* Read in the existing symbols unless we are discarding everything.  */
    {
      if (read_file_symbols (entry) < 0)
	return;
    }

  rewrite_file_symbols (entry);
  if (strip_symbols != 1)
    free (entry->symbols_and_strings);

  file_close ();
}

/** Convenient functions for operating on one or all files being processed.  */

/* Close the file that is now open.  */

void
file_close ()
{
  close (input_desc);
  input_desc = 0;
}

/* Open the file specified by 'entry', and return a descriptor.
   The descriptor is also saved in input_desc.  */

/* JF this also makes sure the file is in rel format */

int
file_open (entry)
     struct file_entry *entry;
{
  int desc;
  int len, magicnum;

  desc = open (entry->filename, O_RDWR, 0);

  if (desc > 0)
    {
      input_desc = desc;
#ifdef HEADER_SEEK_FD
      /* Skip the headers that encapsulate our data in some other format
	 such as COFF.  */
      HEADER_SEEK_FD (desc);
#endif
      len = read (input_desc, (char *)&magicnum, sizeof magicnum);
      if (len != sizeof magicnum)
	{
	  error_with_file ("failure reading header", entry);
	  return -1;
	}
      if (N_BADMAG (*((struct exec *)&magicnum)))
	{
	  error_with_file ("malformed input file (not an object file)", entry);
	  return -1;
	}
      if (read_header (desc, &entry->header, entry) < 0)
	return -1;
      return desc;
    }

  perror_file (entry);
  return -1;
}

/* Print the filename of ENTRY on OUTFILE (a stdio stream), then a newline.  */

prline_file_name (entry, outfile)
     struct file_entry *entry;
     FILE *outfile;
{
  print_file_name (entry, outfile);
  fprintf (outfile, "\n");
}

/* Print the filename of ENTRY on OUTFILE (a stdio stream).  */

print_file_name (entry, outfile)
     struct file_entry *entry;
     FILE *outfile;
{
  fprintf (outfile, "%s", entry->filename);
}

/* Validate file ENTRY and read its symbol and string sections into core. */

int
read_file_symbols (entry)
     struct file_entry *entry;
{
  if (read_entry_symbols (input_desc, entry) < 0)
    return -1;
  count_file_symbols (entry);
  return 0;
}

/* Read a file's header into the proper place in the file_entry.
   Return -1 on failure.  */

int
read_header (desc, loc, entry)
     int desc;
     struct exec *loc;
     struct file_entry *entry;
{
  int len;
  lseek (desc, 0, 0);
#ifdef HEADER_SEEK_FD
  /* Skip the headers that encapsulate our data in some other format
     such as COFF.  */
  HEADER_SEEK_FD (desc);
#endif
  len = read (desc, loc, sizeof (struct exec));
  if (len != sizeof (struct exec))
    {
      error_with_file ("failure reading header", entry);
      return -1;
    }
  if (N_BADMAG (*loc))
    {
      error_with_file ("bad magic number", entry);
      return -1;
    }
  return 0;
}

/* Read the symbols and strings of file ENTRY into core.
   Assume it is already open, on descriptor DESC.
   Return -1 on failure.  */

int
read_entry_symbols (desc, entry)
     struct file_entry *entry;
     int desc;
{
  int string_size;

  lseek (desc, N_STROFF (entry->header), 0);
  if (sizeof string_size != read (desc, &string_size, sizeof string_size))
    {
      error_with_file ("bad string table", entry);
      return -1;
    }

  entry->ss_size = string_size + entry->header.a_syms;
  entry->symbols_and_strings = (struct nlist *) xmalloc (entry->ss_size);

  lseek (desc, N_SYMOFF (entry->header), 0);
  if (entry->ss_size != read (desc, entry->symbols_and_strings, entry->ss_size))
    {
      error_with_file ("premature end of file in symbols/strings", entry);
      return -1;
    }
  return 0;
}


/* Count the number of symbols of various categories in the file of ENTRY.  */

void
count_file_symbols (entry)
     struct file_entry *entry;
{
  struct nlist *p, *end = entry->symbols_and_strings + entry->header.a_syms / sizeof (struct nlist);
  char *name_base = entry->header.a_syms + (char *) entry->symbols_and_strings;

  for (p = entry->symbols_and_strings; p < end; p++)
    if (p->n_type & N_EXT)
      global_sym_count++;
    else if (p->n_un.n_strx && !(p->n_type & (N_STAB | N_EXT)))
      {
	if ((p->n_un.n_strx + name_base)[0] != 'L')
	  non_L_local_sym_count++;
	local_sym_count++;
      }
    else debugger_sym_count++;
}

void write_file_syms (), modify_relocation ();

/* Total size of string table strings allocated so far */
int strtab_size;

/* Vector whose elements are the strings to go in the string table */
char **strtab_vector;

/* Index in strtab_vector at which the next string will be stored */
int strtab_index;

int sym_written_count;

int
assign_string_table_index (name)
     char *name;
{
  int index = strtab_size;

  strtab_size += strlen (name) + 1;
  strtab_vector[strtab_index++] = name;

  return index;
}

void
rewrite_file_symbols (entry)
     struct file_entry *entry;
{
  int i;
  struct nlist *newsyms;

  /* Calculate number of symbols to be preserved.  */

  if (strip_symbols == 1)
    nsyms = 0;
  else
    {
      nsyms = global_sym_count;
      if (discard_locals == 1)
	nsyms += non_L_local_sym_count;
      else if (discard_locals == 0)
	nsyms += local_sym_count;
    }

  if (strip_symbols == 0)
    nsyms += debugger_sym_count;

  strtab_vector = (char **) xmalloc (nsyms * sizeof (char *));
  strtab_index = 0;

  strtab_size = 4;

  /* Accumulate in 'newsyms' the symbol table to be written.  */

  newsyms = (struct nlist *) xmalloc (nsyms * sizeof (struct nlist));

  sym_written_count = 0;

  if (strip_symbols != 1)
    /* Write into newsyms the symbols we want to keep.  */
    write_file_syms (entry, newsyms);

  if (sym_written_count != nsyms)
    {
      fprintf (stderr, "written = %d, expected = %d\n",
	       sym_written_count, nsyms);
      abort ();
    }

  /* Modify the symbol-numbers in the relocation in the file,
     to preserve its meaning */
  modify_relocation (input_desc, entry);

#ifndef	HAVE_FTRUNCATE
  {
    int size = N_SYMOFF (entry->header), mode;
    char *renamed = (char *)concat ("~", entry->filename, "~");
    char *copy_buffer = (char *)xmalloc (size);
    struct stat statbuf;

    lseek (input_desc, 0, 0);
    if (read (input_desc, copy_buffer, size) != size)
      {
	error_with_file ("can't read up to symbol table", entry);
	return;
      }
    mode = fstat (input_desc, &statbuf) ? 0666 : statbuf.st_mode;
    if (rename (entry->filename, renamed))
      {
	perror_file (entry);
	return;
      }
    input_desc = creat (entry->filename, mode);
    if (input_desc < 0)
      {
	perror_file (entry);
	return;
      }
    if (write (input_desc, copy_buffer, size) != size)
      perror_file (entry);
    if (unlink (renamed))
      perror_name (renamed);
    free (copy_buffer);
    free (renamed);
  }
#endif /* not HAVE_FTRUNCATE */

  /* Now write contents of NEWSYMS into the file. */

  lseek (input_desc, N_SYMOFF (entry->header), 0);
  write (input_desc, newsyms, nsyms * sizeof (struct nlist));
  free (newsyms);

  /* Now write the string table.  */

  {
    char *strvec = (char *) xmalloc (strtab_size);
    char *p;

    *((long *) strvec) = strtab_size;

    p = strvec + sizeof (long);

    for (i = 0; i < strtab_index; i++)
      {
	int len = strlen (strtab_vector[i]);
	strcpy (p, strtab_vector[i]);
	*(p+len) = 0;
	p += len + 1;
      }

    write (input_desc, strvec, strtab_size);
    free (strvec);
  }

  /* Adjust file to be smaller */

#ifdef HAVE_FTRUNCATE
  if (ftruncate (input_desc, tell (input_desc)) < 0)
    perror_file (entry);
#endif

  /* Write new symbol table size into file header.  */

  entry->header.a_syms = nsyms * sizeof (struct nlist);

  lseek (input_desc, 0, 0);
  write (input_desc, &entry->header, sizeof (struct exec));

  free (strtab_vector);
}

/* Copy into NEWSYMS the symbol entries to be preserved.
   Count them in sym_written_count.  */

/* We record, for each symbol written, its symbol number in the resulting file.
   This is so that the relocation can be updated later.
   Since the symbol names will not be needed again,
   this index goes in the `n_strx' field.
   If a symbol is not written, -1 is stored there.  */

void
write_file_syms (entry, newsyms)
     struct file_entry *entry;
     struct nlist *newsyms;
{
  struct nlist *p = entry->symbols_and_strings;
  struct nlist *end = p + entry->header.a_syms / sizeof (struct nlist);
  char *string_base = (char *) end;   /* address of start of file's string table */
  struct nlist *outp = newsyms;

  for (; p < end; p++)
    {
      int type = p->n_type;
      int write;
  
      if (p->n_type & N_EXT)
	write = 1;
      else if (p->n_un.n_strx && !(p->n_type & (N_STAB | N_EXT)))
	/* ordinary local symbol */
	write = (discard_locals != 2)
		&& !(discard_locals == 1 &&
		     (p->n_un.n_strx + string_base)[0] == 'L');
      else
	/* debugger symbol */
	write = (strip_symbols == 0);

      if (write)
	{
	  if (p->n_un.n_strx)
	    p->n_un.n_strx = assign_string_table_index (p->n_un.n_strx + string_base);

	  *outp++ = *p;

	  p->n_un.n_strx = sym_written_count++;
	}
      else p->n_un.n_strx = -1;
    }
}

/* Read in ENTRY's relocation, alter the symbolnums in it,
   and write it out again.  */

void
modify_relocation (desc, entry)
     int desc;
     struct file_entry *entry;
{
  struct relocation_info *reloc, *p, *end;
  int size = entry->header.a_trsize + entry->header.a_drsize;
  struct nlist *sym_base = (struct nlist *) entry->symbols_and_strings;
  int losing = 0;

  reloc = (struct relocation_info *) xmalloc (size);
  lseek (desc, N_TXTOFF (entry->header) + entry->header.a_text + entry->header.a_data, 0);
  read (desc, reloc, size);

  p = reloc;
  end = (struct relocation_info *) (size + (char *) reloc);
  while (p < end)
    {
      if (p->r_extern)
	{
	  int newnum = (sym_base == 0 ? -1
			:(sym_base + p->r_symbolnum) -> n_un.n_strx);
	  if (newnum < 0)
	    {
	      if (losing == 0)
		error_with_file ("warning: file is now unlinkable", entry);
	      losing = 1;
	    }
	  p->r_symbolnum = newnum;
	}
      p++;
    }

  lseek (desc, N_TXTOFF (entry->header) + entry->header.a_text + entry->header.a_data, 0);
  write (desc, reloc, size);
}

/* Report a fatal error.
   STRING is a printf format string and ARG is one arg for it.  */

fatal (string, arg)
     char *string, *arg;
{
  fprintf (stderr, "strip: ");
  fprintf (stderr, string, arg);
  fprintf (stderr, "\n");
  exit (1);
}

/* Report an error using the message for the last failed system call,
   followed by the string NAME.  */

perror_name (name)
     char *name;
{
  extern int errno, sys_nerr;
  extern char *sys_errlist[];
  char *s;

  if (errno < sys_nerr)
    s = concat ("", sys_errlist[errno], " for %s");
  else
    s = "cannot open %s";
  error (s, name);
}

/* Report an error using the message for the last failed system call,
   followed by the name of file ENTRY.  */

perror_file (entry)
     struct file_entry *entry;
{
  extern int errno, sys_nerr;
  extern char *sys_errlist[];
  char *s;

  if (errno < sys_nerr)
    s = sys_errlist[errno];
  else
    s = "cannot open";
  error_with_file (s, entry);
}

/* Report an error.   STRING is printed, and the filename of ENTRY.  */

error_with_file (string, entry, arg1, arg2)
     char *string;
     struct file_entry *entry;
     int arg1, arg2;
{
  fprintf (stderr, "strip: ");
  print_file_name (entry, stderr);
  fprintf (stderr, ": ");
  fprintf (stderr, string, arg1, arg2);
  fprintf (stderr, "\n");
}

/* Report a nonfatal error.
   STRING is a format for printf, and ARG1 ... ARG3 are args for it.  */

error (string, arg1, arg2, arg3)
     char *string, *arg1, *arg2, *arg3;
{
  fprintf (stderr, string, arg1, arg2, arg3);
  fprintf (stderr, "\n");
}

/* Return a newly-allocated string whose contents 
   concatenate those of S1, S2, S3.  */

char *
concat (s1, s2, s3)
     char *s1, *s2, *s3;
{
  int len1 = strlen (s1), len2 = strlen (s2), len3 = strlen (s3);
  char *result = (char *) xmalloc (len1 + len2 + len3 + 1);

  strcpy (result, s1);
  strcpy (result + len1, s2);
  strcpy (result + len1 + len2, s3);
  *(result + len1 + len2 + len3) = 0;

  return result;
}

/* Like malloc but get fatal error if memory is exhausted.  */

int
xmalloc (size)
     int size;
{
  int result = malloc (size);
  if (!result)
    fatal ("virtual memory exhausted", 0);
  return result;
}


#ifdef USG
rename (from, to)
     char *from, *to;
{
  if (unlink (to) < 0
      || link (from, to) < 0
      || unlink (from) < 0)
    return -1;
  else
    return 0;
}
#endif
