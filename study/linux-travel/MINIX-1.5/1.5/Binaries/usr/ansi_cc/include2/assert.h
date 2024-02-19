/* The <assert.h> header contains a macro called "assert" that allows 
 * programmers to put assertions in the code.  These assertions can be verified
 * at run time.  If an assertion fails, an error message is printed.  
 * Assertion checking can be disabled by adding the statement
 *
 *	#define NDEBUG
 *
 * to the program before the 
 *
 *	#include <assert.h>
 *
 * statement.
 */

#ifdef assert
#undef assert			/* make this file idempotent */
#endif

#ifndef	_ANSI_H
#include	<ansi.h>
#endif


#ifdef NDEBUG
/* Debugging disabled -- do not evaluate assertions. */
#define assert(expr)  ((void) 0)
#else
/* Debugging enabled -- verify assertions at run time. */
#if	_ANSI
_PROTOTYPE( _VOID __bad_assertion, (const char *__expr, const char *__file, int __line) );
#define assert(expr) ((void) ((expr) ? (void)0 : __bad_assertion( #expr, __FILE__,  __LINE__)))
#else
#define assert(expr) ((void) ((expr) ? 0 : __assert( __FILE__,  __LINE__)))
#endif
#endif
