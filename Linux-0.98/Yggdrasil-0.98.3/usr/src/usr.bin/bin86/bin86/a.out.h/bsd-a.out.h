#ifndef _BSD_A_OUT_H
#define _BSD_A_OUT_H

struct	exec {			/* a.out header */
  unsigned char	a_magic[2];	/* magic number */
  unsigned char	a_flags;	/* flags, see below */
  unsigned char	a_cpu;		/* cpu id */
  unsigned char	a_hdrlen;	/* length of header */
  unsigned char	a_unused;	/* reserved for future use */
  unsigned short a_version;	/* version stamp (not used at present) */
  long		a_text;		/* size of text segement in bytes */
  long		a_data;		/* size of data segment in bytes */
  long		a_bss;		/* size of bss segment in bytes */
  long		a_entry;	/* entry point */
  long		a_total;	/* total memory allocated */
  long		a_syms;		/* size of symbol table */
				/* SHORT FORM ENDS HERE */

  long		a_trsize;	/* text relocation size */
  long		a_drsize;	/* data relocation size */
  long		a_tbase;	/* text relocation base */
  long		a_dbase;	/* data relocation base */
};

#define A_MAGIC0	((unsigned char) 0x01)
#define A_MAGIC1	((unsigned char) 0x03)
#define BADMAG(X)    ((X).a_magic[0] != A_MAGIC0 || (X).a_magic[1] != A_MAGIC1)

/* CPU Id of TARGET machine (byte order coded in low order two bits) */
#define A_NONE	0x00	/* unknown */
#define A_I8086	0x04	/* intel i8086/8088 */
#define A_M68K	0x0B	/* motorola m68000 */
#define A_NS16K	0x0C	/* national semiconductor 16032 */
#define A_I80386 0x10	/* intel i80386 */
#define A_SPARC	0x17	/* Sun SPARC */

#define A_BLR(cputype)	((cputype&0x01)!=0) /* TRUE if bytes left-to-right */
#define A_WLR(cputype)	((cputype&0x02)!=0) /* TRUE if words left-to-right */

/* Flags. */
#define A_UZP	0x01	/* unmapped zero page (pages) */
#define A_EXEC	0x10	/* executable */
#define A_SEP	0x20	/* separate I/D */
#define A_PURE	0x40	/* pure text */		/* not used */
#define A_TOVLY	0x80	/* text overlay */	/* not used */

/* Tell a.out.gnu.h not to define `struct exec'.  */
#define __STRUCT_EXEC_OVERRIDE__

/* Hide M_386 from enum declaration in a.out.h. */
#define M_386 HIDE_M_386

#include "a.out.gnu.h"

#undef M_386
#define M_386 A_I80386

#undef N_MAGIC
#define N_MAGIC3(magic0, magic1, type) \
	((magic0) | ((magic1) << 8) | ((type) << 16))
#define N_MAGIC(exec) \
	N_MAGIC3((exec).a_magic[0], (exec).a_magic[1], (exec).a_flags)
	
#undef N_MACHTYPE
#define N_MACHTYPE(exec) ((enum machine_type)((exec).a_cpu))

#undef N_FLAGS
#define N_FLAGS(exec) 0

#undef N_SET_INFO
#define N_SET_INFO(exec, magic, type, flags) \
	((exec).a_magic[0] = (magic) & 0xff, \
	 (exec).a_magic[1] = ((magic) >> 8) & 0xff, \
	 (exec).a_flags = ((magic) >> 16) & 0xff, \
	 (exec).a_cpu = (type) & 0xff)
	 
#undef N_SET_MAGIC
#define N_SET_MAGIC(exec, magic) \
	((exec).a_magic[0] = (magic) & 0xff, \
	 (exec).a_magic[1] = ((magic) >> 8) & 0xff, \
	 (exec).a_flags = ((magic) >> 16) & 0xff)

#undef N_SET_MACHTYPE
#define N_SET_MACHTYPE(exec, machtype) \
	((exec).a_cpu = (machtype) & 0xff, \
	 (exec).a_hdrlen = sizeof (exec))

#undef N_SET_FLAGS
#define N_SET_FLAGS(exec, flags) /* nothing */

#undef OMAGIC
#define OMAGIC N_MAGIC3(A_MAGIC0, A_MAGIC1, 0)

#undef NMAGIC
#define NMAGIC N_MAGIC3(A_MAGIC0, A_MAGIC1, A_EXEC)

#undef ZMAGIC
#define ZMAGIC N_MAGIC3(A_MAGIC0, A_MAGIC1, A_EXEC)

#undef _N_HDROFF
#define _N_HDROFF(x) 0

#define PAGE_SIZE 16
#define SEGMENT_SIZE PAGE_SIZE
#define getpagesize() PAGE_SIZE

#endif /* _BSD_A_OUT_H */
