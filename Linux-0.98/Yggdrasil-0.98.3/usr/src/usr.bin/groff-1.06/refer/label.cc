
/*  A Bison parser, made from label.y  */

#define YYBISON 1  /* Identify Bison output.  */

#define	TOKEN_LETTER	258
#define	TOKEN_LITERAL	259
#define	TOKEN_DIGIT	260

#line 21 "label.y"


#include "refer.h"
#include "refid.h"
#include "ref.h"
#include "token.h"

int yylex();
void yyerror(const char *);
int yyparse();

static const char *format_serial(char c, int n);

struct label_info {
  int start;
  int length;
  int count;
  int total;
  label_info(const string &);
};

label_info *lookup_label(const string &label);

struct expression {
  enum {
    // Does the tentative label depend on the reference?
    CONTAINS_VARIABLE = 01, 
    CONTAINS_STAR = 02,
    CONTAINS_FORMAT = 04,
    CONTAINS_AT = 010
  };
  virtual ~expression() { }
  virtual void evaluate(int, const reference &, string &,
			substring_position &) = 0;
  virtual unsigned analyze() { return 0; }
};

class at_expr : public expression {
public:
  at_expr() { }
  void evaluate(int, const reference &, string &, substring_position &);
  unsigned analyze() { return CONTAINS_VARIABLE|CONTAINS_AT; }
};

class format_expr : public expression {
  char type;
  int width;
  int first_number;
public:
  format_expr(char c, int w = 0, int f = 1)
    : type(c), width(w), first_number(f) { }
  void evaluate(int, const reference &, string &, substring_position &);
  unsigned analyze() { return CONTAINS_FORMAT; }
};

class field_expr : public expression {
  int number;
  char name;
public:
  field_expr(char nm, int num) : name(nm), number(num) { }
  void evaluate(int, const reference &, string &, substring_position &);
  unsigned analyze() { return CONTAINS_VARIABLE; }
};

class literal_expr : public expression {
  string s;
public:
  literal_expr(const char *ptr, int len) : s(ptr, len) { }
  void evaluate(int, const reference &, string &, substring_position &);
};

class unary_expr : public expression {
protected:
  expression *expr;
public:
  unary_expr(expression *e) : expr(e) { }
  ~unary_expr() { delete expr; }
  void evaluate(int, const reference &, string &, substring_position &) = 0;
  unsigned analyze() { return expr ? expr->analyze() : 0; }
};

// This caches the analysis of an expression.

class analyzed_expr : public unary_expr {
  unsigned flags;
public:
  analyzed_expr(expression *);
  void evaluate(int, const reference &, string &, substring_position &);
  unsigned analyze() { return flags; }
};

class star_expr : public unary_expr {
public:
  star_expr(expression *e) : unary_expr(e) { }
  void evaluate(int, const reference &, string &, substring_position &);
  unsigned analyze() {
    return ((expr ? (expr->analyze() & ~CONTAINS_VARIABLE) : 0)
	    | CONTAINS_STAR);
  }
};

typedef void map_t(const char *, const char *, string &);

class map_expr : public unary_expr {
  map_t *func;
public:
  map_expr(expression *e, map_t *f) : unary_expr(e), func(f) { }
  void evaluate(int, const reference &, string &, substring_position &);
};
  
typedef const char *extractor_t(const char *, const char *, const char **);

class extractor_expr : public unary_expr {
  int part;
  extractor_t *func;
public:
  enum { BEFORE = +1, MATCH = 0, AFTER = -1 };
  extractor_expr(expression *e, extractor_t *f, int pt)
    : unary_expr(e), func(f), part(pt) { }
  void evaluate(int, const reference &, string &, substring_position &);
};

class truncate_expr : public unary_expr {
  int n;
public:
  truncate_expr(expression *e, int i) : n(i), unary_expr(e) { } 
  void evaluate(int, const reference &, string &, substring_position &);
};

class separator_expr : public unary_expr {
public:
  separator_expr(expression *e) : unary_expr(e) { }
  void evaluate(int, const reference &, string &, substring_position &);
};

class binary_expr : public expression {
protected:
  expression *expr1;
  expression *expr2;
public:
  binary_expr(expression *e1, expression *e2) : expr1(e1), expr2(e2) { }
  ~binary_expr() { delete expr1; delete expr2; }
  void evaluate(int, const reference &, string &, substring_position &) = 0;
  unsigned analyze() {
    return (expr1 ? expr1->analyze() : 0) | (expr2 ? expr2->analyze() : 0);
  }
};

class alternative_expr : public binary_expr {
public:
  alternative_expr(expression *e1, expression *e2) : binary_expr(e1, e2) { }
  void evaluate(int, const reference &, string &, substring_position &);
};

class list_expr : public binary_expr {
public:
  list_expr(expression *e1, expression *e2) : binary_expr(e1, e2) { }
  void evaluate(int, const reference &, string &, substring_position &);
};

class substitute_expr : public binary_expr {
public:
  substitute_expr(expression *e1, expression *e2) : binary_expr(e1, e2) { }
  void evaluate(int, const reference &, string &, substring_position &);
};

class ternary_expr : public expression {
protected:
  expression *expr1;
  expression *expr2;
  expression *expr3;
public:
  ternary_expr(expression *e1, expression *e2, expression *e3)
    : expr1(e1), expr2(e2), expr3(e3) { }
  ~ternary_expr() { delete expr1; delete expr2; delete expr3; }
  void evaluate(int, const reference &, string &, substring_position &) = 0;
  unsigned analyze() {
    return ((expr1 ? expr1->analyze() : 0)
	    | (expr2 ? expr2->analyze() : 0)
	    | (expr3 ? expr3->analyze() : 0));
  }
};

class conditional_expr : public ternary_expr {
public:
  conditional_expr(expression *e1, expression *e2, expression *e3)
    : ternary_expr(e1, e2, e3) { }
  void evaluate(int, const reference &, string &, substring_position &);
};

static expression *parsed_label = 0;
static expression *parsed_date_label = 0;
static expression *parsed_short_label = 0;

static expression *parse_result;

string literals;


#line 221 "label.y"
typedef union {
  int num;
  expression *expr;
  struct { int ndigits; int val; } dig;
  struct { int start; int len; } str;
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#include <stdio.h>

#ifndef __STDC__
#define const
#endif



#define	YYFINAL		49
#define	YYFLAG		-32768
#define	YYNTBASE	21

#define YYTRANSLATE(x) ((unsigned)(x) <= 260 ? yytranslate[x] : 32)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,    12,     9,     2,    17,
    18,    16,    14,     2,    15,    13,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     7,     2,    19,
     2,    20,     6,    11,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     8,     2,    10,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,    10,    11,    13,    15,    19,    23,    25,
    28,    30,    34,    36,    38,    40,    43,    46,    49,    55,
    59,    63,    66,    70,    74,    75,    77,    79,    82,    84,
    87,    88,    90
};

#endif

static const short yyrhs[] = {    23,
     0,    24,     0,    24,     6,    23,     7,    22,     0,     0,
    22,     0,    25,     0,    24,     8,    25,     0,    24,     9,
    25,     0,    26,     0,    25,    26,     0,    27,     0,    26,
    10,    27,     0,    11,     0,     4,     0,     3,     0,     3,
    29,     0,    12,     3,     0,    12,    30,     0,    27,    13,
    31,     3,    28,     0,    27,    14,    29,     0,    27,    15,
    29,     0,    27,    16,     0,    17,    23,    18,     0,    19,
    23,    20,     0,     0,    29,     0,     5,     0,    29,     5,
     0,     5,     0,    30,     5,     0,     0,    14,     0,    15,
     0
};

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   248,   253,   256,   260,   263,   267,   270,   272,   276,   279,
   283,   286,   290,   293,   298,   300,   302,   318,   322,   353,
   355,   357,   359,   361,   365,   368,   372,   375,   379,   382,
   387,   390,   392
};

static const char * const yytname[] = {   "$","error","$illegal.","TOKEN_LETTER",
"TOKEN_LITERAL","TOKEN_DIGIT","'?'","':'","'|'","'&'","'~'","'@'","'%'","'.'",
"'+'","'-'","'*'","'('","')'","'<'","'>'","expr","conditional","optional_conditional",
"alternative","list","substitute","string","optional_number","number","digits",
"flag",""
};
#endif

static const short yyr1[] = {     0,
    21,    22,    22,    23,    23,    24,    24,    24,    25,    25,
    26,    26,    27,    27,    27,    27,    27,    27,    27,    27,
    27,    27,    27,    27,    28,    28,    29,    29,    30,    30,
    31,    31,    31
};

static const short yyr2[] = {     0,
     1,     1,     5,     0,     1,     1,     3,     3,     1,     2,
     1,     3,     1,     1,     1,     2,     2,     2,     5,     3,
     3,     2,     3,     3,     0,     1,     1,     2,     1,     2,
     0,     1,     1
};

static const short yydefact[] = {     4,
    15,    14,    13,     0,     4,     4,     5,     1,     2,     6,
     9,    11,    27,    16,    17,    29,    18,     0,     0,     4,
     0,     0,    10,     0,    31,     0,     0,    22,    28,    30,
    23,    24,     0,     7,     8,    12,    32,    33,     0,    20,
    21,     0,    25,     3,    19,    26,     0,     0,     0
};

static const short yydefgoto[] = {    47,
     7,     8,     9,    10,    11,    12,    45,    14,    17,    39
};

static const short yypact[] = {     0,
     4,-32768,-32768,     5,     0,     0,-32768,-32768,     7,     0,
    -5,    13,-32768,    15,-32768,-32768,    27,    -4,    14,     0,
     0,     0,    -5,     0,     8,     4,     4,-32768,-32768,-32768,
-32768,-32768,    26,     0,     0,    13,-32768,-32768,    32,    15,
    15,     0,     4,-32768,-32768,    15,    36,    37,-32768
};

static const short yypgoto[] = {-32768,
    -3,     1,-32768,     9,   -10,    16,-32768,   -25,-32768,-32768
};


#define	YYLAST		40


static const short yytable[] = {    23,
    40,    41,     1,     2,    24,    18,    19,    15,    13,    16,
     3,     4,    20,    31,    21,    22,     5,    46,     6,    29,
    33,    37,    38,    23,    23,    25,    26,    27,    28,    34,
    35,    30,    42,    32,    43,    48,    49,     0,    44,    36
};

static const short yycheck[] = {    10,
    26,    27,     3,     4,    10,     5,     6,     3,     5,     5,
    11,    12,     6,    18,     8,     9,    17,    43,    19,     5,
    20,    14,    15,    34,    35,    13,    14,    15,    16,    21,
    22,     5,     7,    20,     3,     0,     0,    -1,    42,    24
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/local/lib/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#define YYLEX		yylex(&yylval, &yylloc)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_bcopy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 169 "/usr/local/lib/bison.simple"
int
yyparse()
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
#ifdef YYLSP_NEEDED
		 &yyls1, size * sizeof (*yylsp),
#endif
		 &yystacksize);

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symboles being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 250 "label.y"
{ parse_result = (yyvsp[0].expr ? new analyzed_expr(yyvsp[0].expr) : 0); ;
    break;}
case 2:
#line 255 "label.y"
{ yyval.expr = yyvsp[0].expr; ;
    break;}
case 3:
#line 257 "label.y"
{ yyval.expr = new conditional_expr(yyvsp[-4].expr, yyvsp[-2].expr, yyvsp[0].expr); ;
    break;}
case 4:
#line 262 "label.y"
{ yyval.expr = 0; ;
    break;}
case 5:
#line 264 "label.y"
{ yyval.expr = yyvsp[0].expr; ;
    break;}
case 6:
#line 269 "label.y"
{ yyval.expr = yyvsp[0].expr; ;
    break;}
case 7:
#line 271 "label.y"
{ yyval.expr = new alternative_expr(yyvsp[-2].expr, yyvsp[0].expr); ;
    break;}
case 8:
#line 273 "label.y"
{ yyval.expr = new conditional_expr(yyvsp[-2].expr, yyvsp[0].expr, 0); ;
    break;}
case 9:
#line 278 "label.y"
{ yyval.expr = yyvsp[0].expr; ;
    break;}
case 10:
#line 280 "label.y"
{ yyval.expr = new list_expr(yyvsp[-1].expr, yyvsp[0].expr); ;
    break;}
case 11:
#line 285 "label.y"
{ yyval.expr = yyvsp[0].expr; ;
    break;}
case 12:
#line 287 "label.y"
{ yyval.expr = new substitute_expr(yyvsp[-2].expr, yyvsp[0].expr); ;
    break;}
case 13:
#line 292 "label.y"
{ yyval.expr = new at_expr; ;
    break;}
case 14:
#line 294 "label.y"
{
		  yyval.expr = new literal_expr(literals.contents() + yyvsp[0].str.start,
					yyvsp[0].str.len);
		;
    break;}
case 15:
#line 299 "label.y"
{ yyval.expr = new field_expr(yyvsp[0].num, 0); ;
    break;}
case 16:
#line 301 "label.y"
{ yyval.expr = new field_expr(yyvsp[-1].num, yyvsp[0].num - 1); ;
    break;}
case 17:
#line 303 "label.y"
{
		  switch (yyvsp[0].num) {
		  case 'I':
		  case 'i':
		  case 'A':
		  case 'a':
		    yyval.expr = new format_expr(yyvsp[0].num);
		    break;
		  default:
		    command_error("unrecognized format `%1'", char(yyvsp[0].num));
		    yyval.expr = new format_expr('a');
		    break;
		  }
		;
    break;}
case 18:
#line 319 "label.y"
{
		  yyval.expr = new format_expr('0', yyvsp[0].dig.ndigits, yyvsp[0].dig.val);
		;
    break;}
case 19:
#line 323 "label.y"
{
		  switch (yyvsp[-1].num) {
		  case 'l':
		    yyval.expr = new map_expr(yyvsp[-4].expr, lowercase);
		    break;
		  case 'u':
		    yyval.expr = new map_expr(yyvsp[-4].expr, uppercase);
		    break;
		  case 'c':
		    yyval.expr = new map_expr(yyvsp[-4].expr, capitalize);
		    break;
		  case 'r':
		    yyval.expr = new map_expr(yyvsp[-4].expr, reverse_name);
		    break;
		  case 'a':
		    yyval.expr = new map_expr(yyvsp[-4].expr, abbreviate_name);
		    break;
		  case 'y':
		    yyval.expr = new extractor_expr(yyvsp[-4].expr, find_year, yyvsp[-2].num);
		    break;
		  case 'n':
		    yyval.expr = new extractor_expr(yyvsp[-4].expr, find_last_name, yyvsp[-2].num);
		    break;
		  default:
		    yyval.expr = yyvsp[-4].expr;
		    command_error("unknown function `%1'", char(yyvsp[-1].num));
		    break;
		  }
		;
    break;}
case 20:
#line 354 "label.y"
{ yyval.expr = new truncate_expr(yyvsp[-2].expr, yyvsp[0].num); ;
    break;}
case 21:
#line 356 "label.y"
{ yyval.expr = new truncate_expr(yyvsp[-2].expr, -yyvsp[0].num); ;
    break;}
case 22:
#line 358 "label.y"
{ yyval.expr = new star_expr(yyvsp[-1].expr); ;
    break;}
case 23:
#line 360 "label.y"
{ yyval.expr = yyvsp[-1].expr; ;
    break;}
case 24:
#line 362 "label.y"
{ yyval.expr = new separator_expr(yyvsp[-1].expr); ;
    break;}
case 25:
#line 367 "label.y"
{ yyval.num = -1; ;
    break;}
case 26:
#line 369 "label.y"
{ yyval.num = yyvsp[0].num; ;
    break;}
case 27:
#line 374 "label.y"
{ yyval.num = yyvsp[0].num; ;
    break;}
case 28:
#line 376 "label.y"
{ yyval.num = yyvsp[-1].num*10 + yyvsp[0].num; ;
    break;}
case 29:
#line 381 "label.y"
{ yyval.dig.ndigits = 1; yyval.dig.val = yyvsp[0].num; ;
    break;}
case 30:
#line 383 "label.y"
{ yyval.dig.ndigits = yyvsp[-1].dig.ndigits + 1; yyval.dig.val = yyvsp[-1].dig.val*10 + yyvsp[0].num; ;
    break;}
case 31:
#line 389 "label.y"
{ yyval.num = 0; ;
    break;}
case 32:
#line 391 "label.y"
{ yyval.num = 1; ;
    break;}
case 33:
#line 393 "label.y"
{ yyval.num = -1; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 440 "/usr/local/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  for (x = 0; x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = 0; x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 396 "label.y"


/* bison defines const to be empty unless __STDC__ is defined, which it
isn't under cfront */

#ifdef const
#undef const
#endif

const char *spec_ptr;
const char *spec_end;
const char *spec_cur;

int yylex()
{
  while (spec_ptr < spec_end && csspace(*spec_ptr))
    spec_ptr++;
  spec_cur = spec_ptr;
  if (spec_ptr >= spec_end)
    return 0;
  unsigned char c = *spec_ptr++;
  if (csalpha(c)) {
    yylval.num = c;
    return TOKEN_LETTER;
  }
  if (csdigit(c)) {
    yylval.num = c - '0';
    return TOKEN_DIGIT;
  }
  if (c == '\'') {
    yylval.str.start = literals.length();
    for (; spec_ptr < spec_end; spec_ptr++) {
      if (*spec_ptr == '\'') {
	if (++spec_ptr < spec_end && *spec_ptr == '\'')
	  literals += '\'';
	else {
	  yylval.str.len = literals.length() - yylval.str.start;
	  return TOKEN_LITERAL;
	}
      }
      else
	literals += *spec_ptr;
    }
    yylval.str.len = literals.length() - yylval.str.start;
    return TOKEN_LITERAL;
  }
  return c;
}

int set_label_spec(const char *label_spec)
{
  spec_cur = spec_ptr = label_spec;
  spec_end = strchr(label_spec, '\0');
  literals.clear();
  if (yyparse())
    return 0;
  delete parsed_label;
  parsed_label = parse_result;
  return 1;
}

int set_date_label_spec(const char *label_spec)
{
  spec_cur = spec_ptr = label_spec;
  spec_end = strchr(label_spec, '\0');
  literals.clear();
  if (yyparse())
    return 0;
  delete parsed_date_label;
  parsed_date_label = parse_result;
  return 1;
}

int set_short_label_spec(const char *label_spec)
{
  spec_cur = spec_ptr = label_spec;
  spec_end = strchr(label_spec, '\0');
  literals.clear();
  if (yyparse())
    return 0;
  delete parsed_short_label;
  parsed_short_label = parse_result;
  return 1;
}

void yyerror(const char *message)
{
  if (spec_cur < spec_end)
    command_error("label specification %1 before `%2'", message, spec_cur);
  else
    command_error("label specification %1 at end of string",
		  message, spec_cur);
}

void at_expr::evaluate(int tentative, const reference &ref,
		       string &result, substring_position &)
{
  if (tentative)
    ref.canonicalize_authors(result);
  else {
    const char *end, *start = ref.get_authors(&end);
    if (start)
      result.append(start, end - start);
  }
}

void format_expr::evaluate(int tentative, const reference &ref,
			   string &result, substring_position &)
{
  if (tentative)
    return;
  const label_info *lp = ref.get_label_ptr();
  int num = lp == 0 ? ref.get_number() : lp->count;
  if (type != '0')
    result += format_serial(type, num + 1);
  else {
    const char *ptr = itoa(num + first_number);
    int pad = width - strlen(ptr);
    while (--pad >= 0)
      result += '0';
    result += ptr;
  }
}

static const char *format_serial(char c, int n)
{
  assert(n > 0);
  static char buf[128]; // more than enough.
  switch (c) {
  case 'i':
  case 'I':
    {
      char *p = buf;
      // troff uses z and w to represent 10000 and 5000 in Roman
      // numerals; I can find no historical basis for this usage
      const char *s = c == 'i' ? "zwmdclxvi" : "ZWMDCLXVI";
      if (n >= 40000)
	return itoa(n);
      while (n >= 10000) {
	*p++ = s[0];
	n -= 10000;
      }
      for (int i = 1000; i > 0; i /= 10, s += 2) {
	int m = n/i;
	n -= m*i;
	switch (m) {
	case 3:
	  *p++ = s[2];
	  /* falls through */
	case 2:
	  *p++ = s[2];
	  /* falls through */
	case 1:
	  *p++ = s[2];
	  break;
	case 4:
	  *p++ = s[2];
	  *p++ = s[1];
	  break;
	case 8:
	  *p++ = s[1];
	  *p++ = s[2];
	  *p++ = s[2];
	  *p++ = s[2];
	  break;
	case 7:
	  *p++ = s[1];
	  *p++ = s[2];
	  *p++ = s[2];
	  break;
	case 6:
	  *p++ = s[1];
	  *p++ = s[2];
	  break;
	case 5:
	  *p++ = s[1];
	  break;
	case 9:
	  *p++ = s[2];
	  *p++ = s[0];
	}
      }
      *p = 0;
      break;
    }
  case 'a':
  case 'A':
    {
      char *p = buf;
      // this is derived from troff/reg.c
      while (n > 0) {
	int d = n % 26;
	if (d == 0)
	  d = 26;
	n -= d;
	n /= 26;
	*p++ = c + d - 1;	// ASCII dependent
      }
      *p-- = 0;
      // Reverse it.
      char *q = buf;
      while (q < p) {
	char temp = *q;
	*q = *p;
	*p = temp;
	--p;
	++q;
      }
      break;
    }
  default:
    assert(0);
  }
  return buf;
}

void field_expr::evaluate(int, const reference &ref,
			  string &result, substring_position &)
{
  const char *end;
  const char *start = ref.get_field(name, &end);
  if (start) {
    start = nth_field(number, start, &end);
    if (start)
      result.append(start, end - start);
  }
}

void literal_expr::evaluate(int, const reference &,
			    string &result, substring_position &)
{
  result += s;
}

analyzed_expr::analyzed_expr(expression *e)
: unary_expr(e), flags(e ? e->analyze() : 0)
{
}

void analyzed_expr::evaluate(int tentative, const reference &ref,
			     string &result, substring_position &pos)
{
  if (expr)
    expr->evaluate(tentative, ref, result, pos);
}

void star_expr::evaluate(int tentative, const reference &ref,
			 string &result, substring_position &pos)
{
  const label_info *lp = ref.get_label_ptr();
  if (!tentative
      && (lp == 0 || lp->total > 1)
      && expr)
    expr->evaluate(tentative, ref, result, pos);
}

void separator_expr::evaluate(int tentative, const reference &ref,
			      string &result, substring_position &pos)
{
  int start_length = result.length();
  int is_first = pos.start < 0;
  if (expr)
    expr->evaluate(tentative, ref, result, pos);
  if (is_first) {
    pos.start = start_length;
    pos.length = result.length() - start_length;
  }
}

void map_expr::evaluate(int tentative, const reference &ref,
			string &result, substring_position &)
{
  if (expr) {
    string temp;
    substring_position temp_pos;
    expr->evaluate(tentative, ref, temp, temp_pos);
    (*func)(temp.contents(), temp.contents() + temp.length(), result);
  }
}

void extractor_expr::evaluate(int tentative, const reference &ref,
			      string &result, substring_position &)
{
  if (expr) {
    string temp;
    substring_position temp_pos;
    expr->evaluate(tentative, ref, temp, temp_pos);
    const char *end, *start = (*func)(temp.contents(),
				      temp.contents() + temp.length(),
				      &end);
    switch (part) {
    case BEFORE:
      if (start)
	result.append(temp.contents(), start - temp.contents());
      else
	result += temp;
      break;
    case MATCH:
      if (start)
	result.append(start, end - start);
      break;
    case AFTER:
      if (start)
	result.append(end, temp.contents() + temp.length() - end);
      break;
    default:
      assert(0);
    }
  }
}

static void first_part(int len, const char *ptr, const char *end,
			  string &result)
{
  for (;;) {
    const char *token_start = ptr;
    if (!get_token(&ptr, end))
      break;
    const token_info *ti = lookup_token(token_start, ptr);
    int counts = ti->sortify_non_empty(token_start, ptr);
    if (counts && --len < 0)
      break;
    if (counts || ti->is_accent())
      result.append(token_start, ptr - token_start);
  }
}

static void last_part(int len, const char *ptr, const char *end,
		      string &result)
{
  const char *start = ptr;
  int count = 0;
  for (;;) {
    const char *token_start = ptr;
    if (!get_token(&ptr, end))
      break;
    const token_info *ti = lookup_token(token_start, ptr);
    if (ti->sortify_non_empty(token_start, ptr))
      count++;
  }
  ptr = start;
  int skip = count - len;
  if (skip > 0) {
    for (;;) {
      const char *token_start = ptr;
      if (!get_token(&ptr, end))
	assert(0);
      const token_info *ti = lookup_token(token_start, ptr);
      if (ti->sortify_non_empty(token_start, ptr) && --skip < 0) {
	ptr = token_start;
	break;
      }
    }
  }
  first_part(len, ptr, end, result);
}

void truncate_expr::evaluate(int tentative, const reference &ref,
			     string &result, substring_position &)
{
  if (expr) {
    string temp;
    substring_position temp_pos;
    expr->evaluate(tentative, ref, temp, temp_pos);
    const char *start = temp.contents();
    const char *end = start + temp.length();
    if (n > 0)
      first_part(n, start, end, result);
    else if (n < 0)
      last_part(-n, start, end, result);
  }
}

void alternative_expr::evaluate(int tentative, const reference &ref,
				string &result, substring_position &pos)
{
  int start_length = result.length();
  if (expr1)
    expr1->evaluate(tentative, ref, result, pos);
  if (result.length() == start_length && expr2)
    expr2->evaluate(tentative, ref, result, pos);
}

void list_expr::evaluate(int tentative, const reference &ref,
			 string &result, substring_position &pos)
{
  if (expr1)
    expr1->evaluate(tentative, ref, result, pos);
  if (expr2)
    expr2->evaluate(tentative, ref, result, pos);
}

void substitute_expr::evaluate(int tentative, const reference &ref,
			       string &result, substring_position &pos)
{
  int start_length = result.length();
  if (expr1)
    expr1->evaluate(tentative, ref, result, pos);
  if (result.length() > start_length && result[result.length() - 1] == '-') {
    // ought to see if pos covers the -
    result.set_length(result.length() - 1);
    if (expr2)
      expr2->evaluate(tentative, ref, result, pos);
  }
}

void conditional_expr::evaluate(int tentative, const reference &ref,
				string &result, substring_position &pos)
{
  string temp;
  substring_position temp_pos;
  if (expr1)
    expr1->evaluate(tentative, ref, temp, temp_pos);
  if (temp.length() > 0) {
    if (expr2)
      expr2->evaluate(tentative, ref, result, pos);
  }
  else {
    if (expr3)
      expr3->evaluate(tentative, ref, result, pos);
  }
}

void reference::pre_compute_label()
{
  if (parsed_label != 0
      && (parsed_label->analyze() & expression::CONTAINS_VARIABLE)) {
    label.clear();
    substring_position temp_pos;
    parsed_label->evaluate(1, *this, label, temp_pos);
    label_ptr = lookup_label(label);
  }
}

void reference::compute_label()
{
  label.clear();
  if (parsed_label)
    parsed_label->evaluate(0, *this, label, separator_pos);
  if (short_label_flag && parsed_short_label)
    parsed_short_label->evaluate(0, *this, short_label, short_separator_pos);
  if (date_as_label) {
    string new_date;
    if (parsed_date_label) {
      substring_position temp_pos;
      parsed_date_label->evaluate(0, *this, new_date, temp_pos);
    }
    set_date(new_date);
  }
  if (label_ptr)
    label_ptr->count += 1;
}

void reference::immediate_compute_label()
{
  if (label_ptr)
    label_ptr->total = 2;	// force use of disambiguator
  compute_label();
}

int reference::merge_labels(reference **v, int n, label_type type,
			    string &result)
{
  if (abbreviate_label_ranges)
    return merge_labels_by_number(v, n, type, result);
  else
    return merge_labels_by_parts(v, n, type, result);
}

int reference::merge_labels_by_number(reference **v, int n, label_type type,
				      string &result)
{
  if (n <= 1)
    return 0;
  int num = get_number();
  // Only merge three or more labels.
  if (v[0]->get_number() != num + 1
      || v[1]->get_number() != num + 2)
    return 0;
  for (int i = 2; i < n; i++)
    if (v[i]->get_number() != num + i + 1)
      break;
  result = get_label(type);
  result += label_range_indicator;
  result += v[i - 1]->get_label(type);
  return i;
}

const substring_position &reference::get_separator_pos(label_type type) const
{
  if (type == SHORT_LABEL && short_label_flag)
    return short_separator_pos;
  else
    return separator_pos;
}

const string &reference::get_label(label_type type) const
{
  if (type == SHORT_LABEL && short_label_flag)
    return short_label; 
  else
    return label;
}

int reference::merge_labels_by_parts(reference **v, int n, label_type type,
				     string &result)
{
  if (n <= 0)
    return 0;
  const string &lb = get_label(type);
  const substring_position &sp = get_separator_pos(type);
  if (sp.start < 0
      || sp.start != v[0]->get_separator_pos(type).start 
      || memcmp(lb.contents(), v[0]->get_label(type).contents(),
		sp.start) != 0)
    return 0;
  result = lb;
  int i = 0;
  do {
    result += separate_label_second_parts;
    const substring_position &s = v[i]->get_separator_pos(type);
    int sep_end_pos = s.start + s.length;
    result.append(v[i]->get_label(type).contents() + sep_end_pos,
		  v[i]->get_label(type).length() - sep_end_pos);
  } while (++i < n
	   && sp.start == v[i]->get_separator_pos(type).start
	   && memcmp(lb.contents(), v[i]->get_label(type).contents(),
		     sp.start) == 0);
  return i;
}

string label_pool;

label_info::label_info(const string &s)
: count(0), total(1), length(s.length()), start(label_pool.length())
{
  label_pool += s;
}

static label_info **label_table = 0;
static int label_table_size = 0;
static int label_table_used = 0;

label_info *lookup_label(const string &label)
{
  if (label_table == 0) {
    label_table = new label_info *[17];
    label_table_size = 17;
    for (int i = 0; i < 17; i++)
      label_table[i] = 0;
  }
  unsigned h = hash_string(label.contents(), label.length()) % label_table_size;
  for (label_info **ptr = label_table + h;
       *ptr != 0;
       (ptr == label_table)
       ? (ptr = label_table + label_table_size - 1)
       : ptr--)
    if ((*ptr)->length == label.length()
	&& memcmp(label_pool.contents() + (*ptr)->start, label.contents(),
		  label.length()) == 0) {
      (*ptr)->total += 1;
      return *ptr;
    }
  label_info *result = *ptr = new label_info(label);
  if (++label_table_used * 2 > label_table_size) {
    // Rehash the table.
    label_info **old_table = label_table;
    int old_size = label_table_size;
    label_table_size = next_size(label_table_size);
    label_table = new label_info *[label_table_size];
    int i;
    for (i = 0; i < label_table_size; i++)
      label_table[i] = 0;
    for (i = 0; i < old_size; i++)
      if (old_table[i]) {
	unsigned h = hash_string(label_pool.contents() + old_table[i]->start,
				 old_table[i]->length);
	for (label_info **p = label_table + (h % label_table_size);
	     *p != 0;
	     (p == label_table)
	     ? (p = label_table + label_table_size - 1)
	     : --p)
	    ;
	*p = old_table[i];
	}
    a_delete old_table;
  }
  return result;
}

void clear_labels()
{
  for (int i = 0; i < label_table_size; i++) {
    delete label_table[i];
    label_table[i] = 0;
  }
  label_table_used = 0;
  label_pool.clear();
}

static void consider_authors(reference **start, reference **end, int i);

void compute_labels(reference **v, int n)
{
  if (parsed_label
      && (parsed_label->analyze() & expression::CONTAINS_AT)
      && sort_fields.length() >= 2
      && sort_fields[0] == 'A'
      && sort_fields[1] == '+')
    consider_authors(v, v + n, 0);
  for (int i = 0; i < n; i++)
    v[i]->compute_label();
}


/* A reference with a list of authors <A0,A1,...,AN> _needs_ author i
where 0 <= i <= N if there exists a reference with a list of authors
<B0,B1,...,BM> such that <A0,A1,...,AN> != <B0,B1,...,BM> and M >= i
and Aj = Bj for 0 <= j < i. In this case if we can't say ``A0,
A1,...,A(i-1) et al'' because this would match both <A0,A1,...,AN> and
<B0,B1,...,BM>.  If a reference needs author i we only have to call
need_author(j) for some j >= i such that the reference also needs
author j. */

/* This function handles 2 tasks:
determine which authors are needed (cannot be elided with et al.);
determine which authors can have only last names in the labels.

References >= start and < end have the same first i author names.
Also they're sorted by A+. */

static void consider_authors(reference **start, reference **end, int i)
{
  if (start >= end)
    return;
  reference **p = start;
  if (i >= (*p)->get_nauthors()) {
    for (++p; p < end && i >= (*p)->get_nauthors(); p++)
      ;
    if (p < end && i > 0) {
      // If we have an author list <A B C> and an author list <A B C D>,
      // then both lists need C.
      for (reference **q = start; q < end; q++)
	(*q)->need_author(i - 1);
    }
    start = p;
  }
  while (p < end) {
    reference **last_name_start = p;
    reference **name_start = p;
    for (++p;
	 p < end && i < (*p)->get_nauthors()
	 && same_author_last_name(**last_name_start, **p, i);
	 p++) {
      if (!same_author_name(**name_start, **p, i)) {
	consider_authors(name_start, p, i + 1);
	name_start = p;
      }
    }
    consider_authors(name_start, p, i + 1);
    if (last_name_start == name_start) {
      for (reference **q = last_name_start; q < p; q++)
	(*q)->set_last_name_unambiguous(i);
    }
    // If we have an author list <A B C D> and <A B C E>, then the lists
    // need author D and E respectively.
    if (name_start > start || p < end) {
      for (reference **q = last_name_start; q < p; q++)
	(*q)->need_author(i);
    }
  }
}

int same_author_last_name(const reference &r1, const reference &r2, int n)
{
  const char *ae1;
  const char *as1 = r1.get_sort_field(0, n, 0, &ae1);
  assert(as1 != 0);
  const char *ae2;
  const char *as2 = r2.get_sort_field(0, n, 0, &ae2);
  assert(as2 != 0);
  return ae1 - as1 == ae2 - as2 && memcmp(as1, as2, ae1 - as1) == 0;
}

int same_author_name(const reference &r1, const reference &r2, int n)
{
  const char *ae1;
  const char *as1 = r1.get_sort_field(0, n, -1, &ae1);
  assert(as1 != 0);
  const char *ae2;
  const char *as2 = r2.get_sort_field(0, n, -1, &ae2);
  assert(as2 != 0);
  return ae1 - as1 == ae2 - as2 && memcmp(as1, as2, ae1 - as1) == 0;
}


void int_set::set(int i)
{
  assert(i >= 0);
  int bytei = i >> 3;
  if (bytei >= v.length()) {
    int old_length = v.length();
    v.set_length(bytei + 1);
    for (int j = old_length; j <= bytei; j++)
      v[j] = 0;
  }
  v[bytei] |= 1 << (i & 7);
}

int int_set::get(int i) const
{
  assert(i >= 0);
  int bytei = i >> 3;
  return bytei >= v.length() ? 0 : (v[bytei] & (1 << (i & 7))) != 0;
}

void reference::set_last_name_unambiguous(int i)
{
  last_name_unambiguous.set(i);
}

void reference::need_author(int n)
{
  if (n > last_needed_author)
    last_needed_author = n;
}

const char *reference::get_authors(const char **end) const
{
  if (!computed_authors) {
    ((reference *)this)->computed_authors = 1;
    string &result = ((reference *)this)->authors;
    int na = get_nauthors();
    result.clear();
    for (int i = 0; i < na; i++) {
      if (last_name_unambiguous.get(i)) {
	const char *e, *start = get_author_last_name(i, &e);
	assert(start != 0);
	result.append(start, e - start);
      }
      else {
	const char *e, *start = get_author(i, &e);
	assert(start != 0);
	result.append(start, e - start);
      }
      if (i == last_needed_author
	  && et_al.length() > 0
	  && et_al_min_elide > 0
	  && last_needed_author + et_al_min_elide < na
	  && na >= et_al_min_total) {
	result += et_al;
	break;
      }
      if (i < na - 1) {
	if (na == 2)
	  result += join_authors_exactly_two;
	else if (i < na - 2)
	  result += join_authors_default;
	else
	  result += join_authors_last_two;
      }
    }
  }
  const char *start = authors.contents();
  *end = start + authors.length();
  return start;
}

int reference::get_nauthors() const
{
  if (nauthors < 0) {
    const char *dummy;
    for (int na = 0; get_author(na, &dummy) != 0; na++)
      ;
    ((reference *)this)->nauthors = na;
  }
  return nauthors;
}
