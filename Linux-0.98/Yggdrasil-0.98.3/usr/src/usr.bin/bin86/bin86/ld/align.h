/*
 *   bin86/ld/align.h
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* align.h - memory alignment requirements for linker */

#ifndef S_ALIGNMENT
# define align(x)
#else
# define align(x) ((x) = ((int) (x) + (S_ALIGNMENT-1)) & ~(S_ALIGNMENT-1))
				/* assumes sizeof(int) == sizeof(char *) */
#endif
