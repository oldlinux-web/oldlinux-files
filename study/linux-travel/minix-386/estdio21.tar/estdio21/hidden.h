/*				h i d d e n . h
 *
 * (C) Copyright C E Chew
 *
 * Feel free to copy, use and distribute this software provided:
 *
 *	1. you do not pretend that you wrote it
 *	2. you leave this copyright notice intact.
 *
 * This file hides the standard names in the implementor's namespace.
 */

#ifndef	HIDDEN_H
#define HIDDEN_H

#define atexit		_atexit_
#define ctermid		_ctermid_
#define cuserid		_cuserid_
#define exit		_exit_
#define fclose		_fclose_
#define fdopen		_fdopen_
#define fflush		_fflush_
#define fgetc		_fgetc_
#define fgets		_fgets_
#define fopen		_fopen_
#define fprintf		_fprintf_
#define fputc		_fputc_
#define fputs		_fputs_
#define fread		_fread_
#define freopen		_freopen_
#define fscanf		_fscanf_
#define fseek		_fseek_
#define ftell		_ftell_
#define fwrite		_fwrite_
#define gets		_gets_
#define getw		_getw_
#define perror		_perror_
#define printf		_printf_
#define puts		_puts_
#define putw		_putw_
#define rewind		_rewind_
#define scanf		_scanf_
#define setbuf		_setbuf_
#define setvbuf		_setvbuf_
#define sprintf		_sprintf_
#define sscanf		_sscanf_
#define tmpfile		_tmpfile_
#define tmpnam		_tmpnam_
#define ungetc		_ungetc_
#define vfprintf	_vfprintf_
#define vprintf		_vprintf_
#define vsprintf	_vsprintf_
#endif

/* Functions implemented as macros
 *
 * Each function that is implemented as a macro in stdio.h has its name
 * left unchanged, otherwise this will cause redefinition problems when
 * stdio.h is included.
 *
 * When the shadow function for each macro is declared, this file should
 * be re-included with MACRO_SHADOW defined. This will cause the macros
 * to be removed and the function name hiding to come into effect. Names
 * will only be hidden if HIDDEN is defined.
 */

#ifndef	HIDDEN
# undef	MACRO_SHADOW
#endif

#undef clearerr
#ifdef MACRO_SHADOW
#define clearerr	_clearerr_
#endif

#undef feof
#ifdef MACRO_SHADOW
#define feof		_feof_
#endif

#undef ferror
#ifdef MACRO_SHADOW
#define ferror		_ferror_
#endif

#undef fileno
#ifdef MACRO_SHADOW
#define fileno		_fileno_
#endif

#undef getc
#ifdef MACRO_SHADOW
#define getc		_getc_
#endif

#undef getchar
#ifdef MACRO_SHADOW
#define getchar		_getchar_
#endif

#undef putc
#ifdef MACRO_SHADOW
#define putc		_putc_
#endif

#undef putchar
#ifdef MACRO_SHADOW
#define putchar		_putchar_
#endif

#undef remove
#ifdef MACRO_SHADOW
#define remove		_remove_
#endif
