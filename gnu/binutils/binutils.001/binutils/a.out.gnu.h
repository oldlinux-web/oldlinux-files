
struct exec
{
#ifdef COFF_ENCAPSULATE
  unsigned short a_magic;
  unsigned char a_machtype;
  unsigned char a_flags;
#else
  long a_magic;			/* number identifies as .o file and gives type of such. */
#endif
  unsigned a_text;		/* length of text, in bytes */
  unsigned a_data;		/* length of data, in bytes */
  unsigned a_bss;		/* length of uninitialized data area for file, in bytes */
  unsigned a_syms;		/* length of symbol table data in file, in bytes */
  unsigned a_entry;		/* start address */
  unsigned a_trsize;		/* length of relocation info for text, in bytes */
  unsigned a_drsize;		/* length of relocation info for data, in bytes */
};

/* these go in the a_machtype field */
#define M_68010 1 /* sun defined these */
#define M_68020 2

#define M_386 100 /* skip a bunch so we don't conflict with sun's numbers */

/* Code indicating object file or impure executable.  */
#define OMAGIC 0407
/* Code indicating pure executable.  */
#define NMAGIC 0410
/* Code indicating demand-paged executable.  */
#define ZMAGIC 0413

#define N_BADMAG(x)					\
 (((x).a_magic) != OMAGIC && ((x).a_magic) != NMAGIC	\
  && ((x).a_magic) != ZMAGIC)

#define _N_HDROFF(x) (1024 - sizeof (struct exec))

#define N_TXTOFF(x) \
 ((x).a_magic == ZMAGIC ? _N_HDROFF((x)) + sizeof (struct exec) : sizeof (struct exec))

#define N_DATOFF(x) (N_TXTOFF(x) + (x).a_text)

#define N_TRELOFF(x) (N_DATOFF(x) + (x).a_data)

#define N_DRELOFF(x) (N_TRELOFF(x) + (x).a_trsize)

#define N_SYMOFF(x) (N_DRELOFF(x) + (x).a_drsize)

#define N_STROFF(x) (N_SYMOFF(x) + (x).a_syms)

/* Address of text segment in memory after it is loaded.  */
#define N_TXTADDR(x) 0

/* Address of data segment in memory after it is loaded.
   Note that it is up to you to define SEGMENT_SIZE
   on machines not listed here.  */
#ifdef vax
#define SEGMENT_SIZE page_size
#endif
#ifdef is68k
#define SEGMENT_SIZE 0x20000
#endif

#ifndef N_DATADDR
#define N_DATADDR(x) \
    (((x).a_magic==OMAGIC)? (N_TXTADDR(x)+(x).a_text) \
     : (SEGMENT_SIZE + ((N_TXTADDR(x)+(x).a_text-1) & ~(SEGMENT_SIZE-1))))
#endif

/* Address of bss segment in memory after it is loaded.  */
#define N_BSSADDR(x) (N_DATADDR(x) + (x).a_data)

struct nlist {
  union {
    char *n_name;
    struct nlist *n_next;
    long n_strx;
  } n_un;
  char n_type;
  char n_other;
  short n_desc;
  unsigned n_value;
};

#define N_UNDF 0
#define N_ABS 2
#define N_TEXT 4
#define N_DATA 6
#define N_BSS 8
#define N_FN 15

#define N_EXT 1
#define N_TYPE 036
#define N_STAB 0340

/* The following type indicates the definition of a symbol as being
   an indirect reference to another symbol.  The other symbol
   appears as an undefined reference, immediately following this symbol.

   Indirection is asymmetrical.  The other symbol's value will be used
   to satisfy requests for the indirect symbol, but not vice versa.
   If the other symbol does not have a definition, libraries will
   be searched to find a definition.  */
#define N_INDR 0xa

/* The following symbols refer to set elements.
   All the N_SET[ATDB] symbols with the same name form one set.
   Space is allocated for the set in the text section, and each set
   element's value is stored into one word of the space.
   The first word of the space is the length of the set (number of elements).

   The address of the set is made into an N_SETV symbol
   whose name is the same as the name of the set.
   This symbol acts like a N_TEXT global symbol
   in that it can satisfy undefined external references.  */

/* These appear as input to LD, in a .o file.  */
#define	N_SETA	0x14		/* Absolute set element symbol */
#define	N_SETT	0x16		/* Text set element symbol */
#define	N_SETD	0x18		/* Data set element symbol */
#define	N_SETB	0x1A		/* Bss set element symbol */

/* This is output from LD.  */
#define N_SETV	0x1C		/* Pointer to set vector in text area.  */

/* This structure describes a single relocation to be performed.
   The text-relocation section of the file is a vector of these structures,
   all of which apply to the text section.
   Likewise, the data-relocation section applies to the data section.  */

struct relocation_info
{
  /* Address (within segment) to be relocated.  */
  int r_address;
  /* The meaning of r_symbolnum depends on r_extern.  */
  unsigned int r_symbolnum:24;
  /* Nonzero means value is a pc-relative offset
     and it should be relocated for changes in its own address
     as well as for changes in the symbol or section specified.  */
  unsigned int r_pcrel:1;
  /* Length (as exponent of 2) of the field to be relocated.
     Thus, a value of 2 indicates 1<<2 bytes.  */
  unsigned int r_length:2;
  /* 1 => relocate with value of symbol.
          r_symbolnum is the index of the symbol
	  in file's the symbol table.
     0 => relocate with the address of a segment.
          r_symbolnum is N_TEXT, N_DATA, N_BSS or N_ABS
	  (the N_EXT bit may be set also, but signifies nothing).  */
  unsigned int r_extern:1;
  /* Four bits that aren't used, but when writing an object file
     it is desirable to clear them.  */
  unsigned int r_pad:4;
};
