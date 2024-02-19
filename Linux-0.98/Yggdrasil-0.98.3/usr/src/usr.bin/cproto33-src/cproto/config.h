/* $Id: config.h 3.3 92/04/04 13:59:06 cthuang Exp $
 *
 * cproto configuration and system dependencies
 */

/* maximum include file nesting */
#ifndef MAX_INC_DEPTH
#define MAX_INC_DEPTH 15
#endif

/* maximum number of include directories */
#ifndef MAX_INC_DIR
#define MAX_INC_DIR 15
#endif

/* maximum text buffer size */
#ifndef MAX_TEXT_SIZE
#define MAX_TEXT_SIZE 256
#endif

/* Borland C predefines __MSDOS__ */
#ifdef __MSDOS__
#ifndef MSDOS
#define MSDOS
#endif
#endif

#ifdef MSDOS
#include <malloc.h>
#include <stdlib.h>
#else
extern char *malloc();
extern char *getenv();
#endif

#ifdef BSD
#include <strings.h>
#define strchr index
#define strrchr rindex
#else
#include <string.h>
#endif

#ifndef MSDOS
extern char *strstr();
#endif

/* C preprocessor */
#ifdef TURBO_CPP
#define CPP "cpp -P-"
#endif

#ifndef MSDOS
#ifndef CPP
#define CPP "/lib/cpp"
#endif
#endif
