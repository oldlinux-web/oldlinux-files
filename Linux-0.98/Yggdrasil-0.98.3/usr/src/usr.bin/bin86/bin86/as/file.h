/*
 *   bin86/as/file.h
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* file.h - global variables involving files for assembler */

EXTERN fd_t binfil;		/* binary output file (0 = memory) */
EXTERN char *filnamptr;		/* file name pointer */
EXTERN fd_t infil0;		/* initial input file */
EXTERN fd_t infil;		/* current input file (stacked, 0 = memory) */
EXTERN unsigned char infiln;	/* innum when file was opened */
EXTERN unsigned char innum;	/* counter for calls to opem */
EXTERN fd_t lstfil;		/* list output file (0 = standard) */
EXTERN fd_t objfil;		/* object output file */
EXTERN fd_t symfil;		/* symbol table output file */
EXTERN char *truefilename;	/* in case actual source name is a tmpname */
