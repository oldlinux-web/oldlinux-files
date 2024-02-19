 /*
 ** @(#) dis.h, Ver. 2.1 created 00:00:00 87/09/01
 */

 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  *                                                         *
  *  Copyright (C) 1987 G. M. Harding, all rights reserved  *
  *                                                         *
  * Permission to copy and  redistribute is hereby granted, *
  * provided full source code,  with all copyright notices, *
  * accompanies any redistribution.                         *
  *                                                         *
  * This file contains declarations and definitions used by *
  * the 8088 disassembler program. The program was designed *
  * for execution on a machine of its own type (i.e., it is *
  * not designed as a cross-disassembler);  consequently, A *
  * SIXTEEN-BIT INTEGER SIZE HAS BEEN ASSUMED. This assump- *
  * tion is not particularly important,  however, except in *
  * the machine-specific  portions of the code  (i.e.,  the *
  * handler  routines and the optab[] array).  It should be *
  * possible to override this assumption,  for execution on *
  * 32-bit machines,  by use of a  pre-processor  directive *
  * (see below); however, this has not been tested.         *
  *                                                         *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <sys/types.h>
#include <a.out.h>      /* Object file format definitions   */
#include <fcntl.h>      /* System file-control definitions  */
#include <unistd.h>
#include <string.h>
#include <stdio.h>      /* System standard I/O definitions  */

#if i8086 || i8088      /* For CPU's with 16-bit integers   */
#undef int
#else                   /* Defaults (for 32-bit CPU types)  */
#define int short
#endif

#define MAXSYM 1500     /* Maximum entries in symbol table  */

extern struct nlist     /* Array to hold the symbol table   */
   symtab[MAXSYM];

extern struct reloc     /* Array to hold relocation table   */
   relo[MAXSYM];

extern int symptr;      /* Index into the symtab[] array    */

extern int relptr;      /* Index into the relo[] array      */

struct opcode           /* Format for opcode data records   */
   {
   char     *text;            /* Pointer to mnemonic text   */
   void     (*func)();        /* Pointer to handler routine */
   unsigned min;              /* Minimum # of object bytes  */
   unsigned max;              /* Maximum # of object bytes  */
   };

extern struct opcode    /* Array to hold the opcode table   */
  optab[256];

 /*
   +---------------------------------------------
   | The following  functions are the specialized
   | handlers for each opcode group. They are, of
   | course, highly MACHINE-SPECIFIC.  Each entry
   | in the opcode[]  array contains a pointer to
   | one of these  handlers.  The handlers in the
   | first group are in  dishand.c;  those in the
   | second group are in disfp.c.
   +---------------------------------------------
 */

extern void dfhand(),   /* Default handler routine          */
            sbhand(),   /* Single-byte handler              */
            aohand(),   /* Arithmetic-op handler            */
            sjhand(),   /* Short-jump handler               */
            imhand(),   /* Immediate-operand handler        */
            mvhand(),   /* Simple move handler              */
            mshand(),   /* Segreg-move handler              */
            pohand(),   /* Pop memory/reg handler           */
            cihand(),   /* Intersegment call handler        */
            mihand(),   /* Immediate-move handler           */
            mqhand(),   /* Quick-move handler               */
            tqhand(),   /* Quick-test handler               */
            rehand(),   /* Return handler                   */
            mmhand(),   /* Move-to-memory handler           */
            srhand(),   /* Shift and rotate handler         */
            aahand(),   /* ASCII-adjust handler             */
            iohand(),   /* Immediate port I/O handler       */
            ljhand(),   /* Long-jump handler                */
            mahand(),   /* Misc. arithmetic handler         */
            mjhand();   /* Miscellaneous jump handler       */

extern void eshand(),   /* Bus-escape opcode handler        */
            fphand(),   /* Floating-point handler           */
            inhand();   /* Interrupt-opcode handler         */

extern char *REGS[];    /* Table of register names          */

extern char *REGS0[];   /* Mode 0 register name table       */

extern char *REGS1[];   /* Mode 1 register name table       */

#define AL REGS[0]      /* CPU register manifests           */
#define CL REGS[1]
#define DL REGS[2]
#define BL REGS[3]
#define AH REGS[4]
#define CH REGS[5]
#define DH REGS[6]
#define BH REGS[7]
#define AX REGS[8]
#define CX REGS[9]
#define DX REGS[10]
#define BX REGS[11]
#define SP REGS[12]
#define BP REGS[13]
#define SI REGS[14]
#define DI REGS[15]
#define ES REGS[16]
#define CS REGS[17]
#define SS REGS[18]
#define DS REGS[19]
#define BX_SI REGS0[0]
#define BX_DI REGS0[1]
#define BP_SI REGS0[2]
#define BP_DI REGS0[3]

extern int symrank[6][6];     /* Symbol type/rank matrix    */

extern unsigned long PC;      /* Current program counter    */

extern int segflg;      /* Flag: segment override in effect */

extern int objflg;      /* Flag: output object as a comment */

#define OBJMAX 8        /* Size of the object code buffer   */

extern unsigned char    /* Internal buffer for object code  */
   objbuf[OBJMAX];

extern void objini(),   /* Object-buffer init routine       */
            objout();   /* Object-code output routine       */

extern int objptr;      /* Index into the objbuf[] array    */

extern void badseq();   /* Bad-code-sequence function       */

extern char *getnam();  /* Symbol-name string function      */

extern char *lookup();  /* Symbol-table lookup function     */

extern int lookext();   /* Extern-definition lookup routine */

extern char *mtrans();  /* Interpreter for the mode byte    */

extern void mtrunc();   /* Mode string truncator function   */

extern char ADD[],      /* Opcode family mnemonic strings   */
            OR[],
            ADC[],
            SBB[],
            AND[],
            SUB[],
            XOR[],
            CMP[],
            NOT[],
            NEG[],
            MUL[],
            DIV[],
            MOV[],
            ESC[],
            TEST[],
            AMBIG[];

extern char *OPFAM[];   /* Indexed mnemonic family table    */

extern struct exec HDR; /* Holds the object file's header   */

#define LOOK_ABS 0      /* Arguments to lookup() function   */
#define LOOK_REL 1
#define LOOK_LNG 2

#define TR_STD 0        /* Arguments to mtrans() function   */
#define TR_SEG 8

                        /* Macro for byte input primitive   */
#define FETCH(p) \
   ++PC; p = getchar() & 0xff; objbuf[objptr++] = p

#ifdef OBSOLETE		/* Declarations to use if headers   */
			/* are inadequate.  sprintf() and   */
			/* strlen() may have the wrong type.*/
extern int close();     /* System file-close primitive      */
extern long lseek();    /* System file-position primitive   */
extern int open();      /* System file-open primitive       */
extern int read();      /* System file-read primitive       */
extern char *strcat();  /* Library string-join function     */
extern char *strcpy();  /* Library string-copy function     */
extern int strlen();    /* Library string-length function   */
#endif

/* extern int sprintf();   /* Library string-output function   */
/* extern int printf();    /* Library output-format function   */
/* extern int fprintf();   /* Library file-output function     */
 /* * * * * * * * * * *  END OF  dis.h  * * * * * * * * * * */


