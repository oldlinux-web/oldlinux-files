/*
 *   This is dvips, a freely redistributable PostScript driver
 *   for dvi files.  It is (C) Copyright 1987 by Tomas Rokicki.
 *   You may modify and use this program to your heart's content,
 *   so long as you send modifications to Tomas Rokicki.  It can
 *   be included in any distribution, commercial or otherwise, so
 *   long as the banner string defined in structures.h is not
 *   modified (except for the version number) and this banner is
 *   printed on program invocation, or can be printed on program
 *   invocation with the -? option.
 */
/*
 *   This is the prescan main routine.  (C) 1986 Radical Eye Software.
 */
#include "structures.h"
/*
 *   These are all the external routines it calls:
 */
extern void error() ;
extern shalfword dvibyte() ;
extern integer signedquad() ;
extern int skipnop() ;
extern void skipover() ;
extern short scanpage() ;
extern void skippage() ;
/*
 *   These are the globals it accesses.
 */
#ifdef DEBUG
extern integer debug_flag;
#endif	/* DEBUG */
extern fontdesctype *TeXfonts[] ;
extern real conv ;
extern integer curpos ;
extern integer firstpage ;
extern Boolean notfirst ;
extern integer fontmem ;
extern integer pagecount ;
extern integer pagenum ;
extern integer maxpages ;
extern sectiontype *sections ;
extern FILE *dvifile ;
extern integer mag ;
extern integer swmem ;
extern int quiet ;
extern Boolean reverse ;
/*
 *   We declare the type malloc returns.
 */
char *malloc() ;
/*
 *   This routine handles the processing of the preamble in the dvi file.
 */
void
readpreamble()
{
   register integer num, den;
   register int i ;

   if (dvibyte()!=247) error("! First byte not bop") ;
   if (dvibyte()!=2) error("! Revision of dvi file not 2") ;
   num = signedquad() ;
   den = signedquad() ;
   mag = signedquad() ;
   conv = (float) num * DPI * (float) mag / ( den * 254000000.0 ) ; 
   if (! quiet) {
      fprintf(stderr, "'") ;
      for(i=dvibyte();i>0;i--) (void)putc(dvibyte(), stderr) ;
      fprintf(stderr, "'\n") ;
   } else {
      for (i=dvibyte();i>0;i--) (void)dvibyte() ;
   }
}

/*
 *   Finally, here's our main prescan routine.
 */
void
prescanpages()
{
   register int cmd ;
   short ret ;
   register integer thispageloc, thissecloc ;
   register shalfword f, c ;

   readpreamble() ;
/*
 *   Now we look for the first page to process.  If we get to the end of
 *   the file before the page, we complain (fatally).
 */
   while (1) {
      cmd = skipnop() ;
      if (cmd==248)
         error("! end of document before first specified page") ;
      if (cmd!=139)
         error("! expected bop") ;
      thispageloc = curpos ;
#ifdef DEBUG
      if (dd(D_PAGE))
#ifdef SHORTINT
      fprintf(stderr,"bop at %ld\n", thispageloc) ;
#else	/* ~SHORTINT */
      fprintf(stderr,"bop at %d\n", thispageloc) ;
#endif	/* ~SHORTINT */
#endif	/* DEBUG */
      pagenum = signedquad() ;
      skipover(40) ;
      if (notfirst && pagenum != firstpage)
         skippage() ;
      else
         break ;
   }
/*
 *   Here we scan for each of the sections.  First we initialize some of
 *   the variables we need.
 */
   while (maxpages > 0 && cmd != 248) {
      for (f=255; f>=0; f--)
         if (TeXfonts[f]!=NULL) {
            if (TeXfonts[f]->loaded==1 && TeXfonts[f]->resfont==NULL)
               for (c=255; c>=0; c--)
                  TeXfonts[f]->chardesc[c].flags &= EXISTS ;
            TeXfonts[f]->defined = 0 ;
         }
      fontmem = swmem ;
      pagecount = 0 ;
      (void)fseek(dvifile, (long)thispageloc+44, 0) ;
      curpos = thispageloc+44 ;
/*
 *   Now we have the loop which actually scans the pages.  scanpage returns
 *   true if the page scans okay; it returns false if the memory runs out.
 *   After each page, we mark the characters seen on that page as seen for
 *   this section so they are downloaded.
 */
      thissecloc = thispageloc ;
      while (maxpages>0 && (ret=scanpage())) {
         if (reverse)
            thissecloc = thispageloc ;
         pagecount++ ;
         maxpages-- ;
         for (f=255; f>=0; f--)
            if (TeXfonts[f]!=NULL && TeXfonts[f]->loaded==1
                    && TeXfonts[f]->resfont==NULL)
               for (c=255; c>=0; c--)
                  if (TeXfonts[f]->chardesc[c].flags & THISPAGE)
                     TeXfonts[f]->chardesc[c].flags = PREVPAGE |
                             (TeXfonts[f]->chardesc[c].flags & EXISTS) ;
         cmd=skipnop() ;
         if (cmd==248) break ;
         if (cmd!=139)
            error("! expected bop") ;
         thispageloc = curpos ;
#ifdef DEBUG
	 if (dd(D_PAGE))
#ifdef SHORTINT
         fprintf(stderr,"bop at %ld\n", thispageloc) ;
#else	/* ~SHORTINT */
         fprintf(stderr,"bop at %d\n", thispageloc) ;
#endif	/* ~SHORTINT */
#endif	/* DEBUG */
         pagenum = signedquad() ;
         skipover(40) ;
         if (ret==2) break ;
      }
/*
 *   Now we have reached the end of a section for some reason.
 *   If there are any pages, we save the pagecount, section location,
 *   and continue.
 */
      if (pagecount>0) {
         register int fc = 0 ;
         register sectiontype *sp ;
         register charusetype *cp ;

         for (f=255; f>=0; f--)
            if (TeXfonts[f]!=NULL && TeXfonts[f]->loaded!=0)
               fc++ ;
         sp = (sectiontype *)malloc((unsigned int)(sizeof(sectiontype) + 
            fc * sizeof(charusetype) + 2)) ;
         if (sp==NULL)
            error("! out of memory") ;
         sp->bos = thissecloc ;
         if (reverse) {
            sp->next = sections ;
            sections = sp ;
         } else {
            register sectiontype *p ;

            sp->next = NULL ;
            if (sections == NULL)
               sections = sp ;
            else {
               for (p=sections; p->next != NULL; p = p->next) ;
               p->next = sp ;
            }
         }
         sp->numpages = pagecount ;
#ifdef DEBUG
	if (dd(D_PAGE))
#ifdef SHORTINT
         fprintf(stderr,"Have a section: %ld pages at %ld fontmem %ld\n", 
#else	/* ~SHORTINT */
         fprintf(stderr,"Have a section: %d pages at %d fontmem %d\n", 
#endif	/* ~SHORTINT */
             pagecount, thissecloc, fontmem) ;
#endif	/* DEBUG */
         cp = (charusetype *) (sp + 1) ;
         for (f=255; f>=0; f--)
            if (TeXfonts[f]!=NULL && TeXfonts[f]->loaded!=0) {
               register halfword b, bit ;

               cp->fontnum = f ;
               if (TeXfonts[f]->resfont == NULL) {
                  c = 0 ;
                  for (b=0; b<16; b++) {
                     cp->bitmap[b] = 0 ;
                     for (bit=32768; bit!=0; bit>>=1) {
                        if (TeXfonts[f]->chardesc[c].flags & PREVPAGE)
                           cp->bitmap[b] |= bit ;
                     c++ ;
                     }
                  }
               }
               cp++ ;
            }
         cp->fontnum = 256 ;
      }
   }
}
