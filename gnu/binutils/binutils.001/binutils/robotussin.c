/* Convert COFF-format object file to BSD format.
   Used for converting the system libraries so GNU ld can link them.
   Copyright (C) 1988 Free Software Foundation, Inc.

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
"Copyright (C) 1988 Free Software Foundation, Inc.", and include
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
** Robotussin - convert COFF format object files to BSD format.
**
** written by Jeff Lewis, donated to the Free Software Foundation.
**
** BUGS:
**	Should do more to verify that the input COFF file meets our
** expectations.
**  On machines where the structure of the COFF data in the file does not
** match the structure of the COFF data declared (when, for example
** sizeof (struct filhdr) != FILHSZ), this program will fail.  (Don't
** ask me why this is ever allowed to come about).  Accessor functions/
** macros that painstakingly extract the data out of the file and stuff
** it in the memory struct should be written to fix this on such machines.
**
** CAVEATS:
**	This program cannot claim correctness, however, it does appear
** to work on my fairly vanilla Sys5r2 machine.  Someone with the time
** and a fine tooth comb (not to mention some documentation on COFF)
** should correct this!
*/

#ifndef COFF_ENCAPSULATE
#define COFF_ENCAPSULATE
#endif

/* Customization for a particular machine.  */
#define INPUT_MAGIC I386MAGIC
#define nounderscore

#include <stdio.h>
#include <varargs.h>
#include <fcntl.h>

#include "a.out.encap.h"
#define N_ABSOLUTE N_ABS		/* N_ABS will be redefined in syms.h */
#undef N_ABS

#include <filehdr.h>
#include <aouthdr.h>
#include <scnhdr.h>
#include <syms.h>
#include <reloc.h>
/* Because of struct alignment on dwords sizeof (struct syment) is different
   than the syments stored in the file.  Therefore, we must kludge:  */
#define sizeof_syment (SYMESZ)
#define sizeof_reloc (RELSZ)
#define sizeof_section (SCNHSZ)
#define sizeof_coff_header (FILHSZ)

extern long lseek ();
extern void exit ();
extern char *memcpy ();
extern int errno;

void error (), sys_error ();
static void reloc_segment ();
char *mem_alloc ();

int fd_in, fd_out;		/* input and output file descriptors */

struct filehdr coff_header;		/* file header from the input file */
struct exec bsd_header;		/* file header for the output file */

struct syment *coff_sym_listp;		/* list of symbols from the input */
int *symbol_map;			/* mapping of input symbol #'s to
				   output symbol numbers */
char *text_and_data;			/* space for text & data section data */
char *relocations;			/* space for output reloc entries */
int verbose_flag;			/* flag for debugging */

struct scnhdr coff_text_header;		/* COFF text section header */
struct scnhdr coff_data_header;		/* COFF data section header */
struct scnhdr coff_bss_header;		/* COFF bss section header */
int text_sect_num;			/* COFF section # for text */
int data_sect_num;			/* COFF section # for data */
int bss_sect_num;			/* COFF section # for bss */

int
main (argc, argv)
     int argc;
     char **argv;
{
  int i, j;
  char *coff_string_table, *bsd_string_table;
  register char *pc, *pc2;
  int string_table_len;
  int symbol_count;
  struct scnhdr section;
  struct nlist name;

  if (argc < 3)
    error ("usage: %s cofffile bsdfile", argv[0]);
  if (argc > 3)
    verbose_flag = 1;

  fd_in = open (argv[1], O_RDONLY);
  if (fd_in < 0)
    sys_error ("can't open %s", argv[1]);

  fd_out = open (argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd_out < 0)
    sys_error ("can't open %s", argv[2]);

  /*
   ** Read in the file header and all section headers searching
   ** for text, data and bss.  We note the section #'s of these
   ** sections for use when examining symbols.
   */

  if (read (fd_in, &coff_header, sizeof_coff_header) != sizeof_coff_header)
    error ("can't read file header");

  if (coff_header.f_magic != INPUT_MAGIC)
    error ("bad magic number in coff file\n");

  lseek (fd_in, sizeof_coff_header + coff_header.f_opthdr, 0);

  for (i = 1; i <= coff_header.f_nscns; ++i)
    {
      if (read (fd_in, &section, sizeof_section) != sizeof_section)
	error ("can't read section header #%d", i);
      if (strcmp (section.s_name, _TEXT) == 0)
	{
	  text_sect_num = i;
	  memcpy (&coff_text_header, &section, sizeof section);
	} 
      else if (strcmp (section.s_name, _DATA) == 0)
	{
	  data_sect_num = i;
	  memcpy (&coff_data_header, &section, sizeof section);
	} 
      else if (strcmp (section.s_name, _BSS) == 0)
	{
	  bss_sect_num = i;
	  memcpy (&coff_bss_header, &section, sizeof section);
	}
    }

  /*
   ** Pass1 thru the symbol table - count usable symbols and map
   ** old symbol #'s into new ones (as used by relocation
   ** info).  We're only interested in keeping the kinds of symbols
   ** we'd expect to find in a BSD library object file: no debug
   ** symbols, file names, section definition symbols, etc.
   ** Section definition symbols are referenced by reloc entries
   ** in the COFF file, so we note their position with a negative
   ** symbol number indicating the section.  -1 is used to flag
   ** symbols we're not interested in, yielding an unexpected error
   ** if we find any reloc entries referencing them.
   */

  coff_sym_listp =
    (struct syment *) mem_alloc (coff_header.f_nsyms * sizeof (struct syment));
  symbol_map = (int *) mem_alloc (coff_header.f_nsyms * sizeof *symbol_map);
  if (lseek (fd_in, coff_header.f_symptr, 0) < 0L)
    sys_error ("can't seek to COFF symbols");
  for (i = 0; i < coff_header.f_nsyms; ++i)
    {
      if (read (fd_in, coff_sym_listp + i, sizeof_syment) != sizeof_syment)
	error ("can't read COFF symbols");
    }
  symbol_count = 0;
  for (i = 0; i < coff_header.f_nsyms; ++i)
    {
      if (coff_sym_listp[i].n_scnum != N_DEBUG
	  && coff_sym_listp[i].n_name[0] != '.')
	{
	  if (verbose_flag)
	    printf ("map %d to %d\n", i, symbol_count);
	  symbol_map[i] = symbol_count++;
	} 
      else
	{
	  if (coff_sym_listp[i].n_sclass == C_STAT)
	    {
	      if (strcmp (coff_sym_listp[i].n_name, _TEXT) == 0)
		symbol_map[i] = -N_TEXT;
	      else if (strcmp (coff_sym_listp[i].n_name, _DATA) == 0)
		symbol_map[i] = -N_DATA;
	      else if (strcmp (coff_sym_listp[i].n_name, _BSS) == 0)
		symbol_map[i] = -N_BSS;
	      else
		symbol_map[i] = -1;
	    } 
	  else
	    {
	      symbol_map[i] = -1;
	    }
	}
      /* skip auxillary entries */
      j = coff_sym_listp[i].n_numaux;
      if (j != 0)
	{
	  if (j < 0)
	    error ("invalid numaux");
	  if (j != 1)
	    fprintf (stderr, "unlikely numaux value\n");
	  while (--j >= 0)
	    ++i;
	}
    }

  /* now we know enough to write the output file header */

  bsd_header.a_magic = OMAGIC;
  bsd_header.a_text = coff_text_header.s_size;
  bsd_header.a_data = coff_data_header.s_size;
  bsd_header.a_bss = coff_bss_header.s_size;
  bsd_header.a_syms = symbol_count * sizeof (struct nlist);
  bsd_header.a_entry = 0;
  bsd_header.a_trsize = coff_text_header.s_nreloc * sizeof (struct relocation_info);
  bsd_header.a_drsize = coff_data_header.s_nreloc * sizeof (struct relocation_info);
  if (write (fd_out, &bsd_header, sizeof bsd_header) != sizeof bsd_header)
    sys_error ("can't write BSD header");

  /*
   ** Read in and save text and data sections - some data in
   ** these sections may need to be altered due to relocations.
   */

  text_and_data = (char *) mem_alloc (coff_text_header.s_size + coff_data_header.s_size);
  if (lseek (fd_in, coff_text_header.s_scnptr, 0) < 0L)
    sys_error ("can't seek to text section");
  if (read (fd_in, text_and_data, coff_text_header.s_size) != coff_text_header.s_size)
    error ("can't read text section");
  if (lseek (fd_in, coff_data_header.s_scnptr, 0) < 0L)
    sys_error ("can't seek to data section");
  if (read (fd_in, text_and_data + coff_text_header.s_size, coff_data_header.s_size) != coff_data_header.s_size)
    error ("can't read data section");

  /*
   ** Convert the relocation entries and do any text or data
   ** modifications necessary.
   */

  relocations = (char *) mem_alloc (bsd_header.a_trsize + bsd_header.a_drsize);
  reloc_segment (&coff_text_header, relocations);
  reloc_segment (&coff_data_header, relocations + bsd_header.a_trsize);

  if (write (fd_out, text_and_data, coff_text_header.s_size + coff_data_header.s_size)
      != coff_text_header.s_size + coff_data_header.s_size)
    sys_error ("can't write text and data sections");
  /* ZZ - are there any alignment considerations?? */
  if ((coff_text_header.s_size & 1) || (coff_data_header.s_size & 1))
    fprintf (stderr, "non-aligned text or data section\n");
  if (write (fd_out, relocations, bsd_header.a_trsize + bsd_header.a_drsize)
      != bsd_header.a_trsize + bsd_header.a_drsize)
    sys_error ("can't write relocation entries");

  /*
   ** Second pass thru the symbol table.  
   ** a COFF symbol entry may contain up to 8 chars of symbol name
   ** in the entry itself - symbol names > 8 go into the string table,
   ** whereas the BSD entry puts all symbol names into the string
   ** table.
   */

  if (lseek (fd_in, coff_header.f_symptr + coff_header.f_nsyms * sizeof_syment, 0) < 0L)
    error ("can't seek to string table");

  i = read (fd_in, &string_table_len, sizeof string_table_len);
  if (i == sizeof string_table_len)
    {
      coff_string_table = mem_alloc (string_table_len);
      string_table_len -= sizeof string_table_len;
      i = read (fd_in, coff_string_table + sizeof string_table_len, string_table_len);
      if (i < 0)
	error ("can't read string table");
      if (i != string_table_len)
	error ("truncated string table - expected %d, got %d",
	       string_table_len, i);
    } 
  else
    {
      string_table_len = 0;
    }
  bsd_string_table = mem_alloc (string_table_len + coff_header.f_nsyms * (SYMNMLEN + 1));
  pc = bsd_string_table + sizeof string_table_len;
  for (i = 0; i < coff_header.f_nsyms; ++i)
    {
      if (coff_sym_listp[i].n_scnum != N_DEBUG
	  && coff_sym_listp[i].n_name[0] != '.')
	{
	  if (coff_sym_listp[i].n_zeroes == 0)
	    {
	      j = pc - bsd_string_table;
#ifndef nounderscore
	      if (coff_sym_listp[i].n_sclass == C_EXT
		  || coff_sym_listp[i].n_sclass == C_STAT)
		*pc++ = '_';
#endif
	      pc2 = coff_string_table + coff_sym_listp[i].n_offset;
	      while (*pc++ = *pc2++)
		/* null */ ;
	      name.n_un.n_strx = j;
	    } 
	  else
	    {
	      pc2 = &coff_sym_listp[i].n_name[0];
	      j = pc - bsd_string_table;
#ifndef nounderscore
	      if (coff_sym_listp[i].n_sclass == C_EXT
		  || coff_sym_listp[i].n_sclass == C_STAT)
		*pc++ = '_';
#endif
	      {
		int x;
		for (x = 0; x < SYMNMLEN; x++)
		  {
		    if (*pc2 == 0)
		      break;
		    *pc++ = *pc2++;
		  }
		*pc++ = 0;
	      }
	      name.n_un.n_strx = j;
	    }
	  switch (coff_sym_listp[i].n_scnum)
	    {
	    case N_ABS:
	      name.n_type = N_ABSOLUTE;
	      break;
	    case N_UNDEF:
	      name.n_type = N_UNDF;
	      break;
	    default:
	      if (coff_sym_listp[i].n_scnum == text_sect_num)
		name.n_type = N_TEXT;
	      else if (coff_sym_listp[i].n_scnum == data_sect_num)
		name.n_type = N_DATA;
	      else if (coff_sym_listp[i].n_scnum == bss_sect_num)
		name.n_type = N_BSS;
	      break;
	    }
	  if (coff_sym_listp[i].n_sclass == C_EXT)
	    name.n_type |= N_EXT;
	  name.n_other = 0;
	  name.n_desc = 0;
	  name.n_value = coff_sym_listp[i].n_value;

	  if (write (fd_out, &name, sizeof name) != sizeof name)
	    sys_error ("can't write symbol");
	}
      /* skip auxillary entries */
      j = coff_sym_listp[i].n_numaux;
      if (j != 0)
	{
	  while (--j >= 0)
	    ++i;
	}
    }
  i = *((int *) bsd_string_table) = pc - bsd_string_table;
  if (write (fd_out, bsd_string_table, i) != i)
    error ("can't write string table");

  close (fd_in);
  close (fd_out);
  exit (0);
}

/*
** Convert the relocation entries and do any text or data
** modifications necessary.
*/

static void
reloc_segment (section_headerp, reloc_infop)
     struct scnhdr *section_headerp;
     struct relocation_info *reloc_infop;
{
  struct reloc coff_reloc;
  int i;

  if (lseek (fd_in, section_headerp->s_relptr, 0) < 0L)
    error ("can't seek to relocation entries");
  for (i = 0; i < section_headerp->s_nreloc; ++i)
    {
      if (read (fd_in, &coff_reloc, sizeof_reloc) != sizeof_reloc)
	error ("can't read relocation entry");
      if (verbose_flag)
	printf ("vaddr = 0x%x, symndx = %d\n", coff_reloc.r_vaddr, coff_reloc.r_symndx);
      /*
       ** The reloc references a symbol declared common, thus the
       ** value of the symbol holds its size (in bytes).  In COFF,
       ** apparently this info is also put into the binary -
       ** BSD doesn't like this, so we subtract it out.
       */
      if (coff_sym_listp[coff_reloc.r_symndx].n_scnum == N_UNDEF)
	{
	  if (coff_sym_listp[coff_reloc.r_symndx].n_value != 0)
	    {
	      if (verbose_flag)
		printf ("adjust common 0x%x (%d)\n",
			coff_sym_listp[coff_reloc.r_symndx].n_value,
			coff_sym_listp[coff_reloc.r_symndx].n_value);
	      switch (coff_reloc.r_type)
		{
		case R_RELBYTE:
		  *((char *) (text_and_data + coff_reloc.r_vaddr))
		    -= coff_sym_listp[coff_reloc.r_symndx].n_value;
		  break;
		case R_RELWORD:
		  *((short *) (text_and_data + coff_reloc.r_vaddr))
		    -= coff_sym_listp[coff_reloc.r_symndx].n_value;
		  break;
		case R_RELLONG:
	        case R_DIR32:	/* these are the only two that really show up */
	        case R_PCRLONG:
		  *((int *) (text_and_data + coff_reloc.r_vaddr))
		    -= coff_sym_listp[coff_reloc.r_symndx].n_value;
		  break;
	        default:
		  error ("unknown relocation type 0%o", coff_reloc.r_type);
		}
	    }
	}
      /*
       ** >= 0 means its an extern - value is the output symbol #.
       ** < 0 means its an intern - value is N_TEXT, N_DATA or N_BSS.
       */
      if (symbol_map[coff_reloc.r_symndx] >= 0)
	{
	  reloc_infop->r_symbolnum = symbol_map[coff_reloc.r_symndx];
	  reloc_infop->r_extern = 1;
	} 
      else
	{
	  if (symbol_map[coff_reloc.r_symndx] == -1)
	    error ("Oops! possible bug - reloc reference to ignored symbol");
	  reloc_infop->r_symbolnum = -symbol_map[coff_reloc.r_symndx];
	  reloc_infop->r_extern = 0;
	}
      /*
       ** COFF address includes the section address - BSD doesn't, so
       ** subtract it out.
       */
      reloc_infop->r_address = coff_reloc.r_vaddr - section_headerp->s_vaddr;
      switch (coff_reloc.r_type)
	{
	case R_PCRLONG:
	  reloc_infop->r_pcrel = 1;
	  reloc_infop->r_length = 2; /* 4 bytes */
	  break;
	case R_DIR32:
	  reloc_infop->r_pcrel = 0;
	  reloc_infop->r_length = 2;
	  break;
	default:
	  error ("can't handle coff reloction type 0%o", coff_reloc.r_type);
	}

      if (verbose_flag)
	printf ("reloc: addr = 0x%x, synum = %d\n",
		reloc_infop->r_address, reloc_infop->r_symbolnum);
      reloc_infop->r_pad = 0;
      ++reloc_infop;
    }
}

void
error (format, va_alist)
     char *format;
     va_dcl
{
  va_list args;

  va_start (args);
  fprintf (stderr, "robotussin: ");
  vfprintf (stderr, format, args);
  putc ('\n', stderr);
  va_end (args);
  exit (1);
}

extern char *sys_errlist[];
extern int errno;

void
sys_error (format, va_alist)
     char *format;
     va_dcl
{
  va_list args;

  va_start (args);
  fprintf (stderr, "robotussin: ");
  vfprintf (stderr, format, args);
  fprintf (stderr, ": %s\n", sys_errlist[errno]);
  va_end (args);
  exit (1);
}

extern char *malloc ();

char *
mem_alloc (size)
     int size;
{
  char *pc;

  if ((pc = malloc (size)) == NULL)
    error ("memory exhausted!");
  return pc;
}

/* end */
