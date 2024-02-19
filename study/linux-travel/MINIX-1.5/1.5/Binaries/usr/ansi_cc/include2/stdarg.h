/* The <stdarg.h> header is ANSI's way to handle variable numbers of params.
 * Some programming languages require a function that is declared with n
 * parameters to be called with n parameters.  C does not.  A function may
 * called with more parameters than it is declared with.  The well-known
 * printf function, for example, may have arbitrarily many parameters.
 * The question arises how one can access all the parameters in a portable
 * way.  The C standard defines three macros that programs can use to
 * advance through the parameter list.  The definition of these macros for
 * MINIX are given in this file.  The three macros are:
 *
 *	va_start(ap, parmN)	prepare to access parameters
 *	va_arg(ap, type)	get next parameter value and type
 *	va_end(ap)		access is finished
 */

#ifndef _STDARG_H
#define _STDARG_H

typedef	char *va_list;

#define __vasz(x)		((sizeof(x)+sizeof(int)-1) & ~(sizeof(int) -1))

#define va_start(ap, parmN)	((ap) = (va_list)&parmN + __vasz(parmN))
#define va_arg(ap, type)      \
  (*((type *)((va_list)((ap) = (void *)((va_list)(ap) + __vasz(type))) \
						    - __vasz(type))))
#define va_end(ap)

#endif /* _STDARG_H */
