/*
 *   bin86/ld/config.h
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* config.h - configuration for linker */

/* one of these target processors must be defined */

#undef  I8086			/* Intel 8086 */
#define I80386			/* Intel 80386 */
#undef  MC6809			/* Motorola 6809 */

/* one of these target operating systems must be defined */

#undef  EDOS			/* generate EDOS executable */
#define MINIX			/* generate Minix executable */

/* these may need to be defined to suit the source processor */

#undef  S_ALIGNMENT 4		/* source memory alignment, power of 2 */
				/* don't use for 8 bit processors */
				/* don't use even for 80386 - overhead for */
				/* alignment cancels improved access */

/* these should be defined if they are supported by the source compiler */

#undef  PROTO			/* compiler handles prototypes */

/* these must be defined to suit the source libraries */

#define CREAT_PERMS 0666	/* permissions for creat() */
#define EXEC_PERMS  0111	/* extra permissions to set for executable */
