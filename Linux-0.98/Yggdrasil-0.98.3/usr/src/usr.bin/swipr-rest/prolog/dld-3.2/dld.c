/* dld -- dynamic link/unlink editor for C
   Copyright (C) 1990 by W. Wilson Ho.

   The author can be reached electronically by how@ivy.ucdavis.edu or
   through physical mail at:

   W. Wilson Ho
   Division of Computer Science
   University of California at Davis
   Davis, CA 95616
 */

/* MODIFIED

   DATE:
	Sun Sep 16 16:46:52 MET DST 1990

   BUGFIX:
	While allocating common symbols, clear their values to all ZERO.
	For this I introduced xcalloc();

   BUGFIX:
	Changed subfile_wanted_p() to consider files only holding common
	symbols that are undefined sofar to be considered for loading.
	Change indicated with (JW).

   ADDITION:
        int dld_list_undefined()

	prints undefined symbols to stderr and returns the number of
	undefined symbols found.
 
   CHANGE:
	If a multiple definition is found its name is printed on
	stderr.

   ADDITION:
   	char *dld_find_function(long address, int *percentage)

	Debugging tool.  Returns the name for the function which has
	address in it and the percentage from the start of the function.
	If the function cannot be found it returns the string "???" and
	sets percentage to 0.  As this requires us to keep the symbol
	table around, dld_init() has been given a second argument.  If
	this is not equal to 0, the symbol tables are kept in core.

	This is an interface to the internal debugger of PCE.  It is a
	better idea to write a function
	
	dld_dump_symbols(char *filename)

	that dumps a complete symbol table to filename.  Using the attaching
	mechanism of dbx/gdb we can then normally debug the dynamic loaded
	code.

   Jan Wielemaker
   Social Science Informatics (SWI)
   University of Amsterdam
   Herengracht 196
   1016 BS  Amsterdam
   The Netherlands

   jan@swi.psy.uva.nl
*/

/* This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 1, or (at your option) any
   later version.

   This program borrows and modifies a number of functions and data
   structures from the implementation of the GNU `ld' link editor.  The
   original copyleft notice from the GNU `ld' is also included. */

/* Linker `ld' for GNU
   Copyright (C) 1988 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <a.out.h>
#include <ar.h>
#include <stdio.h>
#include <sys/types.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/param.h>
#include <setjmp.h>
#include "dld.h"

/* 
 * Alloca include.
 */
/* If compiled with GNU C, use the built-in alloca */
#ifdef __GNUC__
#define alloca __builtin_alloca
#endif

#if defined(sun) && defined(sparc) && !defined(__GNUC__)
#include <alloca.h>
#endif
    
/* this is commonly used in remove single linked-list elements. */
#define del_link_list_elt(head, prev, current, next) { \
    if (prev == 0) { \
	head = current->next; \
	free (current); \
	current = head; \
    } else { \
	prev->next = current->next; \
	free (current); \
	current = prev->next; \
    } }

/* System dependencies */

/* Ordinary 4.3 bsd lacks these macros in a.out.h */
    
#ifndef N_TXTADDR
#ifdef vax
#define N_TXTADDR(x) 0
#endif
#endif

#ifndef N_DATADDR
#ifdef vax
#define N_DATADDR(x) \
    (((x).a_magic == OMAGIC) ? (N_TXTADDR(x)+(x).a_text) \
     : (page_size + ((N_TXTADDR(x)+(x).a_text-1) & ~(page_size-1))))
#endif
#endif

#ifndef N_BSSADDR
#ifdef vax
#define N_BSSADDR(x) (N_DATADDR(x)+(x).a_data)
#endif
#endif

/*
 * Ok.  Following are the relocation information macros.  If your
 * system cannot use the default set (below), you must define all of these:

 *   relocation_info: This must be typedef'd (or #define'd) to the type
 * of structure that is stored in the relocation info section of your
 * a.out files.  Often this is defined in the a.out.h for your system.
 *
 *   RELOC_ADDRESS (rval): Offset into the current section of the
 * <whatever> to be relocated.  *Must be an lvalue*.
 *
 *   RELOC_EXTERN_P (rval):  Is this relocation entry based on an
 * external symbol (1), or was it fully resolved upon entering the
 * loader (0) in which case some combination of the value in memory
 * (if RELOC_MEMORY_ADD_P) and the extra (if RELOC_ADD_EXTRA) contains
 * what the value of the relocation actually was.  *Must be an lvalue*.
 *
 *   RELOC_SYMBOL (rval): For an external relocation, this is the
 * index of its symbol in the symbol table.  *Must be an lvalue*.
 *
 *   RELOC_PCREL_P (rval): True if the relocation value described is
 * pc relative.
 *
 *   RELOC_ADD_EXTRA (rval): (Optional) This macro, if defined, gives
 * an extra value to be added to the relocation value based on the
 * individual relocation entry.  *Must be an lvalue if defined*.
 *
 *   RELOC_VALUE_RIGHTSHIFT (rval): Number of bits right to shift the
 * final relocation value before putting it where it belongs.
 *
 *   RELOC_TARGET_SIZE (rval): log to the base 2 of the number of
 * bytes of size this relocation entry describes; 1 byte == 0; 2 bytes
 * == 1; 4 bytes == 2, and etc.  This is somewhat redundant (we could
 * do everything in terms of the bit operators below), but having this
 * macro could end up producing better code on machines without fancy
 * bit twiddling.  Also, it's easier to understand/code big/little
 * endian distinctions with this macro.
 *
 *   RELOC_TARGET_BITSIZE (rval): How many bits are to be replaced
 * with the bits of the relocation value.  It may be assumed by the
 * code that the relocation value will fit into this many bits.  This
 * may be larger than RELOC_TARGET_SIZE if such be useful.
 *
 *
 *		Things I haven't implemented
 *		----------------------------
 *
 *    Values for RELOC_TARGET_SIZE other than 0, 1, or 2.
 *
 *    Pc relative relocation for External references.
 *
 *
 */

#if defined(sun) && defined(sparc)
/* Sparc (Sun 4) macros */
#undef relocation_info
#define relocation_info	                reloc_info_sparc
#define RELOC_ADDRESS(r)		((r)->r_address)                 
#define RELOC_EXTERN_P(r)               ((r)->r_extern)      
#define RELOC_SYMBOL(r)                 ((r)->r_index)   
#define RELOC_ADD_EXTRA(r)              ((r)->r_addend)       
#define RELOC_PCREL_P(r)             \
        ((r)->r_type >= RELOC_DISP8 && (r)->r_type <= RELOC_WDISP22)
#define RELOC_VALUE_RIGHTSHIFT(r)       (reloc_target_rightshift[(r)->r_type])
#define RELOC_TARGET_SIZE(r)            (reloc_target_size[(r)->r_type])
#define RELOC_TARGET_BITSIZE(r)         (reloc_target_bitsize[(r)->r_type])

/* Note that these are very dependent on the order of the enums in
   enum reloc_type (in a.out.h); if they change the following must be
   changed */
/* Also note that the last few may be incorrect; I have no information */
static int reloc_target_rightshift[] = {
  0, 0, 0, 0, 0, 0, 2, 2, 10, 0, 0, 0, 0, 0, 0,
};
static int reloc_target_size[] = {
  0, 1, 2, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
};
static int reloc_target_bitsize[] = {
  8, 16, 32, 8, 16, 32, 30, 22, 22, 22, 13, 10, 32, 32, 16,
};

#endif

/* Default macros */
#ifndef RELOC_ADDRESS
#define RELOC_ADDRESS(r)		((r)->r_address)
#define RELOC_EXTERN_P(r)		((r)->r_extern)
#define RELOC_SYMBOL(r)			((r)->r_symbolnum)
#undef RELOC_ADD_EXTRA
#define RELOC_PCREL_P(r)		((r)->r_pcrel)
#define RELOC_TARGET_SIZE(r)		((r)->r_length)
#endif

/* Size of a page; obtained from the operating system. */

static int page_size;

/* Each input file, and each library member ("subfile") being loaded,
   has a `file_entry' structure for it.

   For files specified by command args, these are contained in the vector
   which `file_table' points to.

   For library members, they are dynamically allocated,
   and chained through the `chain' field.
   The chain is found in the `subfiles' field of the `file_entry'.
   The `file_entry' objects for the members have `superfile' fields pointing
   to the one for the library.  */

struct file_entry {
    /* Name of this file.  */
    char *filename;
    /* Name to use for the symbol giving address of text start */
    /* Usually the same as filename, but for a file spec'd with -l
       this is the -l switch itself rather than the filename.  */
    char *local_sym_name;

    /* For library member, points to next entry for next member.
       For object or library *file*, points to previously loaded entry */
    struct file_entry *chain;

    /* number of undefined symbols referenced by this module */
    int undefined_symbol_count;
  
    /* chain of file_entry that defines symbols this file references */
    struct file_chain *refs;

    /* chain of file_entry that references symbols defined in this file */
    struct file_chain *refs_by;
  
    /* reference count -- number of entries referenceing myself */
    int ref_count;
    
    /* Describe the layout of the contents of the file */

    /* The file's a.out header.  */
    struct exec header;

    /* Describe data from the file loaded into core */

    /* Symbol table of the file.  */
    struct nlist *symbols;
    /* Size in bytes of string table.  */
    int string_size;
    /* Pointer to the string table.
       The string table is not kept in core all the time,
       but when it is in core, its address is here.  */
    char *strings;

    /* Relocation information of the file. */

    /* Start of this file's text relocation information. */
    struct dld_reloc_info *text_reloc;
    /* Start of this file's data relocation information. */
    struct dld_reloc_info *data_reloc;
    
    /* Relation of this file's segments to the output buffer */

    /* Start of this file's text seg in the output file core image.  */
    int text_start_address;
    /* Start of this file's data seg in the output file core image.  */
    int data_start_address;
    /* Start of this file's bss seg in the output file core image.  */
    int bss_start_address;

    /* For library members only */

    /* For a library, points to chain of entries for the library members.  */
    struct file_entry *subfiles;
    /* For a library member, offset of the member within the archive.
       Zero for files that are not library members.  */
    int starting_offset;
    /* Size of contents of this file, if library member.  */
    int total_size;
    /* For library member, points to the library's own entry.  */
    struct file_entry *superfile;

    /* 1 if file is a library. */
    char library_flag;

    /* 1 if file's header has been read into this structure.  */
    char header_read_flag;

    /* 1 if this module has all external references resolved */
    char all_symbols_resolved_flag;
  
    /* 1 if functions in this module can be safely executed. */
    char executable_flag;

    /* 1 if this module has already been (soft) unlinked. */
    char already_unlink;
    /* 1 means search a set of directories for this file.  */
    /* char search_dirs_flag; */
};


/* format of file_entry chain */
struct file_chain {
    struct file_chain *next;
    struct file_entry *entry;
};

/* Symbol table */

/* Global symbol data is recorded in these structures,
   one for each global symbol.
   They are found via hashing in 'symtab', which points to a vector of buckets.
   Each bucket is a chain of these structures through the link field.  */

typedef
  struct glosym
    {
      /* Pointer to next symbol in this symbol's hash bucket.  */
      struct glosym *link;
      /* Name of this symbol.  */
      char *name;
      /* Value of this symbol as a global symbol.  */
      long value;
      /* Points to the file_entry that defines this symbol */
      struct file_entry *defined_by;
      /* chain of file_entry that contains reference to this symbol */
      struct file_chain *referenced_by;
      /* Nonzero means a definition of this global symbol is known to exist.
	 Library members should not be loaded on its account.  */
      char defined;
      /* Nonzero means a reference to this global symbol has been seen
	 in a file that is surely being loaded. */
      char referenced;
    }
  symbol;

/* Number of buckets in symbol hash table */
#define	TABSIZE	1009

/* The symbol hash table: a vector of TABSIZE pointers to struct glosym. */
static symbol *symtab[TABSIZE];

/* Count the number of global symbols referenced and not defined.  */
static int undefined_global_sym_count = 0;

/* internal format of relocation info entry. */
struct dld_reloc_info {
    /* corresponding symbol table entry. */
    symbol *sp;

    /* The real relocation info entry.
       This is an ugly design.  In the original relocation_info structure.
       The corresponding symbol definition is located by an index to the
       nlist array.  This array is not kept online, and so this index
       must be replace by the address of the corresponding symbol table
       kept online.  However, on most machine this index takes only 24bits,
       which is not large enough (in general) to hold a pointer.  So we
       need a structure definition with a sp field as shown above.  The
       complete original relocation info entry is kept here.  This is not
       very space economical (24 bits are wasted).  But since different
       system has different format for STRUCT RELOCATION_INFO, I'd rather
       let the include file <a.out.h> take care of the difference, than
       using a separate definition for each system. */
    struct relocation_info reloc_info;
};

/* Format of __.SYMDEF:
   First, a longword containing the size of the 'symdef' data that follows.
   Second, zero or more 'symdef' structures.
   Third, a word containing the length of symbol name strings.
   Fourth, zero or more symbol name strings (each followed by a zero).  */

struct symdef {
  int symbol_name_string_index;
  int library_member_offset;
};

/* variable for saving the environment */
static jmp_buf env;

/* pointer to the lastest (newest) file entry */
static struct file_entry *latest_entry = 0;

/* To avoid close a file and then open the same file again, the following
   two variables remember the file that is currently open.  Both are zero
   if no file is open.
 */
static struct file_entry *input_file = 0;
static int input_desc = 0;

/* global variables to return the error code to the caller */
int dld_errno;

/* allow for debugging (e.g. keep symbol table and string table */
static int allow_debugging;

/* true if the executable flags are up-to-date */
static char executable_flags_up_to_date;

/* Miscellaneous routines */

/* save the error code in dld_errno */
static void
fatal (errno)
register int errno;
{
    dld_errno = errno;
    longjmp (env, 1);
} /* fatal */


/* Like malloc but get fatal error if memory is exhausted.  */

static int
xmalloc (size)
int size;
{
    register int result = malloc (size);
    if (!result)
	fatal (DLD_ENOMEMORY);
    return result;
} /* xmalloc */

static int
xcalloc (nelem, size)
int nelem, size;
{
    register int result = calloc (nelem, size);
    if (!result)
	fatal (DLD_ENOMEMORY);
    return result;
} /* xmalloc */


/* Return a newly-allocated string
   whose contents concatenate the strings S1, S2, S3.  */

static char *
concat (s1, s2, s3)
char *s1, *s2, *s3;
{
    register int len1 = strlen (s1), len2 = strlen (s2), len3 = strlen (s3);
    register char *result = (char *) xmalloc (len1 + len2 + len3 + 1);
    
    strcpy (result, s1);
    strcpy (result + len1, s2);
    strcpy (result + len1 + len2, s3);
    result[len1 + len2 + len3] = 0;
    
    return result;
} /* concat */


/* Add a new entry to the file chain if it is not already there.
   Return 0 if no actual insertion is needed, otherwise, return 1. */
static
insert_entry (head, entry)
struct file_chain **head;
register struct file_entry *entry;
{
    register struct file_chain *fc = *head;
    
    while (fc)  {
        if (fc->entry == entry)
            return 0;
        else fc = fc->next;
    }
    
    fc = (struct file_chain *) xmalloc (sizeof (struct file_chain));

    fc->next = *head;
    fc->entry = entry;
    *head = fc;
    return 1;
} /* insert_entry */


/* Close the input file that is now open.  */

static void
file_close ()
{
  if (input_desc) close (input_desc);
  input_desc = 0;
  input_file = 0;
} /* file_close */

/* Open the input file specified by 'entry', and return a descriptor.
   The open file is remembered; if the same file is opened twice in a row,
   a new open is not actually done.  */

static int
file_open (entry)
register struct file_entry *entry;
{
    register int desc;
    
    if (entry == 0) fatal (DLD_ENOFILE);
    
    if (entry->superfile)
	return file_open (entry->superfile);
    
    if (entry == input_file)
	return input_desc;
    
    if (input_file) file_close ();
    
    desc = open (entry->filename, O_RDONLY, 0);
    
    if (desc > 0) {
	input_file = entry;
	input_desc = desc;
	return desc;
    }
    
    fatal (DLD_ENOFILE);
} /* file_open */

/* Medium-level input routines for rel files.  */

/* Read a file's header into the proper place in the file_entry.
   DESC is the descriptor on which the file is open.
   ENTRY is the file's entry.  */

static void
read_header (desc, entry)
int desc;
register struct file_entry *entry;
{
    register int len;
    struct exec *loc = &entry->header;
    
    lseek (desc, entry->starting_offset, 0);
    len = read (desc, loc, sizeof (struct exec));
    if (len != sizeof (struct exec))
	fatal (DLD_EBADHEADER);
    if (N_BADMAG (*loc))
	fatal (DLD_EBADMAGIC);
    
    entry->header_read_flag = 1;
} /* read_header */


/* Read the symbols of file ENTRY into core.
   Assume it is already open, on descriptor DESC.
   Also read the length of the string table, which follows the symbol table,
   but don't read the contents of the string table.  */

static void
read_entry_symbols (desc, entry)
struct file_entry *entry;
int desc;
{
    int str_size;
    
    if (!entry->header_read_flag)
	read_header (desc, entry);
    
    if (entry->header.a_syms < 0)
	fatal (DLD_ENOSYMBOLS);

    if (entry->header.a_syms == 0) {	/* added (JW) */
        entry->symbols = (struct nlist *) xmalloc (sizeof(int));
	entry->string_size = 0;
	return;
    }
    
    entry->symbols = (struct nlist *) xmalloc (entry->header.a_syms);
    
    lseek (desc, N_SYMOFF (entry->header) + entry->starting_offset, 0);
    if (entry->header.a_syms !=
	read (desc, entry->symbols, entry->header.a_syms)) {
	free (entry->symbols);
	entry->symbols = 0;
	fatal (DLD_ENOSYMBOLS);
    }
    
    lseek (desc, N_STROFF (entry->header) + entry->starting_offset, 0);
    if (sizeof str_size != read (desc, &str_size, sizeof str_size)) {
	free (entry->symbols);
	entry->symbols = 0;
	fatal (DLD_ENOSTRINGS);
    }
    
    entry->string_size = str_size;
} /* read_entry_symbols */


/* Read the string table of file ENTRY into core.
   Assume it is already open, on descriptor DESC. */

static void
read_entry_strings (desc, entry)
struct file_entry *entry;
int desc;
{
    if (!entry->header_read_flag)
	read_header (desc, entry);
    
    lseek (desc, N_STROFF (entry->header) + entry->starting_offset, 0);
    if (entry->string_size != read (desc, entry->strings, entry->string_size))
	fatal (DLD_ENOSTRINGS);
    
} /* read_entry_strings */


/* Verify the validity of the relocation information.
   DATA_SIZE is the length of the contents.
   RELOC_INFO is the address of the relocation info, in core.
   RELOC_SIZE is its length in bytes.
   If everything is ok, return the number of external relocation entry.
   Otherwise, return -1. */
reloc_info_ok (data_size, reloc_info, reloc_size, sym_size)
register int data_size;
register struct relocation_info *reloc_info;
register int reloc_size;
register int sym_size;
{
    register struct relocation_info *p = reloc_info;
    register struct relocation_info *end = p +
	reloc_size / sizeof (struct relocation_info);
    register int extern_count = 0;  /* number of external relocation */
    
    for (; p < end; p++) {
	register int symbolnum = RELOC_SYMBOL(p);
	
	if (RELOC_ADDRESS(p) >= data_size)
	    return -1;
	if (RELOC_EXTERN_P(p)) {
	    extern_count++;
	    if (symbolnum * sizeof (struct nlist) >= sym_size)
		return -1;
	} else if (symbolnum != N_TEXT && symbolnum != (N_TEXT | N_EXT) &&
		   symbolnum != N_DATA && symbolnum != (N_DATA | N_EXT) &&
		   symbolnum != N_BSS  && symbolnum != (N_BSS | N_EXT))
	    return -1;

	if (RELOC_TARGET_SIZE(p) > 2) return -1;
    }
    return extern_count++;
    
} /* reloc_info_ok */

/* symbol table management */

/* Compute the hash code for symbol name KEY.  */

static int
hash_string (key)
char *key;
{
    register char *cp;
    register int k;
    
    cp = key;
    k = 0;
    while (*cp)
	k = (((k << 1) + (k >> 14)) ^ (*cp++)) & 0x3fff;
    
    return k;
} /* hash_string */


/* Get the symbol table entry for the global symbol named KEY.
   Create one if there is none.  */

static symbol *
getsym (key)
char *key;
{
    register int hashval;
    register symbol *bp;
    
    /* Determine the proper bucket.  */
    
    hashval = hash_string (key) % TABSIZE;
    
    /* Search the bucket.  */
    
    for (bp = symtab[hashval]; bp; bp = bp->link)
	if (! strcmp (key, bp->name))
	    return bp;
    
    /* Nothing was found; create a new symbol table entry.  */
    
    bp = (symbol *) xmalloc (sizeof (symbol));
    bzero (bp, sizeof (symbol));
    bp->name = (char *) xmalloc (strlen (key) + 1);
    strcpy (bp->name, key);
    
    /* Add the entry to the bucket.  */
    
    bp->link = symtab[hashval];
    symtab[hashval] = bp;
    
    return bp;
} /* getsym */


/* Like `getsym' but return 0 if the symbol is not already known.  */

static symbol *
getsym_soft (key)
char *key;
{
    register int hashval;
    register symbol *bp;
    
    /* Determine which bucket.  */
    
    hashval = hash_string (key) % TABSIZE;
    
    /* Search the bucket.  */
    
    for (bp = symtab[hashval]; bp; bp = bp->link)
	if (! strcmp (key, bp->name))
	    return bp;
    
    return 0;
} /* getsym_soft */

/* Enter one global symbol in the hash table.
   NLIST_P points to the `struct nlist' read from the file
   that describes the global symbol.  NAME is the symbol's name.
   ENTRY is the file entry for the file the symbol comes from.

   The `struct nlist' is modified by making n_name point back to the
   corresponding symbols.

   For the common definition, the symbol is considered defined by the
   first entry that defines it.  The problem here is that uninitialized
   global variables are all treated as common definitions.  Strictly
   speacking, in a program there should be exactly one definition
   (initialized or uninitialized) of any global variable and all other
   files that reference that variable should declare it with the 'extern'
   keyword.  However, most compiler allows the omission of the 'extern'
   keyword, and let the linker map all these definitions to the same
   location.  In other words, multiple definitions of the same symbol is
   allowed.  Now, when one of the files that defines such symbol is to be
   unlinked, should this symbol become undefined?  Or should it remain in
   core?  The decision made is to treat the symbol defined by the first
   file that defines it.  All subsequence definitions of the same symbol
   will be treated as extern references.  However, if any of the
   following definition defines the symbol as initialized global, it will
   be considered multiple definition and be treated as an error.

*/
static void
enter_global_ref (entry, nlist_p, name)
struct file_entry *entry;
register struct nlist *nlist_p;
char *name;
{
    register symbol *sp = getsym (name);
    register int type = nlist_p->n_type;
    register int common = (type == (N_UNDF | N_EXT) && nlist_p->n_value);
    int oldref = sp->referenced;
    int olddef = sp->defined;

    nlist_p->n_un.n_name = (char *) sp;
    
    sp->referenced = 1;

    /* common definition */
    if (common && !olddef)
	type = N_COMM | N_EXT;

    if (type != (N_UNDF | N_EXT)) {
	/* definition of a symbol */
	
	if (olddef) {
	    fprintf(stderr, "dld: multiple defined: %s\n", name);
	    fatal (DLD_EMULTDEFS);
	} else {
	    sp->defined = type;
	    sp->value = common ? (long) xcalloc (1, nlist_p->n_value) :
		nlist_p->n_value; 
	    sp->defined_by = entry;
	}

	if (oldref && !olddef) {
	    register struct file_chain *p = sp->referenced_by;
	    
	    undefined_global_sym_count--;
	    
	    while (p) {
		p->entry->undefined_symbol_count--;
		if (insert_entry (&(p->entry->refs), entry) &&
		    insert_entry (&(entry->refs_by), p->entry) &&
		    p->entry != entry)
		    entry->ref_count++;
		p = p->next;
	    }
	}
    } else {
	/* this is just a reference */
	if (sp->defined_by)
	    if (insert_entry (&(entry->refs), sp->defined_by) &&
		entry != sp->defined_by)
		sp->defined_by->ref_count++;
	
	if (insert_entry (&(sp->referenced_by), entry) && !sp->defined)
	    entry->undefined_symbol_count++;
	
	if (!oldref) {
	    undefined_global_sym_count++;
	    sp->defined = 0;
	    sp->value = 0;
	}
    }
}


/* Enter the external symbol defs and refs of ENTRY in the hash table.  */

static void
enter_file_symbols (entry)
struct file_entry *entry;
{
    register struct nlist *p,
	*end = entry->symbols + entry->header.a_syms / sizeof (struct nlist);
    
    for (p = entry->symbols; p < end; p++)
	if (p->n_type & N_EXT)
	    enter_global_ref (entry, p, p->n_un.n_strx + entry->strings);
}

/* remove all symbols that are no longer needed */
static void
cleanup_symtab ()
{
    register int i;

    for (i = 0; i < TABSIZE; i++) {
	register symbol *sp = symtab[i];
	register symbol *prev_sp = 0;

	while (sp) {
	    register struct file_chain *p = sp->referenced_by;
	    register struct file_chain *prev = 0;
	    register int obsolete = (sp->defined_by &&
				     sp->defined_by->ref_count == 0);
	    
	    while (p)
		if (p->entry->ref_count == 0) {
		    del_link_list_elt (sp->referenced_by, prev, p, next);
		} else {
		    if (obsolete)
			p->entry->undefined_symbol_count++;

		    prev = p;
		    p = p->next;
		}

	    if (obsolete) {
		if (sp->defined == (N_COMM | N_EXT) && sp->value)
		    free (sp->value);
		if (sp->referenced_by == 0) {
		    free (sp->name);
		    del_link_list_elt (symtab[i], prev_sp, sp, link);
		    continue;
		} else {
		    undefined_global_sym_count++;
		    sp->defined = 0;
		    sp->defined_by = 0;
		    sp->value = 0;
		}
	    }

	    if (sp->defined == 0 && sp->referenced_by == 0) {
		undefined_global_sym_count--;
		free (sp->name);
		if (sp->defined == (N_COMM | N_EXT) && sp->value)
		    free (sp->value);
		del_link_list_elt (symtab[i], prev_sp, sp, link);
		continue;
	    }
	    
	    prev_sp = sp;
	    sp = sp->link;
	}
    }
} /* cleanup_symtab */

/* Searching libraries */

static struct file_entry *decode_library_subfile ();
static void linear_library (), symdef_library ();
static int subfile_wanted_p ();
static void read_text_and_data ();

/* Search the library ENTRY, already open on descriptor DESC.
   This means deciding which library members to load,
   making a chain of `struct file_entry' for those members,
   and entering their global symbols in the hash table.  */

static void
search_library (desc, entry)
int desc;
struct file_entry *entry;
{
    int member_length;
    register char *name;
    register struct file_entry *subentry;
    
    if (!undefined_global_sym_count) return;
    
    /* Examine its first member, which starts SARMAG bytes in.  */
    subentry = decode_library_subfile (desc, entry, SARMAG, &member_length);
    if (!subentry) return;
    
    name = subentry->filename;
    free (subentry);
    
    /* Search via __.SYMDEF if that exists, else linearly.  */
    
    if (!strcmp (name, "__.SYMDEF"))
	symdef_library (desc, entry, member_length);
    else
	linear_library (desc, entry);
    
    free (name);
} /* search_library */

/* Construct and return a file_entry for a library member.
   The library's file_entry is library_entry, and the library is open on DESC.
   SUBFILE_OFFSET is the byte index in the library of this member's header.
   We store the length of the member into *LENGTH_LOC.  */

static struct file_entry *
decode_library_subfile (desc, library_entry, subfile_offset, length_loc)
int desc;
struct file_entry *library_entry;
int subfile_offset;
int *length_loc;
{
    int bytes_read;
    register int namelen;
    int member_length;
    register char *name;
    struct ar_hdr hdr1;
    register struct file_entry *subentry;
    
    lseek (desc, subfile_offset, 0);
    
    bytes_read = read (desc, &hdr1, sizeof hdr1);
    if (!bytes_read)
	return 0;		/* end of archive */
    
    if (sizeof hdr1 != bytes_read)
	fatal (DLD_EBADLIBRARY);
    
    if (sscanf (hdr1.ar_size, "%d", &member_length) != 1)
	fatal (DLD_EBADLIBRARY);
    
    subentry = (struct file_entry *) xmalloc (sizeof (struct file_entry));
    bzero (subentry, sizeof (struct file_entry));
    
    for (namelen = 0;
	 namelen < sizeof hdr1.ar_name
	 && hdr1.ar_name[namelen] != 0 && hdr1.ar_name[namelen] != ' ';
	 namelen++);
    
    name = (char *) xmalloc (namelen+1);
    strncpy (name, hdr1.ar_name, namelen);
    name[namelen] = 0;
    
    subentry->filename = name;
    subentry->local_sym_name = name;
    subentry->starting_offset = subfile_offset + sizeof hdr1;
    subentry->superfile = library_entry;
    subentry->total_size = member_length;
    
    (*length_loc) = member_length;
    
    return subentry;
} /* decode_library_subfile */


/* Search a library that has a __.SYMDEF member.
   DESC is a descriptor on which the library is open.
     The file pointer is assumed to point at the __.SYMDEF data.
   ENTRY is the library's file_entry.
   MEMBER_LENGTH is the length of the __.SYMDEF data.  */

static void
symdef_library (desc, entry, member_length)
int desc;
struct file_entry *entry;
int member_length;
{
    int *symdef_data = (int *) alloca (member_length);
    register struct symdef *symdef_base;
    char *sym_name_base;
    int number_of_symdefs;
    int length_of_strings;
    int not_finished;
    int bytes_read;
    register int i;
    struct file_entry *prev = 0;
    int prev_offset = 0;
    
    bytes_read = read (desc, symdef_data, member_length);
    if (bytes_read != member_length)
	fatal (DLD_EBADLIBRARY);
    
    number_of_symdefs = *symdef_data / sizeof (struct symdef);
    if (number_of_symdefs < 0 ||
	number_of_symdefs * sizeof (struct symdef) + 2 * sizeof (int) >=
	member_length)
	fatal (DLD_EBADLIBRARY);
    
    symdef_base = (struct symdef *) (symdef_data + 1);
    length_of_strings = *(int *) (symdef_base + number_of_symdefs);
    
    if (length_of_strings < 0
	|| number_of_symdefs * sizeof (struct symdef) + length_of_strings
	+ 2 * sizeof (int) != member_length)
	fatal (DLD_EBADLIBRARY);
    
    sym_name_base = sizeof (int) + (char *) (symdef_base + number_of_symdefs);
    
    /* Check all the string indexes for validity.  */
    
    for (i = 0; i < number_of_symdefs; i++) {
	register int index = symdef_base[i].symbol_name_string_index;
	if (index < 0 || index >= length_of_strings
	    || (index && *(sym_name_base + index - 1)))
	    fatal (DLD_EBADLIBRARY);
    }
    
    /* Search the symdef data for members to load.
       Do this until one whole pass finds nothing to load.  */
    
    not_finished = 1;
    while (not_finished) {
	not_finished = 0;
	    
	/* Scan all the symbols mentioned in the symdef for ones that we need.
	   Load the library members that contain such symbols.  */
	    
	for (i = 0; i < number_of_symdefs && undefined_global_sym_count; i++)
	    if (symdef_base[i].symbol_name_string_index >= 0) {
		register symbol *sp;
			
		sp = getsym_soft (sym_name_base
				   + symdef_base[i].symbol_name_string_index);
			
		/* If we find a symbol that appears to be needed,
		   think carefully about the archive member that
		   the symbol is in.  */
			
		if (sp && sp->referenced && !sp->defined) {
		    int junk;
		    register int j;
		    register int offset = symdef_base[i].library_member_offset;
		    struct file_entry *subentry;
				
		    /* Don't think carefully about any archive member
		       more than once in a given pass.  */
		    
		    if (prev_offset == offset)
			continue;
		    prev_offset = offset;
		    
		    /* Read the symbol table of the archive member.  */
		    
		    subentry = decode_library_subfile (desc, entry, offset, &junk);
		    if (prev)
			prev->chain = subentry;
		    else entry->subfiles = subentry;
				
		    read_entry_symbols (desc, subentry);
		    if ( allow_debugging )
		        subentry->strings =
				      (char *) xmalloc (subentry->string_size);
		    else
		        subentry->strings =
				      (char *) alloca (subentry->string_size);
		    read_entry_strings (desc, subentry);
				
		    /* Now scan the symbol table and decide whether to load.  */
		    if (!subfile_wanted_p (subentry)) {
			if (prev)
			    prev->chain = 0;
			else entry->subfiles = 0;
			free (subentry->filename);
			free (subentry->symbols);
			free (subentry);
		    } else {
			/* This member is needed; load it.
			   Since we are loading something on this pass,
			   we must make another pass through the symdef data.  */
			
			not_finished = 1;
					
			enter_file_symbols (subentry);
					
			read_text_and_data (desc, subentry);
			
			prev = subentry;
					
			/* Clear out this member's symbols from the symdef data
			   so that following passes won't waste time on them.  */
					
			for (j = 0; j < number_of_symdefs; j++) {
			    if (symdef_base[j].library_member_offset == offset)
				symdef_base[j].symbol_name_string_index = -1;
			}
		    }
		}
	    }
    }
} /* symdef_library */ 


/* Search a library that has no __.SYMDEF.
   ENTRY is the library's file_entry.
   DESC is the descriptor it is open on.  */

static void
linear_library (desc, entry)
int desc;
struct file_entry *entry;
{
    register struct file_entry *prev = 0;
    register int this_subfile_offset = SARMAG;
    
    while (undefined_global_sym_count) {
	int member_length;
	register struct file_entry *subentry;
	    
	subentry = decode_library_subfile (desc, entry, this_subfile_offset, &member_length);
	    
	if (!subentry) return;
	    
	read_entry_symbols (desc, subentry);
	if ( allow_debugging )
	    subentry->strings = (char *) xmalloc (subentry->string_size);
	else
	    subentry->strings = (char *) alloca (subentry->string_size);
	read_entry_strings (desc, subentry);
	    
	if (!subfile_wanted_p (subentry)) {
	    free (subentry->filename);
	    free (subentry->symbols);
	    free (subentry);
	} else {
	    enter_file_symbols (subentry);
		    
	    read_text_and_data (desc, subentry);
	    
	    if (prev)
		prev->chain = subentry;
	    else entry->subfiles = subentry;
	    prev = subentry;
	}
	    
	this_subfile_offset += member_length + sizeof (struct ar_hdr);
	if (this_subfile_offset & 1) this_subfile_offset++;
    }
} /* linear_library */


/* ENTRY is an entry for a library member.
   Its symbols have been read into core, but not entered.
   Return nonzero if we ought to load this member.  */

static int
subfile_wanted_p (entry)
struct file_entry *entry;
{
    register struct nlist *p;
    register struct nlist *end
	= entry->symbols + entry->header.a_syms / sizeof (struct nlist);
    
    for (p = entry->symbols; p < end; p++) {
	register int type = p->n_type;
	    
	if (type & N_EXT && (type != (N_UNDF | N_EXT) || p->n_value)) {
	    register char *name = p->n_un.n_strx + entry->strings;
	    register symbol *sp = getsym_soft (name);
		    
	    /* If this symbol has not been hashed, we can't be looking for it. */
		    
	    if (!sp) continue;

	    if (sp->referenced && !sp->defined &&
		(type != (N_UNDF | N_EXT) || p->n_value))
		/* This is a symbol we are looking for.  */
		return 1;
	}
    }
    
    return 0;
} /* subfile_wanted_p */

/* Relocate the addresses of the file's symbols.  */
static void
relocate_symbol_address (desc, entry)
int desc;
register struct file_entry *entry;
{
    register struct nlist *p;
    register struct nlist *end
	= entry->symbols + entry->header.a_syms / sizeof (struct nlist);
    register int text_relocation, data_relocation, bss_relocation;

    text_relocation = entry->text_start_address;
    data_relocation = entry->data_start_address - entry->header.a_text;
    bss_relocation = entry->bss_start_address - entry->header.a_text -
	entry->header.a_data;
	
    for (p = entry->symbols; p < end; p++) {
	/* If this belongs to a section,
	   update it by the section's start address */
		
	register int type = p->n_type;
	register symbol *sp;
	
	if ((type & N_EXT) == 0) continue;
	
	sp = (symbol *) p->n_un.n_name;
	if (type == (N_TEXT | N_EXT))
	    sp->value += text_relocation;
	else if (type == (N_DATA | N_EXT))
	    /* A symbol whose value is in the data section
	       is present in the input file as if the data section
	       started at an address equal to the length of the
	       file's text.  */
	    sp->value += data_relocation;
	else if (type == (N_BSS | N_EXT))
	    /* likewise for symbols with value in BSS.  */
	    sp->value += bss_relocation;
    }
} /* relocate_symbol_address */


/* Actually performs the relocation of local symbols.
   Do it once and for all for each entry.
   Then keep only those with external references online.
   This function is copied almost directly from perform_relocation (). */
static void
do_local_relocation (data, pc_relocation, reloc_info, dld_reloc_p, reloc_size,
		     entry)
char *data;
struct relocation_info *reloc_info;
struct dld_reloc_info *dld_reloc_p;
struct file_entry *entry;
int pc_relocation;
int reloc_size;
{
    register struct relocation_info *p = reloc_info;

    struct relocation_info *end
	= reloc_info + reloc_size / sizeof (struct relocation_info);

    int text_relocation = entry->text_start_address;
    int data_relocation = entry->data_start_address - entry->header.a_text;
    int bss_relocation
	= entry->bss_start_address - entry->header.a_text - entry->header.a_data;
    
    for (; p < end; p++) {
	register int relocation = 0;
	register int addr = RELOC_ADDRESS(p);
	register int symbolnum = RELOC_SYMBOL(p);

#if defined(sun) && defined(sparc)
	register unsigned int mask = 0;
#else
	register int length = RELOC_TARGET_SIZE(p);
#endif	
	    
	if (RELOC_EXTERN_P(p)) {
	    register int symindex = symbolnum * sizeof (struct nlist);

	    dld_reloc_p->sp = ((symbol *)
			       (((struct nlist *)
				 (((char *)entry->symbols) + symindex))
				->n_un.n_name));
	    bcopy (p, &(dld_reloc_p->reloc_info),
		   sizeof (struct relocation_info));
	    dld_reloc_p++;
	    continue;
	} else switch (symbolnum) {
	    case N_TEXT:
	    case N_TEXT | N_EXT:
	    	relocation = text_relocation;
		break;
		    
	    case N_DATA:
	    case N_DATA | N_EXT:
		/* A word that points to beginning of the the data section
		   initially contains not 0 but rather the "address" of
		   that section in the input file, which is the length of
		   the file's text.  */
		relocation = data_relocation;
		break;
		    
	    case N_BSS:
	    case N_BSS | N_EXT:
		/* Similarly, an input word pointing to the beginning of
		   the bss initially contains the length of text plus data
		   of the file.  */
		relocation = bss_relocation;
		break;
		    
	    case N_ABS:
	    case N_ABS | N_EXT:
		/* just in case */
		break;
		
	    default:
		break;
	}

	if (RELOC_PCREL_P(p))
	    relocation -= pc_relocation;

#if defined(sun) && defined(sparc)

	relocation += RELOC_ADD_EXTRA(p);

	relocation >>= RELOC_VALUE_RIGHTSHIFT(p);

	/* Unshifted mask for relocation */
	mask = 1 << RELOC_TARGET_BITSIZE(p) - 1;
	mask |= mask - 1;
	relocation &= mask;

	switch (RELOC_TARGET_SIZE(p)) {
	    case 0:
	    	*(char *) (data + addr) &= ~mask;
		*(char *) (data + addr) |= relocation;
		break;
		    
	    case 1:
	    	*(short *) (data + addr) &= ~mask;
		*(short *) (data + addr) |= relocation;
		break;
		    
	    case 2:
	    	*(long *) (data + addr) &= ~mask;
		*(long *) (data + addr) |= relocation;
		break;

	    default: break;
	}

#else	
	
	switch (length) {
	    case 0: *(char *) (data + addr) += relocation;
		    break;
		    
	    case 1: *(short *) (data + addr) += relocation;
		    break;
		    
	    case 2: *(int *) (data + addr) += relocation;
		    break;

	    default: break;
	}
#endif
    }
} /* do_local_relocation */


/* Read the relocation information of file ENTRY into core.
   Assume it is already open, on descriptor DESC.
   Then relocate all the local (non-external) symbols.  Save only the
   relocation info for the external symbol references.
   Update entry->header.a_{trsize,drsize} to reflect the new relocation
   table size (in bytes).
*/
static void
relocate_local_refs (desc, entry)
int desc;
struct file_entry *entry;
{
    int text_offset;

    /* number of relocation info that describes an external references. */
    int tr_entry_count = 0, dr_entry_count = 0;	
				     
    struct relocation_info *reloc_buf;

    if (!entry->header_read_flag)
	read_header (desc, entry);

    text_offset = entry->starting_offset + N_TXTOFF (entry->header);

    /* For the text segment */
    
    if (entry->header.a_trsize) {
	reloc_buf = (struct relocation_info *)
	    xmalloc (entry->header.a_trsize);

	lseek (desc, text_offset + entry->header.a_text +
	       entry->header.a_data, 0);
	if (entry->header.a_trsize !=
	    read (desc, reloc_buf, entry->header.a_trsize)) {
	    free (reloc_buf);
	    fatal (DLD_ENOTXTRELOC);
	}
	if ((tr_entry_count =
	     reloc_info_ok (entry->header.a_text, reloc_buf,
			    entry->header.a_trsize, entry->header.a_syms)
	     ) == -1) {
	    free (reloc_buf);
	    fatal (DLD_EBADRELOC);
	} else {
	    entry->text_reloc = (struct dld_reloc_info *)
		xmalloc (tr_entry_count * sizeof (struct dld_reloc_info));
	    do_local_relocation (entry->text_start_address,
				 entry->text_start_address,
				 reloc_buf, entry->text_reloc,
				 entry->header.a_trsize, entry);
	    free (reloc_buf);
	}
    }

    /* For the data segment */
    
    if (entry->header.a_drsize) {
	reloc_buf = (struct relocation_info *)
	    xmalloc (entry->header.a_drsize);

	lseek (desc, text_offset + entry->header.a_text +
	       entry->header.a_data + entry->header.a_trsize, 0);
	if (entry->header.a_drsize !=
	    read (desc, reloc_buf, entry->header.a_drsize)) {
	    free (reloc_buf);
	    fatal (DLD_ENODATRELOC);
	}
	if ((dr_entry_count =
	     reloc_info_ok (entry->header.a_data, reloc_buf,
			    entry->header.a_drsize, entry->header.a_syms)
	     ) == -1) {
	    free (reloc_buf);
	    fatal (DLD_EBADRELOC);
	} else {
	    entry->data_reloc = (struct dld_reloc_info *)
		xmalloc (dr_entry_count * sizeof (struct dld_reloc_info));
	    do_local_relocation (entry->data_start_address,
				 entry->data_start_address -
				 entry->header.a_data, 
				 reloc_buf, entry->data_reloc,
				 entry->header.a_drsize, entry);
	    free (reloc_buf);
	}
    }

    entry->header.a_trsize = tr_entry_count * sizeof (struct dld_reloc_info);
    entry->header.a_drsize = dr_entry_count * sizeof (struct dld_reloc_info);

    /* Free the nlist array. */
    if ( !allow_debugging && entry->symbols ) {
	free (entry->symbols);
	entry->symbols = 0;
    }
    
} /* relocate_local_refs */


/* Relocate ENTRY's text or data section contents.
   DATA is the address of the contents, in core.
   DATA_SIZE is the length of the contents.
   PC_RELOCATION is the difference between the address of the contents
     in the output file and its address in the input file.
   RELOC_INFO is the address of the relocation info, in core.
   RELOC_SIZE is its length in bytes.
   REVERSE is true when an 'un-relocation' is to be done.
*/   

static void
perform_relocation (data, pc_relocation, reloc_info, reloc_size, entry,
		    reverse)
char *data;
struct dld_reloc_info *reloc_info;
struct file_entry *entry;
int pc_relocation;
int reloc_size;
int reverse;
{
    register struct dld_reloc_info *p = reloc_info;

    struct dld_reloc_info *end
	= reloc_info + reloc_size / sizeof (struct dld_reloc_info);
    
#if defined(sun) && defined(sparc)
    if (reverse) return;
#endif
    
    for (; p < end; p++) {
	register int relocation = p->sp->value;
	register struct relocation_info *r = &(p->reloc_info);
	register int addr = RELOC_ADDRESS(r);
	
#if defined(sun) && defined(sparc)
	register unsigned int mask = 0;
#else
	register int length = RELOC_TARGET_SIZE(r);
#endif	
	    
	if (RELOC_PCREL_P(r))
	    relocation -= pc_relocation;

#if defined(sun) && defined(sparc)

	relocation += RELOC_ADD_EXTRA(r);

	relocation >>= RELOC_VALUE_RIGHTSHIFT(r);

	/* Unshifted mask for relocation */
	mask = 1 << RELOC_TARGET_BITSIZE(r) - 1;
	mask |= mask - 1;
	relocation &= mask;

	switch (RELOC_TARGET_SIZE(r)) {
	    case 0:
	    	*(char *) (data + addr) &= ~mask;
		*(char *) (data + addr) |= relocation;
		break;
		    
	    case 1:
	    	*(short *) (data + addr) &= ~mask;
		*(short *) (data + addr) |= relocation;
		break;
		    
	    case 2:
	    	*(long *) (data + addr) &= ~mask;
		*(long *) (data + addr) |= relocation;
		break;

	    default: break;
	}

#else	
	
	if (reverse) relocation = - relocation;
	    
	switch (length) {
	    case 0: *(char *) (data + addr) += relocation;
		    break;
		    
	    case 1: *(short *) (data + addr) += relocation;
		    break;
		    
	    case 2: *(int *) (data + addr) += relocation;
		    break;

	    default: break;
	}
#endif
    }
} /* perform_relocation */

/* given a file name, create an appropriate file_entry for it */
static struct file_entry *
make_entry (filename)
char *filename;
{
    register struct file_entry *entry =
	(struct file_entry *) xmalloc (sizeof (struct file_entry));

    bzero (entry, sizeof (struct file_entry));
    entry->filename = entry->local_sym_name =
	(char *) xmalloc (strlen (filename) + 1);
    strcpy (entry->local_sym_name, filename);
    
    if (filename[0] != '/') {
	char name[MAXPATHLEN];
	entry->filename = concat (getwd(name), "/", filename);
    }
    entry->chain = latest_entry;
    entry->ref_count = 1;
    return entry;
} /* make_entry */


/* If ENTRY is a rel file, read its symbol and string sections into core.
   If it is a library, search it and load the appropriate members
   (which means calling this function recursively on those members).  */

static void
read_file_symbols (desc, entry, load_text)
register int desc;
register struct file_entry *entry;
int load_text;			    /* used only by dld_init */
{
    register int len;
    int magicnum;
    
    len = read (desc, &magicnum, sizeof magicnum);
    if (len != sizeof magicnum)
	fatal (DLD_EBADHEADER);
    
    if (!N_BADMAG (*((struct exec *)&magicnum))) {
	read_entry_symbols (desc, entry);
	if ( allow_debugging )
	  entry->strings = (char *) xmalloc (entry->string_size);
	else
	  entry->strings = (char *) alloca (entry->string_size);
	read_entry_strings (desc, entry);
	enter_file_symbols (entry);
	if ( !allow_debugging )
	  entry->strings = 0;
	if (load_text) read_text_and_data (desc, entry);
    } else {
	char armag[SARMAG];
	
	lseek (desc, 0, 0);
	if (SARMAG != read (desc, armag, SARMAG) || strncmp (armag, ARMAG, SARMAG))
	    fatal (DLD_EBADOBJECT);
	entry->library_flag = 1;
	search_library (desc, entry);
    }
} /* read_file_symbols */


/* Allocate memory for all text, data and bss segments and read them in
   from the file. */
static void
read_text_and_data (desc, entry)
int desc;
register struct file_entry *entry;
{
    register size = entry->header.a_text + entry->header.a_data +
	entry->header.a_bss;

    entry->text_start_address = xmalloc (size);
    if (size - entry->header.a_text > 0) {
	entry->data_start_address = entry->text_start_address +
	    entry->header.a_text;
	entry->bss_start_address = entry->data_start_address +
	    entry->header.a_data;
    } else entry->data_start_address = entry->bss_start_address = 0;
    

    /* Read text and data sections into core.
       Note that the bss segment does not actually take up space in the
       object file, so its size must be subtracted from SIZE */
    
    lseek (desc, entry->starting_offset + N_TXTOFF(entry->header), 0);
    size -= entry->header.a_bss;
    if (size != read (desc, entry->text_start_address, size)) {
	free (entry->text_start_address);
	entry->text_start_address = entry->data_start_address =
	    entry->bss_start_address = 0;
	fatal (DLD_ENODATA);
    }

    /* zero the bss segment */
    if (entry->header.a_bss)
	bzero (entry->bss_start_address, entry->header.a_bss);
} /* read_text_and_data */
    

/* Allocate memory for the text and data segments and relocate all local
   symbols */
static void
relocate_entry_symbols (desc, entry)
int desc;
register struct file_entry *entry;
{
    /* Compute start addresses of each sections and symbols.  */
    
    if (entry->library_flag) {
	register struct file_entry *subentry = entry->subfiles;
	for (; subentry; subentry = subentry->chain) {
	    relocate_symbol_address (desc, subentry);
	    relocate_local_refs (desc, subentry);
	}
    } else {
	relocate_symbol_address (desc, entry);
	relocate_local_refs (desc, entry);
    }
} /* relocate_entry_symbols */
	
 
/*  Find all modules have all external references defined but not resolved. */
static void
patch_all_files (entry)
register struct file_entry *entry;
{
    while (entry) {
	if (entry->library_flag)
	    patch_all_files (entry->subfiles);
	else if (!entry->all_symbols_resolved_flag &&
		 entry->undefined_symbol_count == 0) {
	    /* entry whose global references have just been resolved */
	    perform_relocation (entry->text_start_address,
				entry->text_start_address,
				entry->text_reloc,
				entry->header.a_trsize, entry, 0);
	    perform_relocation (entry->data_start_address,
				entry->data_start_address -
				entry->header.a_data,
				entry->data_reloc, entry->header.a_drsize,
				entry, 0);
	    entry->all_symbols_resolved_flag = 1;
	} else if (entry->all_symbols_resolved_flag &&
		   entry->undefined_symbol_count != 0) {
	    /* entry that has some of its global references being
	       "un-defined" */
	    perform_relocation (entry->text_start_address,
				entry->text_start_address,
				entry->text_reloc,
				entry->header.a_trsize, entry, 1);
	    perform_relocation (entry->data_start_address,
				entry->data_start_address -
				entry->header.a_data,
				entry->data_reloc, entry->header.a_drsize,
				entry, 1);
	    entry->all_symbols_resolved_flag = 0;
	}

	entry = entry->chain;
    }
} /* patch_all_files */


/*
 * reset the executable_flag of the given entry, and then recursively
 * propagate this to all modules that reference symbols in this entry.
 */
static void
invalidate (entry)
struct file_entry *entry;
{
    register struct file_chain *p;
    
    if (entry == 0)
	return;
    
    entry->executable_flag = 0;

    for (p = entry->refs_by; p; p = p->next)
	if (p->entry->executable_flag)
	    invalidate (p->entry);
} /* invalidate */

/*
 * For all modules loaded, determine which of them can be safely executed.
 * For those that can, set the flag executable_flag.
 */
static void
find_all_executable_modules ()
{
    register struct file_entry *p;

    /* set all executable flags */
    for (p = latest_entry; p ; p = p->chain) {
	if (p->library_flag) {
	    register struct file_entry *q = p->subfiles;
	    while (q) {
		q->executable_flag = q->all_symbols_resolved_flag;
		q = q->chain;
	    }
	}
	p->executable_flag = p->all_symbols_resolved_flag;
    }
    
    /* invalidate those modules that are not (yet) executable. */
    for (p = latest_entry; p; p = p->chain) {
	if (p->library_flag) {
	    register struct file_entry *q = p->subfiles;
	    while (q) {
		if (!q->all_symbols_resolved_flag && q->refs_by)
		    invalidate (q);
		q = q->chain;
	    }
	} else if (!p->all_symbols_resolved_flag && p->refs_by)
	    invalidate (p);
    }
    executable_flags_up_to_date = 1;
} /* find_all_executable_modules */


/* remove all reference pointers *TO* ENTRY */
static void 
remove_cross_references (head_of_chain, entry)
struct file_entry *head_of_chain, *entry;
{
    register struct file_entry *ep = head_of_chain;
    
    while (ep) {
	register struct file_chain *p = ep->refs;
	
	if (ep->library_flag)
	    remove_cross_references (ep->subfiles, entry);
	else {
	    register struct file_chain *prev = 0;
	    
	    while (p) {
		if (p->entry == entry || p->entry->superfile == entry) {
		    del_link_list_elt (ep->refs, prev, p, next);
		} else {
		    prev = p;
		    p = p->next;
		}
	    }
	}
	ep = ep->chain;
    }
} /* remove_cross_references */


/* remove all cross reference pointers related to an obsolete file entry.
   It is assumed that the ref_count of this entry is zero.
   Also clear up all other entries that become obsolete when this entry is
   removed. */
static void
cleanup_obsolete_entries (entry)
struct file_entry *entry;
{
    register struct file_chain *p;

    if (entry->ref_count != 0) return;

    if (entry->library_flag) {
	register struct file_entry *subentry = entry->subfiles;

	for (; subentry; subentry = subentry->chain)
	    cleanup_obsolete_entries (subentry);
    }

    p = entry->refs;
    entry->refs = 0;		    /* to prevent loop; might be redundant*/

    while (p) {
	register struct file_chain *next = p->next;
	
	if (p->entry->ref_count)    /* ref_count may already be set to zero */
	    (p->entry->ref_count)--;
	if (p->entry->ref_count == 0)
	    cleanup_obsolete_entries (p->entry);
	free (p);
	p = next;
    }
} /* cleanup_obsolete_entries */


/* remove all memory blocks assigned for ENTRY,
   return the pointers to the next entry in chain */
static struct file_entry *
kill_entry (entry)
register struct file_entry *entry;
{
    register struct file_chain *p;

    if (entry->library_flag) {
	register struct file_entry *subentry = entry->subfiles;
	while (subentry)
	    subentry = kill_entry (subentry);
    }
	
    p = entry->refs;
    while (p) {
	register struct file_chain *next_chain = p->next;
	free (p);
	p = next_chain;
    }

    p = entry->refs_by;
    while (p) {
	register struct file_chain *next_chain = p->next;
	free (p);
	p = next_chain;
    }

    if (entry->local_sym_name) free (entry->local_sym_name);

    if (entry->filename != entry->local_sym_name && entry->filename)
	free (entry->filename);

    if (entry->symbols) free (entry->symbols);

    if (entry->text_reloc) free (entry->text_reloc);

    if (entry->data_reloc) free (entry->data_reloc);

    if (entry->text_start_address) free (entry->text_start_address);

    {
	register struct file_entry *next = entry->chain;
	free (entry);
	return (next);
    }
} /* kill_entry */


/* clean all data structures so that they return to the original states
   after last call to dld */
static void
clean_up ()
{

    if (!latest_entry)
	return;

    latest_entry->ref_count = 0;
    if (latest_entry->library_flag) {
	register struct file_entry *subentry = latest_entry->subfiles;
	for (; subentry; subentry = subentry->chain)
	    subentry->ref_count = 0;
    }
	
    remove_cross_references (latest_entry, latest_entry);

    cleanup_obsolete_entries (latest_entry);

    cleanup_symtab ();
    
    latest_entry = kill_entry (latest_entry);
} /* clean_up */


/* Actually perform the unlink operation.
   Search through the list of file entries, unlink those whose ref_count is
   zero. */
static struct file_entry *
do_unlink (entry)
struct file_entry *entry;
{
    register struct file_entry *p = entry;
    register struct file_entry *prev = 0;
    
    if (p == 0) return p;

    while (p) {
	if (p->library_flag) {
	    p->subfiles = do_unlink (p->subfiles);

	    /* if all subentries are gone, remove itself */
	    if (p->subfiles == 0)
		p->ref_count = 0;
	}

	if (p->ref_count == 0) {
	    register struct file_entry *next;
	    
	    next = kill_entry (p);
	
	    if (prev == 0) {
		entry = next;
		p = entry;
	    } else {
		prev->chain = next;
		p = prev->chain;
	    }
	} else {
	    prev = p;
	    p = p->chain;
	}
    }
    executable_flags_up_to_date = 0;
    return entry;
} /* do_unlink */

dld_init (myname, debug)
char *myname;
int debug;
{
    int desc;
    
    allow_debugging = debug;
    page_size = getpagesize ();
    bzero (symtab, TABSIZE * sizeof(symbol *));
    latest_entry = 0;
    undefined_global_sym_count = 0;
    executable_flags_up_to_date = 0;
    
    if (myname == 0) {
	dld_errno = DLD_ENOFILE;
	return dld_errno;
    }
    
    if (setjmp (env)) {
	clean_up ();
	file_close ();
	return dld_errno;
    }
    latest_entry = make_entry (myname);
    desc = file_open (latest_entry);
    read_file_symbols (desc, latest_entry, 0);

    if ( !allow_debugging && latest_entry->symbols ) {
	free (latest_entry->symbols);
	latest_entry->symbols = 0;
    }
    
    file_close ();

    latest_entry->undefined_symbol_count = 0;
    latest_entry->all_symbols_resolved_flag = 1;
    
    return 0;
} /* dld_init */


dld_link (object_file)
char *object_file;
{
    register int desc;
    struct file_entry *old_latest_entry = latest_entry;

    if (setjmp (env)) {
	if (old_latest_entry != latest_entry)
	    clean_up ();
	file_close ();
	return dld_errno;
    }

    dld_errno = 0;
    file_close ();		    /* file might be opened in the last
				       call */
    
    if (object_file == 0) {
	dld_errno = DLD_ENOFILE;
	return dld_errno;
    }
    
    latest_entry = make_entry (object_file);

    desc = file_open (latest_entry);
    
    read_file_symbols (desc, latest_entry, 1);

    if (latest_entry->library_flag && latest_entry->subfiles == 0) {
	free (latest_entry->local_sym_name);
	if (latest_entry->filename != latest_entry->local_sym_name)
	    free (latest_entry->filename);
	
	latest_entry = kill_entry (latest_entry);
	file_close ();
	return 0;
    }
    
    relocate_entry_symbols (desc, latest_entry);
    
    file_close ();
    
    patch_all_files (latest_entry);

    executable_flags_up_to_date = 0;
    return 0;
} /* dld_link */


/* return the location of the given symbol without prepending a '_'. */
unsigned long
dld_get_bare_symbol (name)
char *name;
{
    register symbol *sp;
    
    if (name == 0)
	return 0;

    sp = getsym_soft (name);

    if (sp)
	if (sp->defined)
	    return sp->value;
    return 0;
} /* dld_get_bare_symbol */


/* given a symbol name, return the location of that symbol (in core) */
unsigned long
dld_get_symbol (name)
char name[];
{
    register symbol *sp;
    register char *p;

    if (name == 0)
	return 0;

    /* prepend a '_' to name, as required by C's convention */
    p = (char *) alloca (strlen(name) + 2);
    *p = '_';
    strcpy (p+1, name);

    return dld_get_bare_symbol (p);
} /* dld_get_symbol */

	
/*  given a function name, return the location of that function (in core) */
unsigned long
dld_get_func (name)
char name[];
{
    register symbol *sp;
    register char *p;

    if (name == 0)
	return 0;
    
    /* prepend a '_' to name, as required by C's convention */
    p = (char *) alloca (strlen(name) + 2);
    *p = '_';
    strcpy (p+1, name);

    sp = getsym_soft (p);

    if (sp)
	if (sp->defined == (N_EXT | N_TEXT))
	    return sp->value;
    return 0;
} /* dld_get_func */


/* given a file_entry, unlink that (and all its decendents).  Modules still
   referenced by the remainings will not be unlinked.
   ENTRY is assumed to be a valid pointer to a file_entry structure.
   if FORCE is true, remove this entry event regardless if it is still
   referenced by others */
static void
unlink_entry (entry, force)
struct file_entry *entry;
int force;
{
    if (force) {
	entry->ref_count = 0;
	if (entry->library_flag) {
	    register struct file_entry *subentry = entry->subfiles;
	    for (; subentry; subentry = subentry->chain)
		subentry->ref_count = 0;
	}
    } else {
	register struct file_entry *fe;
	register int found;
	
	if (entry->library_flag || entry->already_unlink) return;
	for (found = 0, fe = latest_entry; fe; fe = fe->chain)
	    if (fe == entry) {
		entry->ref_count--;
		entry->already_unlink = 1;
		found = 1;
		break;
	    }
	if (!found) return;
    }

    if (entry->ref_count == 0) {
	remove_cross_references (latest_entry, entry);

	cleanup_obsolete_entries (entry);

	cleanup_symtab ();

	latest_entry = do_unlink (latest_entry);
    }

    patch_all_files (latest_entry);
    executable_flags_up_to_date = 0;
} /* unlink_entry */


static struct file_entry *
search_files (entry, name)
register struct file_entry *entry;
register char *name;
{
    while (entry) {
	if (entry->library_flag) {
	    register struct file_entry *subentry;

	    if ((subentry = search_files (entry->subfiles, name)) != 0)
		return subentry;
	} else {
	    if (!strcmp (entry->local_sym_name, name))
		return entry;
	    if (entry->local_sym_name != entry->filename)
		if (!strcmp (entry->filename, name))
		    return entry;
	}

	entry = entry->chain;
    }
    return 0;
} /* search_files */


dld_unlink_by_file (name, force)
char *name;
int force;
{
    register struct file_entry *entry;

    if (entry = search_files (latest_entry, name)) {
	unlink_entry (entry, force);
	return 0;
    }

    dld_errno = DLD_EUNDEFSYM;
    return dld_errno;
} /* dld_unlink_by_file */


dld_unlink_by_symbol (name, force)
char *name;
int force;
{
    register symbol *sp;
    register char *p;

    if (name == 0) {
	dld_errno = DLD_EUNDEFSYM;
	return dld_errno;
    }

    /* prepend a '_' to name, as required by C's convention */
    p = (char *) alloca (strlen(name) + 2);
    *p = '_';
    strcpy (p+1, name);

    sp = getsym_soft (p);

    if (sp && sp->defined_by) {
	unlink_entry (sp->defined_by, force);
	return 0;
    }

    dld_errno = DLD_EUNDEFSYM;
    return dld_errno;
} /* dld_unlink_by_symbol */


/*
 * return true if the named function can be safely exeucted.
 */
dld_function_executable_p (name)
char name[];
{
    register symbol *sp;
    register char *p;

    if (name == 0)
	return 0;

    /* prepend an '_' to name, as required by the C convention */
    p = (char *) alloca (strlen(name) + 2);
    *p = '_';
    strcpy (p+1, name);
    sp = getsym_soft (p);

    if (sp && sp->defined == (N_EXT | N_TEXT)) {
	register struct file_entry *fe = sp->defined_by;

	if (fe == 0) return 0;
	
	if (!executable_flags_up_to_date)
	    find_all_executable_modules ();

	return fe->executable_flag;
    }

    return 0;
} /* dld_function_executable_p */

	
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Additions  from Jan Wielemaker  (jan@swi.psy.uva.nl).  These functions
are to provide some  elementary debugging facilities.   I do not claim
this to be a neat debugging interface; it's just  to help me finish my
program.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


int
dld_list_undefined()
{ symbol *s;
  int n;
  int warned = 0;

  for(n=0; n<TABSIZE; n++)
  { for( s = symtab[n]; s; s = s->link )
      if ( !s->defined )
      { if ( warned++ == 0 )
	  fprintf(stderr, "dld: undefined:\n");
	fprintf(stderr, "\t%s\n", s->name);
      }
  }

  return warned;
}
  

/* nlist_symbol_name() returns the name of a symbol.  This would have been
   simple but the global nlist entries are patched such that n_un.n_name points
   to a global symbol.  We simply check for one of the three reasonable
   representations.  As long as pointers start at addresses larger than the
   string table of the entry this is save.
*/

static char *
nlist_symbol_name(entry, symb)
struct file_entry *entry;
struct nlist *symb;
{ if ( symb->n_un.n_strx < entry->string_size )
    return entry->strings + symb->n_un.n_strx;

  if ( symb->n_un.n_name >= entry->strings &&
       symb->n_un.n_name  < entry->strings + entry->string_size )
    return symb->n_un.n_name;

  return ((symbol *)symb->n_un.n_name)->name;
}


/* find_function_in_entry() checks whether an address is in the text area
   the specified entry.  If so it scans the symbol table to find the
   text symbols with the highest value below and with the lowest value
   above the address.  From there it determines the name and percentage
   we are in the function.
*/

static int
find_function_in_entry(entry, address, name, perc) 
struct file_entry *entry;
unsigned long address;
char **name;
int *perc;
{ long start = entry->text_start_address;
  long end   = start + entry->header.a_text;

					/* HACK; if it is the original file */
					/* use etext for the end, so it */
					/* cooperates nice with ld -A based */
					/* foreign interfaces that loaded */
					/* me. */
  if ( !entry->superfile && !entry->chain )
  { extern etext;

    end = (long) &etext;
  }

  if ( address >= start && address < end )
  { if ( !entry->symbols || !entry->strings )
    { *perc = 0;			/* no symbols */
      *name = "???";
    } else
    { struct nlist *symbol = entry->symbols;
      struct nlist *target = NULL;
      int nsymbols = entry->header.a_syms / sizeof(struct nlist);
      int n;
      unsigned long below = 0;
      unsigned long above = end - start;

      address -= start;

      for(n=0; n<nsymbols; n++, symbol++)
      { if ( symbol->n_type & N_TEXT )	/* a function */
	{ unsigned long value = symbol -> n_value;

	  if ( value <= address && value >= below )
	  { below = value;
	    target = symbol;
	  } else if ( value > address && value < above )
	    above = value;
	}
      }

      if ( target )
      { *name = nlist_symbol_name(entry, target) + 1; /* 1 for the '_' */

	if ( above > below )
	  *perc = ((address - below) * 100) / (above - below);
	else
	  *perc = 0;
      } else
      { *name = "???";
        *perc = 0;
      }
    }

    return 1;
  }

  return 0;
}


/* dld_find_function () is the entry point for this debugging stuff.  It
   returns the name for the function that contains text address address as
   well as the percentage from the beginning of the function.
*/

char *
dld_find_function(address, perc)
unsigned long address;
int *perc;
{ struct file_entry *entry = latest_entry;
  char *name;

  for( ; entry ; entry = entry->chain )
  { if ( entry->library_flag )
    { struct file_entry *subfile = entry->subfiles;

      for( ; subfile ; subfile = subfile->chain )
      	if ( find_function_in_entry(subfile, address, &name, perc) )
	  return name;
    } else
    { if ( find_function_in_entry(entry, address, &name, perc) )
	return name;
    }
  }

  *perc = 0;

  return "???";
}


long
dld_text_start(file)
char *file;
{ struct file_entry *entry = latest_entry;

  for( ; entry ; entry = entry->chain )
  { if ( entry->library_flag )
    { struct file_entry *subfile = entry->subfiles;

      for( ; subfile ; subfile = subfile->chain )
      	if ( strcmp(file, subfile->filename) )
	  return subfile->text_start_address;
    } else
    { if ( strcmp(file, entry->filename) )
	return entry->text_start_address;
    }
  }

  return 0;
}
