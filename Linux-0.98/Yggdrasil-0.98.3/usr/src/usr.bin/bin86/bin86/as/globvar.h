/*
 *   bin86/as/globvar.h
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* globvar.h - global variables for assembler */

/* global control and bookkeeping */

EXTERN bool_t binaryc;		/* current binary code flag */
EXTERN bool_t binaryg;		/* global binary code flag */
EXTERN offset_t binmbuf;	/* offset in binary code buffer for memory */

EXTERN unsigned char dirpag;	/* direct page */

EXTERN bool_t globals_only_in_obj;	/* global symbols only in object file */

EXTERN bool_t jumps_long;	/* make all jumps long */

EXTERN unsigned char mapnum;	/* global map number */

EXTERN bool_t objectc;		/* current object code flag */
EXTERN bool_t objectg;		/* global object code flag */

EXTERN bool_t pass;		/* pass, FALSE means 0, TRUE means 1 */

EXTERN offset_t progent;	/* program entry point */

EXTERN bool_t symgen;		/* generate symbol table flag */

EXTERN unsigned toterr;		/* total errors */
EXTERN unsigned totwarn;	/* total warnings */

/* bookeeping for current line */

EXTERN char *linebuf;		/* buffer */

/* for symbol table routines */

EXTERN char *heapend;		/* end of free space for symbol list */
EXTERN char *heapptr;		/* next free space in symbol list */
EXTERN unsigned char inidata;	/* init sym entry data governed by "u" flag */
EXTERN struct sym_s **spt;	/* symbol pointer table */
EXTERN struct sym_s **spt_top;	/* top of symbol ptr table */

/* for translator */

EXTERN struct sym_s *label;	/* non-null if valid label starts line */
EXTERN unsigned char pedata;	/* shows how PROGENT bound, flags like LCDATA*/
EXTERN unsigned char popflags;	/* pseudo-op flags */

/* for BLOCK stack */

EXTERN struct block_s *blockstak;	/* stack ptr */
EXTERN unsigned char blocklevel;	/* nesting level */

/* for IF stack */

EXTERN struct if_s *ifstak;	/* stack ptr */
EXTERN unsigned char iflevel;	/* nesting level */
EXTERN bool_t ifflag;		/* set if assembling */

/* location counters for various segments */

EXTERN offset_t lc;		/* location counter */
EXTERN unsigned char lcdata;	/* shows how lc is bound */
				/* FORBIT is set if lc is forward referenced */
				/* RELBIT is is if lc is relocat. (not ASEG) */
EXTERN offset_t lcjump;		/* lc jump between lines */
#define mcount (((unsigned char *) &lcjump)[LOW_BYTE])
				/* low byte of lcjump */
EXTERN struct lc_s *lcptr;	/* top of current spot in lctab */
EXTERN struct lc_s *lctab;	/* start of lctab */
EXTERN struct lc_s *lctabtop;	/* top of lctab */

/* for code generator */

EXTERN opsize_t mnsize;		/* 1 if forced byte operand size, else 0 */
EXTERN opcode_t page;
EXTERN opcode_t opcode;
EXTERN opcode_t postb;		/* postbyte, 0 if none */
EXTERN unsigned char pcrflag;	/* OBJ_RMASK set if addressing is PC-relative */

#ifdef I80386

EXTERN opcode_t aprefix;	/* address size prefix or 0 */
EXTERN bool_t asld_compatible;	/* asld compatibility flag */
EXTERN opsize_t defsize;	/* current default size */
EXTERN opsize_t idefsize;	/* initial default size */
EXTERN opcode_t oprefix;	/* operand size prefix or 0 */
EXTERN opcode_t sprefix;	/* segment prefix or 0 */
EXTERN opcode_t sib;		/* scale-index-base byte */

#endif
