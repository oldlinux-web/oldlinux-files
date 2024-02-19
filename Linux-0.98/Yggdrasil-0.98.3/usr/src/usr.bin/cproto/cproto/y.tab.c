extern char *malloc(), *realloc();
# define T_IDENTIFIER 257
# define T_TYPEDEF_NAME 258
# define T_AUTO 259
# define T_EXTERN 260
# define T_REGISTER 261
# define T_STATIC 262
# define T_TYPEDEF 263
# define T_INLINE 264
# define T_CHAR 265
# define T_DOUBLE 266
# define T_FLOAT 267
# define T_INT 268
# define T_VOID 269
# define T_LONG 270
# define T_SHORT 271
# define T_SIGNED 272
# define T_UNSIGNED 273
# define T_ENUM 274
# define T_STRUCT 275
# define T_UNION 276
# define T_TYPE_QUALIFIER 277
# define T_LBRACE 278
# define T_MATCHRBRACE 279
# define T_BRACKETS 280
# define T_ELLIPSIS 281
# define T_INITIALIZER 282
# define T_STRING_LITERAL 283
# define T_ASM 284
# define T_ASMARG 285

# line 67 "grammar.y"
#include <stdio.h>
#include "cproto.h"
#include "symbol.h"
#include "semantic.h"

#define YYMAXDEPTH 150

/* Declaration specifier attributes for the typedef statement currently being
 * scanned.
 */
static int cur_decl_spec_flags;

/* Pointer to parameter list for the current function definition. */
static ParameterList *func_params;

/* Pointer to current declarator in function parameter declaration. */
static Declarator *cur_declarator;

/* temporary string buffer */
static char buf[MAX_TEXT_SIZE];

/* Table of typedef names */
static SymbolTable *typedef_names;

/* Table of type qualifiers */
static SymbolTable *type_qualifiers;
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 618 "grammar.y"

#ifdef MSDOS
#include "lex_yy.c"
#else
#include "lex.yy.c"
#endif

void
yyerror (msg)
char *msg;
{
    func_params = NULL;
    put_error();
    fprintf(stderr, "%s\n", msg);
}

/* Initialize the table of type qualifier keywords recognized by the lexical
 * analyzer.
 */
void
init_parser ()
{
    static char *keywords[] = {
	"const", "volatile",
#ifdef MSDOS
	"_cdecl", "_export", "_far", "_fastcall", "_fortran", "_huge",
	"_interrupt", "_loadds", "_near", "_pascal", "_saveregs", "_segment",
	"_cs", "_ds", "_es", "_ss", "_seg",
	"cdecl", "far", "huge", "interrupt", "near", "pascal",
#endif
    };
    int i;

    /* Initialize type qualifier table. */
    type_qualifiers = new_symbol_table();
    for (i = 0; i < sizeof(keywords)/sizeof(keywords[0]); ++i) {
	new_symbol(type_qualifiers, keywords[i], DS_EXTERN);
    }
}

/* Process the C source file.  Write function prototypes to the standard
 * output.  Convert function definitions and write the converted source
 * code to a temporary file.
 */
void
process_file (infile, name)
FILE *infile;
char *name;
{
    char *s;

    if (strlen(name) > 2) {
	s = name + strlen(name) - 2;
	if (*s == '.') {
	    ++s;
	    if (*s == 'l' || *s == 'y')
		BEGIN LEXYACC;
#ifdef MSDOS
	    if (*s == 'L' || *s == 'Y')
		BEGIN LEXYACC;
#endif
	}
    }

    included_files = new_symbol_table();
    typedef_names = new_symbol_table();
    inc_depth = -1;
    curly = 0;
    ly_count = 0;
    func_params = NULL;
    yyin = infile;
    include_file(name, func_style != FUNC_NONE);
    if (proto_comments)
	printf("/* %s */\n", cur_file_name());
    yyparse();
    free_symbol_table(typedef_names);
    free_symbol_table(included_files);
}
int yyexca[] ={
-1, 0,
	0, 1,
	-2, 0,
-1, 1,
	0, -1,
	-2, 0,
-1, 2,
	0, 2,
	-2, 0,
-1, 51,
	59, 63,
	44, 63,
	-2, 23,
	};
# define YYNPROD 105
# define YYLAST 467
int yyact[]={

    21,    45,    19,    57,   116,   136,   131,    72,   115,    67,
    68,   103,    48,   122,    76,   117,    71,    44,   118,   125,
    60,   113,   128,    19,    19,    38,    22,    54,    23,    24,
    48,    25,    26,    27,    28,    29,    34,    30,    31,    32,
    33,    40,    41,    42,    37,   113,    84,    19,   120,    38,
    22,    54,    23,    24,    10,    25,    26,    27,    28,    29,
    34,    30,    31,    32,    33,    40,    41,    42,    37,    21,
   110,    19,    88,   112,   108,   113,   107,   137,   135,   134,
    78,   106,    53,   105,    93,    99,    79,   119,    49,    38,
    22,    54,    23,    24,    10,    25,    26,    27,    28,    29,
    34,    30,    31,    32,    33,    40,    41,    42,    37,    48,
   133,    89,    38,    22,    54,    23,    24,    65,    25,    26,
    27,    28,    29,    34,    30,    31,    32,    33,    40,    41,
    42,    37,    38,    22,    54,    23,    24,    46,    25,    26,
    27,    28,    29,    34,    30,    31,    32,    33,    40,    41,
    42,    37,    38,    21,    21,    19,    98,    96,    69,    74,
   127,     3,    56,   129,    43,    75,   101,   100,    77,     6,
     5,    37,   132,     2,    73,     1,    62,    61,    39,    87,
   121,    85,    86,    50,   126,   123,    36,    35,    17,    16,
     0,     0,     0,     0,     0,     0,     0,    15,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    58,     0,     0,     0,     8,    20,    38,    22,
    12,    23,    24,    10,    25,    26,    27,    28,    29,    34,
    30,    31,    32,    33,    40,    41,    42,    37,    20,    38,
    22,    54,    23,    24,     7,    25,    26,    27,    28,    29,
    34,    30,    31,    32,    33,    40,    41,    42,    37,   124,
    59,   114,    20,    38,    22,    54,    23,    24,     0,    25,
    26,    27,    28,    29,    34,    30,    31,    32,    33,    40,
    41,    42,    37,     0,     0,   114,    20,    38,    22,    54,
    23,    24,    20,    25,    26,    27,    28,    29,    34,    30,
    31,    32,    33,    40,    41,    42,    37,    81,     9,    58,
     9,    13,    14,     0,    47,   114,     0,     0,    55,     4,
     0,    52,     0,     0,     0,     0,     0,    38,    22,    54,
    23,    24,     0,    25,    26,    27,    28,    29,    34,    30,
    31,    32,    33,    40,    41,    42,    37,    11,     0,     0,
     0,     0,     0,    18,    66,    70,    48,    51,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    52,    90,    64,
    20,    20,    82,    63,    91,     0,    80,    83,     0,     0,
    94,     0,     0,     0,    95,     0,     0,     0,     0,     0,
     0,     0,     0,    52,     0,    80,     0,     0,     0,   104,
     0,     0,    52,   111,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    90,    92,     0,     0,     0,
     0,    90,    97,     0,     0,   102,   111,     0,     0,    97,
     0,     0,     0,    90,     0,     0,     0,     0,   109,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    64,     0,     0,     0,     0,   130 };
int yypact[]={

   -40, -1000,   -40, -1000, -1000,   -42, -1000,  -284,    78,    29,
  -126, -1000,  -280, -1000,   114,   -20, -1000, -1000, -1000,  -106,
 -1000,   113, -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
 -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,  -248,
  -248, -1000, -1000, -1000, -1000,   -43, -1000, -1000,  -272, -1000,
   115,   -47, -1000, -1000, -1000,  -126,  -209,  -169,   -20, -1000,
  -146,   -18,  -106, -1000,    43,  -266, -1000, -1000, -1000,  -266,
 -1000, -1000, -1000, -1000,   113,  -209, -1000,   113,  -267,  -209,
 -1000,    29, -1000, -1000,    42,    40,    32,    30, -1000, -1000,
     5, -1000, -1000, -1000, -1000, -1000, -1000,   -47,  -270,  -278,
   -44,   -26, -1000, -1000, -1000, -1000, -1000,  -233,  -244, -1000,
 -1000,    35,   -21,   -19, -1000, -1000, -1000, -1000,   113,  -273,
 -1000, -1000, -1000,   -21, -1000,    69,    38,    37, -1000,  -274,
 -1000, -1000,    36, -1000, -1000, -1000, -1000, -1000 };
int yypgo[]={

     0,   307,   311,   189,   188,   353,   187,   186,   183,    82,
   347,   197,    70,    73,    46,   182,    72,   181,   179,   178,
   312,   177,   176,   117,   175,   173,   161,   319,   170,   169,
   314,   168,   167,   166,   165,    80,   163,   162,    87,    86,
    85 };
int yyr1[]={

     0,    24,    24,    25,    25,    26,    26,    26,    26,    26,
    26,    26,    30,    29,    29,    27,    27,    31,    27,    32,
    32,    33,    33,    34,    36,    28,    37,    38,    28,    35,
    35,    39,    39,     1,     1,     2,     2,     2,     3,     3,
     3,     3,     3,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     5,     5,     6,     6,     6,    19,
    19,     8,     8,     9,    40,     9,     7,     7,     7,    23,
    23,    10,    10,    11,    11,    11,    11,    11,    20,    20,
    21,    21,    22,    22,    14,    14,    15,    15,    16,    16,
    16,    17,    17,    18,    18,    12,    12,    12,    13,    13,
    13,    13,    13,    13,    13 };
int yyr2[]={

     0,     0,     2,     2,     4,     2,     2,     4,     2,     6,
     4,     4,     4,     6,     6,     5,     7,     1,    10,     0,
     2,     3,     7,     1,     1,    14,     1,     1,    12,     0,
     2,     2,     4,     2,     5,     2,     2,     2,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     2,     2,     3,     3,     7,     5,     5,     2,
     2,     3,     7,     3,     1,     8,     7,     5,     5,     2,
     2,     5,     2,     3,     7,     5,     9,     9,     5,     7,
     1,     2,     3,     5,     2,     7,     3,     7,     5,     5,
     3,     1,     2,     3,     7,     3,     5,     2,     7,     5,
     3,     9,     7,     7,     5 };
int yychk[]={

 -1000,   -24,   -25,   -26,   -27,   -28,   -29,   284,   256,    -1,
   263,   -10,   260,    -2,   -20,   -11,    -3,    -4,    -5,    42,
   257,    40,   259,   261,   262,   264,   265,   266,   267,   268,
   270,   271,   272,   273,   269,    -6,    -7,   277,   258,   -19,
   274,   275,   276,   -26,    59,   285,    59,   -30,   278,    59,
    -8,   -10,    -2,    -9,   260,    -1,   -37,   283,   -11,   280,
    40,   -21,   -22,    -5,   -10,   -23,   -30,   257,   258,   -23,
   -30,    59,   279,    59,    44,   -34,    61,   -31,   -35,   -39,
   -27,    -1,   -30,   -27,   -14,   -17,   -15,   -18,   -16,   257,
    -1,   -20,    -5,    41,   -30,   -30,    -9,   -10,   -35,   -40,
   -32,   -33,   -10,   278,   -27,    41,    41,    44,    44,   -10,
   -12,   -20,   -13,    40,   280,   278,   282,    59,    44,   -38,
   281,   -16,   257,   -13,   280,    40,   -12,   -14,    41,   -36,
   -10,   279,   -14,    41,    41,    41,   279,    41 };
int yydef[]={

    -2,    -2,    -2,     3,     5,     6,     8,     0,     0,     0,
     0,    26,    39,    33,     0,    72,    35,    36,    37,    80,
    73,     0,    38,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,    54,    55,     0,
     0,    59,    60,     4,     7,     0,    10,    11,     0,    15,
     0,    -2,    34,    61,    39,    17,    29,     0,    71,    75,
    91,    78,    81,    82,     0,    58,    57,    69,    70,    68,
    67,     9,    12,    16,     0,    29,    64,    19,     0,    30,
    31,     0,    13,    14,     0,     0,    84,    92,    86,    93,
    90,    79,    83,    74,    56,    66,    62,    63,     0,     0,
     0,    20,    21,    27,    32,    76,    77,     0,     0,    88,
    89,    95,    97,     0,   100,    24,    65,    18,     0,     0,
    85,    87,    94,    96,    99,     0,     0,     0,   104,     0,
    22,    28,     0,   102,    98,   103,    25,   101 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"T_IDENTIFIER",	257,
	"T_TYPEDEF_NAME",	258,
	"T_AUTO",	259,
	"T_EXTERN",	260,
	"T_REGISTER",	261,
	"T_STATIC",	262,
	"T_TYPEDEF",	263,
	"T_INLINE",	264,
	"T_CHAR",	265,
	"T_DOUBLE",	266,
	"T_FLOAT",	267,
	"T_INT",	268,
	"T_VOID",	269,
	"T_LONG",	270,
	"T_SHORT",	271,
	"T_SIGNED",	272,
	"T_UNSIGNED",	273,
	"T_ENUM",	274,
	"T_STRUCT",	275,
	"T_UNION",	276,
	"T_TYPE_QUALIFIER",	277,
	"T_LBRACE",	278,
	"T_MATCHRBRACE",	279,
	"T_BRACKETS",	280,
	"T_ELLIPSIS",	281,
	"T_INITIALIZER",	282,
	"T_STRING_LITERAL",	283,
	"T_ASM",	284,
	"T_ASMARG",	285,
	"(",	40,
	"*",	42,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"program : /* empty */",
	"program : translation_unit",
	"translation_unit : external_declaration",
	"translation_unit : translation_unit external_declaration",
	"external_declaration : declaration",
	"external_declaration : function_definition",
	"external_declaration : function_definition ';'",
	"external_declaration : linkage_specification",
	"external_declaration : T_ASM T_ASMARG ';'",
	"external_declaration : error ';'",
	"external_declaration : error braces",
	"braces : T_LBRACE T_MATCHRBRACE",
	"linkage_specification : T_EXTERN T_STRING_LITERAL braces",
	"linkage_specification : T_EXTERN T_STRING_LITERAL declaration",
	"declaration : decl_specifiers ';'",
	"declaration : decl_specifiers init_declarator_list ';'",
	"declaration : T_TYPEDEF decl_specifiers",
	"declaration : T_TYPEDEF decl_specifiers opt_declarator_list ';'",
	"opt_declarator_list : /* empty */",
	"opt_declarator_list : declarator_list",
	"declarator_list : declarator",
	"declarator_list : declarator_list ',' declarator",
	"function_definition : decl_specifiers declarator",
	"function_definition : decl_specifiers declarator opt_declaration_list T_LBRACE",
	"function_definition : decl_specifiers declarator opt_declaration_list T_LBRACE T_MATCHRBRACE",
	"function_definition : declarator",
	"function_definition : declarator opt_declaration_list T_LBRACE",
	"function_definition : declarator opt_declaration_list T_LBRACE T_MATCHRBRACE",
	"opt_declaration_list : /* empty */",
	"opt_declaration_list : declaration_list",
	"declaration_list : declaration",
	"declaration_list : declaration_list declaration",
	"decl_specifiers : decl_specifier",
	"decl_specifiers : decl_specifiers decl_specifier",
	"decl_specifier : storage_class",
	"decl_specifier : type_specifier",
	"decl_specifier : type_qualifier",
	"storage_class : T_AUTO",
	"storage_class : T_EXTERN",
	"storage_class : T_REGISTER",
	"storage_class : T_STATIC",
	"storage_class : T_INLINE",
	"type_specifier : T_CHAR",
	"type_specifier : T_DOUBLE",
	"type_specifier : T_FLOAT",
	"type_specifier : T_INT",
	"type_specifier : T_LONG",
	"type_specifier : T_SHORT",
	"type_specifier : T_SIGNED",
	"type_specifier : T_UNSIGNED",
	"type_specifier : T_VOID",
	"type_specifier : struct_or_union_specifier",
	"type_specifier : enum_specifier",
	"type_qualifier : T_TYPE_QUALIFIER",
	"type_qualifier : T_TYPEDEF_NAME",
	"struct_or_union_specifier : struct_or_union any_id braces",
	"struct_or_union_specifier : struct_or_union braces",
	"struct_or_union_specifier : struct_or_union any_id",
	"struct_or_union : T_STRUCT",
	"struct_or_union : T_UNION",
	"init_declarator_list : init_declarator",
	"init_declarator_list : init_declarator_list ',' init_declarator",
	"init_declarator : declarator",
	"init_declarator : declarator '='",
	"init_declarator : declarator '=' T_INITIALIZER",
	"enum_specifier : T_ENUM any_id braces",
	"enum_specifier : T_ENUM braces",
	"enum_specifier : T_ENUM any_id",
	"any_id : T_IDENTIFIER",
	"any_id : T_TYPEDEF_NAME",
	"declarator : pointer direct_declarator",
	"declarator : direct_declarator",
	"direct_declarator : T_IDENTIFIER",
	"direct_declarator : '(' declarator ')'",
	"direct_declarator : direct_declarator T_BRACKETS",
	"direct_declarator : direct_declarator '(' parameter_type_list ')'",
	"direct_declarator : direct_declarator '(' opt_identifier_list ')'",
	"pointer : '*' opt_type_qualifiers",
	"pointer : '*' opt_type_qualifiers pointer",
	"opt_type_qualifiers : /* empty */",
	"opt_type_qualifiers : type_qualifier_list",
	"type_qualifier_list : type_qualifier",
	"type_qualifier_list : type_qualifier_list type_qualifier",
	"parameter_type_list : parameter_list",
	"parameter_type_list : parameter_list ',' T_ELLIPSIS",
	"parameter_list : parameter_declaration",
	"parameter_list : parameter_list ',' parameter_declaration",
	"parameter_declaration : decl_specifiers declarator",
	"parameter_declaration : decl_specifiers abs_declarator",
	"parameter_declaration : decl_specifiers",
	"opt_identifier_list : /* empty */",
	"opt_identifier_list : identifier_list",
	"identifier_list : T_IDENTIFIER",
	"identifier_list : identifier_list ',' T_IDENTIFIER",
	"abs_declarator : pointer",
	"abs_declarator : pointer direct_abs_declarator",
	"abs_declarator : direct_abs_declarator",
	"direct_abs_declarator : '(' abs_declarator ')'",
	"direct_abs_declarator : direct_abs_declarator T_BRACKETS",
	"direct_abs_declarator : T_BRACKETS",
	"direct_abs_declarator : direct_abs_declarator '(' parameter_type_list ')'",
	"direct_abs_declarator : direct_abs_declarator '(' ')'",
	"direct_abs_declarator : '(' parameter_type_list ')'",
	"direct_abs_declarator : '(' ')'",
};
#endif /* YYDEBUG */
#line 1 "/usr/lib/yaccpar"
/*	@(#)yaccpar 1.10 89/04/04 SMI; from S5R3 1.10	*/

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	{ free(yys); free(yyv); return(0); }
#define YYABORT		{ free(yys); free(yyv); return(1); }
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** static variables used by the parser
*/
static YYSTYPE *yyv;			/* value stack */
static int *yys;			/* state stack */

static YYSTYPE *yypv;			/* top of value stack */
static int *yyps;			/* top of state stack */

static int yystate;			/* current state */
static int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */

int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */


/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */
	unsigned yymaxdepth = YYMAXDEPTH;

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yyv = (YYSTYPE*)malloc(yymaxdepth*sizeof(YYSTYPE));
	yys = (int*)malloc(yymaxdepth*sizeof(int));
	if (!yyv || !yys)
	{
		yyerror( "out of memory" );
		return(1);
	}
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			(void)printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				(void)printf( "end-of-file\n" );
			else if ( yychar < 0 )
				(void)printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				(void)printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			int yyps_index = (yy_ps - yys);
			int yypv_index = (yy_pv - yyv);
			int yypvt_index = (yypvt - yyv);
			yymaxdepth += YYMAXDEPTH;
			yyv = (YYSTYPE*)realloc((char*)yyv,
				yymaxdepth * sizeof(YYSTYPE));
			yys = (int*)realloc((char*)yys,
				yymaxdepth * sizeof(int));
			if (!yyv || !yys)
			{
				yyerror( "yacc stack overflow" );
				return(1);
			}
			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			(void)printf( "Received token " );
			if ( yychar == 0 )
				(void)printf( "end-of-file\n" );
			else if ( yychar < 0 )
				(void)printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				(void)printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				(void)printf( "Received token " );
				if ( yychar == 0 )
					(void)printf( "end-of-file\n" );
				else if ( yychar < 0 )
					(void)printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					(void)printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						(void)printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					(void)printf( "Error recovery discards " );
					if ( yychar == 0 )
						(void)printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						(void)printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						(void)printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			(void)printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 15:
# line 127 "grammar.y"
{
	    free_decl_spec(&yypvt[-1].decl_spec);
	} break;
case 16:
# line 131 "grammar.y"
{
	    check_untagged(&yypvt[-2].decl_spec);
	    if (func_params != NULL) {
		set_param_types(func_params, &yypvt[-2].decl_spec, &yypvt[-1].decl_list);
	    } else {
		gen_declarations(&yypvt[-2].decl_spec, &yypvt[-1].decl_list);
		free_decl_list(&yypvt[-1].decl_list);
	    }
	    free_decl_spec(&yypvt[-2].decl_spec);
	} break;
case 17:
# line 142 "grammar.y"
{
	    cur_decl_spec_flags = yypvt[-0].decl_spec.flags;
	    free_decl_spec(&yypvt[-0].decl_spec);
	} break;
case 21:
# line 156 "grammar.y"
{
	    new_symbol(typedef_names, yypvt[-0].declarator->name, cur_decl_spec_flags);
	    free_declarator(yypvt[-0].declarator);
	} break;
case 22:
# line 161 "grammar.y"
{
	    new_symbol(typedef_names, yypvt[-0].declarator->name, cur_decl_spec_flags);
	    free_declarator(yypvt[-0].declarator);
	} break;
case 23:
# line 169 "grammar.y"
{
	    check_untagged(&yypvt[-1].decl_spec);
	    if (yypvt[-0].declarator->func_def == FUNC_NONE) {
		yyerror("syntax error");
		YYERROR;
	    }
	    func_params = &(yypvt[-0].declarator->head->params);
	    func_params->begin_comment = cur_file->begin_comment;
	    func_params->end_comment = cur_file->end_comment;
	} break;
case 24:
# line 180 "grammar.y"
{
	    func_params = NULL;

	    if (cur_file->convert)
		gen_func_definition(&yypvt[-4].decl_spec, yypvt[-3].declarator);
	    gen_prototype(&yypvt[-4].decl_spec, yypvt[-3].declarator);
	    free_decl_spec(&yypvt[-4].decl_spec);
	    free_declarator(yypvt[-3].declarator);
	} break;
case 26:
# line 191 "grammar.y"
{
	    if (yypvt[-0].declarator->func_def == FUNC_NONE) {
		yyerror("syntax error");
		YYERROR;
	    }
	    func_params = &(yypvt[-0].declarator->head->params);
	    func_params->begin_comment = cur_file->begin_comment;
	    func_params->end_comment = cur_file->end_comment;
	} break;
case 27:
# line 201 "grammar.y"
{
	    DeclSpec decl_spec;

	    func_params = NULL;

	    new_decl_spec(&decl_spec, "int", yypvt[-3].declarator->begin, DS_EXTERN);
	    if (cur_file->convert)
		gen_func_definition(&decl_spec, yypvt[-3].declarator);
	    gen_prototype(&decl_spec, yypvt[-3].declarator);
	    free_decl_spec(&decl_spec);
	    free_declarator(yypvt[-3].declarator);
	} break;
case 34:
# line 229 "grammar.y"
{
	    join_decl_specs(&yyval.decl_spec, &yypvt[-1].decl_spec, &yypvt[-0].decl_spec);
	    free(yypvt[-1].decl_spec.text);
	    free(yypvt[-0].decl_spec.text);
	} break;
case 38:
# line 244 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_EXTERN);
	} break;
case 39:
# line 248 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_JUNK);
	} break;
case 40:
# line 252 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_EXTERN);
	} break;
case 41:
# line 256 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_STATIC);
	} break;
case 42:
# line 260 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_JUNK);
	} break;
case 43:
# line 267 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_EXTERN | DS_CHAR);
	} break;
case 44:
# line 271 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_EXTERN);
	} break;
case 45:
# line 275 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_EXTERN | DS_FLOAT);
	} break;
case 46:
# line 279 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_EXTERN);
	} break;
case 47:
# line 283 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_EXTERN);
	} break;
case 48:
# line 287 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_EXTERN | DS_SHORT);
	} break;
case 49:
# line 291 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_EXTERN);
	} break;
case 50:
# line 295 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_EXTERN);
	} break;
case 51:
# line 299 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_EXTERN);
	} break;
case 54:
# line 308 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, DS_EXTERN);
	} break;
case 55:
# line 312 "grammar.y"
{
	    /* A typedef name is actually a type specifier, but since the
	     * typedef symbol table also stores #define names, this production
	     * is here so the <pointer> nonterminal will scan #define names.
	     */
	    Symbol *s;
	    s = find_symbol(typedef_names, yypvt[-0].text.text);
	    new_decl_spec(&yyval.decl_spec, yypvt[-0].text.text, yypvt[-0].text.begin, s->flags);
	} break;
case 56:
# line 325 "grammar.y"
{
	    sprintf(buf, "%s %s", yypvt[-2].text.text, yypvt[-1].text.text);
	    new_decl_spec(&yyval.decl_spec, buf, yypvt[-2].text.begin, DS_EXTERN);
	} break;
case 57:
# line 330 "grammar.y"
{
	    sprintf(buf, "%s {}", yypvt[-1].text.text);
	    new_decl_spec(&yyval.decl_spec, buf, yypvt[-1].text.begin, DS_EXTERN);
	} break;
case 58:
# line 335 "grammar.y"
{
	    sprintf(buf, "%s %s", yypvt[-1].text.text, yypvt[-0].text.text);
	    new_decl_spec(&yyval.decl_spec, buf, yypvt[-1].text.begin, DS_EXTERN);
	} break;
case 61:
# line 348 "grammar.y"
{
	    new_decl_list(&yyval.decl_list, yypvt[-0].declarator);
	} break;
case 62:
# line 352 "grammar.y"
{
	    add_decl_list(&yyval.decl_list, &yypvt[-2].decl_list, yypvt[-0].declarator);
	} break;
case 63:
# line 359 "grammar.y"
{
	    if (yypvt[-0].declarator->func_def != FUNC_NONE && func_params == NULL &&
		func_style == FUNC_TRADITIONAL && cur_file->convert) {
		gen_func_declarator(yypvt[-0].declarator);
		fputs(yytext, cur_file->tmp_file);
	    }
	    cur_declarator = yyval.declarator;
	} break;
case 64:
# line 368 "grammar.y"
{
	    if (yypvt[-1].declarator->func_def != FUNC_NONE && func_params == NULL &&
		func_style == FUNC_TRADITIONAL && cur_file->convert) {
		gen_func_declarator(yypvt[-1].declarator);
		fputs(" =", cur_file->tmp_file);
	    }
	} break;
case 66:
# line 380 "grammar.y"
{
	    sprintf(buf, "enum %s", yypvt[-1].text.text);
	    new_decl_spec(&yyval.decl_spec, buf, yypvt[-2].text.begin, DS_EXTERN);
	} break;
case 67:
# line 385 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "enum {}", yypvt[-1].text.begin, DS_EXTERN);
	} break;
case 68:
# line 389 "grammar.y"
{
	    sprintf(buf, "enum %s", yypvt[-0].text.text);
	    new_decl_spec(&yyval.decl_spec, buf, yypvt[-1].text.begin, DS_EXTERN);
	} break;
case 71:
# line 402 "grammar.y"
{
	    yyval.declarator = yypvt[-0].declarator;
	    sprintf(buf, "%s%s", yypvt[-1].text.text, yyval.declarator->text);
	    free(yyval.declarator->text);
	    yyval.declarator->text = xstrdup(buf);
	    yyval.declarator->begin = yypvt[-1].text.begin;
	} break;
case 73:
# line 414 "grammar.y"
{
	    yyval.declarator = new_declarator(yypvt[-0].text.text, yypvt[-0].text.text, yypvt[-0].text.begin);
	} break;
case 74:
# line 418 "grammar.y"
{
	    yyval.declarator = yypvt[-1].declarator;
	    sprintf(buf, "(%s)", yyval.declarator->text);
	    free(yyval.declarator->text);
	    yyval.declarator->text = xstrdup(buf);
	    yyval.declarator->begin = yypvt[-2].text.begin;
	} break;
case 75:
# line 426 "grammar.y"
{
	    yyval.declarator = yypvt[-1].declarator;
	    sprintf(buf, "%s%s", yyval.declarator->text, yypvt[-0].text.text);
	    free(yyval.declarator->text);
	    yyval.declarator->text = xstrdup(buf);
	} break;
case 76:
# line 433 "grammar.y"
{
	    yyval.declarator = new_declarator("%s()", yypvt[-3].declarator->name, yypvt[-3].declarator->begin);
	    yyval.declarator->params = yypvt[-1].param_list;
	    yyval.declarator->func_stack = yypvt[-3].declarator;
	    yyval.declarator->head = (yypvt[-3].declarator->func_stack == NULL) ? yyval.declarator : yypvt[-3].declarator->head;
	    yyval.declarator->func_def = FUNC_ANSI;
	} break;
case 77:
# line 441 "grammar.y"
{
	    yyval.declarator = new_declarator("%s()", yypvt[-3].declarator->name, yypvt[-3].declarator->begin);
	    yyval.declarator->params = yypvt[-1].param_list;
	    yyval.declarator->func_stack = yypvt[-3].declarator;
	    yyval.declarator->head = (yypvt[-3].declarator->func_stack == NULL) ? yyval.declarator : yypvt[-3].declarator->head;
	    yyval.declarator->func_def = FUNC_TRADITIONAL;
	} break;
case 78:
# line 452 "grammar.y"
{
	    sprintf(yyval.text.text, "*%s", yypvt[-0].text.text);
	    yyval.text.begin = yypvt[-1].text.begin;
	} break;
case 79:
# line 457 "grammar.y"
{
	    sprintf(yyval.text.text, "*%s%s", yypvt[-1].text.text, yypvt[-0].text.text);
	    yyval.text.begin = yypvt[-2].text.begin;
	} break;
case 80:
# line 465 "grammar.y"
{
	    strcpy(yyval.text.text, "");
	    yyval.text.begin = 0L;
	} break;
case 82:
# line 474 "grammar.y"
{
	    strcpy(yyval.text.text, yypvt[-0].decl_spec.text);
	    yyval.text.begin = yypvt[-0].decl_spec.begin;
	    free(yypvt[-0].decl_spec.text);
	} break;
case 83:
# line 480 "grammar.y"
{
	    sprintf(yyval.text.text, "%s %s ", yypvt[-1].text.text, yypvt[-0].decl_spec.text);
	    yyval.text.begin = yypvt[-1].text.begin;
	    free(yypvt[-0].decl_spec.text);
	} break;
case 85:
# line 490 "grammar.y"
{
	    add_ident_list(&yyval.param_list, &yypvt[-2].param_list, "...");
	} break;
case 86:
# line 497 "grammar.y"
{
	    new_param_list(&yyval.param_list, &yypvt[-0].parameter);
	} break;
case 87:
# line 501 "grammar.y"
{
	    add_param_list(&yyval.param_list, &yypvt[-2].param_list, &yypvt[-0].parameter);
	} break;
case 88:
# line 508 "grammar.y"
{
	    check_untagged(&yypvt[-1].decl_spec);
	    new_parameter(&yyval.parameter, &yypvt[-1].decl_spec, yypvt[-0].declarator);
	} break;
case 89:
# line 513 "grammar.y"
{
	    check_untagged(&yypvt[-1].decl_spec);
	    new_parameter(&yyval.parameter, &yypvt[-1].decl_spec, yypvt[-0].declarator);
	} break;
case 90:
# line 518 "grammar.y"
{
	    check_untagged(&yypvt[-0].decl_spec);
	    new_parameter(&yyval.parameter, &yypvt[-0].decl_spec, NULL);
	} break;
case 91:
# line 526 "grammar.y"
{
	    new_ident_list(&yyval.param_list);
	} break;
case 93:
# line 534 "grammar.y"
{
	    new_ident_list(&yyval.param_list);
	    add_ident_list(&yyval.param_list, &yyval.param_list, yypvt[-0].text.text);
	} break;
case 94:
# line 539 "grammar.y"
{
	    add_ident_list(&yyval.param_list, &yypvt[-2].param_list, yypvt[-0].text.text);
	} break;
case 95:
# line 546 "grammar.y"
{
	    yyval.declarator = new_declarator(yypvt[-0].text.text, "", yypvt[-0].text.begin);
	} break;
case 96:
# line 550 "grammar.y"
{
	    yyval.declarator = yypvt[-0].declarator;
	    sprintf(buf, "%s%s", yypvt[-1].text.text, yyval.declarator->text);
	    free(yyval.declarator->text);
	    yyval.declarator->text = xstrdup(buf);
	    yyval.declarator->begin = yypvt[-1].text.begin;
	} break;
case 98:
# line 562 "grammar.y"
{
	    yyval.declarator = yypvt[-1].declarator;
	    sprintf(buf, "(%s)", yyval.declarator->text);
	    free(yyval.declarator->text);
	    yyval.declarator->text = xstrdup(buf);
	    yyval.declarator->begin = yypvt[-2].text.begin;
	} break;
case 99:
# line 570 "grammar.y"
{
	    yyval.declarator = yypvt[-1].declarator;
	    sprintf(buf, "%s%s", yyval.declarator->text, yypvt[-0].text.text);
	    free(yyval.declarator->text);
	    yyval.declarator->text = xstrdup(buf);
	} break;
case 100:
# line 577 "grammar.y"
{
	    yyval.declarator = new_declarator(yypvt[-0].text.text, "", yypvt[-0].text.begin);
	} break;
case 101:
# line 581 "grammar.y"
{
	    yyval.declarator = new_declarator("%s()", "", yypvt[-3].declarator->begin);
	    yyval.declarator->params = yypvt[-1].param_list;
	    yyval.declarator->func_stack = yypvt[-3].declarator;
	    yyval.declarator->head = (yypvt[-3].declarator->func_stack == NULL) ? yyval.declarator : yypvt[-3].declarator->head;
	    yyval.declarator->func_def = FUNC_ANSI;
	} break;
case 102:
# line 589 "grammar.y"
{
	    yyval.declarator = new_declarator("%s()", "", yypvt[-2].declarator->begin);
	    yyval.declarator->func_stack = yypvt[-2].declarator;
	    yyval.declarator->head = (yypvt[-2].declarator->func_stack == NULL) ? yyval.declarator : yypvt[-2].declarator->head;
	    yyval.declarator->func_def = FUNC_ANSI;
	} break;
case 103:
# line 596 "grammar.y"
{
	    Declarator *d;
	    
	    d = new_declarator("", "", yypvt[-2].text.begin);
	    yyval.declarator = new_declarator("%s()", "", yypvt[-2].text.begin);
	    yyval.declarator->params = yypvt[-1].param_list;
	    yyval.declarator->func_stack = d;
	    yyval.declarator->head = yyval.declarator;
	    yyval.declarator->func_def = FUNC_ANSI;
	} break;
case 104:
# line 607 "grammar.y"
{
	    Declarator *d;
	    
	    d = new_declarator("", "", yypvt[-1].text.begin);
	    yyval.declarator = new_declarator("%s()", "", yypvt[-1].text.begin);
	    yyval.declarator->func_stack = d;
	    yyval.declarator->head = yyval.declarator;
	    yyval.declarator->func_def = FUNC_ANSI;
	} break;
	}
	goto yystack;		/* reset registers in driver code */
}
