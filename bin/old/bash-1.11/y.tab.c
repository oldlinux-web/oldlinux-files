
/*  A Bison parser, made from parse.y  */

#define	IF	258
#define	THEN	259
#define	ELSE	260
#define	ELIF	261
#define	FI	262
#define	CASE	263
#define	ESAC	264
#define	FOR	265
#define	WHILE	266
#define	UNTIL	267
#define	DO	268
#define	DONE	269
#define	FUNCTION	270
#define	IN	271
#define	BANG	272
#define	WORD	273
#define	NUMBER	274
#define	AND_AND	275
#define	OR_OR	276
#define	GREATER_GREATER	277
#define	LESS_LESS	278
#define	LESS_AND	279
#define	GREATER_AND	280
#define	SEMI_SEMI	281
#define	LESS_LESS_MINUS	282
#define	AND_GREATER	283
#define	LESS_GREATER	284
#define	GREATER_BAR	285
#define	yacc_EOF	286

#line 21 "parse.y"

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include "shell.h"
#include "flags.h"

#if defined (READLINE)
#include <readline/readline.h>
#endif /* READLINE */

#include <readline/history.h>

#if defined (JOB_CONTROL)
#  include "jobs.h"
#endif /* JOB_CONTROL */

#define YYDEBUG 1
extern int eof_encountered;
extern int no_line_editing;
extern int interactive, interactive_shell;

/* **************************************************************** */
/*								    */
/*		    "Forward" declarations			    */
/*								    */
/* **************************************************************** */

/* This is kind of sickening.  In order to let these variables be seen by
   all the functions that need them, I am forced to place their declarations
   far away from the place where they should logically be found. */

static int reserved_word_acceptable ();

/* PROMPT_STRING_POINTER points to one of these, never to an actual string. */
char *ps1_prompt, *ps2_prompt;

/* Handle on the current prompt string.  Indirectly points through
   ps1_ or ps2_prompt. */
char **prompt_string_pointer = (char **)NULL;
char *current_prompt_string;

/* The number of lines read from input while creating the current command. */
int current_command_line_count = 0;

/* Variables to manage the task of reading here documents, because we need to
   defer the reading until after a complete command has been collected. */
REDIRECT *redirection_needing_here_doc = (REDIRECT *)NULL;
int need_here_doc = 0;

#line 72 "parse.y"
typedef union {
  WORD_DESC *word;		/* the word that we read. */
  int number;			/* the number that we read. */
  WORD_LIST *word_list;
  COMMAND *command;
  REDIRECT *redirect;
  ELEMENT element;
  PATTERN_LIST *pattern;
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



#define	YYFINAL		228
#define	YYFLAG		-32768
#define	YYNTBASE	43

#define YYTRANSLATE(x) ((unsigned)(x) <= 286 ? yytranslate[x] : 68)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    33,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    31,     2,    41,
    42,     2,     2,     2,    38,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    32,    37,
     2,    36,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    39,    35,    40,     2,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    34
};

static const short yyrline[] = {     0,
   113,   122,   129,   145,   155,   157,   161,   163,   165,   167,
   169,   171,   173,   179,   185,   189,   193,   197,   201,   206,
   211,   216,   221,   228,   235,   237,   239,   241,   243,   245,
   247,   257,   259,   263,   265,   269,   273,   284,   286,   290,
   292,   296,   298,   301,   305,   307,   309,   311,   314,   316,
   319,   321,   323,   328,   330,   332,   335,   338,   341,   344,
   347,   351,   353,   355,   360,   364,   366,   368,   372,   373,
   377,   379,   381,   383,   387,   389,   393,   395,   397,   399,
   403,   405,   414,   423,   424,   425,   427,   431,   433,   435,
   437,   439,   441,   443,   450,   451,   452,   455,   456,   465,
   472,   479,   488,   490,   492,   494,   496,   498,   505,   508
};

static const char * const yytname[] = {     0,
"error","$illegal.","IF","THEN","ELSE","ELIF","FI","CASE","ESAC","FOR",
"WHILE","UNTIL","DO","DONE","FUNCTION","IN","BANG","WORD","NUMBER","AND_AND",
"OR_OR","GREATER_GREATER","LESS_LESS","LESS_AND","GREATER_AND","SEMI_SEMI","LESS_LESS_MINUS","AND_GREATER","LESS_GREATER","GREATER_BAR",
"'&'","';'","'\\n'","yacc_EOF","'|'","'>'","'<'","'-'","'{'","'}'",
"'('","')'","inputunit"
};

static const short yyr1[] = {     0,
    43,    43,    43,    43,    44,    44,    45,    45,    45,    45,
    45,    45,    45,    45,    45,    45,    45,    45,    45,    45,
    45,    45,    45,    45,    45,    45,    45,    45,    45,    45,
    45,    45,    45,    46,    46,    47,    47,    48,    48,    49,
    49,    50,    50,    50,    51,    51,    51,    51,    51,    51,
    51,    51,    51,    51,    51,    51,    51,    51,    51,    51,
    51,    52,    52,    52,    53,    54,    54,    54,    55,    55,
    56,    56,    56,    56,    57,    57,    58,    58,    58,    58,
    59,    59,    60,    61,    61,    61,    61,    62,    62,    62,
    62,    62,    62,    62,    63,    63,    63,    64,    64,    65,
    65,    65,    66,    66,    66,    66,    66,    66,    67,    67
};

static const short yyr2[] = {     0,
     2,     1,     2,     1,     0,     2,     2,     2,     3,     3,
     2,     3,     2,     3,     2,     3,     2,     3,     2,     3,
     2,     3,     2,     3,     2,     3,     2,     3,     2,     3,
     2,     2,     3,     1,     1,     1,     2,     1,     2,     1,
     1,     1,     2,     2,     6,     6,     7,     7,    10,    10,
     6,     7,     6,     1,     5,     5,     3,     1,     5,     6,
     4,     5,     7,     6,     3,     4,     6,     5,     1,     2,
     4,     4,     5,     5,     1,     2,     5,     5,     6,     6,
     1,     3,     2,     1,     3,     3,     3,     4,     4,     4,
     4,     4,     1,     2,     1,     1,     1,     0,     2,     1,
     2,     2,     4,     4,     3,     3,     1,     2,     4,     1
};

static const short yydefact[] = {     0,
     0,    98,     0,     0,    98,    98,     0,     0,    34,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     2,     4,
     0,     0,    98,    98,    35,    38,     0,    40,   110,    41,
    42,    54,    58,     0,   100,   107,     3,     0,     0,    98,
    98,     0,     0,    98,   108,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    11,    13,    19,    15,    27,
    21,    17,    25,    23,    29,    31,    32,     7,     8,     0,
     0,     0,    37,    44,    34,    35,    39,    36,    43,     1,
    98,    98,   101,   102,    98,    98,     0,    99,    83,    84,
    93,     0,    98,     0,    98,    98,     0,     0,    98,    12,
    14,    20,    16,    28,    22,    18,    26,    24,    30,    33,
     9,    10,    65,    57,     0,     0,   105,   106,     0,     0,
    94,    98,    98,    98,    98,    98,    98,     0,    98,     5,
    98,     0,     0,    98,    61,     0,   103,   104,     0,     0,
   109,    98,    98,    62,     0,     0,     0,    86,    87,    85,
     0,    69,    98,    75,     0,    98,    98,     0,     0,     0,
    55,    56,     0,    59,     0,     0,    64,    88,    89,    90,
    91,    92,    53,    70,    76,     0,    51,    81,     0,     0,
     0,     0,    45,     6,    96,    95,    97,    98,    46,    60,
    63,    98,    98,    98,    98,    52,     0,     0,    98,    47,
    48,     0,    66,     0,     0,     0,    98,    82,    71,    72,
    98,    98,    98,    68,    73,    74,    77,    78,     0,     0,
    67,    79,    80,    49,    50,     0,     0,     0
};

static const short yydefgoto[] = {   226,
   159,    25,    26,    27,    28,    29,    30,    31,    32,    33,
   145,   151,   152,   153,   154,   180,    38,    89,    90,   188,
    39,    34,   117,    91
};

static const short yypact[] = {   202,
   -18,-32768,     9,    23,-32768,-32768,    31,   394,    10,   466,
    38,    44,    35,    56,    54,    74,    77,    87,-32768,-32768,
    92,   100,-32768,-32768,    18,-32768,   426,   450,-32768,-32768,
    84,-32768,-32768,    86,    96,    88,-32768,   118,   266,-32768,
   102,   135,   136,   121,    88,   128,   145,   154,    61,    64,
   158,   159,   170,   172,   173,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   160,
   157,    10,-32768,-32768,-32768,-32768,-32768,-32768,    84,-32768,
-32768,-32768,   330,   330,-32768,-32768,   394,-32768,-32768,   111,
    88,     1,-32768,    19,-32768,-32768,   162,     7,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   298,   298,     3,     3,   362,   140,
    88,-32768,-32768,-32768,-32768,-32768,-32768,    -8,-32768,-32768,
-32768,   182,   184,-32768,-32768,     7,-32768,-32768,   330,   330,
    88,-32768,-32768,-32768,   194,   266,   266,   266,   266,   266,
   200,-32768,-32768,-32768,    27,-32768,-32768,   197,   107,   175,
-32768,-32768,     7,-32768,   209,   214,-32768,-32768,-32768,    17,
    17,    17,-32768,-32768,-32768,    30,-32768,-32768,   204,    58,
   219,   183,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   123,   210,-32768,-32768,
-32768,    37,    72,   266,   266,   266,-32768,-32768,   208,   156,
-32768,-32768,-32768,-32768,   221,   234,-32768,-32768,   226,   215,
-32768,-32768,-32768,-32768,-32768,   248,   250,-32768
};

static const short yypgoto[] = {-32768,
-32768,   -15,   237,   223,-32768,-32768,-32768,   239,-32768,   -94,
    65,-32768,   119,-32768,   126,   101,    -5,-32768,  -139,-32768,
   -38,-32768,    20,    14
};


#define	YYLAST		503


static const short yytable[] = {    42,
    43,    92,    94,   135,   156,    98,   168,   169,   170,   171,
   172,    73,    76,    36,    37,    78,   127,    70,    71,    35,
   -36,    45,    81,    82,    88,   -36,    40,   -36,   -36,   -36,
   157,   129,   -36,    88,   130,   177,   122,   123,   196,    88,
    41,   164,   115,   116,   178,    23,   119,   178,    44,   211,
    46,    88,    58,    59,   128,    56,   -36,   131,   -36,    88,
   136,    57,    88,    73,   170,   171,   172,   179,   190,    88,
   179,    64,    60,    61,    62,   212,   213,   143,   102,   103,
   120,   105,   106,   146,   147,   148,   149,   150,   155,   132,
   133,    65,   198,    63,    66,   163,    36,    36,   104,   199,
   121,   107,    10,   118,    67,    11,    12,    13,    14,    68,
    15,    16,    17,    18,   176,    81,    82,    69,    80,    21,
    22,    86,    85,   158,   184,   160,    83,    84,    36,    36,
   122,   123,   141,    93,   137,   138,   165,   166,   185,   186,
   187,   124,   125,   126,   142,   143,   144,    95,    96,   202,
   181,   182,    36,    36,   204,   205,   206,   198,     2,   118,
   210,    97,   100,     3,   207,     4,     5,     6,   216,    99,
     7,   101,    87,     9,    10,   108,   109,    11,    12,    13,
    14,   218,    15,    16,    17,    18,   203,   110,    88,   111,
   112,    21,    22,   209,    23,   161,    24,   162,   114,   113,
   167,   215,     1,   134,     2,   219,   220,   221,   173,     3,
   183,     4,     5,     6,   189,   191,     7,   192,     8,     9,
    10,   178,   201,    11,    12,    13,    14,   208,    15,    16,
    17,    18,   200,   217,    19,    20,     2,    21,    22,   224,
    23,     3,    24,     4,     5,     6,   222,   227,     7,   228,
    87,     9,    10,    79,   225,    11,    12,    13,    14,   223,
    15,    16,    17,    18,    77,    74,    88,   214,     2,    21,
    22,   174,    23,     3,    24,     4,     5,     6,   175,   197,
     7,     0,    87,     9,    10,     0,     0,    11,    12,    13,
    14,     0,    15,    16,    17,    18,     0,     0,    88,     0,
     2,    21,    22,     0,    23,     3,    24,     4,     5,     6,
     0,     0,     7,     0,     8,     9,    10,     0,     0,    11,
    12,    13,    14,     0,    15,    16,    17,    18,     0,     0,
    88,     0,     2,    21,    22,     0,    23,     3,    24,     4,
     5,     6,     0,     0,     7,     0,     8,     9,    10,     0,
     0,    11,    12,    13,    14,     0,    15,    16,    17,    18,
     0,     0,     0,     0,     2,    21,    22,     0,    23,     3,
    24,     4,     5,     6,     0,     0,     7,     0,     0,     9,
    10,     0,     0,    11,    12,    13,    14,     0,    15,    16,
    17,    18,     0,     0,    88,     0,     2,    21,    22,     0,
    23,     3,    24,     4,     5,     6,     0,     0,     7,     0,
     0,     9,    10,     0,     0,    11,    12,    13,    14,     0,
    15,    16,    17,    18,     0,     0,     0,     0,     2,    21,
    22,     0,    23,     3,    24,     4,     5,     6,     0,     0,
     7,     0,     0,    72,    10,     0,     0,    11,    12,    13,
    14,     0,    15,    16,    17,    18,     0,     0,     0,     0,
     0,    21,    22,     0,    23,     0,    24,    75,    10,     0,
     0,    11,    12,    13,    14,     0,    15,    16,    17,    18,
     0,     0,     0,     0,     0,    21,    22,    47,    48,    49,
    50,     0,    51,     0,    52,    53,     0,     0,     0,     0,
     0,    54,    55
};

static const short yycheck[] = {     5,
     6,    40,    41,    98,    13,    44,   146,   147,   148,   149,
   150,    27,    28,     0,    33,    31,    16,    23,    24,     0,
     3,     8,    20,    21,    33,     8,    18,    10,    11,    12,
    39,    13,    15,    33,    16,     9,    20,    21,     9,    33,
    18,   136,    81,    82,    18,    39,    85,    18,    18,    13,
    41,    33,    18,    19,    93,    18,    39,    39,    41,    33,
    99,    18,    33,    79,   204,   205,   206,    41,   163,    33,
    41,    18,    38,    18,    19,    39,     5,     6,    18,    19,
    86,    18,    19,   122,   123,   124,   125,   126,   127,    95,
    96,    18,    35,    38,    18,   134,    83,    84,    38,    42,
    87,    38,    19,    84,    18,    22,    23,    24,    25,    18,
    27,    28,    29,    30,   153,    20,    21,    18,    33,    36,
    37,     4,    35,   129,    18,   131,    31,    32,   115,   116,
    20,    21,   119,    32,   115,   116,   142,   143,    32,    33,
    34,    31,    32,    33,     5,     6,     7,    13,    13,   188,
   156,   157,   139,   140,   193,   194,   195,    35,     3,   140,
   199,    41,    18,     8,    42,    10,    11,    12,   207,    42,
    15,    18,    17,    18,    19,    18,    18,    22,    23,    24,
    25,    26,    27,    28,    29,    30,   192,    18,    33,    18,
    18,    36,    37,   199,    39,    14,    41,    14,    42,    40,
     7,   207,     1,    42,     3,   211,   212,   213,     9,     8,
    14,    10,    11,    12,    40,     7,    15,     4,    17,    18,
    19,    18,    40,    22,    23,    24,    25,    18,    27,    28,
    29,    30,    14,    26,    33,    34,     3,    36,    37,    14,
    39,     8,    41,    10,    11,    12,    26,     0,    15,     0,
    17,    18,    19,    31,    40,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    28,    27,    33,   203,     3,    36,
    37,   153,    39,     8,    41,    10,    11,    12,   153,   179,
    15,    -1,    17,    18,    19,    -1,    -1,    22,    23,    24,
    25,    -1,    27,    28,    29,    30,    -1,    -1,    33,    -1,
     3,    36,    37,    -1,    39,     8,    41,    10,    11,    12,
    -1,    -1,    15,    -1,    17,    18,    19,    -1,    -1,    22,
    23,    24,    25,    -1,    27,    28,    29,    30,    -1,    -1,
    33,    -1,     3,    36,    37,    -1,    39,     8,    41,    10,
    11,    12,    -1,    -1,    15,    -1,    17,    18,    19,    -1,
    -1,    22,    23,    24,    25,    -1,    27,    28,    29,    30,
    -1,    -1,    -1,    -1,     3,    36,    37,    -1,    39,     8,
    41,    10,    11,    12,    -1,    -1,    15,    -1,    -1,    18,
    19,    -1,    -1,    22,    23,    24,    25,    -1,    27,    28,
    29,    30,    -1,    -1,    33,    -1,     3,    36,    37,    -1,
    39,     8,    41,    10,    11,    12,    -1,    -1,    15,    -1,
    -1,    18,    19,    -1,    -1,    22,    23,    24,    25,    -1,
    27,    28,    29,    30,    -1,    -1,    -1,    -1,     3,    36,
    37,    -1,    39,     8,    41,    10,    11,    12,    -1,    -1,
    15,    -1,    -1,    18,    19,    -1,    -1,    22,    23,    24,
    25,    -1,    27,    28,    29,    30,    -1,    -1,    -1,    -1,
    -1,    36,    37,    -1,    39,    -1,    41,    18,    19,    -1,
    -1,    22,    23,    24,    25,    -1,    27,    28,    29,    30,
    -1,    -1,    -1,    -1,    -1,    36,    37,    22,    23,    24,
    25,    -1,    27,    -1,    29,    30,    -1,    -1,    -1,    -1,
    -1,    36,    37
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#if !defined (alloca) && ((!defined (__STDC__) && defined (sparc)) || defined (__sparc__))
#include <alloca.h>
#endif

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
#define YYFAIL		goto yyerrlab;
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYIMPURE
#define YYLEX		yylex()
#endif

#ifndef YYPURE
#define YYLEX		yylex(&yylval, &yylloc)
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYIMPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/

int yynerrs;			/*  number of parse errors so far       */
#endif  /* YYIMPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYMAXDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYMAXDEPTH
#define YYMAXDEPTH 200
#endif

/*  YYMAXLIMIT is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#ifndef YYMAXLIMIT
#define YYMAXLIMIT 10000
#endif


#line 90 "bison.simple"
int
yyparse()
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  YYLTYPE *yylsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYMAXDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYMAXDEPTH];	/*  the semantic value stack		*/
  YYLTYPE yylsa[YYMAXDEPTH];	/*  the location stack			*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */
  YYLTYPE *yyls = yylsa;

  int yymaxdepth = YYMAXDEPTH;

#ifndef YYPURE
  int yychar;
  YYSTYPE yylval;
  YYLTYPE yylloc;
  int yynerrs;
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
  yylsp = yyls;

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yymaxdepth - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      YYLTYPE *yyls1 = yyls;
      short *yyss1 = yyss;

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yymaxdepth);

      yyss = yyss1; yyvs = yyvs1; yyls = yyls1;
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yymaxdepth >= YYMAXLIMIT)
	yyerror("parser stack overflow");
      yymaxdepth *= 2;
      if (yymaxdepth > YYMAXLIMIT)
	yymaxdepth = YYMAXLIMIT;
      yyss = (short *) alloca (yymaxdepth * sizeof (*yyssp));
      bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yymaxdepth * sizeof (*yyvsp));
      bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yymaxdepth * sizeof (*yylsp));
      bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yymaxdepth);
#endif

      if (yyssp >= yyss + yymaxdepth - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
yyresume:

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
	fprintf(stderr, "Next token is %d (%s)\n", yychar, yytname[yychar1]);
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
      if (yylen == 1)
	fprintf (stderr, "Reducing 1 value via line %d, ",
		 yyrline[yyn]);
      else
	fprintf (stderr, "Reducing %d values via line %d, ",
		 yylen, yyrline[yyn]);
    }
#endif


  switch (yyn) {

case 1:
#line 114 "parse.y"
{
			  /* Case of regular command.  Discard the error
			     safety net,and return the command just parsed. */
			  global_command = yyvsp[-1].command;
			  eof_encountered = 0;
			  discard_parser_constructs (0);
			  YYACCEPT;
			;
    break;}
case 2:
#line 123 "parse.y"
{
			  /* Case of regular command, but not a very
			     interesting one.  Return a NULL command. */
			  global_command = (COMMAND *)NULL;
			  YYACCEPT;
			;
    break;}
case 3:
#line 131 "parse.y"
{
			  /* Error during parsing.  Return NULL command. */
			  global_command = (COMMAND *)NULL;
			  eof_encountered = 0;
			  discard_parser_constructs (1);
			  if (interactive)
			    {
			      YYACCEPT;
			    }
			  else
			    {
			      YYABORT;
			    }
			;
    break;}
case 4:
#line 146 "parse.y"
{
			  /* Case of EOF seen by itself.  Do ignoreeof or 
			     not. */
			  global_command = (COMMAND *)NULL;
			  handle_eof_input_unit ();
			  YYACCEPT;
			;
    break;}
case 5:
#line 156 "parse.y"
{ yyval.word_list = (WORD_LIST *)NULL; ;
    break;}
case 6:
#line 158 "parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, yyvsp[-1].word_list); ;
    break;}
case 7:
#line 162 "parse.y"
{ yyval.redirect = make_redirection ( 1, r_output_direction, yyvsp[0].word); ;
    break;}
case 8:
#line 164 "parse.y"
{ yyval.redirect = make_redirection ( 0, r_input_direction, yyvsp[0].word); ;
    break;}
case 9:
#line 166 "parse.y"
{ yyval.redirect = make_redirection (yyvsp[-2].number, r_output_direction, yyvsp[0].word); ;
    break;}
case 10:
#line 168 "parse.y"
{ yyval.redirect = make_redirection (yyvsp[-2].number, r_input_direction, yyvsp[0].word); ;
    break;}
case 11:
#line 170 "parse.y"
{ yyval.redirect = make_redirection ( 1, r_appending_to, yyvsp[0].word); ;
    break;}
case 12:
#line 172 "parse.y"
{ yyval.redirect = make_redirection (yyvsp[-2].number, r_appending_to, yyvsp[0].word); ;
    break;}
case 13:
#line 174 "parse.y"
{
			  yyval.redirect = make_redirection ( 0, r_reading_until, yyvsp[0].word);
			  redirection_needing_here_doc = yyval.redirect;
			  need_here_doc = 1;
			;
    break;}
case 14:
#line 180 "parse.y"
{
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_reading_until, yyvsp[0].word);
			  redirection_needing_here_doc = yyval.redirect;
			  need_here_doc = 1;
			;
    break;}
case 15:
#line 186 "parse.y"
{
			  yyval.redirect = make_redirection ( 0, r_duplicating_input, yyvsp[0].number);
			;
    break;}
case 16:
#line 190 "parse.y"
{
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_input, yyvsp[0].number);
			;
    break;}
case 17:
#line 194 "parse.y"
{
			  yyval.redirect = make_redirection ( 1, r_duplicating_output, yyvsp[0].number);
			;
    break;}
case 18:
#line 198 "parse.y"
{
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_output, yyvsp[0].number);
			;
    break;}
case 19:
#line 202 "parse.y"
{
			  yyval.redirect = make_redirection
			    (0, r_duplicating_input_word, yyvsp[0].word);
			;
    break;}
case 20:
#line 207 "parse.y"
{
			  yyval.redirect = make_redirection
			    (yyvsp[-2].number, r_duplicating_input_word, yyvsp[0].word);
			;
    break;}
case 21:
#line 212 "parse.y"
{
			  yyval.redirect = make_redirection
			    (1, r_duplicating_output_word, yyvsp[0].word);
			;
    break;}
case 22:
#line 217 "parse.y"
{
			  yyval.redirect = make_redirection
			    (yyvsp[-2].number, r_duplicating_output_word, yyvsp[0].word);
			;
    break;}
case 23:
#line 222 "parse.y"
{
			  yyval.redirect = make_redirection
			    (0, r_deblank_reading_until, yyvsp[0].word);
			  redirection_needing_here_doc = yyval.redirect;
			  need_here_doc = 1;
			;
    break;}
case 24:
#line 229 "parse.y"
{
			  yyval.redirect = make_redirection
			    (yyvsp[-2].number, r_deblank_reading_until, yyvsp[0].word);
			  redirection_needing_here_doc = yyval.redirect;
			  need_here_doc = 1;
			;
    break;}
case 25:
#line 236 "parse.y"
{ yyval.redirect = make_redirection ( 1, r_close_this, 0); ;
    break;}
case 26:
#line 238 "parse.y"
{ yyval.redirect = make_redirection (yyvsp[-2].number, r_close_this, 0); ;
    break;}
case 27:
#line 240 "parse.y"
{ yyval.redirect = make_redirection ( 0, r_close_this, 0); ;
    break;}
case 28:
#line 242 "parse.y"
{ yyval.redirect = make_redirection (yyvsp[-2].number, r_close_this, 0); ;
    break;}
case 29:
#line 244 "parse.y"
{ yyval.redirect = make_redirection ( 1, r_err_and_out, yyvsp[0].word); ;
    break;}
case 30:
#line 246 "parse.y"
{ yyval.redirect = make_redirection ( yyvsp[-2].number, r_input_output, yyvsp[0].word); ;
    break;}
case 31:
#line 248 "parse.y"
{
			  REDIRECT *t1, *t2;
			  extern WORD_DESC *copy_word ();

			  t1 = make_redirection ( 0, r_input_direction, yyvsp[0].word);
			  t2 = make_redirection ( 1, r_output_direction, copy_word (yyvsp[0].word));
			  t1->next = t2;
			  yyval.redirect = t1;
			;
    break;}
case 32:
#line 258 "parse.y"
{ yyval.redirect = make_redirection ( 1, r_output_force, yyvsp[0].word); ;
    break;}
case 33:
#line 260 "parse.y"
{ yyval.redirect = make_redirection ( yyvsp[-2].number, r_output_force, yyvsp[0].word); ;
    break;}
case 34:
#line 264 "parse.y"
{ yyval.element.word = yyvsp[0].word; yyval.element.redirect = 0; ;
    break;}
case 35:
#line 266 "parse.y"
{ yyval.element.redirect = yyvsp[0].redirect; yyval.element.word = 0; ;
    break;}
case 36:
#line 270 "parse.y"
{
			  yyval.redirect = yyvsp[0].redirect;
			;
    break;}
case 37:
#line 274 "parse.y"
{ 
			  register REDIRECT *t = yyvsp[-1].redirect;

			  while (t->next)
			    t = t->next;
			  t->next = yyvsp[0].redirect; 
			  yyval.redirect = yyvsp[-1].redirect;
			;
    break;}
case 38:
#line 285 "parse.y"
{ yyval.command = make_simple_command (yyvsp[0].element, (COMMAND *)NULL); ;
    break;}
case 39:
#line 287 "parse.y"
{ yyval.command = make_simple_command (yyvsp[0].element, yyvsp[-1].command); ;
    break;}
case 40:
#line 291 "parse.y"
{ yyval.command = clean_simple_command (yyvsp[0].command); ;
    break;}
case 41:
#line 293 "parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 42:
#line 297 "parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 43:
#line 299 "parse.y"
{ yyvsp[-1].command->redirects = yyvsp[0].redirect; yyval.command = yyvsp[-1].command; ;
    break;}
case 44:
#line 302 "parse.y"
{ yyvsp[0].command->redirects = yyvsp[-1].redirect; yyval.command = yyvsp[0].command; ;
    break;}
case 45:
#line 306 "parse.y"
{ yyval.command = make_for_command (yyvsp[-4].word, (WORD_LIST *)add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); ;
    break;}
case 46:
#line 308 "parse.y"
{ yyval.command = make_for_command (yyvsp[-4].word, (WORD_LIST *)add_string_to_list ("$@", (WORD_LIST *)NULL), yyvsp[-1].command); ;
    break;}
case 47:
#line 310 "parse.y"
{ yyval.command = make_for_command (yyvsp[-5].word, (WORD_LIST *)add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); ;
    break;}
case 48:
#line 312 "parse.y"
{ yyval.command = make_for_command (yyvsp[-5].word, (WORD_LIST *)add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); ;
    break;}
case 49:
#line 315 "parse.y"
{ yyval.command = make_for_command (yyvsp[-8].word, (WORD_LIST *)reverse_list (yyvsp[-5].word_list), yyvsp[-1].command); ;
    break;}
case 50:
#line 317 "parse.y"
{ yyval.command = make_for_command (yyvsp[-8].word, (WORD_LIST *)reverse_list (yyvsp[-5].word_list), yyvsp[-1].command); ;
    break;}
case 51:
#line 320 "parse.y"
{ yyval.command = make_case_command (yyvsp[-4].word, (PATTERN_LIST *)NULL); ;
    break;}
case 52:
#line 322 "parse.y"
{ yyval.command = make_case_command (yyvsp[-5].word, yyvsp[-2].pattern); ;
    break;}
case 53:
#line 324 "parse.y"
{ /* Nobody likes this...
			     report_syntax_error ("Inserted `;;'"); */
			  yyval.command = make_case_command (yyvsp[-4].word, yyvsp[-1].pattern); ;
    break;}
case 54:
#line 329 "parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 55:
#line 331 "parse.y"
{ yyval.command = make_while_command (yyvsp[-3].command, yyvsp[-1].command); ;
    break;}
case 56:
#line 333 "parse.y"
{ yyval.command = make_until_command (yyvsp[-3].command, yyvsp[-1].command); ;
    break;}
case 57:
#line 336 "parse.y"
{ yyvsp[-1].command->flags |= CMD_WANT_SUBSHELL; yyval.command = yyvsp[-1].command; ;
    break;}
case 58:
#line 339 "parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 59:
#line 342 "parse.y"
{ yyval.command = make_function_def (yyvsp[-4].word, yyvsp[0].command); ;
    break;}
case 60:
#line 345 "parse.y"
{ yyval.command = make_function_def (yyvsp[-4].word, yyvsp[0].command); ;
    break;}
case 61:
#line 348 "parse.y"
{ yyval.command = make_function_def (yyvsp[-2].word, yyvsp[0].command); ;
    break;}
case 62:
#line 352 "parse.y"
{ yyval.command = make_if_command (yyvsp[-3].command, yyvsp[-1].command, (COMMAND *)NULL); ;
    break;}
case 63:
#line 354 "parse.y"
{ yyval.command = make_if_command (yyvsp[-5].command, yyvsp[-3].command, yyvsp[-1].command); ;
    break;}
case 64:
#line 356 "parse.y"
{ yyval.command = make_if_command (yyvsp[-4].command, yyvsp[-2].command, yyvsp[-1].command); ;
    break;}
case 65:
#line 361 "parse.y"
{ yyval.command = make_group_command (yyvsp[-1].command); ;
    break;}
case 66:
#line 365 "parse.y"
{ yyval.command = make_if_command (yyvsp[-2].command, yyvsp[0].command, (COMMAND *)NULL); ;
    break;}
case 67:
#line 367 "parse.y"
{ yyval.command = make_if_command (yyvsp[-4].command, yyvsp[-2].command, yyvsp[0].command); ;
    break;}
case 68:
#line 369 "parse.y"
{ yyval.command = make_if_command (yyvsp[-3].command, yyvsp[-1].command, yyvsp[0].command); ;
    break;}
case 70:
#line 374 "parse.y"
{ yyvsp[0].pattern->next = yyvsp[-1].pattern; yyval.pattern = yyvsp[0].pattern; ;
    break;}
case 71:
#line 378 "parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, yyvsp[0].command); ;
    break;}
case 72:
#line 380 "parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, (COMMAND *)NULL); ;
    break;}
case 73:
#line 382 "parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, yyvsp[0].command); ;
    break;}
case 74:
#line 384 "parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, (COMMAND *)NULL); ;
    break;}
case 76:
#line 390 "parse.y"
{ yyvsp[0].pattern->next = yyvsp[-1].pattern; yyval.pattern = yyvsp[0].pattern; ;
    break;}
case 77:
#line 394 "parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-3].word_list, yyvsp[-1].command); ;
    break;}
case 78:
#line 396 "parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-3].word_list, (COMMAND *)NULL); ;
    break;}
case 79:
#line 398 "parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-3].word_list, yyvsp[-1].command); ;
    break;}
case 80:
#line 400 "parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-3].word_list, (COMMAND *)NULL); ;
    break;}
case 81:
#line 404 "parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, (WORD_LIST *)NULL); ;
    break;}
case 82:
#line 406 "parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, yyvsp[-2].word_list); ;
    break;}
case 83:
#line 415 "parse.y"
{
			  yyval.command = yyvsp[0].command;
			  if (need_here_doc)
			    make_here_document (redirection_needing_here_doc);
			  need_here_doc = 0;
			 ;
    break;}
case 86:
#line 426 "parse.y"
{ yyval.command = command_connect (yyvsp[-2].command, 0, '&'); ;
    break;}
case 88:
#line 432 "parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, AND_AND); ;
    break;}
case 89:
#line 434 "parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, OR_OR); ;
    break;}
case 90:
#line 436 "parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, '&'); ;
    break;}
case 91:
#line 438 "parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, ';'); ;
    break;}
case 92:
#line 440 "parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, ';'); ;
    break;}
case 93:
#line 442 "parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 94:
#line 444 "parse.y"
{
			  yyvsp[0].command->flags |= CMD_INVERT_RETURN;
			  yyval.command = yyvsp[0].command;
			;
    break;}
case 100:
#line 466 "parse.y"
{
			  yyval.command = yyvsp[0].command;
			  if (need_here_doc)
			    make_here_document (redirection_needing_here_doc);
			  need_here_doc = 0;
			;
    break;}
case 101:
#line 473 "parse.y"
{
			  yyval.command = command_connect (yyvsp[-1].command, (COMMAND *)NULL, '&');
			  if (need_here_doc)
			    make_here_document (redirection_needing_here_doc);
			  need_here_doc = 0;
			;
    break;}
case 102:
#line 480 "parse.y"
{
			  yyval.command = yyvsp[-1].command;
			  if (need_here_doc)
			    make_here_document (redirection_needing_here_doc);
			  need_here_doc = 0;
			;
    break;}
case 103:
#line 489 "parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, AND_AND); ;
    break;}
case 104:
#line 491 "parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, OR_OR); ;
    break;}
case 105:
#line 493 "parse.y"
{ yyval.command = command_connect (yyvsp[-2].command, yyvsp[0].command, '&'); ;
    break;}
case 106:
#line 495 "parse.y"
{ yyval.command = command_connect (yyvsp[-2].command, yyvsp[0].command, ';'); ;
    break;}
case 107:
#line 497 "parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 108:
#line 499 "parse.y"
{
			  yyvsp[0].command->flags |= CMD_INVERT_RETURN;
			  yyval.command = yyvsp[0].command;
			;
    break;}
case 109:
#line 507 "parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, '|'); ;
    break;}
case 110:
#line 509 "parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 327 "bison.simple"

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
      yyerror("parse error");
    }

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
#line 511 "parse.y"


/* Initial size to allocate for tokens, and the
   amount to grow them by. */
#define TOKEN_DEFAULT_GROW_SIZE 512

/* The token currently being read. */
int current_token = 0;

/* The last read token, or NULL.  read_token () uses this for context
   checking. */
int last_read_token = 0;

/* The token read prior to last_read_token. */
int token_before_that = 0;

/* Global var is non-zero when end of file has been reached. */
int EOF_Reached = 0;

/* yy_getc () returns the next available character from input or EOF.
   yy_ungetc (c) makes `c' the next character to read.
   init_yy_io (get, unget), makes the function `get' the installed function
   for getting the next character, and makes `unget' the installed function
   for un-getting a character. */
return_EOF ()			/* does nothing good. */
{
  return (EOF);
}

/* Variables containing the current get and unget functions. */

/* Some stream `types'. */
#define st_stream 0
#define st_string 1

Function *get_yy_char = return_EOF;
Function *unget_yy_char = return_EOF;
int yy_input_type = st_stream;
FILE *yy_input_dev = (FILE *)NULL;

/* The current stream name.  In the case of a file, this is a filename. */
char *stream_name = (char *)NULL;

/* Function to set get_yy_char and unget_yy_char. */
init_yy_io (get_function, unget_function, type, location)
     Function *get_function, *unget_function;
     int type;
     FILE *location;
{
  get_yy_char = get_function;
  unget_yy_char = unget_function;
  yy_input_type = type;
  yy_input_dev = location;
}

/* Call this to get the next character of input. */
yy_getc ()
{
  return (*get_yy_char) ();
}

/* Call this to unget C.  That is, to make C the next character
   to be read. */
yy_ungetc (c)
{
  return (*unget_yy_char) (c);
}

/* **************************************************************** */
/*								    */
/*		  Let input be read from readline ().		    */
/*								    */
/* **************************************************************** */

#if defined (READLINE)
char *current_readline_prompt = (char *)NULL;
char *current_readline_line = (char *)NULL;
int current_readline_line_index = 0;

static int readline_initialized_yet = 0;
int
yy_readline_get ()
{
  if (!current_readline_line)
    {
      extern sighandler sigint_sighandler ();
      extern int interrupt_immediately;
      extern char *readline ();
      SigHandler *old_sigint;
#if defined (JOB_CONTROL)
      extern pid_t shell_pgrp;
      extern int job_control;
#endif /* JOB_CONTROL */

      if (!readline_initialized_yet)
	{
	  initialize_readline ();
	  readline_initialized_yet = 1;
	}

#if defined (JOB_CONTROL)
      if (job_control)
	give_terminal_to (shell_pgrp);
#endif /* JOB_CONTROL */

      old_sigint = (SigHandler *)signal (SIGINT, sigint_sighandler);
      interrupt_immediately++;

      if (!current_readline_prompt)
	current_readline_line = readline ("");
      else
	current_readline_line = readline (current_readline_prompt);

      interrupt_immediately--;
      signal (SIGINT, old_sigint);

      /* Reset the prompt to whatever is in the decoded value of
	 prompt_string_pointer. */
      reset_readline_prompt ();

      current_readline_line_index = 0;

      if (!current_readline_line)
	{
	  current_readline_line_index = 0;
	  return (EOF);
	}

      current_readline_line =
	(char *)xrealloc (current_readline_line,
			  2 + strlen (current_readline_line));
      strcat (current_readline_line, "\n");
    }

  if (!current_readline_line[current_readline_line_index])
    {
      free (current_readline_line);
      current_readline_line = (char *)NULL;
      return (yy_readline_get ());
    }
  else
    {
      int c = current_readline_line[current_readline_line_index++];
      return (c);
    }
}

int
yy_readline_unget (c)
{
  if (current_readline_line_index && current_readline_line)
    current_readline_line[--current_readline_line_index] = c;
  return (c);
}
  
with_input_from_stdin ()
{
  init_yy_io (yy_readline_get, yy_readline_unget,
	      st_string, (FILE *)current_readline_line);
  stream_name = savestring ("readline stdin");
}

#else  /* !READLINE */

with_input_from_stdin ()
{
  with_input_from_stream (stdin, "stdin");
}
#endif	/* !READLINE */

/* **************************************************************** */
/*								    */
/*   Let input come from STRING.  STRING is zero terminated.	    */
/*								    */
/* **************************************************************** */

int
yy_string_get ()
{
  /* If the string doesn't exist, or is empty, EOF found. */
  if (!(char *)yy_input_dev || !*(char *)yy_input_dev)
    return (EOF);
  else
    {
      register char *temp = (char *)yy_input_dev;
      int c = *temp++;
      yy_input_dev = (FILE *)temp;
      return (c);
    }
}

int
yy_string_unget (c)
     int c;
{
  register char *temp = (char *)yy_input_dev;
  *(--temp) = c;
  yy_input_dev = (FILE *)temp;
  return (c);
}

with_input_from_string (string, name)
     char *string;
     char *name;
{
  init_yy_io (yy_string_get, yy_string_unget, st_string, (FILE *)string);
  stream_name = savestring (name);
}

/* **************************************************************** */
/*								    */
/*		     Let input come from STREAM.		    */
/*								    */
/* **************************************************************** */

int
yy_stream_get ()
{
  if (yy_input_dev)
#if defined (USG) || (defined (_POSIX_VERSION) && defined (Ultrix))
    return (sysv_getc (yy_input_dev));
#else
    return (getc (yy_input_dev));
#endif	/* USG || (posix & ultrix) */
  else return (EOF);
}

int
yy_stream_unget (c)
     int c;
{
  return (ungetc (c, yy_input_dev));
}

with_input_from_stream (stream, name)
     FILE *stream;
     char *name;
{
  init_yy_io (yy_stream_get, yy_stream_unget, st_stream, stream);
  stream_name = savestring (name);
}

typedef struct stream_saver {
  struct stream_saver *next;
  Function *getter, *putter;
  int type, line;
  char *location, *name;
} STREAM_SAVER;

/* The globally known line number. */
int line_number = 0;

STREAM_SAVER *stream_list = (STREAM_SAVER *)NULL;

push_stream ()
{
  STREAM_SAVER *temp = (STREAM_SAVER *)xmalloc (sizeof (STREAM_SAVER));
  temp->type = yy_input_type;
  temp->location = (char *)yy_input_dev;
  temp->getter = get_yy_char;
  temp->putter = unget_yy_char;
  temp->line = line_number;
  temp->name = stream_name; stream_name = (char *)NULL;
  temp->next = stream_list;
  stream_list = temp;
  EOF_Reached = line_number = 0;
}

pop_stream ()
{
  if (!stream_list)
    {
      EOF_Reached = 1;
    }
  else
    {
      STREAM_SAVER *temp = stream_list;
    
      EOF_Reached = 0;
      stream_list = stream_list->next;

      if (stream_name)
	free (stream_name);
      stream_name = temp->name;

      init_yy_io (temp->getter, temp->putter, temp->type, (FILE *)temp->location);
      line_number = temp->line;
      free (temp);
    }
}

/*
 * This is used to inhibit alias expansion and reserved word recognition
 * inside case statement pattern lists.  A `case statement pattern list'
 * is:
 *	everything between the `in' in a `case word in' and the next ')'
 *	or `esac'
 *	everything between a `;;' and the next `)' or `esac'
 */
static int in_case_pattern_list = 0;

#if defined (ALIAS)
/*
 * Pseudo-global variables used in implementing token-wise alias expansion.
 */

static int expand_next_token = 0;
static char *current_token_being_expanded = (char *)NULL;
static char *pending_token_being_expanded = (char *)NULL;

/*
 * Pushing and popping strings.  This works together with shell_getc to 
 * implement alias expansion on a per-token basis.
 */

typedef struct string_saver {
  struct string_saver *next;
  int expand_alias;  /* Value to set expand_alias to when string is popped. */
  char *saved_line;
  int saved_line_size, saved_line_index, saved_line_terminator;
  char *saved_token_being_expanded;
} STRING_SAVER;

STRING_SAVER *pushed_string_list = (STRING_SAVER *)NULL;

static void save_expansion ();

/*
 * Push the current shell_input_line onto a stack of such lines and make S
 * the current input.  Used when expanding aliases.  EXPAND is used to set
 * the value of expand_next_token when the string is popped, so that the
 * word after the alias in the original line is handled correctly when the
 * alias expands to multiple words.  TOKEN is the token that was expanded
 * into S; it is saved and used to prevent infinite recursive expansion.
 */
static void
push_string (s, expand, token)
     char *s;
     int expand;
     char *token;
{
  extern char *shell_input_line;
  extern int shell_input_line_size, shell_input_line_index,
	     shell_input_line_terminator;
  STRING_SAVER *temp = (STRING_SAVER *) xmalloc (sizeof (STRING_SAVER));

  temp->expand_alias = expand;
  temp->saved_line = shell_input_line;
  temp->saved_line_size = shell_input_line_size;
  temp->saved_line_index = shell_input_line_index;
  temp->saved_line_terminator = shell_input_line_terminator;
  temp->saved_token_being_expanded = current_token_being_expanded;
  temp->next = pushed_string_list;
  pushed_string_list = temp;

  save_expansion (token);

  current_token_being_expanded = token;
  shell_input_line = s;
  shell_input_line_size = strlen (s);
  shell_input_line_index = 0;
  shell_input_line_terminator = '\0';
  expand_next_token = 0;
}

/*
 * Make the top of the pushed_string stack be the current shell input.
 * Only called when there is something on the stack.  Called from shell_getc
 * when it thinks it has consumed the string generated by an alias expansion
 * and needs to return to the original input line.
 */
static void
pop_string ()
{
  extern char *shell_input_line;
  extern int shell_input_line_size, shell_input_line_index,
	     shell_input_line_terminator;
  STRING_SAVER *t;

  if (shell_input_line)
    free (shell_input_line);
  shell_input_line = pushed_string_list->saved_line;
  shell_input_line_index = pushed_string_list->saved_line_index;
  shell_input_line_size = pushed_string_list->saved_line_size;
  shell_input_line_terminator = pushed_string_list->saved_line_terminator;
  expand_next_token = pushed_string_list->expand_alias;
  pending_token_being_expanded = pushed_string_list->saved_token_being_expanded;
  t = pushed_string_list;
  pushed_string_list = pushed_string_list->next;
  free((char *)t);
}

static void
free_string_list ()
{
  register STRING_SAVER *t = pushed_string_list, *t1;

  while (t)
    {
      t1 = t->next;
      if (t->saved_line)
	free (t->saved_line);
      if (t->saved_token_being_expanded)
	free (t->saved_token_being_expanded);
      free ((char *)t);
      t = t1;
    }
  pushed_string_list = (STRING_SAVER *)NULL;
}

/* This is a stack to save the values of all tokens for which alias
   expansion has been performed during the current call to read_token ().
   It is used to prevent alias expansion loops:

      alias foo=bar
      alias bar=baz
      alias baz=foo

   Ideally this would be taken care of by push and pop string, but because
   of when strings are popped the stack will not contain the correct
   strings to test against.  (The popping is done in shell_getc, so that when
   the current string is exhausted, shell_getc can simply pop that string off
   the stack, restore the previous string, and continue with the character
   following the token whose expansion was originally pushed on the stack.)

   What we really want is a record of all tokens that have been expanded for
   aliases during the `current' call to read_token().  This does that, at the
   cost of being somewhat special-purpose (OK, OK vile and unclean).  Brian,
   you had better rewrite this whole piece of garbage before the next version
   is released.
*/

typedef struct _exp_saver {
      struct _exp_saver *next;
      char *saved_token;
} EXPANSION_SAVER;

EXPANSION_SAVER *expanded_token_stack = (EXPANSION_SAVER *)NULL;

static void
save_expansion (s)
     char *s;
{
  EXPANSION_SAVER *t;

  t = (EXPANSION_SAVER *) xmalloc (sizeof (EXPANSION_SAVER));
  t->saved_token = savestring (s);
  t->next = expanded_token_stack;
  expanded_token_stack = t;
}

/*
 * Return 1 if TOKEN has already been expanded in the current `stack' of
 * expansions.  If it has been expanded already, it will appear as the value
 * of saved_token for some entry in the stack of expansions created for the
 * current token being expanded.
 */
static int
token_has_been_expanded (token)
     char *token;
{
  register EXPANSION_SAVER *t = expanded_token_stack;

  while (t)
    {
      if (STREQ (token, t->saved_token))
	return (1);
      t = t->next;
    }
  return (0);
}

static void
free_expansion_stack ()
{
  register EXPANSION_SAVER *t = expanded_token_stack, *t1;

  while (t)
    {
      t1 = t->next;
      free (t->saved_token);
      free (t);
      t = t1;
    }
  expanded_token_stack = (EXPANSION_SAVER *)NULL;
}

#endif /* ALIAS */

/* Return a line of text, taken from wherever yylex () reads input.
   If there is no more input, then we return NULL. */
char *
read_a_line ()
{
  char *line_buffer = (char *)NULL;
  int indx = 0, buffer_size = 0;
  int c;

  while (1)
    {
      c = yy_getc ();

      if (c == 0)
	continue;

      /* If there is no more input, then we return NULL. */
      if (c == EOF)
	{
	  c = '\n';
	  if (!line_buffer)
	    return ((char *)NULL);
	}

      /* `+2' in case the final (200'th) character in the buffer is a newline;
	 otherwise the code below that NULL-terminates it will write over the
	 201st slot and kill the range checking in free(). */
      if (indx + 2 > buffer_size)
	if (!buffer_size)
	  line_buffer = (char *)xmalloc (buffer_size = 200);
	else
	  line_buffer = (char *)xrealloc (line_buffer, buffer_size += 200);

      line_buffer[indx++] = c;
      if (c == '\n')
	{
	  line_buffer[indx] = '\0';
	  return (line_buffer);
	}
    }
}

/* Return a line as in read_a_line (), but insure that the prompt is
   the secondary prompt. */
char *
read_secondary_line ()
{
  prompt_string_pointer = &ps2_prompt;
  prompt_again ();
  return (read_a_line ());
}


/* **************************************************************** */
/*								    */
/*				YYLEX ()			    */
/*								    */
/* **************************************************************** */

/* Reserved words.  These are only recognized as the first word of a
   command.  TOKEN_WORD_ALIST. */
STRING_INT_ALIST word_token_alist[] = {
  {"if", IF},
  {"then", THEN},
  {"else", ELSE},
  {"elif", ELIF},
  {"fi", FI},
  {"case", CASE},
  {"esac", ESAC},
  {"for", FOR},
  {"while", WHILE},
  {"until", UNTIL},
  {"do", DO},
  {"done", DONE},
  {"in", IN},
  {"function", FUNCTION},
  {"{", '{'},
  {"}", '}'},
  {"!", BANG},
  {(char *)NULL, 0}
};

/* Where shell input comes from.  History expansion is performed on each
   line when the shell is interactive. */
char *shell_input_line = (char *)NULL;
int shell_input_line_index = 0;
int shell_input_line_size = 0;	/* Amount allocated for shell_input_line. */
int shell_input_line_len = 0;	/* strlen (shell_input_line) */

/* Either zero, or EOF. */
int shell_input_line_terminator = 0;

/* Return the next shell input character.  This always reads characters
   from shell_input_line; when that line is exhausted, it is time to
   read the next line. */
int
shell_getc (remove_quoted_newline)
     int remove_quoted_newline;
{
  int c;

  QUIT;

#if defined (ALIAS)
  /* If shell_input_line[shell_input_line_index] == 0, but there is
     something on the pushed list of strings, then we don't want to go
     off and get another line.  We let the code down below handle it. */

  if (!shell_input_line || ((!shell_input_line[shell_input_line_index]) &&
			    (pushed_string_list == (STRING_SAVER *)NULL)))
#else /* !ALIAS */
  if (!shell_input_line || !shell_input_line[shell_input_line_index])
#endif /* !ALIAS */
    {
      register int i, l;
      char *pre_process_line (), *expansions;

      restart_read_next_line:

      line_number++;

    restart_read:

      QUIT;	/* XXX experimental */

      i = 0;
      shell_input_line_terminator = 0;

#if defined (JOB_CONTROL)
      notify_and_cleanup ();
#endif

      clearerr (stdin);
      while (c = yy_getc ())
	{
	  if (i + 2 > shell_input_line_size)
	    shell_input_line = (char *)
	      xrealloc (shell_input_line, shell_input_line_size += 256);

	  if (c == EOF)
	    {
	      clearerr (stdin);

	      if (!i)
		shell_input_line_terminator = EOF;

	      shell_input_line[i] = '\0';
	      break;
	    }

	  shell_input_line[i++] = c;

	  if (c == '\n')
	    {
	      shell_input_line[--i] = '\0';
	      current_command_line_count++;
	      break;
	    }
	}
      shell_input_line_index = 0;
      shell_input_line_len = i;		/* == strlen (shell_input_line) */

      if (!shell_input_line || !shell_input_line[0])
	goto after_pre_process;

      if (interactive)
	{
	  expansions = pre_process_line (shell_input_line, 1, 1);

	  free (shell_input_line);
	  shell_input_line = expansions;
	  shell_input_line_len = shell_input_line ?
				 strlen (shell_input_line) :
				 0;
	  /* We have to force the xrealloc below because we don't know the
	     true allocated size of shell_input_line anymore. */
	  shell_input_line_size = shell_input_line_len;
	}

  after_pre_process:
      if (shell_input_line)
	{
	  if (echo_input_at_read)
	    fprintf (stderr, "%s\n", shell_input_line);
	}
      else
	{
	  shell_input_line_size = 0;
	  prompt_string_pointer = &current_prompt_string;
	  prompt_again ();
	  goto restart_read;
	}

      /* Add the newline to the end of this string, iff the string does
	 not already end in an EOF character.  */
      if (shell_input_line_terminator != EOF)
	{
	  l = shell_input_line_len;	/* was a call to strlen */

	  if (l + 3 > shell_input_line_size)
	    shell_input_line = (char *)xrealloc (shell_input_line,
					1 + (shell_input_line_size += 2));

	  strcpy (shell_input_line + l, "\n");
	}
    }
  
  c = shell_input_line[shell_input_line_index];

  if (c)
    shell_input_line_index++;

  if (c == '\\' && remove_quoted_newline &&
      shell_input_line[shell_input_line_index] == '\n')
    {
	prompt_again ();
	goto restart_read_next_line;
    }

#if defined (ALIAS)
  /*
   * If c is NULL, we have reached the end of the current input string.  If
   * pushed_string_list is non-empty, it's time to pop to the previous string
   * because we have fully consumed the result of the last alias expansion.
   * Do it transparently; just return the next character of the string popped
   * to.  We need to hang onto current_token_being_expanded until the token
   * currently being read has been recognized; we can't restore it in
   * pop_string () because the token currently being read would be
   * inappropriately compared with it.  We defer restoration until the next
   * call to read_token ().
   */

  if (!c && (pushed_string_list != (STRING_SAVER *)NULL))
    {
      pop_string ();
      c = shell_input_line[shell_input_line_index];
      if (c)
	shell_input_line_index++;
    }
#endif /* ALIAS */

  if (!c && shell_input_line_terminator == EOF)
    {
      if (shell_input_line_index != 0)
	return ('\n');
      else
	return (EOF);
    }

  return (c);
}

/* Put C back into the input for the shell. */
shell_ungetc (c)
     int c;
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line[--shell_input_line_index] = c;
}

/* Discard input until CHARACTER is seen. */
discard_until (character)
     int character;
{
  int c;

  while ((c = shell_getc (0)) != EOF && c != character)
    ;

  if (c != EOF)
    shell_ungetc (c);
}

#if defined (HISTORY_REEDITING)
/* Tell readline () that we have some text for it to edit. */
re_edit (text)
     char *text;
{
#if defined (READLINE)
  if (strcmp (stream_name, "readline stdin") == 0)
    bash_re_edit (text);
#endif /* READLINE */
}
#endif /* HISTORY_REEDITING */

/* Non-zero means do no history expansion on this line, regardless
   of what history_expansion says. */
int history_expansion_inhibited = 0;

/* Do pre-processing on LINE.  If PRINT_CHANGES is non-zero, then
   print the results of expanding the line if there were any changes.
   If there is an error, return NULL, otherwise the expanded line is
   returned.  If ADDIT is non-zero the line is added to the history
   list after history expansion.  ADDIT is just a suggestion;
   REMEMBER_ON_HISTORY can veto, and does.
   Right now this does history expansion. */
char *
pre_process_line (line, print_changes, addit)
     char *line;
     int print_changes, addit;
{
  extern int remember_on_history;
  extern int history_expansion;
  extern int history_expand ();
  char *history_value;
  char *return_value;
  int expanded = 0;

  return_value = line;

  /* History expand the line.  If this results in no errors, then
     add that line to the history if ADDIT is non-zero. */
  if (!history_expansion_inhibited && history_expansion)
    {
      expanded = history_expand (line, &history_value);

      if (expanded)
	{
	  if (print_changes)
	    fprintf (stderr, "%s\n", history_value);

	  /* If there was an error, return NULL. */
	  if (expanded < 0)
	    {
	      free (history_value);

#if defined (HISTORY_REEDITING)
	      /* New hack.  We can allow the user to edit the
		 failed history expansion. */
	      re_edit (line);
#endif /* HISTORY_REEDITING */
	      return ((char *)NULL);
	    }
	}

      /* Let other expansions know that return_value can be free'ed,
	 and that a line has been added to the history list.  Note
	 that we only add lines that have something in them. */
      expanded = 1;
      return_value = history_value;
    }

  if (addit && remember_on_history && *return_value)
    {
      extern int history_control;

      switch (history_control)
	{
	  case 0:
	    bash_add_history (return_value);
	    break;
	  case 1:
	    if (*return_value != ' ')
	      bash_add_history (return_value);
	    break;
	  case 2:
	    {
	      HIST_ENTRY *temp;

	      using_history ();
	      temp = previous_history ();

	      if (!temp || (strcmp (temp->line, return_value) != 0))
		bash_add_history (return_value);

	      using_history ();
	    }
	    break;
	}
    }

  if (!expanded)
    return_value = savestring (line);

  return (return_value);
}


/* Place to remember the token.  We try to keep the buffer
   at a reasonable size, but it can grow. */
char *token = (char *)NULL;

/* Current size of the token buffer. */
int token_buffer_size = 0;

/* Command to read_token () explaining what we want it to do. */
#define READ 0
#define RESET 1
#define prompt_is_ps1 \
      (!prompt_string_pointer || prompt_string_pointer == &ps1_prompt)

/* Function for yyparse to call.  yylex keeps track of
   the last two tokens read, and calls read_token.  */

yylex ()
{
  if (interactive && (!current_token || current_token == '\n'))
    {
      /* Before we print a prompt, we might have to check mailboxes.
	 We do this only if it is time to do so. Notice that only here
	 is the mail alarm reset; nothing takes place in check_mail ()
	 except the checking of mail.  Please don't change this. */
      if (prompt_is_ps1 && time_to_check_mail ())
	{
	  check_mail ();
	  reset_mail_timer ();
	}

      /* Allow the execution of a random command just before the printing
	 of each primary prompt.  If the shell variable PROMPT_COMMAND
	 is set then the value of it is the command to execute. */
      if (prompt_is_ps1)
	{
	  char *command_to_execute = get_string_value ("PROMPT_COMMAND");

	  if (command_to_execute)
	    {
	      extern Function *last_shell_builtin, *this_shell_builtin;
	      extern int last_command_exit_value;
	      Function *temp_last, *temp_this;
	      int temp_exit_value, temp_eof_encountered;

	      temp_last = last_shell_builtin;
	      temp_this = this_shell_builtin;
	      temp_exit_value = last_command_exit_value;
	      temp_eof_encountered = eof_encountered;

	      parse_and_execute
		(savestring (command_to_execute), "PROMPT_COMMAND");

	      last_shell_builtin = temp_last;
	      this_shell_builtin = temp_this;
	      last_command_exit_value = temp_exit_value;
	      eof_encountered = temp_eof_encountered;
	    }
	}
      prompt_again ();
    }

  token_before_that = last_read_token;
  last_read_token = current_token;
  current_token = read_token (READ);
  return (current_token);
}

/* Called from shell.c when Control-C is typed at top level.  Or
   by the error rule at top level. */
reset_parser ()
{
  read_token (RESET);
}
  
/* When non-zero, we have read the required tokens
   which allow ESAC to be the next one read. */
static int allow_esac_as_next = 0;

/* When non-zero, accept single '{' as a token itself. */
static int allow_open_brace = 0;

/* DELIMITER is the value of the delimiter that is currently
   enclosing, or zero for none. */
static int delimiter = 0;
static int old_delimiter = 0;

/* When non-zero, an open-brace used to create a group is awaiting a close
   brace partner. */
static int open_brace_awaiting_satisfaction = 0;

/* If non-zero, it is the token that we want read_token to return regardless
   of what text is (or isn't) present to be read.  read_token resets this. */
int token_to_read = 0;

/* Read the next token.  Command can be READ (normal operation) or 
   RESET (to normalize state). */
read_token (command)
     int command;
{
  extern int interactive_shell;	/* Whether the current shell is interactive. */
  int character;		/* Current character. */
  int peek_char;		/* Temporary look-ahead character. */
  int result;			/* The thing to return. */
  WORD_DESC *the_word;		/* The value for YYLVAL when a WORD is read. */

  if (token_buffer_size < TOKEN_DEFAULT_GROW_SIZE)
    {
      if (token)
	free (token);
      token = (char *)xmalloc (token_buffer_size = TOKEN_DEFAULT_GROW_SIZE);
    }

  if (command == RESET)
    {
      delimiter = old_delimiter = 0;
      open_brace_awaiting_satisfaction = 0;
      in_case_pattern_list = 0;

#if defined (ALIAS)
      if (pushed_string_list)
	{
	  free_string_list ();
	  pushed_string_list = (STRING_SAVER *)NULL;
	}

      if (pending_token_being_expanded)
	{
	  free (pending_token_being_expanded);
	  pending_token_being_expanded = (char *)NULL;
	}

      if (current_token_being_expanded)
	{
	  free (current_token_being_expanded);
	  current_token_being_expanded = (char *)NULL;
	}

      if (expanded_token_stack)
	{
	  free_expansion_stack ();
	  expanded_token_stack = (EXPANSION_SAVER *)NULL;
	}

      expand_next_token = 0;
#endif /* ALIAS */

      if (shell_input_line)
	{
	  free (shell_input_line);
	  shell_input_line = (char *)NULL;
	  shell_input_line_size = shell_input_line_index = 0;
	}
      last_read_token = '\n';
      token_to_read = '\n';
      return ('\n');
    }

  if (token_to_read)
    {
      int rt = token_to_read;
      token_to_read = 0;
      return (rt);
    }

#if defined (ALIAS)
  /*
   * Now we can replace current_token_being_expanded with 
   * pending_token_being_expanded, since the token that would be 
   * inappropriately compared has already been returned.
   *
   * To see why restoring current_token_being_expanded in pop_string ()
   * could be a problem, consider "alias foo=foo".  Then try to
   * expand `foo'.  The initial value of current_token_being_expanded is
   * NULL, so that is what is pushed onto pushed_string_list as the
   * value of saved_token_being_expanded.  "foo" then becomes shell_input_line.
   * read_token calls shell_getc for `f', `o', `o', and then shell_getc
   * hits the end of shell_input_line.  pushed_string_list is not empty
   * so it gets popped.  If we were to blindly restore
   * current_token_being_expanded at this point, `foo' would be compared
   * with a NULL string in the check for recursive expansion, and would
   * infinitely recurse.
   */
  if (pending_token_being_expanded)
    {
      if (current_token_being_expanded)
	free (current_token_being_expanded);
      current_token_being_expanded = pending_token_being_expanded;
      pending_token_being_expanded = (char *)NULL;
    }

  /* If we hit read_token () and there are no saved strings on the
     pushed_string_list, then we are no longer currently expanding a
     token.  This can't be done in pop_stream, because pop_stream
     may pop the stream before the current token has finished being
     completely expanded (consider what happens when we alias foo to foo,
     and then try to expand it). */
  if (!pushed_string_list && current_token_being_expanded)
    {
      free (current_token_being_expanded);
      current_token_being_expanded = (char *)NULL;

      if (expanded_token_stack)
	{
	  free_expansion_stack ();
	  expanded_token_stack = (EXPANSION_SAVER *)NULL;
	}
    }

  /* This is a place to jump back to once we have successfully expanded a
     token with an alias and pushed the string with push_string () */
re_read_token:

#endif /* ALIAS */

  /* Read a single word from input.  Start by skipping blanks. */
  while ((character = shell_getc (1)) != EOF && whitespace (character));

  if (character == EOF)
    return (yacc_EOF);

  if (character == '#' && !interactive)
    {
      /* A comment.  Discard until EOL or EOF, and then return a newline. */
      discard_until ('\n');
      shell_getc (0);

      /* If we're about to return an unquoted newline, we can go and collect
	 the text of any pending here document. */
      if (need_here_doc)
	make_here_document (redirection_needing_here_doc);
      need_here_doc = 0;

#if defined (ALIAS)
      expand_next_token = 0;
#endif /* ALIAS */

      return ('\n');
    }

  if (character == '\n')
    {
      /* If we're about to return an unquoted newline, we can go and collect
	 the text of any pending here document. */
      if (need_here_doc)
	make_here_document (redirection_needing_here_doc);
      need_here_doc = 0;

#if defined (ALIAS)
      expand_next_token = 0;
#endif /* ALIAS */

      return (character);
    }

  if (member (character, "()<>;&|"))
    {
#if defined (ALIAS)
      /* Turn off alias tokenization iff this character sequence would
	 not leave us ready to read a command. */
      if (character == '<' || character == '>')
	expand_next_token = 0;
#endif /* ALIAS */

      /* Please note that the shell does not allow whitespace to
	 appear in between tokens which are character pairs, such as
	 "<<" or ">>".  I believe this is the correct behaviour. */
      if (character == (peek_char = shell_getc (1)))
	{
	  switch (character)
	    {
	      /* If '<' then we could be at "<<" or at "<<-".  We have to
		 look ahead one more character. */
	    case '<':
	      peek_char = shell_getc (1);
	      if (peek_char == '-')
		return (LESS_LESS_MINUS);
	      else
		{
		  shell_ungetc (peek_char);
		  return (LESS_LESS);
		}

	    case '>':
	      return (GREATER_GREATER);

	    case ';':
	      in_case_pattern_list = 1;
#if defined (ALIAS)
	      expand_next_token = 0;
#endif /* ALIAS */
	      return (SEMI_SEMI);

	    case '&':
	      return (AND_AND);

	    case '|':
	      return (OR_OR);
	    }
	}
      else
	{
	  if (peek_char == '&')
	    {
	      switch (character)
		{
		case '<': return (LESS_AND);
		case '>': return (GREATER_AND);
		}
	    }
	  if (character == '<' && peek_char == '>')
	    return (LESS_GREATER);
	  if (character == '>' && peek_char == '|')
	    return (GREATER_BAR);
	  if (peek_char == '>' && character == '&')
	    return (AND_GREATER);
	}
      shell_ungetc (peek_char);

      /* If we look like we are reading the start of a function
	 definition, then let the reader know about it so that
	 we will do the right thing with `{'. */
      if (character == ')' &&
	  last_read_token == '(' && token_before_that == WORD)
	{
	  allow_open_brace = 1;
#if defined (ALIAS)
	  expand_next_token = 0;
#endif /* ALIAS */
	}

      if (in_case_pattern_list && (character == ')'))
	in_case_pattern_list = 0;

      return (character);
    }

  /* Hack <&- (close stdin) case. */
  if (character == '-')
    {
      switch (last_read_token)
	{
	case LESS_AND:
	case GREATER_AND:
	  return (character);
	}
    }
  
  /* Okay, if we got this far, we have to read a word.  Read one,
     and then check it against the known ones. */
  {
    /* Index into the token that we are building. */
    int token_index = 0;

    /* ALL_DIGITS becomes zero when we see a non-digit. */
    int all_digits = digit (character);

    /* DOLLAR_PRESENT becomes non-zero if we see a `$'. */
    int dollar_present = 0;

    /* QUOTED becomes non-zero if we see one of ("), ('), (`), or (\). */
    int quoted = 0;

    /* Non-zero means to ignore the value of the next character, and just
       to add it no matter what. */
    int pass_next_character = 0;

    /* Non-zero means parsing a dollar-paren construct.  It is the count of
       un-quoted closes we need to see. */
    int dollar_paren_level = 0;

    /* Non-zero means parsing a dollar-bracket construct ($[...]).  It is
       the count of un-quoted `]' characters we need to see. */
    int dollar_bracket_level = 0;

    /* Another level variable.  This one is for dollar_parens inside of
       double-quotes. */
    int delimited_paren_level = 0;

    for (;;)
      {
	if (character == EOF)
	  goto got_token;

	if (pass_next_character)
	  {
	    pass_next_character = 0;
	    goto got_character;
	  }

      if (delimiter && character == '\\' && delimiter != '\'')
	{
	  peek_char = shell_getc (0);
	  if (peek_char != '\\')
	    shell_ungetc (peek_char);
	  else
	    {
	      token[token_index++] = character;
	      goto got_character;
	    }
	}

	/* Handle backslashes.  Quote lots of things when not inside of
	   double-quotes, quote some things inside of double-quotes. */
	   
	if (character == '\\' && delimiter != '\'')
	  {
	    peek_char = shell_getc (0);

	    /* Backslash-newline is ignored in all cases excepting
	       when quoted with single quotes. */
	    if (peek_char == '\n')
	      {
		character = '\n';
		goto next_character;
	      }
	    else
	      {
		shell_ungetc (peek_char);

		/* If the next character is to be quoted, do it now. */
		if (!delimiter || delimiter == '`' ||
		    ((delimiter == '"' ) &&
		     (member (peek_char, slashify_in_quotes))))
		  {
		    pass_next_character++;
		    quoted = 1;
		    goto got_character;
		  }
	      }
	  }

	/* This is a hack, in its present form.  If a backquote substitution
	   appears within double quotes, everything within the backquotes
	   should be read as part of a single word.  Jesus.  Now I see why
	   Korn introduced the $() form. */
	if (delimiter && delimiter == '"' && character == '`')
	  {
	    old_delimiter = delimiter;
	    delimiter = character;
	    goto got_character;
	  }

	if (delimiter)
	  {
	    if (character == delimiter)
	      {
		if (delimited_paren_level)
		  {
#if defined (NOTDEF)
		    report_error ("Expected ')' before %c", character);
		    return ('\n');
#else
		    goto got_character;
#endif /* NOTDEF */
		  }

		delimiter = 0;

		if (old_delimiter == '"' && character == '`')
		  {
		    delimiter = old_delimiter;
		    old_delimiter = 0;
		  }

		goto got_character;
	      }
	  }

	if (!delimiter || delimiter == '`' || delimiter == '"')
	  {
	    if (character == '$')
	      {
		peek_char = shell_getc (1);
		shell_ungetc (peek_char);
		if (peek_char == '(')
		  {
		    if (!delimiter)
		      dollar_paren_level++;
		    else
		      delimited_paren_level++;

		    pass_next_character++;
		    goto got_character;
		  }
		else if (peek_char == '[')
		  {
		    if (!delimiter)
		      dollar_bracket_level++;

		    pass_next_character++;
		    goto got_character;
		  }
	      }

	    /* If we are parsing a $() or $[] construct, we need to balance
	       parens and brackets inside the construct.  This whole function
	       could use a rewrite. */
	    if (character == '(')
	      {
		if (delimiter && delimited_paren_level)
		  delimited_paren_level++;

		if (!delimiter && dollar_paren_level)
		  dollar_paren_level++;
	      }

	    if (character == '[')
	      {
		if (!delimiter && dollar_bracket_level)
		  dollar_bracket_level++;
	      }

	    /* This code needs to take into account whether we are inside a
	       case statement pattern list, and whether this paren is supposed
	       to terminate it (hey, it could happen).  It's not as simple
	       as just using in_case_pattern_list, because we're not parsing
	       anything while we're reading a $( ) construct.  Maybe we
	       should move that whole mess into the yacc parser. */
	    if (character == ')')
	      {
		if (delimiter && delimited_paren_level)
		  delimited_paren_level--;

		if (!delimiter && dollar_paren_level)
		  {
		    dollar_paren_level--;
		    goto got_character;
		  }
	      }

	    if (character == ']')
	      {
		if (!delimiter && dollar_bracket_level)
		  {
		    dollar_bracket_level--;
		    goto got_character;
		  }
	      }
	  }

	if (!dollar_paren_level && !dollar_bracket_level && !delimiter &&
	    member (character, " \t\n;&()|<>"))
	  {
	    shell_ungetc (character);
	    goto got_token;
	  }
    
	if (!delimiter)
	  {
	    if (character == '"' || character == '`' || character == '\'')
	      {
		quoted = 1;
		delimiter = character;
		goto got_character;
	      }
	  }

	if (all_digits) all_digits = digit (character);
	if (character == '$') dollar_present = 1;

      got_character:

	token[token_index++] = character;

	if (token_index == (token_buffer_size - 1))
	  token = (char *)xrealloc (token, (token_buffer_size
					    += TOKEN_DEFAULT_GROW_SIZE));
	{
	  char *decode_prompt_string ();

	next_character:
	  if (character == '\n' && interactive && yy_input_type != st_string)
	    prompt_again ();
	}
	/* We want to remove quoted newlines (that is, a \<newline> pair)
	   unless we are within single quotes or pass_next_character is
	   set (the shell equivalent of literal-next). */
	character = shell_getc ((delimiter != '\'') && (!pass_next_character));
      }

  got_token:

    token[token_index] = '\0';
	
    if ((delimiter || dollar_paren_level || dollar_bracket_level) &&
	character == EOF)
      {
	if (dollar_paren_level && !delimiter)
	  delimiter = ')';
	else if (dollar_bracket_level && !delimiter)
	  delimiter = ']';

	report_error ("Unexpected EOF.  Looking for `%c'.", delimiter);
	return (-1);
      }

    if (all_digits)
      {
	/* Check to see what thing we should return.  If the last_read_token
	   is a `<', or a `&', or the character which ended this token is
	   a '>' or '<', then, and ONLY then, is this input token a NUMBER.
	   Otherwise, it is just a word, and should be returned as such. */

	if ((character == '<' || character == '>') ||
	    (last_read_token == LESS_AND ||
	     last_read_token == GREATER_AND))
	  {
	    yylval.number = atoi (token); /* was sscanf (token, "%d", &(yylval.number)); */
	    return (NUMBER);
	  }
      }

    /* Handle special case.  IN is recognized if the last token
       was WORD and the token before that was FOR or CASE. */
    if ((last_read_token == WORD) &&
	((token_before_that == FOR) || (token_before_that == CASE)) &&
	(STREQ (token, "in")))
      {
	if (token_before_that == CASE)
	  {
	    in_case_pattern_list = 1;
	    allow_esac_as_next++;
	  }
	return (IN);
      }

    /* Ditto for DO in the FOR case. */
    if ((last_read_token == WORD) && (token_before_that == FOR) &&
	(STREQ (token, "do")))
      return (DO);

    /* Ditto for ESAC in the CASE case. 
       Specifically, this handles "case word in esac", which is a legal
       construct, certainly because someone will pass an empty arg to the
       case construct, and we don't want it to barf.  Of course, we should
       insist that the case construct has at least one pattern in it, but
       the designers disagree. */
    if (allow_esac_as_next)
      {
	allow_esac_as_next--;
	if (STREQ (token, "esac"))
	  {
	    in_case_pattern_list = 0;
	    return (ESAC);
	  }
      }

    /* Ditto for `{' in the FUNCTION case. */
    if (allow_open_brace)
      {
	allow_open_brace = 0;
	if (STREQ (token, "{"))
	  {
	    open_brace_awaiting_satisfaction++;
	    return ('{');
	  }
      }

#if defined (ALIAS)

#define command_token_position(token) \
    ((token) != SEMI_SEMI && reserved_word_acceptable (token))

    /* OK, we have a token.  Let's try to alias expand it, if (and only if)
       it's eligible. 

       It is eligible for expansion if the shell is in interactive mode, and
       the token is unquoted and the last token read was a command
       separator (or expand_next_token is set), and we are currently
       processing an alias (pushed_string_list is non-empty) and this
       token is not the same as the current or any previously
       processed alias.

       Special cases that disqualify:
	 In a pattern list in a case statement (in_case_pattern_list). */
    if (interactive_shell && !quoted && !in_case_pattern_list &&
	(command_token_position (last_read_token) || expand_next_token))
      {
	char *alias_expand_word (), *expanded;
	if (current_token_being_expanded &&
	     ((STREQ (token, current_token_being_expanded)) ||
	      (token_has_been_expanded (token))))
	  goto no_expansion;

	expanded = alias_expand_word (token);
	if (expanded)
	  {
	    int len = strlen (expanded), expand_next;
	    char *temp;

	    /* Erase the current token. */
	    token_index = 0;

	    expand_next = (expanded[len - 1] == ' ') ||
			  (expanded[len - 1] == '\t');

	    temp = savestring (token);
	    push_string (expanded, expand_next, temp);
	    goto re_read_token;
	  }
	else
	  /* This is an eligible token that does not have an expansion. */
no_expansion:
	  expand_next_token = 0;
      }
    else
      {
	expand_next_token = 0;
      }
#endif /* ALIAS */

    /* Check to see if it is a reserved word.  */
    if (!dollar_present && !quoted &&
	reserved_word_acceptable (last_read_token))
      {
	int i;
	for (i = 0; word_token_alist[i].word != (char *)NULL; i++)
	  if (STREQ (token, word_token_alist[i].word))
	    {
	      if (in_case_pattern_list && (word_token_alist[i].token != ESAC))
		break;

	      if (word_token_alist[i].token == ESAC)
		in_case_pattern_list = 0;

	      if (word_token_alist[i].token == '{')
		open_brace_awaiting_satisfaction++;

	      return (word_token_alist[i].token);
	    }
      }

    /* What if we are attempting to satisfy an open-brace grouper? */
    if (open_brace_awaiting_satisfaction && strcmp (token, "}") == 0)
      {
	open_brace_awaiting_satisfaction--;
	return ('}');
      }

    the_word = (WORD_DESC *)xmalloc (sizeof (WORD_DESC));
    the_word->word = (char *)xmalloc (1 + strlen (token));
    strcpy (the_word->word, token);
    the_word->dollar_present = dollar_present;
    the_word->quoted = quoted;
    the_word->assignment = assignment (token);

    yylval.word = the_word;
    result = WORD;
    if (last_read_token == FUNCTION)
      allow_open_brace = 1;
  }
  return (result);
}

#if defined (NOTDEF)		/* Obsoleted function no longer used. */
/* Return 1 if this token is a legal shell `identifier'; that is, it consists
   solely of letters, digits, and underscores, and does not begin with a 
   digit. */
legal_identifier (name)
     char *name;
{
  register char *s;

  if (!name || !*name)
    return (0);

  if (digit (*name))
    return (0);

  for (s = name; s && *s; s++)
    {
      if (!isletter (*s) && !digit (*s) && (*s != '_'))
	return (0);
    }
  return (1);
}
#endif /* NOTDEF */

/* Return 1 if TOKEN is a token that after being read would allow
   a reserved word to be seen, else 0. */
static int
reserved_word_acceptable (token)
     int token;
{
  if (member (token, "\n;()|&{") ||
      token == AND_AND ||
      token == BANG ||
      token == DO ||
      token == ELIF ||
      token == ELSE ||
      token == IF ||
      token == OR_OR ||
      token == SEMI_SEMI ||
      token == THEN ||
      token == UNTIL ||
      token == WHILE ||
      token == 0)
    return (1);
  else
    return (0);
}

#if defined (READLINE)
/* Called after each time readline is called.  This insures that whatever
   the new prompt string is gets propagated to readline's local prompt
   variable. */
reset_readline_prompt ()
{
  if (prompt_string_pointer && *prompt_string_pointer)
    {
      char *temp_prompt, *decode_prompt_string ();

      temp_prompt = decode_prompt_string (*prompt_string_pointer);

      if (!temp_prompt)
	temp_prompt = savestring ("");

      if (current_readline_prompt)
	free (current_readline_prompt);

      current_readline_prompt = temp_prompt;
    }
}
#endif

/* A list of tokens which can be followed by newlines, but not by
   semi-colons.  When concatenating multiple lines of history, the
   newline separator for such tokens is replaced with a space. */
int no_semi_successors[] = {
  '\n', '{', '(', ')', ';',
  CASE, DO, ELSE, IF, IN, SEMI_SEMI, THEN, UNTIL, WHILE,
  0
};

/* Add a line to the history list.
   The variable COMMAND_ORIENTED_HISTORY controls the style of history
   remembering;  when non-zero, and LINE is not the first line of a
   complete parser construct, append LINE to the last history line instead
   of adding it as a new line. */
bash_add_history (line)
     char *line;
{
  extern int command_oriented_history;
  int add_it = 1;

  if (command_oriented_history && current_command_line_count > 1)
    {
      register int offset;
      register HIST_ENTRY *current, *old;
      char *chars_to_add, *new_line;

      /* If we are not within a delimited expression, try to be smart
	 about which separators can be semi-colons and which must be
	 newlines. */
      if (!delimiter)
	{
	  register int i;

	  chars_to_add = (char *)NULL;

	  for (i = 0; no_semi_successors[i]; i++)
	    {
	      if (token_before_that == no_semi_successors[i])
		{
		  chars_to_add = " ";
		  break;
		}
	    }
	  if (!chars_to_add)
	    chars_to_add = "; ";
	}
      else
	chars_to_add = "\n";

      using_history ();

      current = previous_history ();

      if (current)
	{
	  offset = where_history ();
	  new_line = (char *) xmalloc (1
				       + strlen (current->line)
				       + strlen (line)
				       + strlen (chars_to_add));
	  sprintf (new_line, "%s%s%s", current->line, chars_to_add, line);
	  old = replace_history_entry (offset, new_line, current->data);
	  free (new_line);

	  if (old)
	    {
	      /* Note that the old data is not freed, since it was simply
		 copied to the new history entry. */
	      if (old->line)
		free (old->line);

	      free (old);
	    }
	  add_it = 0;
	}
    }

  if (add_it)
    {
      extern int history_lines_this_session;

      add_history (line);
      history_lines_this_session++;
    }
  using_history ();
}

/* Issue a prompt, or prepare to issue a prompt when the next character
   is read. */
prompt_again ()
{
  char *temp_prompt, *decode_prompt_string ();

  ps1_prompt = get_string_value ("PS1");
  ps2_prompt = get_string_value ("PS2");

  if (!prompt_string_pointer)
    prompt_string_pointer = &ps1_prompt;

  if (*prompt_string_pointer)
    temp_prompt = decode_prompt_string (*prompt_string_pointer);
  else
    temp_prompt = savestring ("");

  current_prompt_string = *prompt_string_pointer;
  prompt_string_pointer = &ps2_prompt;

#if defined (READLINE)
  if (!no_line_editing)
    {
      if (current_readline_prompt)
	free (current_readline_prompt);
      
      current_readline_prompt = temp_prompt;
    }
  else
#endif	/* READLINE */
    {
      if (interactive)
	{
	  fprintf (stderr, "%s", temp_prompt);
	  fflush (stderr);
	}
      free (temp_prompt);
    }
}

#include "maxpath.h"

/* Return a string which will be printed as a prompt.  The string
   may contain special characters which are decoded as follows:
   
	\t	the time
	\d	the date
	\n	CRLF
	\s	the name of the shell
	\w	the current working directory
	\W	the last element of PWD
	\u	your username
	\h	the hostname
	\#	the command number of this command
	\!	the history number of this command
	\$	a $ or a # if you are root
	\<octal> character code in octal
	\\	a backslash
*/
#include <sys/param.h>
#include <time.h>

#define PROMPT_GROWTH 50
char *
decode_prompt_string (string)
     char *string;
{
  int result_size = PROMPT_GROWTH;
  int result_index = 0;
  char *result = (char *)xmalloc (PROMPT_GROWTH);
  int c;
  char *temp = (char *)NULL;

  result[0] = 0;
  while (c = *string++)
    {
      if (c == '\\')
	{
	  c = *string;

	  switch (c)
	    {
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	      {
		char octal_string[4];
		int n;

		strncpy (octal_string, string, 3);
		octal_string[3] = '\0';

		n = read_octal (octal_string);

		temp = savestring ("\\");
		if (n != -1)
		  {
		    string += 3;
		    temp[0] = n;
		  }

		c = 0;
		goto add_string;
	      }
	  
	    case 't':
	    case 'd':
	      /* Make the current time/date into a string. */
	      {
		long the_time = time (0);
		char *ttemp = ctime (&the_time);
		temp = savestring (ttemp);

		if (c == 't')
		  {
		    strcpy (temp, temp + 11);
		    temp[8] = '\0';
		  }
		else
		  temp[10] = '\0';

		goto add_string;
	      }

	    case 'n':
	      if (!no_line_editing)
		temp = savestring ("\r\n");
	      else
		temp = savestring ("\n");
	      goto add_string;

	    case 's':
	      {
		extern char *shell_name;
		temp = savestring (shell_name);
		goto add_string;
	      }
	
	    case 'w':
	    case 'W':
	      {
		/* Use the value of PWD because it is much more effecient. */
#define EFFICIENT
#ifdef EFFICIENT
		char *polite_directory_format (), t_string[MAXPATHLEN];

		temp = get_string_value ("PWD");

		if (!temp)
		  getwd (t_string);
		else
		  strcpy (t_string, temp);
#else
		getwd (t_string);
#endif	/* EFFICIENT */

		if (c == 'W')
		  {
		    char *dir = (char *)rindex (t_string, '/');
		    if (dir && dir != t_string)
		      strcpy (t_string, dir + 1);
		    temp = savestring (t_string);
		  }
		else
		  temp = savestring (polite_directory_format (t_string));
		goto add_string;
	      }
      
	    case 'u':
	      {
		extern char *current_user_name;
		temp = savestring (current_user_name);

		goto add_string;
	      }

	    case 'h':
	      {
		extern char *current_host_name;
		char *t_string;

		temp = savestring (current_host_name);
		if (t_string = (char *)index (temp, '.'))
		  *t_string = '\0';
		
		goto add_string;
	      }

	    case '#':
	      {
		extern int current_command_number;
		char number_buffer[20];
		sprintf (number_buffer, "%d", current_command_number);
		temp = savestring (number_buffer);
		goto add_string;
	      }

	    case '!':
	      {
		extern int history_base, where_history ();
		char number_buffer[20];

		using_history ();
		if (get_string_value ("HISTSIZE"))
		  sprintf (number_buffer, "%d",
			   history_base + where_history ());
		else
		  strcpy (number_buffer, "!");
		temp = savestring (number_buffer);
		goto add_string;
	      }

	    case '$':
	      temp = savestring (geteuid () == 0 ? "#" : "$");
	      goto add_string;

	    case '\\':
	      temp = savestring ("\\");
	      goto add_string;

	    default:
	      temp = savestring ("\\ ");
	      temp[1] = c;

	    add_string:
	      if (c)
		string++;
	      result =
		(char *)sub_append_string (temp, result,
					   &result_index, &result_size);
	      temp = (char *)NULL; /* Free ()'ed in sub_append_string (). */
	      result[result_index] = '\0';
	      break;
	    }
	}
      else
	{
	  while (3 + result_index > result_size)
	    result = (char *)xrealloc (result, result_size += PROMPT_GROWTH);

	  result[result_index++] = c;
	  result[result_index] = '\0';
	}
    }

  /* I don't really think that this is a good idea.  Do you? */
  if (!find_variable ("NO_PROMPT_VARS"))
    {
      WORD_LIST *expand_string (), *list;
      char *string_list ();

      list = expand_string (result, 1);
      free (result);
      result = string_list (list);
      dispose_words (list);
    }

  return (result);
}

/* Report a syntax error, and restart the parser.  Call here for fatal
   errors. */
yyerror ()
{
  report_syntax_error ((char *)NULL);
  reset_parser ();
}

/* Report a syntax error with line numbers, etc.
   Call here for recoverable errors.  If you have a message to print,
   then place it in MESSAGE, otherwise pass NULL and this will figure
   out an appropriate message for you. */
report_syntax_error (message)
     char *message;
{
  if (message)
    {
      if (!interactive)
	{
	  char *name = stream_name ? stream_name : "stdin";
	  report_error ("%s:%d: `%s'", name, line_number, message);
	}
      else
	report_error ("%s", message);

      return;
    }

  if (shell_input_line && *shell_input_line)
    {
      char *error_token, *t = shell_input_line;
      register int i = shell_input_line_index;
      int token_end = 0;

      if (!t[i] && i)
	i--;

      while (i && (t[i] == ' ' || t[i] == '\t' || t[i] == '\n'))
	i--;

      if (i)
	token_end = i + 1;

      while (i && !member (t[i], " \n\t;|&"))
	i--;

      while (i != token_end && member (t[i], " \n\t"))
	i++;

      if (token_end)
	{
	  error_token = (char *)alloca (1 + (token_end - i));
	  strncpy (error_token, t + i, token_end - i);
	  error_token[token_end - i] = '\0';

	  report_error ("syntax error near `%s'", error_token);
	}
      else if ((i == 0) && (token_end == 0))	/* a 1-character token */
	{
	  error_token = (char *) alloca (2);
	  strncpy(error_token, t + i, 1);
	  error_token[1] = '\0';

	  report_error ("syntax error near `%s'", error_token);
	}

      if (!interactive)
	{
	  char *temp = savestring (shell_input_line);
	  char *name = stream_name ? stream_name : "stdin";
	  int l = strlen (temp);

	  while (l && temp[l - 1] == '\n')
	    temp[--l] = '\0';

	  report_error ("%s:%d: `%s'", name, line_number, temp);
	  free (temp);
	}
    }
  else
    report_error ("Syntax error");
}

/* ??? Needed function. ??? We have to be able to discard the constructs
   created during parsing.  In the case of error, we want to return
   allocated objects to the memory pool.  In the case of no error, we want
   to throw away the information about where the allocated objects live.
   (dispose_command () will actually free the command. */
discard_parser_constructs (error_p)
     int error_p;
{
/*   if (error_p) {
     fprintf (stderr, "*");
  } */
}
   
/* Do that silly `type "bye" to exit' stuff.  You know, "ignoreeof". */

/* The number of times that we have encountered an EOF character without
   another character intervening.  When this gets above the limit, the
   shell terminates. */
int eof_encountered = 0;

/* The limit for eof_encountered. */
int eof_encountered_limit = 10;

/* If we have EOF as the only input unit, this user wants to leave
   the shell.  If the shell is not interactive, then just leave.
   Otherwise, if ignoreeof is set, and we haven't done this the
   required number of times in a row, print a message. */
handle_eof_input_unit ()
{
  extern int login_shell, EOF_Reached;

  if (interactive)
    {
      /* If the user wants to "ignore" eof, then let her do so, kind of. */
      if (find_variable ("ignoreeof") || find_variable ("IGNOREEOF"))
	{
	  if (eof_encountered < eof_encountered_limit)
	    {
	      fprintf (stderr, "Use \"%s\" to leave the shell.\n",
		       login_shell ? "logout" : "exit");
	      eof_encountered++;
	      /* Reset the prompt string to be $PS1. */
	      prompt_string_pointer = (char **)NULL;
	      prompt_again ();
	      last_read_token = current_token = '\n';
	      return;
	    } 
	}

      /* In this case EOF should exit the shell.  Do it now. */
      reset_parser ();
      exit_builtin ((WORD_LIST *)NULL);
    }
  else
    {
      /* We don't write history files, etc., for non-interactive shells. */
      EOF_Reached = 1;
    }
}
