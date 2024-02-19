/*
 *   bin86/as/flag.h
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* flag.h - global structured-flag variables for assembler */

EXTERN struct flags_s list;	/* listing on/off */
EXTERN struct flags_s maclist;	/* list macros on/off */
EXTERN struct flags_s warn;	/* warnings on/off */
