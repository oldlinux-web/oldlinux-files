/* argpars.c - argument parser for command line paramters */

#define INT_ARG 0
#define BOOL_ARG 1
#define CHAR_ARG 2
#define STRING_ARG 3

typedef struct
{
  char arg, type;
  void *variable;
} ARG;

/* set an argument.  argp points to the entry in the argument table, which
   fits to *linep.  returns linep which points after the processed argument.
*/
char *setarg( argp, linep ) ARG *argp; char *linep;
{
  ++linep;

  switch( argp->type )
  {
    case INT_ARG:
    {
      *(int*)argp->variable=atoi(linep);
      while (*linep) linep++;
      break;
    }

    case BOOL_ARG: { *(int*)argp->variable = 1; break; }

    case CHAR_ARG: { *(char*)argp->variable = *linep++; break; }

    case STRING_ARG:
    {
      char *s;

      s = (char*) argp->variable;
      while (*linep) *s++ = *linep++;
      *s='\0';
      break;
    }
  }
  return(linep);
}

/*----------------------------------------------------------
  Liefert einen Zeiger auf den Eintrag der Argumententabelle
  der mit c korrespondiert (oder Null, falls c nicht in der
  Tabelle ist).
*/
static ARG *findarg(c, tabp, tabsize) char c; ARG *tabp; int tabsize;
{
  for (; --tabsize >= 0 ; tabp++ )
  if (tabp->arg == c ) return tabp;

  return NULL;
}

/*----------------------------------------------------------
  Bearbeite Kommandozeilen-Argumente. Hole alle Kommando-
  zeilenschalter aus argv. Liefere ein neues argc.
*/
int argparse(argc,argv,tabp,tabsize) int argc; char **argv; ARG *tabp; int tabsize;
{
  int nargc;
  char **nargv, *p;
  ARG   *argp;

  nargc = 1 ;
  for(nargv = ++argv ; --argc > 0 ; argv++ )
  {
    if (**argv=='-' && (argp = findarg(*(p=argv[0]+1), tabp, tabsize)))
      do 
        if (*(p=setarg(argp,p))) argp=findarg(*p,tabp,tabsize);
      while (*p);
    else
    {
      *nargv++ = *argv ;
      nargc++;
    }
  }
  return nargc ;
}

/* Ende von argpars.c */
