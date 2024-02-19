/*
 *      Returns true if string s matches pattern p.
 */

#include <ctype.h>

int
patmatch(p, s, f)
register char  *p;                                       /* pattern */
register char  *s;                               /* string to match */
int            f;                            /* flag for case force */
{
   char  pc;                     /* a single character from pattern */

   while (pc = ((f && islower(*p)) ? toupper(*p++) : *p++))
      {
      if (pc == '*')
         {
         do {                    /* look for match till s exhausted */
            if (patmatch (p, s, f))
                  return (1);
            } while (*s++);
         return (0);
         }
      else
         if (*s == 0)
            return (0);                       /* s exhausted, p not */
         else
            if (pc == '?')
               s++;                       /* matches all, just bump */
            else
               if (pc != ((f && islower(*s)) ? toupper(*s++) : *s++))
                  return (0);
      }
   return (!*s);            /* p exhausted, ret true if s exhausted */
}
