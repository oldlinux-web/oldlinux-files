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
 *   dospecial of dvisw package.  (C) 1986 Radical Eye Software.
 *   This routine handles special commands.  Currently the only
 *   special command handled is the landscape special.
 */
#include "structures.h"
#include <ctype.h>
/*
 *   These are the external routines called:
 */
extern shalfword dvibyte() ;
extern void error();
extern void add_header() ;
#ifdef TPIC
extern void setPenSize();
extern void flushPath();
extern void flushDashed();
extern void flushDashed();
extern void addPath();
extern void arc();
extern void flushSpline();
extern void shadeLast();
extern void whitenLast();
extern void blackenLast();
#endif /* TPIC */

/*
 *   And the external variables accessed.
 */
#ifdef DEBUG
extern integer debug_flag;
#endif	/* DEBUG */
extern Boolean landscape ;
extern Boolean prescan ;
extern Boolean orientationset ;
extern char *nextstring, *maxstring ;
/*
 *   Now our routine.  We get the number of bytes specified and place them
 *   into the string buffer, and then parse it for alpha.  This could and
 *   should be improved later.
 */
void
dospecial(numbytes)
	integer numbytes ;
{
   register char *p=nextstring ;
   register int i ;

   if (nextstring + numbytes > maxstring)
      error("! out of string space in dospecial") ;
   for (i=numbytes; i>0; i--)
      *p++ = dvibyte() ;
   *p = 0 ;
   p = nextstring ;
#ifdef DEBUG
   if (dd(D_SPECIAL))
	fprintf(stderr,"Processing special: %s\n", p) ;
#endif
   while (*p == ' ') p++ ;
   if (strncmp(p, "landscape", 9)==0) {
      if (prescan) {
         if (orientationset)
            error("you specified landscape too late.") ;
         else
            landscape = 1 ;
      }
   } else if (strncmp(p, "header", 6) == 0) {
      char *q ;
      p += 6 ;
      while ((*p <= ' ' || *p == '=' || *p == ')') && *p != 0)
         p++ ;
      q = p ;
      p = p + strlen(p) - 1 ;
      if (*p == ')') {
         p-- ;
         while (*p <= ' ')
            p-- ;
         p[1] = 0 ;
      }
      add_header(q) ;
   }
#ifdef TPIC
   else if (strncmp(p, "pn ", 3) == 0) {if (!prescan) setPenSize(p+2);}
   else if (strncmp(p, "fp",2) == 0) {if (!prescan) flushPath();}
   else if (strncmp(p, "da ", 3) == 0) {if (!prescan) flushDashed(p+2, 0);}
   else if (strncmp(p, "dt ", 3) == 0) {if (!prescan) flushDashed(p+2, 1);}
   else if (strncmp(p, "pa ", 3) == 0) {if (!prescan) addPath(p+2);}
   else if (strncmp(p, "ar ", 3) == 0) {if (!prescan) arc(p+2);}
   else if (strncmp(p, "sp", 2) == 0) {if (!prescan) flushSpline();}
   else if (strncmp(p, "sh", 2) == 0) {if (!prescan) shadeLast();}
   else if (strncmp(p, "wh", 2) == 0) {if (!prescan) whitenLast();}
   else if (strncmp(p, "bk", 2) == 0) {if (!prescan) blackenLast();}
   else if (strncmp(p, "tx ", 3) == 0)
      {error("\\special texture command -- ignored");}
#endif /* TPIC */
#ifdef PSFIG
   else {
     void DoPsFigSpecial();

     if (!prescan) {
       DoPsFigSpecial(p, numbytes);
     }
   }
#else
   else {
      error("didn't understand special argument:") ;
      fprintf(stderr,"%s\n", nextstring) ;
   }
#endif /* PSFIG */
}

/*	Following code implements pfig specials */

extern shalfword hh, vv ;
#define TRUE 1
#define FALSE 0

#define STRSIZE 256

typedef enum {
  None, String, Integer, Number, Dimension
  } ValTyp;

typedef struct {
  char *Key;		/* the keyword string */
  char *Val;		/* the value string */
  ValTyp vt;		/* the value type */
  union {			/* the decoded value */
    int i;
    float n;
  } v;
} KeyWord;

typedef struct {
  char *Entry;
  ValTyp Type;
} KeyDesc;

#define PSFILE 0
KeyDesc KeyTab[] = {{"psfile", String},
		      {"hsize", Dimension},
		      {"vsize", Dimension},
		      {"hoffset", Dimension},
		      {"voffset", Dimension},
		      {"hscale", Number},
		      {"vscale", Number}};

#define NKEYS (sizeof(KeyTab)/sizeof(KeyTab[0]))



/*
 * extract first keyword-value pair from string (value part may be null)
 * return pointer to remainder of string return NULL if none found 
 */

char KeyStr[STRSIZE];
char ValStr[STRSIZE];

char *
  GetKeyStr (str, kw)
char *str;
KeyWord *kw;
{
  char *s, *k, *v, t;
  
  if (!str)
    return (NULL);
  
  for (s = str; *s == ' '; s++);	/* skip over blanks */
  if (*s == '\0')
    return (NULL);
  
  for (k = KeyStr;	/* extract keyword portion */
       *s != ' ' && *s != '\0' && *s != '=';
       *k++ = *s++);
  *k = '\0';
  kw->Key = KeyStr;
  kw->Val = v = NULL;
  kw->vt = None;
  
  for (; *s == ' '; s++);	/* skip over blanks */
  if (*s != '=')		/* look for "=" */
    return (s);
  
  for (s++; *s == ' '; s++);	/* skip over blanks */
  if (*s == '\'' || *s == '\"')	/* get string delimiter */
    t = *s++;
  else
    t = ' ';
  for (v = ValStr;	/* copy value portion up to delim */
       *s != t && *s != '\0';
       *v++ = *s++);
  if (t != ' ' && *s == t)
    s++;
  *v = '\0';
  kw->Val = ValStr;
  kw->vt = String;
  
  return (s);
}


/*
 * get next keyword-value pair decode value according to table entry 
 */
int 
  GetKeyVal (kw, tab, nt, tno)
KeyWord *kw;
KeyDesc tab[];
int nt;
int *tno;
{
  int i;
  char c = '\0';
  
  *tno = -1;
  
  for (i = 0; i < nt; i++)
    if (IsSame (kw->Key, tab[i].Entry)) {
      *tno = i;
      switch (tab[i].Type) {
      case None:
	if (kw->vt != None)
	  return (FALSE);
	break;
      case String:
	if (kw->vt != String)
	  return (FALSE);
	break;
      case Integer:
	if (kw->vt != String)
	  return (FALSE);
	if (sscanf (kw->Val, "%d%c", &(kw->v.i), &c) != 1
	    || c != '\0')
	  return (FALSE);
	break;
      case Number:
      case Dimension:
	if (kw->vt != String)
	  return (FALSE);
	if (sscanf (kw->Val, "%f%c", &(kw->v.n), &c) != 1
	    || c != '\0')
	  return (FALSE);
	break;
      }
      kw->vt = tab[i].Type;
      return (TRUE);
    }
  return (TRUE);
}

int
IsSame (a, b)			/* compare strings, ignore case */
char *a, *b;
{
  for (; *a != '\0';)
    if (tolower (*a++) != tolower (*b++))
      return (FALSE);
  return (*a == *b ? TRUE : FALSE);
}


void
SetPosn (x, y)			/* output a positioning command */
int x, y;
{
  hh = x;
  vv = y;
  hvpos();
}

void
DoPsFigSpecial (str, n)		/* interpret a \special command, made up of
					 * keyword=value pairs */
char *str;
int n;
{
  char spbuf[STRSIZE];
  char *sf = NULL;
  KeyWord k;
  int i;
  
  char outfp[STRSIZE];
  char warn[STRSIZE];
  
  str[n] = '\0';
  spbuf[0] = '\0';
  
  SetPosn (hh, vv);
  /* check for psfig "ps:" prefix */
  if (strncmp(str, "ps:", 3) == 0) {
    if (strncmp(str, "ps::[begin]", 11) == 0) {
      sprintf(outfp, "%s\n", &str[11]);
      cmdout(outfp);
    }
    else if (strncmp(str, "ps::[end]", 9) == 0) {
      sprintf (outfp, "%s\n", &str[9]);
      cmdout(outfp);
    }
    else if (strncmp(str, "ps: plotfile ", 13) == 0) {
      str += 13;
      for (sf = str; *sf && *sf != ' '; sf++) ;
      *sf = '\0';
      copyfile (str);
    } else if (strncmp(str, "ps::", 4) == 0) {
      sprintf (outfp, "%s\n", &str[4]);
      cmdout(outfp);
    }
    else {
      sprintf (outfp, "%s\n", &str[3]);
      cmdout(outfp);
    }
    return;
  }
  else if (strncmp(str,"ln03:", 5) == 0) {
    /* for changebar style -- just pass it through */
    cmdout(str);
  }
  else {
    cmdout("@beginspecial\n");
    
    while ((str = GetKeyStr (str, &k)) != NULL) {
      /* get all keyword-value pairs */
      /* for compatibility, single words are taken as file names */
      
      if (k.vt == None && access (k.Key, 0) == 0) {
	if (sf) {
	  sprintf (warn,
		   "  More than one \\special file name given. %s ignored",
		   sf);
	  error(warn);
	}
	(void) strcpy (spbuf, k.Key);
	sf = spbuf;
	
      } else if (GetKeyVal (&k, KeyTab, NKEYS, &i) && i != -1) {
	if (i == PSFILE) {
	  if (sf) {
	    sprintf(warn, "  More than one \\special file name given. %s ignored", sf);
	    error(warn);
	  }
	  (void) strcpy (spbuf, k.Val);
	  sf = spbuf;
	} else	/* the keywords are simply output as PS
		 * procedure calls */
	  {
	    sprintf (outfp, "%f @%s\n", k.v.n, KeyTab[i].Entry);
	    cmdout(outfp);
	  }
      } else {
	sprintf(warn, "  Invalid keyword or value in \\special - \"%s\" ignored", k.Key);
	error(warn);
      }
    }
    
    cmdout ("@setspecial\n");
    
    if (sf) {
      copyfile (sf);
    }
    else {
      sprintf(warn, "  No special file name provided.");
      error(warn);
    }
    
    cmdout ("@endspecial\n");
  }
}
