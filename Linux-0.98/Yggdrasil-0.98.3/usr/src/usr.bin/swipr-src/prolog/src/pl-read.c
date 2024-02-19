/*  @(#) pl-read.c 1.5.0 (UvA SWI) Jul 30, 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    See ../LICENCE to find out about your rights.
    jan@swi.psy.uva.nl

    Purpose: read/1, 2
*/

#include "pl-incl.h"
#include "pl-ctype.h"
#include <math.h>

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
This module defines the Prolog parser.  Reading a term takes two passes:

	* Reading the term into memory, deleting multiple blanks, comments
	  etc.
	* Parsing this string into a Prolog term.

The separation has two reasons: we can call the  first  part  separately
and  insert  the  read  strings in the history and we can produce better
error messages as the parsed part of the source is available.

The raw reading pass is quite tricky as PCE requires  us  to  allow  for
callbacks  from  C  during  this  process  and the callback might invoke
another read.  Notable raw reading needs to be studied studied once more
as it  takes  about  30%  of  the  entire  compilation  time  and  looks
promissing  for  optimisations.   It  also  could  be  made  a  bit more
readable.

This module is considerably faster when compiled  with  GCC,  using  the
-finline-functions option.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

forwards void	startRead P((void));
forwards void	stopRead P((void));
forwards void	errorWarning P((char *));
forwards void	singletonWarning P((char *));
forwards void	clearBuffer P((void));
forwards void	addToBuffer P((char));
forwards void	delBuffer P((void));
forwards void	extendBeep P((void));
forwards void	extendDeleteEscape P((void));
forwards void	extendDeleteEof P((void));
forwards void	extendReprint P((bool));
forwards Char	getchr P((void));
forwards char *	raw_read2 P((void));
forwards char *	raw_read P((void));

typedef struct token * Token;
typedef struct variable * Variable;

struct token
{ int type;			/* type of token */
  union
  { word prolog;		/* a Prolog value */
    char character;		/* a punctuation character (T_PUNCTUATION) */
    Variable variable;		/* a variable record (T_VARIABLE) */
  } value;			/* value of token */
};

struct variable
{ Word		address;	/* address of variable */
  char *	name;		/* name of variable */
  int		times;		/* number of occurences */
  Variable 	next;		/* next of chain */
};

#define T_FUNCTOR	0	/* name of a functor (atom, followed by '(') */
#define T_NAME		1	/* ordinary name */
#define T_VARIABLE	2	/* variable name */
#define T_VOID		3	/* void variable */
#define T_REAL		4	/* realing point number */
#define T_INTEGER	5	/* integer */
#define T_STRING	6	/* "string" */
#define T_PUNCTUATION	7	/* punctuation character */
#define T_FULLSTOP	8	/* Prolog end of clause */

extern int Input;		/* current input stream (from pl-file.c) */
static char *here;		/* current character */
static char *base;		/* base of clause */
static char *token_start;	/* start of most recent read token */
static struct token token;	/* current token */
static bool unget = FALSE;	/* unget_token() */
static int start_line;		/* starting line of clause */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
The reading function (raw_read()) can  be  called  recursively  via  the
notifier  when  running  under  notifier  based packages (like O_PCE).  To
avoid corruption of the database we push the read buffer rb on  a  stack
and pop in back when finished.  See raw_read() and raw_read2().
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define RBSIZE	512		/* initial size of read buffer */
#define MAX_READ_NESTING 5	/* nesting of read (O_PCE only) */

static
struct read_buffer
{ int	size;			/* current size of read buffer */
  int	left;			/* left space in read buffer */
  char *base;			/* base of read buffer */
  char *here;			/* current position in read buffer */
  int   stream;			/* stream we are reading from */
  FILE *fd;			/* file descriptor we are reading from */
  bool	doExtend;		/* extension mode on? */
} rb;

#if O_PCE
static struct read_buffer rb_stack[MAX_READ_NESTING];
int read_nesting = 0;		/* current nesting level */
#endif O_PCE

void
resetRead()
{ 
#if O_PCE
  read_nesting = 0;
#endif
}

static
void
startRead()
{
#if O_PCE
  if (read_nesting >= MAX_READ_NESTING)
  { warning("Read stack too deeply nested");
    pl_abort();
  }
  rb_stack[read_nesting++] = rb;
  rb = rb_stack[read_nesting];
#endif O_PCE
  rb.doExtend = (Input == 0 && status.notty == FALSE);
  rb.stream = Input;
  rb.fd = checkInput(rb.stream);
}

static void
stopRead()
{
#if O_PCE
  rb = rb_stack[--read_nesting];
  if (read_nesting < 0)
    fatalError("Read stack underflow???");
  if (read_nesting > 0)
    rb.fd = checkInput(rb.stream);
#endif O_PCE
}

		/********************************
		*         ERROR HANDLING        *
		*********************************/

#define syntaxError(what) { errorWarning(what); fail; }

static void
errorWarning(what)
char *what;
{ char c = *token_start;
  
  if (seeingString())
    fprintf(stderr, "\n[WARNING: Syntax error: %s \n", what);
  else
    fprintf(stderr, "\n[WARNING: Syntax error (line %d of %s): %s\n", 
					  start_line, 
					  currentStreamName(), 
					  what);
  *token_start = EOS;
  fprintf(stderr, "%s\n** here **\n", base);  
  if (c != EOS)
  { *token_start = c;
    fprintf(stderr, "%s]\n", token_start);
  }
}

static void
singletonWarning(var)
char *var;
{ fprintf(stderr, "[WARNING: Singleton variable (line %d of %s): %s]\n", 
				start_line, 
				currentStreamName(), 
				var);
}


		/********************************
		*           RAW READING         *
		*********************************/


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Scan the input, give prompts when necessary and return a char *  holding
a  stripped  version of the next term.  Contigeous white space is mapped
on a single space, block and % ... \n comment  is  deleted.   Memory  is
claimed automatically en enlarged if necessary.

Earlier versions used to local stack for building the term.   This  does
not  work  with  O_PCE  as  we might be called back via the notifier while
reading.

(char *) NULL is returned on a syntax error.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void
clearBuffer()
{ if (rb.size == 0)
  { if ((rb.base = Malloc(RBSIZE)) == (char *) NULL)
      fatalError("%s", OsError());
    rb.size = RBSIZE;
  }
  SECURE( if ( rb.base == 0 ) fatalError("read/1: nesting=%d, size=%d",
						read_nesting, rb.size) );
  rb.left = rb.size;
  base = rb.here = rb.base;
  DEBUG(8, printf("Cleared read buffer.rb at %ld, base at %ld\n", &rb, rb.base));
}      

#if PROTO
static void
addToBuffer(register char c)
#else
static void
addToBuffer(c)
register char c;
#endif
{ if (rb.left-- == 0)
  { if ((rb.base = Realloc(rb.base, rb.size * 2)) == (char *)NULL)
      fatalError("%s", OsError());
    DEBUG(8, printf("Reallocated read buffer at %ld\n", rb.base));
    base = rb.base;
    rb.here = rb.base + rb.size;
    rb.left = rb.size - 1;
    rb.size *= 2;
  }
  *rb.here++ = c;
}

static void
delBuffer()
{ if ( rb.here > rb.base )
  { rb.here--;
    rb.left++;
  }
}

#define ensure_space   { if (rb.here > rb.base && rb.here[-1] != ' ') \
			   addToBuffer(' '); \
		       }
#define set_start_line { if (start_line < 0) \
			   start_line = currentInputLine(); \
		       }

#define rawSyntaxError(what) { addToBuffer(EOS); \
			       base = rb.base, token_start = rb.here-1; \
			       syntaxError(what); \
			     }

#if O_EXTEND_ATOMS

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Primitive functions to print and delete characters for  atom-completion.
Should be abstracted from a bit and incorporated in the operating system
interface.   As  this  model of atom-completion is unlikely to work on a
non-Unix machine anyway this will do for the moment.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void
extendBeep()
{ if ( status.beep == TRUE )
  { putchar(07);		/* ^G: the bel */
    fflush(stdout);
  }
}

#if O_LINE_EDIT				/* otherwise, define in md.h */
#define DEL_ESC	"\b\b  \b\b"
#define DEL_EOF "\b\b  \b\b"
#endif O_LINE_EDIT

static void
extendDeleteEscape()
{ printf(DEL_ESC);
  fflush(stdout);
}

static void
extendDeleteEof()
{ printf(DEL_EOF);
  fflush(stdout);
}

static void
extendReprint(reprint)
bool reprint;
{ ttybuf buf;
  char *s, *start = rb.here - 1;

  for(; *start != '\n' && start >= rb.base; start-- ) ;
  start++;
  PushTty(&buf, reprint ? TTY_APPEND : TTY_RETYPE);
  for( s=start; s < rb.here; s++ )
    PretendTyped(*s);
  PopTty(&buf);
  while(rb.here > start)
    delBuffer();
  fflush(stdout);
}

#endif O_EXTEND_ATOMS

static Char
getchr()
{ register Char c;

  if (rb.fd == (FILE *)NULL)
  { c =  Get0();
    base = rb.base;
  } else if ((c = (Char) Getc(rb.fd)) == '\n')
    newLineInput();

  return c;
}

static char *
raw_read2()
{ register Char c;
  bool something_read = FALSE;
  int newlines;

  clearBuffer();				/* clear input buffer */
  prompt(FALSE);				/* give prompt */
  start_line = -1;

  for(;;)
  { c = getchr();
    DEBUG(3, if ( Input == 0 ) printf("getchr() -> %d (%c)\n", c, c));
    DEBUG(3, if ( Input == 0 ) printf("here = %d, base = %d", rb.here, rb.base));
    switch(c)
    { case EOF:
      case 04:
		if (seeingString())		/* do not require '. ' when */
		{ addToBuffer('.');		/* reading from a string */
		  addToBuffer(' ');
		  addToBuffer(EOS);
		  return rb.base;
		}
		if (something_read)
		{
#if O_EXTEND_ATOMS
		  if ( rb.doExtend == TRUE )
		  { char *a;

		    addToBuffer(EOS);		/* allocates if need be */
		    delBuffer();
		    a = rb.here - 1;
		    for( ;a >= rb.base && isAlpha(*a); a--) ;
		    a++;
		    extendDeleteEof();
		    if ( a >= rb.here || !isLower(*a) )
		    { extendReprint(FALSE);
		      extendBeep();
		      break;
		    }
		    Put('\n');
		    extendAlternatives(a);
		    extendReprint(TRUE);
		    break;
		  }		  
#endif O_EXTEND_ATOMS
		  rawSyntaxError("Unexpected end of file");
		}
      e_o_f:
		strcpy(rb.base, "end_of_file. ");
		return rb.base;
      case '*':	if (rb.here-rb.base >= 1 && rb.here[-1] == '/')
		{ register char last;
		  int  level;

		  level = 1;
		  if ((last = getchr()) == EOF)
		    rawSyntaxError("End of file in ``/* ... */'' comment");
		  for(;;)
		  { if ((c = getchr()) == EOF)
		      rawSyntaxError("End of file in ``/* ... */'' comment");
		
		    if (last == '/' && c == '*')
		      level++;
		    if (last == '*' && c == '/')
		      if (--level == 0)
			break;
		    last = c;
		  }
		  rb.here--, rb.left++;		  
		  c = ' ';
		  goto case_default;		/* Hack */
		}

		addToBuffer(c);
		something_read = TRUE;
		set_start_line;
		break;
      case '%': while((c=getchr()) != EOF && c != '\n') ;
		if (c == EOF)
		{ if (something_read)
		    rawSyntaxError("Unexpected end of file")
		  else
		    goto e_o_f;		  
		}

		goto case_default;		/* Hack */
     case '\'': if ( rb.here > rb.base && isDigit(rb.here[-1]) )
		{ addToBuffer(c);			/* <n>' */
		  if ( rb.here[-2] == '0' )		/* 0'<c> */
		  { if ( (c=getchr()) != EOF )
		    { addToBuffer(c);
		      break;
		    }
		    rawSyntaxError("Unexpected end of file");
		  }
		  break;
		}

		something_read = TRUE;
		newlines = 0;
		set_start_line;
		addToBuffer(c);
		while((c=getchr()) != EOF && c != '\'')
		{ if (c == '\n' &&
		       newlines++ > MAXNEWLINES &&
		       (debugstatus.styleCheck & LONGATOM_CHECK))
		    rawSyntaxError("Atom too long");
		  addToBuffer(c);
		}
		if (c == EOF)
		  rawSyntaxError("End of file in quoted atom");
		addToBuffer(c);
		break;
      case '"':	something_read = TRUE;
		newlines = 0;
      		set_start_line;
		addToBuffer(c);
		while((c=getchr()) != EOF && c != '"')
		{ if (c == '\n' &&
		       newlines++ > MAXNEWLINES &&
		       (debugstatus.styleCheck & LONGATOM_CHECK))
		    rawSyntaxError("String too long");
		  addToBuffer(c);
		}
		if (c == EOF)
		  rawSyntaxError("End of file in string");
		addToBuffer(c);
		break;
#if O_EXTEND_ATOMS
      case 27:  if ( rb.doExtend == TRUE )
		{ char *a;
		  char *extend;
		  bool unique;

		  addToBuffer(EOS);		/* allocates if need be */
		  delBuffer();
		  a = rb.here - 1;
		  for( ;a >= rb.base && isAlpha(*a); a--) ;
		  a++;
		  if ( a >= rb.here || !isLower(*a) )
		  { extendDeleteEscape();
		    extendReprint(FALSE);
		    extendBeep();
		    break;
		  }
		  if ( (extend = extendAtom(a, &unique)) != (char *)NULL )
		  { ttybuf buf;

		    extendDeleteEscape();
		    extendReprint(FALSE);
		    PushTty(&buf, TTY_APPEND);
		    while(*extend)
		      PretendTyped(*extend++);
		    PopTty(&buf);
		    if ( unique == FALSE )
		      extendBeep();
		  } else
		  { extendDeleteEscape();
		    extendReprint(FALSE);
		    extendBeep();
		  }
		  break;		  
		}
		/*FALLTHROUGH*/
#endif O_EXTEND_ATOMS
      case_default:				/* Hack, needs fixing */
      default:	if (isBlank(c))
		{ long rd;

		  rd = rb.here - rb.base;
		  if (rd == 1 && rb.here[-1] == '.')
		    rawSyntaxError("Unexpected end of clause");
		  if (rd >= 2)
		  { if ( rb.here[-1] == '.' &&
			 !isSymbol(rb.here[-2])  &&
			 !(rb.here[-2] == '\'' && rd >= 3 && rb.here[-3] == '0'))
		    { ensure_space;
		      addToBuffer(EOS);
		      return rb.base;
		    }
		  }
		  ensure_space;
		  if (c == '\n')
		    prompt(TRUE);
		} else
		{ addToBuffer(c);
		  if (c != '/')		/* watch comment start */
		  { something_read = TRUE;
		    set_start_line;
		  }
		}
		break;
    }
  }
}  

static char *
raw_read()
{ char *s;

  startRead();
#if O_EXTEND_ATOMS
  if ( Input == 0 && status.notty == FALSE )
  { ttybuf buf;

    PushTty(&buf, TTY_EXTEND_ATOMS);
    s = raw_read2();
    PopTty(&buf);
  } else
#endif
    s = raw_read2();
  stopRead();

  return s;
}


		/*********************************
		*        VARIABLE DATABASE       *
		**********************************/

#define VARHASHSIZE 64

static char *	 allocBase;		/* local allocation base */
#if !O_DYNAMIC_STACKS
static char *    allocTop;		/* top of allocation */
#endif
static Variable* varTable;		/* hashTable for variables */

forwards void	check_singletons P((void));
forwards bool	bind_variables P((Word));
forwards char *	alloc_var P((size_t));
forwards char *	save_var_name P((char *));
forwards Variable lookupVariable P((char *));
forwards void	initVarTable P((void));

static void
check_singletons()
{ register Variable var;
  int n;

  for(n=0; n<VARHASHSIZE; n++)
  { for(var = varTable[n]; var; var=var->next)
    { if (var->times == 1 && var->name[0] != '_')
	singletonWarning(var->name);
    }
  }
}

/*  construct a list of Var = <name> terms wich indicate the bindings
    of the variables. Anonymous variables are skipped. The result is
    unified with the argument.

 ** Sat Apr 16 23:09:04 1988  jan@swivax.UUCP (Jan Wielemaker)  */

static bool
bind_variables(bindings)
Word bindings;
{ Variable var;
  int n;
  word binding;
  Word arg;

  for(n=0; n<VARHASHSIZE; n++)
  { for(var = varTable[n]; var; var=var->next)
    { if (var->name[0] != '_')
      { binding = globalFunctor(FUNCTOR_equals2);
	arg     = argTermP(binding, 0);
	*arg++  = (word) lookupAtom(var->name);
	*arg    = makeRef(var->address);
	APPENDLIST(bindings, &binding);
      }
    }
  }
  CLOSELIST(bindings);

  succeed;
}

static char *
alloc_var(n)
register size_t n;
{ register char *space;

  n = ROUND(n, sizeof(word));

  STACKVERIFY(if (allocBase + n > allocTop) outOf((Stack)&stacks.local) );

  space = allocBase;
  allocBase += n;

  return space;
}

static char *
save_var_name(s)
char *s;
{ char *copy = alloc_var(strlen(s) + 1);

  strcpy(copy, s);

  return copy;
}

static Variable
lookupVariable(s)
char *s;
{ int v = stringHashValue(s, VARHASHSIZE);
  Variable var;

  for(var=varTable[v]; var; var=var->next)
  { if (streq(s, var->name) )
    { var->times++;
      return var;
    }
  }
  var = (Variable) alloc_var((size_t) sizeof(struct variable));
  DEBUG(9, printf("Allocated var at %ld\n", var));
  var->next = varTable[v];
  varTable[v] = var;
  var->name = save_var_name(s);
  var->times = 1;
  var->address = (Word) NULL;

  return var;
}

static void
initVarTable()
{ int n;

  allocBase = (char *)(lTop+1) + (MAXARITY+MAXVARIABLES) * sizeof(word);
#if !O_DYNAMIC_STACKS
  allocTop  = (char *)lMax;
#endif

  varTable = (Variable *)alloc_var((size_t) sizeof(Variable)*VARHASHSIZE);
  for(n=0; n<VARHASHSIZE; n++)
    varTable[n] = (Variable) NULL;
}

		/********************************
		*           TOKENISER           *
		*********************************/

#define skipSpaces	{ while(isBlank(*here) ) here++; c = *here++; }
#define unget_token()	{ unget = TRUE; }

forwards Token	get_token P((void));
forwards word	build_term P((Atom, int, Word));
forwards int	complex_term P((int, Word));
forwards int	simple_term P((Word, bool *));
forwards bool	read_term P((Word, Word, bool));

typedef union
{ long	i;
  real  r;
} number;

#define V_ERROR 0
#define V_REAL  1
#define V_INT   2

forwards int scan_number P((char **, int, number *));

static int
scan_number(s, b, n)
char **s;
int b;
number *n;
{ int d;

  n->i = 0;
  while((d = digitValue(b, **s)) >= 0)
  { (*s)++;
    n->i = n->i * b + d;
    if ( n->i > PLMAXINT )
    { n->r = (real) n->i;
      while((d = digitValue(b, **s)) >= 0)
      { (*s)++;
        if ( n->r > MAXREAL / (real) b - (real) d )
	  return V_ERROR;
        n->r = n->r * (real)b + (real)d;
      }
      return V_REAL;
    }
  }  

  return V_INT;
}


static Token
get_token()
{ char c;
  char *start;
  char end;

  if (unget)
  { unget = FALSE;
    return &token;
  }

  skipSpaces;
  token_start = here - 1;
  switch(char_type[(unsigned)c & 0xff])
  { case LC:	{ start = here-1;
		  while(isAlpha(*here) )
		    here++;
		  c = *here;
		  *here = EOS;
		  token.value.prolog = (word)lookupAtom(start);
		  *here = c;
		  token.type = (c == '(' ? T_FUNCTOR : T_NAME);
		  DEBUG(9, printf("%s: %s\n", c == '(' ? "FUNC" : "NAME", stringAtom(token.value.prolog)));

		  return &token;
		}
    case UC:	{ start = here-1;
		  while(isAlpha(*here) )
		    here++;
		  c = *here;
		  *here = EOS;
		  if (start[0] == '_')
		  { setVar(token.value.prolog);
		    DEBUG(9, printf("VOID\n"));
		    token.type = T_VOID;
		  } else
		  { token.value.variable = lookupVariable(start);
		    DEBUG(9, printf("VAR: %s\n", token.value.variable->name));
		    token.type = T_VARIABLE;
		  }
		  *here = c;

		  return &token;
		}
    case DI:	{ number value;
		  int tp;

		  if (c == '0' && *here == '\'')		/* 0'<char> */
		  { if (isAlpha(here[2]))
		    { here += 2;
		      syntaxError("Illegal number");
		    }
		    token.value.prolog = consNum((long)here[1]);
		    token.type = T_INTEGER;
		    here += 2;

		    DEBUG(9, printf("INT: %ld\n", valNum(token.value.prolog)));
		    return &token;
		  }

		  here--;		/* start of token */
		  if ( (tp = scan_number(&here, 10, &value)) == V_ERROR )
		    syntaxError("Number too large");

					/* base'value number */
		  if ( *here == '\'' )
		  { here++;

		    if ( tp == V_REAL || value.i > 36 )
		      syntaxError("Base of <base>'<value> too large");
		    if ( (tp = scan_number(&here, (int)value.i, &value))
								== V_ERROR )
		      syntaxError("Number too large"); 

		    if (isAlpha(*here) )
		      syntaxError("Illegal number");

		    if ( tp == V_INT )
		    { token.value.prolog = consNum(value.i);
		      token.type = T_INTEGER;
		    } else
		    { token.value.prolog = globalReal(value.r);
		      token.type = T_REAL;
		    }

		    return &token;
		  }
					/* Real numbers */
		  if ( *here == '.' && isDigit(here[1]) )
		  { real n;

		    if ( tp == V_INT )
		    { value.r = (real) value.i;
		      tp = V_REAL;
		    }
		    n = 10.0, here++;
		    while(isDigit(c = *here) )
		    { here++;
		      value.r += (real)(c-'0') / n;
		      n *= 10.0;
		    }
		  }

		  if ( *here == 'e' || c == 'E' )
		  { number exponent;
		    bool neg_exponent;

		    here++;
		    DEBUG(9, printf("Exponent\n"));
		    switch(*here)
		    { case '-':		here++;
					neg_exponent = TRUE;
					break;
		      case '+':		here++;
		      default:		neg_exponent = FALSE;
					break;
		    }

		    if ( scan_number(&here, 10, &exponent) != V_INT )
		      syntaxError("Exponent too large");

		    if ( tp == V_INT )
		    { value.r = (real) value.i;
		      tp = V_REAL;
		    }

		    value.r *= pow((double)10.0,
				   neg_exponent ? -(double)exponent.i
				                : (double)exponent.i);
		  }

		  if ( isAlpha(c = *here) )
		    syntaxError("Illegal number");

		  if ( tp == V_REAL )
		  { token.value.prolog = globalReal(value.r);
		    token.type = T_REAL;
		  } else
		  { token.value.prolog = consNum(value.i);
		    token.type = T_INTEGER;
		  }

		  return &token;
		}		  
    case SO:	{ char tmp[2];

		  tmp[0] = c, tmp[1] = EOS;
		  token.value.prolog = (word) lookupAtom(tmp);
		  token.type = T_NAME;
		  DEBUG(9, printf("NAME: %s\n", stringAtom(token.value.prolog)));

		  return &token;
		}
    case SY:	{ if (c == '.' && isBlank(here[0]))
		  { token.type = T_FULLSTOP;
		    return &token;
		  }
		  start = here - 1;
		  while(isSymbol(*here) )
		    here++;
		  end = *here, *here = EOS;
		  token.value.prolog = (word) lookupAtom(start);
		  *here = end;
		  token.type = (end == '(' ? T_FUNCTOR : T_NAME);
		  DEBUG(9, printf("%s: %s\n", end == '(' ? "FUNC" : "NAME", stringAtom(token.value.prolog)));

		  return &token;
		}
    case PU:	{ switch(c)
		  { case '{':
		    case '[': while(isBlank(*here) )
				here++;
			      if (here[0] == matchingBracket(c))
			      { here++;
				token.value.prolog =
				    (word)(c == '[' ? ATOM_nil : ATOM_curl);
				token.type = T_NAME;
				DEBUG(9, printf("NAME: %s\n", stringAtom(token.value.prolog)));
				return &token;
			      }
		  }
		  token.value.character = c;
		  token.type = T_PUNCTUATION;
		  DEBUG(9, printf("PUNCT: %c\n", token.value.character));

		  return &token;
		}
    case SQ:	{ char *s;

		  start = here;
		  for(s=start;;)
		  { if (*here == '\'')
		    { if (here[1] != '\'')
		      { end = *s, *s = EOS;
			token.value.prolog = (word) lookupAtom(start);
			*s = end;
			token.type = (here[1] == '(' ? T_FUNCTOR : T_NAME);
			here++;
			DEBUG(9, printf("%s: %s\n", here[1] == '(' ? "FUNC" : "NAME", stringAtom(token.value.prolog)));
			return &token;
		      }
		      here++;
		    }
		    *s++ = *here++;
		  }
		}
    case DQ:	{ char *s;

		  start = here;
		  for(s=start;;)
		  { if (*here == '"')
		    { if (here[1] != '"')
		      { end = *s, *s = EOS;
#if O_STRING
			if ( debugstatus.styleCheck & O_STRING_STYLE )
			  token.value.prolog = globalString(start);
			else
			  token.value.prolog = (word) stringToList(start);
#else
			token.value.prolog = (word) stringToList(start);
#endif O_STRING
			DEBUG(9, printf("STR: %s\n", start));
			*s = end;
			token.type = T_STRING;
			here++;
			return &token;
		      }
		      here++;
		    }
		    *s++ = *here++;
		  }
		}
    default:	{ sysError("read/1: tokeniser internal error");
    		  return &token;	/* make lint happy */
		}
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Build a term on the global stack, given the atom  of  the  functor,  the
arity  and  a  vector of arguments.  The argument vector either contains
nonvar terms or a reference to a variable block.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static word
build_term(atom, arity, argv)
Atom atom;
int arity;
Word argv;
{ FunctorDef functor = lookupFunctorDef(atom, arity);
  word term;
  Word argp;

  DEBUG(9, printf("Building term %s/%d ... ", stringAtom(atom), arity));
  term = globalFunctor(functor);
  argp = argTermP(term, 0);
  while(arity-- > 0)
  { if (isRef(*argv) )
    { Variable var;
#if O_NO_LEFT_CAST
      Word w;
      deRef2(argv, w);
      var = (Variable) w;
#else
      deRef2(argv, (Word)var);
#endif
      if (var->address == (Word) NULL)
	var->address = argp++;
      else
	*argp++ = makeRef(var->address);
      argv++;
    } else
      *argp++ = *argv++;
  }
  DEBUG(9, printf("result: "); pl_write(&term); printf("\n") );

  return term;
}


		/********************************
		*             PARSER            *
		*********************************/

#define TERM_ERROR	0
#define TERM_OK		1
#define TERM_END	2

#define priorityClash { syntaxError("Operator priority clash"); }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
This part of the parser actually constructs  the  term.   It  calls  the
tokeniser  to  find  the next token and assumes the tokeniser implements
one-token pushback efficiently.  It consists  of  two  mutual  recursive
functions:  complex_term()  which is involved with operator handling and
simple_term() which reads everything, except for operators.

I have a very strong feeling complex_term() can be simpler and easier to
read.  One day ...
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int
complex_term(max_pri, term)
int max_pri;
Word term;
{ bool isname;
  word op, arg[2];
  int arg1_pri = 0;
  int type, priority;
  Token token;

  switch(simple_term(&arg[0], &isname) )
  { case TERM_ERROR:	return TERM_ERROR;
    case TERM_END:	return TERM_END;
  }

  if (isname && isPrefixOperator((Atom)arg[0], &type, &priority) )
  { if (priority > max_pri)
      priorityClash;
    arg1_pri = priority;
    if (type == OP_FX)
      priority--;
    if ((token = get_token()) == (Token) NULL)
      return TERM_ERROR;
    if (token->type == T_PUNCTUATION)
    { switch(token->value.character)
      { case ')':
	case ']':
	case '}':	*term = arg[0];
			unget_token();
			return TERM_OK;
	case ',':
	case '|':	unget_token();
			goto next;
      }
    } else if (token->type == T_NAME)
    { int pri, t;
      if (isInfixOperator((Atom)token->value.prolog, &t, &pri) )
      { if (pri > priority)
	{ unget_token();
	  goto next;
	}
      } else if (isPostfixOperator((Atom)token->value.prolog, &t, &pri) )
      { if (pri > priority)
	{ unget_token();
	  goto next;
	}
      }
    }
    unget_token();
    switch(complex_term(priority, &arg[1]) )
    { case TERM_ERROR:	return TERM_ERROR;
      case TERM_END:	*term = arg[0];
			unget_token();
			return TERM_OK;
    }

    if (arg[0] == (word) ATOM_minus && !isRef(arg[1]) )  /* -number case */
    { if (isInteger(arg[1]))
      { arg[0] = consNum(-valNum(arg[1]));
        arg1_pri = 0;
      } else if (isReal(arg[1]))
      { arg[0] = arg[1];
	setReal(arg[0], -valReal(arg[0]));
        arg1_pri = 0;
      } else
      { arg[0] = build_term((Atom)arg[0], 1, &arg[1]);
      }
    } else if (arg[0] == (word) ATOM_plus &&
		!isRef(arg[1]) &&
		(isInteger(arg[1]) || isReal(arg[1])))
    { arg[0] = arg[1];				/* +number case */
      arg1_pri = 0;
    } else
      arg[0] = build_term((Atom)arg[0], 1, &arg[1]);
  }

next:
  if ((token = get_token()) == (Token) NULL)
    return TERM_ERROR;
  if (token->type == T_PUNCTUATION)
  { if (token->value.character == ',' && max_pri >= 1000)
      op = (word) ATOM_comma;
    else if (token->value.character == '|' && max_pri >= 1100)
      op = (word) ATOM_bar;
    else
    { unget_token();
      *term = arg[0];
      return TERM_OK;
    }
  } else
  { if (token->type != T_NAME && token->type != T_FUNCTOR)
    { unget_token();
      *term = arg[0];

      return TERM_OK;
    }
    op = token->value.prolog;
  }

  if (isInfixOperator((Atom)op, &type, &priority) )
  { if (priority > max_pri)
    { unget_token();
      *term = arg[0];
      return TERM_OK;
    }
    if (arg1_pri > (type == OP_XFX || type == OP_XFY ? priority - 1
						      : priority))
      priorityClash;
    arg1_pri = priority;
    if (type == OP_XFX || type == OP_YFX)
      priority--;
    switch(complex_term(priority, &arg[1]) )
    { case TERM_ERROR:	return TERM_ERROR;
      case TERM_END:	syntaxError("2nd argument expected");
    }
    arg[0] = build_term((Atom)op, 2, arg);
    goto next;
  }

  if (isPostfixOperator((Atom)op, &type, &priority) )
  { if (priority > max_pri)
    { unget_token();
      *term = arg[0];
      return TERM_OK;
    }
    if (arg1_pri > (type == OP_XF ? priority - 1 : priority))
      priorityClash;

    arg1_pri = priority;
    arg[0] = build_term((Atom)op, 1, &arg[0]);
    goto next;
  }

  unget_token();
  *term = arg[0];

  return TERM_OK;
}

static int
simple_term(term, name)
Word term;
bool *name;
{ Token token;

  DEBUG(9, printf("simple_term(): Stack at %ld\n", &term));

  *name = FALSE;

  if ((token = get_token()) == (Token) NULL)
    return TERM_ERROR;
  switch(token->type)
  { case T_FULLSTOP:
      { return TERM_END;
      }
    case T_VOID:
      { *term = token->value.prolog;
	return TERM_OK;
      }
    case T_VARIABLE:
      { *term = makeRef(token->value.variable);
	return TERM_OK;
      }
    case T_NAME:
      *name = TRUE;
    case T_REAL:
    case T_INTEGER:
    case T_STRING:
      {	*term = token->value.prolog;
	return TERM_OK;
      }
    case T_FUNCTOR:
      { word argv[MAXARITY+1];
	int argc;
	Word argp;
	word functor;

	functor = token->value.prolog;
	argc = 0, argp = argv;
	get_token();			/* skip '(' */

	do
	{ switch(complex_term(999, argp++) )
	  { case TERM_ERROR:	return TERM_ERROR;
	    case TERM_END:	syntaxError("Unexpected end of clause");
	  }
	  if (++argc > MAXARITY)
	    syntaxError("Arity too high");
	  if ((token = get_token()) == (Token) NULL)
	    return TERM_ERROR;
	  if (token->type != T_PUNCTUATION ||
	      (token->value.character != ')' &&
	       token->value.character != ','))
	    syntaxError("`)' or `,' expected");
	} while(token->value.character == ',');

	*term = build_term((Atom)functor, argc, argv);
	return TERM_OK;
      }
    case T_PUNCTUATION:
      { switch(token->value.character)
	{ case '(':
	    { word arg;

	      switch(complex_term(1200, &arg) )
	      { case TERM_ERROR:	return TERM_ERROR;
		case TERM_END:		syntaxError("Unexpected end of clause");
	      }
	      if ((token = get_token()) == (Token) NULL)
		return TERM_ERROR;
	      if (token->type != T_PUNCTUATION || token->value.character != ')')
		syntaxError("`)' expected");
	      *term = arg;

	      return TERM_OK;
	    }
	  case '{':
	    { word arg;

	      switch(complex_term(1200, &arg) )
	      { case TERM_ERROR:	return TERM_ERROR;
		case TERM_END:		syntaxError("Unexpected end of clause");
	      }
	      if ((token = get_token()) == (Token) NULL)
		return TERM_ERROR;
	      if (token->type != T_PUNCTUATION || token->value.character != '}')
		syntaxError("`}' expected");
	      *term = build_term(ATOM_curl, 1, &arg);

	      return TERM_OK;
	    }
	  case '[':
	    { Word tail = term;
	      word arg[2];
	      Atom dot = ATOM_dot;

	      for(;;)
	      { switch(complex_term(999, &arg[0]) )
		{ case TERM_ERROR:   return TERM_ERROR;
		  case TERM_END:     syntaxError("Unexpected end of clause");
		}
		arg[1] = (word) NULL;
		*tail = build_term(dot, 2, arg);
		tail = argTermP(*tail, 1);

		if ((token = get_token()) == (Token) NULL)
		  return TERM_ERROR;
		if (token->type != T_PUNCTUATION)
		  syntaxError("`|', `,' or ']' expected");
		switch(token->value.character)
		{ case ']':
		    { *tail = (word) ATOM_nil;
		      return TERM_OK;
		    }
		  case '|':
		    { switch(complex_term(999, &arg[0]) )
		      { case TERM_ERROR:   return TERM_ERROR;
			case TERM_END: syntaxError("Unexpected end of clause");
		      }
		      if (isRef(arg[0]))
		      { Variable var;
#if O_NO_LEFT_CAST
			Word w;
			deRef2(&arg[0], w);
			var = (Variable) w;
#else
			deRef2(&arg[0], (Word)var);
#endif
			if (var->address == (Word) NULL)
			  var->address = tail;
			else
			  *tail = makeRef(var->address);
		      } else
			*tail = arg[0];
		      if ((token = get_token()) == (Token) NULL)
			return TERM_ERROR;
		      if (token->type != T_PUNCTUATION ||
			   token->value.character != ']')
			syntaxError("`]' expected");

		      return TERM_OK;
		    }
		  case ',':
		      continue;
		  default:
		      syntaxError("`|', `,' or ']' expected");
		}
	      }
	    }
	  case ',':
	    { *name = TRUE; 
	      *term = (word) ATOM_comma;

	      return TERM_OK;
	    }
	  default:
	    { syntaxError("`)', `]' or '|' unexpected");
	      /*NOTREACHED*/
	    }
	}
      } /* case T_PUNCTUATION */
    default:;
      fatalError("read/1: Illegal token type (%d)", token->type);
      /*NOTREACHED*/
      return TERM_ERROR;		/* make stupid compilers happy */
  }
}

static bool
read_term(term, variables, check)
Word term, variables;
bool check;
{ Token token;
  word result;

  if ((base = raw_read()) == (char *) NULL)
    fail;

  initVarTable();
  here = base;

  if (complex_term(1200, &result) == TERM_OK)
  { if ((token = get_token()) == (Token) NULL)
      fail;
    if (token->type != T_FULLSTOP)
      syntaxError("End of clause expected");

    if ( isRef(result) )	/* term is a single variable! */
    { Variable var;
#if O_NO_LEFT_CAST
      Word w;
      deRef2(&result, w);
      var = (Variable) w;
#else
      deRef2(&result, (Word)var);
#endif
      if ( var->times != 1 || var->address != (Word)NULL )
	sysError("Error while reading a single variable??");
      var->address = allocGlobal(sizeof(word));
      setVar(*var->address);
      result = makeRef(var->address);
    }

    TRY(pl_unify(term, &result) );
    if (variables != (Word) NULL)
      TRY(bind_variables(variables) );
    if (check)
      check_singletons();

    succeed;
  }

  fail;
}

		/********************************
		*       PROLOG CONNECTION       *
		*********************************/

word
pl_raw_read(term)
Word term;
{ char *s;
  register char *top;

  s = raw_read();

  if ( s == (char *) NULL )
    fail;
  
  for(top = s+strlen(s)-1; isBlank(*top); top--);
  if (*top == '.')
    *top = EOS;
  for(; isBlank(*s); s++);

  return unifyAtomic(term, lookupAtom(s) );
}

word
pl_read_variables(term, variables)
Word term, variables;
{ return read_term(term, variables, FALSE);
}

word
pl_read_variables3(stream, term, variables)
Word stream, term, variables;
{ streamInput(stream, pl_read_variables(term, variables));
}

word
pl_read(term)
Word term;
{ return read_term(term, (Word)NULL, FALSE);
}

word
pl_read2(stream, term)
Word stream, term;
{ streamInput(stream, pl_read(term));
}

word
pl_read_clause(term)
Word term;
{ return read_term(term, (Word) NULL, debugstatus.styleCheck & SINGLETON_CHECK ? TRUE
								       : FALSE);
}

word
pl_read_clause2(stream, term)
Word stream, term;
{ streamInput(stream, pl_read_clause(term));
}
