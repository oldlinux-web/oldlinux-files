/* @(#) assert.h 2.1 87/12/25 12:21:32 */

/*
The contents of this file are hereby released to the public domain.

                           -- Rahul Dhesi 1991/07/04

Defines a macro assert() that causes an assertion error if the assertion
fails.

Conditional compilation:

   If NDEBUG is defined then
      assert() is defined as null so all assertions vanish
   else
		if __FILE__ and __LINE__ are defined then
         assertions print message including filename and line number
      else
         assertions print a message but not the filename and line number
      endif
   endif
*/

#ifdef NDEBUG
# define assert(E)
#else

#undef LINE_FILE

#ifdef __LINE__
# ifdef __FILE__
#  define LINE_FILE
# endif
#endif

#ifdef LINE_FILE
# undef LINE_FILE
# define assert(E) \
   { if (!(E)) \
      prterror ('w',"Assertion error in %s:%d.\n", __FILE__, __LINE__); \
   }
#else
# define assert(E) \
   { if (!(E)) \
      prterror ('w', "Assertion error.\n"); \
   }
#endif
#endif /* NDEBUG */
