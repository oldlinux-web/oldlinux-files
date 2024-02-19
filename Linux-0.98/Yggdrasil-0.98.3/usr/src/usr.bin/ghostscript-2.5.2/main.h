/* Copyright (C) 1992 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */

/* main.h */
/* Interface between gs.c and gsmain.c */

/* Exported data from gsmain.c */
extern char *gs_lib_env_path;
extern uint gs_memory_chunk_size;
extern int gs_user_errors;

/* Exported procedures from gsmain.c */
extern	void	gs_init0(P4(FILE *, FILE *, FILE *, int)),
		gs_init1(P0()),
		gs_init2(P0());
extern	void	gs_add_lib_path(P1(const char *));
/* The value returned by gs_run_file and gs_run_string is */
/* 0 if the interpreter ran to completion, e_Quit for a normal quit, */
/* e_Fatal for a non-zero quit or a fatal error. */
/* e_Fatal stores the exit code in the third argument. */
extern	int	gs_run_file(P4(const char *fname, int user_errors, int *pexit_code, ref *perror_object)),
		gs_run_string(P4(const char *str, int user_errors, int *pexit_code, ref *perror_object));
extern	void	gs_debug_dump_stack(P2(int code, ref *perror_object));
extern	void	gs_finit(P0()),
		gs_exit(P1(int));
/* Direct interface to the interpreter. */
/* Clients do not normally use this. */
extern	int	gs_interpret(P4(ref *pref, int user_errors, int *pexit_code, ref *perror_object));
