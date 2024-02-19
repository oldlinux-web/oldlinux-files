/*
   This file is the interface to the rest of rc for any locally
   defined addon builtins.  By default there are none.
   The interface consists of the following macros.
  
   ADDON_FUN	A comma-separated list of the function names for the
  		builtins.
  
   ADDON_STR	A comma-separated list of string literals corresponding
  		to the function names in ADDON_FUN.
  
   The addon functions must also have proper prototypes in this file.
   The builtins all have the form:
  
  	void b_NAME(char **av);
  
   Builtins report their exit status using set(TRUE) or set(FALSE).
  
   Example:
  
  	#define ADDON_FUN	b_test, b_printf
  	#define ADDON_STR	"test", "printf"
  	extern void b_test(char **av);
  	extern void b_printf(char **av);
*/

#define ADDON_FUN	/* no addons by default */
#define ADDON_STR	/* no addons by default */

#ifdef	DWS

/*
   This is what DaviD Sanderson (dws@cs.wisc.edu) uses.
*/

#undef	ADDON_FUN
#define ADDON_FUN	b_access, b_test, b_test
#undef	ADDON_STR
#define ADDON_STR	"access", "test", "["

extern void b_access(char **av);
extern void b_test(char **av);

#endif
