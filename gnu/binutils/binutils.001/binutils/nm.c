/* Describe symbol table of a rel file.
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

/*
 * Wed Jul 27 14:33:16 EDT 1988--RDS
 * Added entries for set vectors, all set elements, and the
 * equivalence type symbol.  The mappings are:
 *
 * 	SETV		v
 *
 *	SETA		l
 *	SETT		x
 *	SETD		z
 *	SETB		s
 *
 *	INDR		i
 */
/*
 * Tue Aug  2 11:57:38 EDT 1988--RDS
 * Added entry for the new N_DSLNE symbol in stab.h.
 */

#include <stdio.h>
#include <ar.h>
#include <sys/types.h>
#include <sys/file.h>

#ifdef COFF_ENCAPSULATE
#include "a.out.encap.h"
#else
/* On native BSD systems, use the system's own a.out.h.  */
#include <a.out.h>
#endif

/* Always use the GNU version of debugging symbol type codes, if possible.  */
#include "stab.h"

/* Struct or union for header of object file.  */

#define HEADER_TYPE struct exec

#ifdef USG
#include <string.h>
#include <sys/fcntl.h>
#else
#include <strings.h>
#endif

/* Special global symbol types understood by GNU LD.  */

/* The following type indicates the definition of a symbol as being
   an indirect reference to another symbol.  The other symbol
   appears as an undefined reference, immediately following this symbol.

   Indirection is asymmetrical.  The other symbol's value will be used
   to satisfy requests for the indirect symbol, but not vice versa.
   If the other symbol does not have a definition, libraries will
   be searched to find a definition.  */
#ifndef N_INDR
#define N_INDR 0xa
#endif

/* The following symbols refer to set elements.
   All the N_SET[ATDB] symbols with the same name form one set.
   Space is allocated for the set in the text section, and each set
   element's value is stored into one word of the space.
   The first word of the space is the length of the set (number of elements).

   The address of the set is made into an N_SETV symbol
   whose name is the same as the name of the set.
   This symbol acts like a N_TEXT global symbol
   in that it can satisfy undefined external references.  */

#ifndef N_SETA
#define	N_SETA	0x14		/* Absolute set element symbol */
#endif				/* This is input to LD, in a .o file.  */

#ifndef N_SETT
#define	N_SETT	0x16		/* Text set element symbol */
#endif				/* This is input to LD, in a .o file.  */

#ifndef N_SETD
#define	N_SETD	0x18		/* Data set element symbol */
#endif				/* This is input to LD, in a .o file.  */

#ifndef N_SETB
#define	N_SETB	0x1A		/* Bss set element symbol */
#endif				/* This is input to LD, in a .o file.  */

/* Macros dealing with the set element symbols defined in a.out.h */
#define	SET_ELEMENT_P(x)	((x)>=N_SETA&&(x)<=(N_SETB|N_EXT))
#define TYPE_OF_SET_ELEMENT(x)	((x)-N_SETA+N_ABS)

#ifndef N_SETV
#define N_SETV	0x1C		/* Pointer to set vector in text area.  */
#endif				/* This is output from LD.  */

#ifndef __GNU_STAB__

/* Line number for the data section.  This is to be used to describe
   the source location of a variable declaration.  */
#ifndef N_DSLINE
#define N_DSLINE (N_SLINE+N_DATA-N_TEXT)
#endif

/* Line number for the bss section.  This is to be used to describe
   the source location of a variable declaration.  */
#ifndef N_BSLINE
#define N_BSLINE (N_SLINE+N_BSS-N_TEXT)
#endif

#endif /* not __GNU_STAB__ */

/* Name of this program.  */

char *program_name;

/* Number of input files specified.  */

int number_of_files;

/* Current file's name.  */

char *input_name;

/* Current member's name, or 0 if processing a non-library file.  */

char *input_member;

/* Offset within archive of the current member,
   if we are processing an archive.  */

int member_offset;

/* Command options.  */

int external_only;	/* nonzero means print external symbols only.  */
int sort_numerically;   /* sort in numerical order rather than alphabetic  */
int reverse_sort;	/* sort in downward (alphabetic or numeric) order.  */
int no_sort;		/* don't sort; print symbols in order in the file.  */
int undefined_only;	/* print undefined symbols only.  */
int file_on_each_line;	/* print file name on each line.  */
int debugger_syms;	/* print the debugger-only symbols.  */
int print_symdefs;	/* describe the __.SYMDEF data in any archive file specified.  */

/* The __.SYMDEF member of an archive has the following format:
   1) A longword saying the size of the symdef data that follows
   2) Zero or more  struct symdef  filling that many bytes
   3) A longword saying how many bytes of strings follow
   4) That many bytes of string data.
*/

struct symdef
  {
    long stringoffset;	/* Offset of this symbol's name in the string data */
    long offset;	/* Offset in the archive of the header-data for the member that
			   defines this symbol.  */
  };

/* Create a table of debugging stab-codes and corresponding names.  */
#ifdef __GNU_STAB__
#define __define_stab(NAME, CODE, STRING) {NAME, STRING},
struct {enum __stab_debug_code code; char *string;} stab_names[]
  = {
#include "stab.def"
    };
#undef __define_stab
#endif

void decode_switch ();
int decode_arg ();
void do_one_file (), do_one_rel_file (), do_symdef_member ();
char *concat ();

main (argc, argv)
     char **argv;
     int argc;
{
  int i;
  int c;
  extern int optind;

  program_name = argv[0];

  number_of_files = 0;
  external_only = 0;
  sort_numerically = 0;
  reverse_sort = 0;
  no_sort = 0;
  undefined_only = 0;
  file_on_each_line = 0;
  debugger_syms = 0;
  print_symdefs = 0;

  while ((c = getopt (argc, argv, "agnoprsu")) != EOF)
    switch (c)
      {
      case 'a':
	debugger_syms = 1;
	break;

      case 'g':
	external_only = 1;
	break;

      case 'n':
	sort_numerically = 1;
	break;

      case 'o':
	file_on_each_line = 1;
	break;

      case 'p':
	no_sort = 1;
	break;

      case 'r':
	reverse_sort = 1;
	break;

      case 's':
	print_symdefs = 1;
	break;

      case 'u':
	undefined_only = 1;
	break;
      }

  number_of_files = argc - optind;

  /* Now scan again and print the files.  */

  if (argc == optind)
    do_one_file ("a.out");
  else
    for (i = optind; i < argc; i++)
      do_one_file (argv[i]);
}

/* Print the filename of the current file on 'outfile' (a stdio stream).  */

print_file_name (outfile)
     FILE *outfile;
{
  fprintf (outfile, "%s", input_name);
  if (input_member)
    fprintf (outfile, "(%s)", input_member);
}

/* process one input file */
void scan_library ();

void
do_one_file (name)
     char *name;
{
  int len, magicnum, desc;

  desc = open (name, O_RDONLY, 0);

  if (desc < 0)
    {
      perror_name (name);
      return;
    }

  input_name = name;
  input_member = 0;

#ifdef HEADER_SEEK_FD
  /* Skip the headers that encapsulate our data in some other format
     such as COFF.  */
  HEADER_SEEK_FD (desc);
#endif
  len = read (desc, &magicnum, sizeof magicnum);
  if (len != sizeof magicnum)
    error_with_file ("failure reading header of ");
  else if (!N_BADMAG (*((struct exec *)&magicnum)))
    do_one_rel_file (desc, 0);
  else
    {
      char armag[SARMAG];
      int nchars;

      lseek (desc, 0, 0);
      nchars = read (desc, armag, SARMAG);

      if (SARMAG != nchars || strncmp (armag, ARMAG, SARMAG))
	error_with_file ("malformed input file (not rel or archive) ");
      else
        scan_library (desc);
    }

  close (desc);
}

/* Read in the archive data about one member.
   SUBFILE_OFFSET is the address within the archive of the start of that data.
   The value returned is the length of the member's contents, which does
   not include the archive data about the member.
   A pointer to the member's name is stored into *MEMBER_NAME_PTR.

   If there are no more valid members, zero is returned.  */

int
decode_library_subfile (desc, subfile_offset, member_name_ptr)
     int desc;
     int subfile_offset;
     char **member_name_ptr;
{
  int bytes_read;
  int namelen;
  int member_length;
  char *name;
  struct ar_hdr hdr1;

  lseek (desc, subfile_offset, 0);

  bytes_read = read (desc, &hdr1, sizeof hdr1);
  if (!bytes_read)
    ;		/* end of archive */

  else if (sizeof hdr1 != bytes_read)
    error_with_file ("malformed library archive ");

  else if (sscanf (hdr1.ar_size, "%d", &member_length) != 1)
    error_with_file ("malformatted header of archive member in ");

  else
    {
      for (namelen = 0; ; namelen++)
	if (hdr1.ar_name[namelen] == 0 || hdr1.ar_name[namelen] == ' '
	    /* Some systems use a slash?  Strange.  */
	    || hdr1.ar_name[namelen] == '/')
	  break;

      name = (char *) xmalloc (namelen+1);
      strncpy (name, hdr1.ar_name, namelen);
      name[namelen] = 0;

      *member_name_ptr = name;

      return member_length;
    }
  return 0;   /* tell caller to exit loop */
}

/* Scan a library and describe each member.  */

void
scan_library (desc)
     int desc;
{
  int this_subfile_offset = SARMAG;
  int member_length;

  if (!file_on_each_line)
    printf ("\n%s:\n", input_name);
  
  while (1)
    {
      member_length
	= decode_library_subfile (desc, this_subfile_offset, &input_member);
      if (member_length == 0)
	break;

      /* Describe every member except the ranlib data if any.  */

      if (strcmp (input_member, "__.SYMDEF"))
	do_one_rel_file (desc, this_subfile_offset + sizeof (struct ar_hdr));
      else if (print_symdefs)
	do_symdef_member (desc, this_subfile_offset + sizeof (struct ar_hdr), member_length);

      this_subfile_offset += ((member_length + sizeof (struct ar_hdr)) + 1) & -2;
    }
}

void print_symbols (), print_one_symbol ();
void read_header ();
int alphacompare (), valuecompare ();
int filter_symbols ();

void
do_one_rel_file (desc, offset)
     int desc;
     int offset;
{
  HEADER_TYPE header;   /* file header read in here */
  int string_size;
  struct nlist *symbols_and_strings;
  int symcount;
  int symlen;
  int totalsize;
  char *strings;

  header.a_magic = 0;

  read_header (desc, &header, offset);

  if (N_BADMAG (header))
    {
      error_with_file ("bad magic number in ");
      return;
    }

  /* Read the string-table-length out of the file.  */

  lseek (desc, N_STROFF (header) + offset, 0);
  if (sizeof string_size != read (desc, &string_size, sizeof string_size))
    {
      error_with_file ("bad string table in ");
      return;
    }

  /* Compute size of symbols in a way that robotussin can fool.
     (With BSD format, this is just header.a_syms.)  */
  symlen = N_STROFF (header) - N_SYMOFF (header);
  /* Number of symbol entries in the file.  */
  symcount = symlen / sizeof (struct nlist);

  totalsize = string_size + symlen;

  /* Allocate space for symbol entries and string table.  */
  symbols_and_strings = (struct nlist *) xmalloc (totalsize);
  strings = (char *) symbols_and_strings + symlen;

  /* Read them both in all at once.  */
  lseek (desc, N_SYMOFF (header) + offset, 0);
  if (totalsize != read (desc, symbols_and_strings, totalsize))
    {
      error_with_file ("premature end of file in symbols/strings of ");
      return;
    }

  /* Identify this file, if desired.  */

  if (!file_on_each_line && (number_of_files > 1 || input_member))
    printf ("\n%s:\n", input_member ? input_member : input_name);

  /* Discard the symbols we don't want to print; compact the rest down.  */

  symcount = filter_symbols (symbols_and_strings, symcount, strings);
    
  /* Modify each symbol entry to point directly at the symbol name.
     This is so the sort routine does not need to be passed
     the value of `strings' separately.  */

  {
    struct nlist *p = symbols_and_strings;
    struct nlist *end = symbols_and_strings + symcount;

    for (; p < end; p++)
      if (p->n_un.n_strx)
        p->n_un.n_name = strings + p->n_un.n_strx;
  }

  /* Sort the symbols if desired.  */

  if (!no_sort)
    qsort (symbols_and_strings, symcount, sizeof (struct nlist),
	   sort_numerically ? valuecompare : alphacompare);

  /* Print the symbols in the order they are now in.  */

  print_symbols (symbols_and_strings, symcount);

  free (symbols_and_strings);
}


/* Read a file's header.  */

void
read_header (desc, loc, offset)
     int desc;
     HEADER_TYPE *loc;
     int offset;
{
  int len;

  lseek (desc, offset, 0);
#ifdef HEADER_SEEK_FD
  /* Skip the headers that encapsulate our data in some other format
     such as COFF.  */
  HEADER_SEEK_FD (desc);
#endif
  len = read (desc, loc, sizeof (struct exec));
  if (len != sizeof (struct exec))
    error_with_file ("failure reading header of ");
}

/* Choose which symbol entries to print;
   compact them downward to get rid of the rest.
   Return the number of symbols to be printed.  */

int
filter_symbols (syms, symcount, strings)
     struct nlist *syms;
     int symcount;
     char *strings;
{
  struct nlist *from = syms, *to = syms;
  struct nlist *end = syms + symcount;

  while (from < end)
    {
      int keep = 0;

      /* undefined sym or common sym */
      if (from->n_type == N_EXT) keep = !undefined_only || !from->n_value;
      /* global defined sym */
      else if (from->n_type & N_EXT) keep = !undefined_only;
      /* debugger sym: normally don't print */
      else if (from->n_type & ~(N_TYPE | N_EXT)) keep = debugger_syms;
      /* local sym */
      else keep = !external_only && !undefined_only;

      if (keep)
	*to++ = *from;
      from++;
    }

  return to - syms;
}

/* Comparison functions for sorting symbols.  */

int
alphacompare (sym1, sym2)
     struct nlist *sym1, *sym2;
{
  if (reverse_sort)
    {
      if (!sym2->n_un.n_name)
	{
	  if (sym1->n_un.n_name) return -1;
	  else return 0;
	}
      if (!sym1->n_un.n_name) return 1;
      return strcmp (sym2->n_un.n_name, sym1->n_un.n_name);
    }
  else
    {
      if (!sym1->n_un.n_name)
	{
	  if (sym2->n_un.n_name) return -1;
	  else return 0;
	}
      if (!sym2->n_un.n_name) return 1;
      return strcmp (sym1->n_un.n_name, sym2->n_un.n_name);
    }
}


int
valuecompare (sym1, sym2)
     struct nlist *sym1, *sym2;
{
  if (reverse_sort)
    return sym2->n_value - sym1->n_value;
  else
    return sym1->n_value - sym2->n_value;
}

void
print_symbols (syms, symcount)
     struct nlist *syms;
     int symcount;
{
  int i;

  for (i = 0; i < symcount; i++)
    print_one_symbol (&syms[i]);
}

void
print_one_symbol (sym)
     struct nlist *sym;
{
  if (file_on_each_line)
    {
      print_file_name (stdout);
      printf (":");
    }

  if (undefined_only)
    {
      if (sym->n_type == N_EXT && !sym->n_value)
        printf ("%s\n", sym->n_un.n_name);
      return;
    }

  if (sym->n_type & ~N_EXT || sym->n_value)
    printf ("%08x ", sym->n_value);
  else printf ("         ");

  switch (sym->n_type)
    {
      case N_EXT:
        if (sym->n_value) printf ("C");
        else printf ("U");
	break;

      case 0:
        if (sym->n_value) printf ("c");
        else printf ("u");
	break;

      case N_ABS | N_EXT:
	printf ("A");
	break;

      case N_ABS:
	printf ("a");
	break;

      case N_TEXT | N_EXT:
	printf ("T");
	break;

      case N_TEXT:
	printf ("t");
	break;

      case N_DATA | N_EXT:
	printf ("D");
	break;

      case N_DATA:
	printf ("d");
	break;

      case N_BSS | N_EXT:
	printf ("B");
	break;

      case N_BSS:
	printf ("b");
	break;

      case N_SETV | N_EXT:
	printf ("V");
	break;

      case N_SETV:
	printf ("v");
	break;

      case N_SETA | N_EXT:
	printf ("L");
	break;
	
      case N_SETA:
	printf ("l");
	break;
	
      case N_SETT | N_EXT:
	printf ("X");
	break;
	
      case N_SETT:
	printf ("x");
	break;
	
      case N_SETD | N_EXT:
	printf ("Z");
	break;
	
      case N_SETD:
	printf ("z");
	break;
	
      case N_SETB | N_EXT:
	printf ("S");
	break;
	
      case N_SETB:
	printf ("s");
	break;
	
      case N_INDR | N_EXT:
	printf ("I");
	break;
	
      case N_INDR:
	printf ("i");
	break;
	
      default:
	{
	  char *s;
	  int i;
#ifdef __GNU_STAB__
	  s = "";
	  for (i = sizeof (stab_names) / sizeof (stab_names[0]) - 1;
	       i >= 0; i--)
	    {
	      if (stab_names[i].code
		  == (enum __stab_debug_code) sym->n_type)
		{
		  s = stab_names[i].string;
		  break;
		}
	    }
#else /* not __GNU_STAB__ */
	  switch (sym->n_type)
	    {
	    case N_GSYM:
	      s = "GSYM";
	      break;
	    case N_FNAME:
	      s = "FNAME";
	      break;
	    case N_FUN:
	      s = "FUN";
	      break;
	    case N_STSYM:
	      s = "STSYM";
	      break;
	    case N_LCSYM:
	      s = "LCSYM";
	      break;
	    case N_RSYM:
	      s = "RSYM";
	      break;
	    case N_SLINE:
	      s = "SLINE";
	      break;
	    case N_DSLINE:
	      s = "DSLINE";
	      break;
	    case N_BSLINE:
	      s = "BSLINE";
	      break;
	    case N_SSYM:
	      s = "SSYM";
	      break;
	    case N_SO:
	      s = "SO";
	      break;
	    case N_LSYM:
	      s = "LSYM";
	      break;
	    case N_SOL:
	      s = "SOL";
	      break;
	    case N_PSYM:
	      s = "PSYM";
	      break;
	    case N_ENTRY:
	      s = "ENTRY";
	      break;
	    case N_LBRAC:
	      s = "LBRAC";
	      break;
	    case N_RBRAC:
	      s = "RBRAC";
	      break;
	    case N_BCOMM:
	      s = "BCOMM";
	      break;
	    case N_ECOMM:
	      s = "ECOMM";
	      break;
	    case N_ECOML:
	      s = "ECOML";
	      break;
	    case N_LENG:
	      s = "LENG";
	      break;
	    default:
	      s = "";
	    }
#endif /* not __GNU_STAB__ */
	  printf ("- %02x %04x %5s", sym->n_other, sym->n_desc, s);
	}
    }

  if (sym->n_un.n_name)
    printf (" %s\n", sym->n_un.n_name);
  else
    printf (" \n");
}

void
do_symdef_member (desc, offset, member_length)
     int desc;
     int offset;
     int member_length;
{
  int symdef_size;
  int nsymdefs;
  struct symdef *symdefs;
  int stringsize;
  char *strings;
  int i;
  char *member_name;
  int member_offset;

  /* read the string-table-length out of the file */

  lseek (desc, offset, 0);
  if (sizeof symdef_size != read (desc, &symdef_size, sizeof symdef_size))
    {
      error_with_file ("premature eof in ");
      return;
    }

  if (symdef_size < 0)
    {
      error_with_file ("invalid size value in ");
      return;
    }

  nsymdefs = symdef_size / sizeof (struct symdef);
  symdefs = (struct symdef *) alloca (symdef_size);
  if (symdef_size != read (desc, symdefs, symdef_size))
    {
      error_with_file ("premature eof in ");
      return;
    }

  if (stringsize < 0)
    {
      error_with_file ("invalid size value in ");
      return;
    }

  if (sizeof stringsize != read (desc, &stringsize, sizeof stringsize))
    {
      error_with_file ("premature eof in ");
      return;
    }

  strings = (char *) alloca (stringsize);
  if (stringsize != read (desc, strings, stringsize))
    {
      error_with_file ("premature eof in ");
      return;
    }

  if (stringsize + symdef_size + sizeof stringsize + sizeof symdef_size != member_length)
    {
      error_with_file ("size of data isn't what the data calls for in ");
      return;
    }

  if (!file_on_each_line && (number_of_files > 1 || input_member))
    printf ("\n%s:\n", input_member ? input_member : input_name);
    
  member_offset = -1;
  for (i = 0; i < nsymdefs; i++)
    {
      if (symdefs[i].stringoffset < 0 || symdefs[i].stringoffset >= stringsize)
	{
	  error_with_file ("invalid entry in ");
	  return;
	}
      if (member_offset != symdefs[i].offset)
	{
	  member_offset = symdefs[i].offset;
	  decode_library_subfile (desc, member_offset, &member_name);
	}
      if (file_on_each_line)
	{
	  print_file_name (stdout);
	  printf (":");
	}
      printf ("%s in %s\n", symdefs[i].stringoffset + strings, member_name);
    }
}

/* Report a fatal error.
   STRING is a printf format string and ARG is one arg for it.  */

fatal (string, arg)
     char *string, *arg;
{
  fprintf (stderr, "%s: ", program_name);
  fprintf (stderr, string, arg);
  fprintf (stderr, "\n");
  exit (1);
}

/* Report a nonfatal error.
   STRING is a printf format string and ARG is one arg for it.  */

error (string, arg)
     char *string, *arg;
{
  fprintf (stderr, "%s: ", program_name);
  fprintf (stderr, string, arg);
  fprintf (stderr, "\n");
}

/* Report a nonfatal error.
   STRING is printed, followed by the current file name.  */

error_with_file (string)
     char *string;
{
  fprintf (stderr, "%s: ", program_name);
  fprintf (stderr, string);
  print_file_name (stderr);
  fprintf (stderr, "\n");
}

/* Report a fatal error using the message for the last failed system call,
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

/* Return a newly-allocated string
   whose contents concatenate those of S1, S2, S3.  */

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
