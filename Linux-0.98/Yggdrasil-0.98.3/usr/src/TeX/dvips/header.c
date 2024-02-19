/*
 *   This routine handles the PostScript prologues which might
 *   be included through:
 *
 *      - Default
 *      - Use of PostScript fonts
 *      - Specific inclusion through specials, etc.
 *      - Use of graphic specials which require them.
 *
 *   Things are real simple.  We build a linked list of headers to
 *   include.  Then, when the time comes, we simply copy those
 *   headers down.
 */
#include "structures.h"
struct header_list {
   char *name ;
   struct header_list *next ;
} *header_head ;
/*
 *   The external routines we use.
 */
char *malloc() ;
char *newstring() ;
void error() ;
void copyfile() ;
/*
 *   This routine is responsible for adding a header file.
 */
void
add_header(s)
   char *s ;
{
   struct header_list *p, *q ;

   for (p = header_head ; p != NULL; p = p->next)
      if (strcmp(p->name, s)==0)
         return ;
   q = (struct header_list *)malloc((unsigned)sizeof(struct header_list)) ;
   if (q==NULL)
      error("! out of memory") ;
   q->next = NULL ;
   q->name = newstring(s) ;
   if (header_head == NULL)
      header_head = q ;
   else {
      for (p=header_head; p->next != NULL; p = p->next) ;
      p->next = q ;
   }
}
/*
 *   This routine actually sends the headers.
 */
void
send_headers() {
   struct header_list *p ;

   for (p=header_head; p!=NULL; p = p->next)
      copyfile(p->name) ;
}
