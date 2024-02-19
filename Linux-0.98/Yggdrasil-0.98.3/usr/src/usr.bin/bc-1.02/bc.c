
/*  A Bison parser, made from bc.y  */

#define YYBISON 1  /* Identify Bison output.  */

#define	NEWLINE	258
#define	AND	259
#define	OR	260
#define	NOT	261
#define	STRING	262
#define	NAME	263
#define	NUMBER	264
#define	MUL_OP	265
#define	ASSIGN_OP	266
#define	REL_OP	267
#define	INCR_DECR	268
#define	Define	269
#define	Break	270
#define	Quit	271
#define	Length	272
#define	Return	273
#define	For	274
#define	If	275
#define	While	276
#define	Sqrt	277
#define	Else	278
#define	Scale	279
#define	Ibase	280
#define	Obase	281
#define	Auto	282
#define	Read	283
#define	Warranty	284
#define	Halt	285
#define	Last	286
#define	Continue	287
#define	Print	288
#define	Limits	289
#define	UNARY_MINUS	290

#line 1 "bc.y"

/* bc.y: The grammar for a POSIX compatable bc processor with some
         extensions to the language. */

/*  This file is part of bc written for MINIX.
    Copyright (C) 1991, 1992 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License , or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

    You may contact the author by:
       e-mail:  phil@cs.wwu.edu
      us-mail:  Philip A. Nelson
                Computer Science Department, 9062
                Western Washington University
                Bellingham, WA 98226-9062
       
*************************************************************************/

#include "bcdefs.h"
#include "global.h"
#include "proto.h"

#line 38 "bc.y"
typedef union {
	char	 *s_value;
	char	  c_value;
	int	  i_value;
	arg_list *a_value;
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



#define	YYFINAL		167
#define	YYFLAG		-32768
#define	YYNTBASE	47

#define YYTRANSLATE(x) ((unsigned)(x) <= 290 ? yytranslate[x] : 79)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    40,
    41,     2,    35,    44,    36,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    39,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    45,     2,    46,    37,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    42,     2,    43,     2,     2,     2,     2,     2,
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
    26,    27,    28,    29,    30,    31,    32,    33,    34,    38
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     7,     9,    12,    13,    15,    19,    22,
    23,    25,    28,    32,    35,    39,    41,    44,    46,    48,
    50,    52,    54,    56,    58,    60,    62,    67,    68,    69,
    70,    71,    85,    86,    94,    95,    96,   104,   108,   109,
   113,   115,   119,   121,   123,   124,   125,   129,   130,   143,
   144,   146,   147,   151,   155,   157,   161,   165,   171,   172,
   174,   176,   180,   184,   190,   191,   193,   194,   196,   197,
   202,   203,   208,   209,   214,   217,   221,   225,   229,   233,
   237,   240,   242,   244,   248,   253,   256,   259,   264,   269,
   274,   278,   280,   285,   287,   289,   291
};

#endif

static const short yyrhs[] = {    -1,
    47,    48,     0,    49,     3,     0,    65,     0,     1,     3,
     0,     0,    51,     0,    49,    39,    51,     0,    49,    39,
     0,     0,    51,     0,    50,     3,     0,    50,     3,    51,
     0,    50,    39,     0,    50,    39,    52,     0,    52,     0,
     1,    52,     0,    29,     0,    34,     0,    74,     0,     7,
     0,    15,     0,    32,     0,    16,     0,    30,     0,    18,
     0,    18,    40,    73,    41,     0,     0,     0,     0,     0,
    19,    53,    40,    72,    39,    54,    72,    39,    55,    72,
    41,    56,    52,     0,     0,    20,    40,    74,    41,    57,
    52,    63,     0,     0,     0,    21,    58,    40,    74,    59,
    41,    52,     0,    42,    50,    43,     0,     0,    33,    60,
    61,     0,    62,     0,    62,    44,    61,     0,     7,     0,
    74,     0,     0,     0,    23,    64,    52,     0,     0,    14,
     8,    40,    67,    41,    42,     3,    68,    66,    50,     3,
    43,     0,     0,    69,     0,     0,    27,    69,     3,     0,
    27,    69,    39,     0,     8,     0,     8,    45,    46,     0,
    69,    44,     8,     0,    69,    44,     8,    45,    46,     0,
     0,    71,     0,    74,     0,     8,    45,    46,     0,    71,
    44,    74,     0,    71,    44,     8,    45,    46,     0,     0,
    74,     0,     0,    74,     0,     0,    78,    11,    75,    74,
     0,     0,    74,     4,    76,    74,     0,     0,    74,     5,
    77,    74,     0,     6,    74,     0,    74,    12,    74,     0,
    74,    35,    74,     0,    74,    36,    74,     0,    74,    10,
    74,     0,    74,    37,    74,     0,    36,    74,     0,    78,
     0,     9,     0,    40,    74,    41,     0,     8,    40,    70,
    41,     0,    13,    78,     0,    78,    13,     0,    17,    40,
    74,    41,     0,    22,    40,    74,    41,     0,    24,    40,
    74,    41,     0,    28,    40,    41,     0,     8,     0,     8,
    45,    74,    46,     0,    25,     0,    26,     0,    24,     0,
    31,     0
};

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   105,   114,   116,   118,   120,   126,   128,   129,   130,   132,
   134,   135,   136,   137,   138,   140,   141,   144,   146,   148,
   157,   164,   174,   185,   187,   189,   191,   193,   198,   209,
   220,   231,   238,   245,   251,   257,   264,   270,   272,   275,
   276,   277,   279,   285,   288,   289,   298,   298,   311,   317,
   319,   321,   323,   325,   328,   330,   332,   334,   337,   339,
   341,   346,   352,   357,   364,   369,   371,   376,   382,   394,
   409,   417,   422,   430,   438,   444,   472,   477,   482,   489,
   494,   499,   508,   524,   526,   542,   561,   584,   586,   588,
   590,   596,   598,   603,   605,   607,   609
};

static const char * const yytname[] = {   "$","error","$illegal.","NEWLINE",
"AND","OR","NOT","STRING","NAME","NUMBER","MUL_OP","ASSIGN_OP","REL_OP","INCR_DECR",
"Define","Break","Quit","Length","Return","For","If","While","Sqrt","Else","Scale",
"Ibase","Obase","Auto","Read","Warranty","Halt","Last","Continue","Print","Limits",
"'+'","'-'","'^'","UNARY_MINUS","';'","'('","')'","'{'","'}'","','","'['","']'",
"program","input_item","semicolon_list","statement_list","statement_or_error",
"statement","@1","@2","@3","@4","@5","@6","@7","@8","print_list","print_element",
"opt_else","@9","function","@10","opt_parameter_list","opt_auto_define_list",
"define_list","opt_argument_list","argument_list","opt_expression","return_expression",
"expression","@11","@12","@13","named_expression",""
};
#endif

static const short yyr1[] = {     0,
    47,    47,    48,    48,    48,    49,    49,    49,    49,    50,
    50,    50,    50,    50,    50,    51,    51,    52,    52,    52,
    52,    52,    52,    52,    52,    52,    52,    53,    54,    55,
    56,    52,    57,    52,    58,    59,    52,    52,    60,    52,
    61,    61,    62,    62,    63,    64,    63,    66,    65,    67,
    67,    68,    68,    68,    69,    69,    69,    69,    70,    70,
    71,    71,    71,    71,    72,    72,    73,    73,    75,    74,
    76,    74,    77,    74,    74,    74,    74,    74,    74,    74,
    74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
    74,    78,    78,    78,    78,    78,    78
};

static const short yyr2[] = {     0,
     0,     2,     2,     1,     2,     0,     1,     3,     2,     0,
     1,     2,     3,     2,     3,     1,     2,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     4,     0,     0,     0,
     0,    13,     0,     7,     0,     0,     7,     3,     0,     3,
     1,     3,     1,     1,     0,     0,     3,     0,    12,     0,
     1,     0,     3,     3,     1,     3,     3,     5,     0,     1,
     1,     3,     3,     5,     0,     1,     0,     1,     0,     4,
     0,     4,     0,     4,     2,     3,     3,     3,     3,     3,
     2,     1,     1,     3,     4,     2,     2,     4,     4,     4,
     3,     1,     4,     1,     1,     1,     1
};

static const short yydefact[] = {     1,
     0,     0,     0,    21,    92,    83,     0,     0,    22,    24,
     0,    26,    28,     0,    35,     0,    96,    94,    95,     0,
    18,    25,    97,    23,    39,    19,     0,     0,     0,     2,
     0,     7,    16,     4,    20,    82,     5,    17,    75,    59,
     0,    92,    96,    86,     0,     0,    67,     0,     0,     0,
     0,     0,     0,     0,    81,     0,     0,     0,    11,     3,
     0,    71,    73,     0,     0,     0,     0,     0,    69,    87,
    92,     0,    60,    61,     0,    50,     0,     0,    68,    65,
     0,     0,     0,     0,    91,    43,    40,    41,    44,    84,
     0,    14,    38,     8,     0,     0,    79,    76,    77,    78,
    80,     0,     0,    85,     0,    93,    55,     0,    51,    88,
    27,     0,    66,    33,    36,    89,    90,     0,    13,    15,
    72,    74,    70,    62,    92,    63,     0,     0,     0,    29,
     0,     0,    42,     0,    56,     0,    57,    65,    45,     0,
    64,    52,     0,     0,    46,    34,    37,     0,    48,    58,
    30,     0,     0,     0,    65,    47,    53,    54,     0,     0,
     0,    31,    49,     0,    32,     0,     0
};

static const short yydefgoto[] = {     1,
    30,    31,    58,    59,    33,    48,   138,   155,   164,   131,
    50,   132,    54,    87,    88,   146,   152,    34,   154,   108,
   149,   109,    72,    73,   112,    78,    35,   102,    95,    96,
    36
};

static const short yypact[] = {-32768,
   157,   436,   618,-32768,   -31,-32768,    62,    12,-32768,-32768,
   -18,   -17,-32768,   -15,-32768,    -5,     2,-32768,-32768,    20,
-32768,-32768,-32768,-32768,-32768,-32768,   618,   618,   197,-32768,
     4,-32768,-32768,-32768,    73,     8,-32768,-32768,    22,   639,
   618,   -12,-32768,-32768,    28,   618,   618,    29,   618,    31,
   618,   618,    21,   597,-32768,   469,   560,    13,-32768,-32768,
   326,-32768,-32768,   618,   618,   618,   618,   618,-32768,-32768,
   -30,    32,    30,    73,     1,    64,   478,    34,    73,   618,
   481,   618,   490,   524,-32768,-32768,-32768,    36,    73,-32768,
   240,   560,-32768,-32768,   618,   618,    39,    -7,    -6,    -6,
    39,   618,    90,-32768,   660,-32768,    37,    43,    45,-32768,
-32768,    53,    73,-32768,    73,-32768,-32768,   597,-32768,-32768,
    22,   520,    -7,-32768,   -28,    73,    51,    58,    97,-32768,
   560,    65,-32768,   401,-32768,   108,    68,   618,    96,   560,
-32768,    93,    76,    84,-32768,-32768,-32768,    64,-32768,-32768,
-32768,   560,     5,   366,   618,-32768,-32768,-32768,    15,    83,
   283,-32768,-32768,   560,-32768,   125,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,   -27,     0,    -2,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,    10,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   -16,-32768,-32768,  -114,-32768,    -1,-32768,-32768,-32768,
   124
};


#define	YYLAST		700


static const short yytable[] = {    38,
    32,    39,    64,    64,    62,    63,    60,   157,    40,    40,
    64,    40,    65,    41,   103,    91,   134,   161,    69,    45,
    70,    46,    47,   144,    49,    55,    56,    66,    67,    68,
    68,    64,    41,    65,    51,    66,    67,    68,    74,    75,
   160,    52,    61,   158,    77,    79,   106,    81,   129,    83,
    84,    92,    89,    92,    38,    93,    66,    67,    68,    53,
    94,    85,    97,    98,    99,   100,   101,    76,    80,    42,
    82,   107,   104,   105,   111,    68,    62,    63,   113,   118,
   115,   127,    64,   128,    65,    43,    18,    19,   129,   120,
   119,   130,    23,   121,   122,     3,   135,     5,     6,   136,
   123,    75,     7,   126,   137,   140,    11,    66,    67,    68,
   142,    16,   143,    17,    18,    19,    89,    20,   145,   148,
    23,   150,   151,   162,   167,    27,   159,   133,   139,    28,
    44,   153,    75,     0,     0,   124,   113,   147,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   156,
     0,     0,     0,   113,     0,     0,   166,     2,     0,    -6,
   119,   165,     3,     4,     5,     6,     0,     0,     0,     7,
     8,     9,    10,    11,    12,    13,    14,    15,    16,     0,
    17,    18,    19,     0,    20,    21,    22,    23,    24,    25,
    26,     0,    27,     0,     0,    -6,    28,    57,    29,   -10,
     0,     0,     3,     4,     5,     6,     0,     0,     0,     7,
     0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
    17,    18,    19,     0,    20,    21,    22,    23,    24,    25,
    26,     0,    27,     0,     0,   -10,    28,     0,    29,   -10,
    57,     0,   -12,     0,     0,     3,     4,     5,     6,     0,
     0,     0,     7,     0,     9,    10,    11,    12,    13,    14,
    15,    16,     0,    17,    18,    19,     0,    20,    21,    22,
    23,    24,    25,    26,     0,    27,     0,     0,   -12,    28,
     0,    29,   -12,    57,     0,   -12,     0,     0,     3,     4,
     5,     6,     0,     0,     0,     7,     0,     9,    10,    11,
    12,    13,    14,    15,    16,     0,    17,    18,    19,     0,
    20,    21,    22,    23,    24,    25,    26,     0,    27,     0,
     0,   -12,    28,     0,    29,   163,    57,     0,    -9,     0,
     0,     3,     4,     5,     6,     0,     0,     0,     7,     0,
     9,    10,    11,    12,    13,    14,    15,    16,     0,    17,
    18,    19,     0,    20,    21,    22,    23,    24,    25,    26,
     0,    27,     0,     0,    -9,    28,    57,    29,   -10,     0,
     0,     3,     4,     5,     6,     0,     0,     0,     7,     0,
     9,    10,    11,    12,    13,    14,    15,    16,     0,    17,
    18,    19,     0,    20,    21,    22,    23,    24,    25,    26,
     0,    27,     0,     0,   -10,    28,     3,    29,     5,     6,
     0,     0,     0,     7,     0,     0,     0,    11,     0,     0,
     0,     0,    16,     0,    17,    18,    19,     0,    20,     0,
     0,    23,     0,     0,     0,     0,    27,     0,    37,     0,
    28,     3,     4,     5,     6,     0,   141,     0,     7,     0,
     9,    10,    11,    12,    13,    14,    15,    16,     0,    17,
    18,    19,     0,    20,    21,    22,    23,    24,    25,    26,
     0,    27,    62,    63,     0,    28,     0,    29,    64,     0,
    65,    62,    63,     0,    62,    63,     0,    64,     0,    65,
    64,     0,    65,    62,    63,     0,     0,     0,     0,    64,
     0,    65,     0,    66,    67,    68,     0,     0,     0,    90,
     0,     0,    66,    67,    68,    66,    67,    68,   110,     0,
     0,   114,     0,    62,    66,    67,    68,    62,    63,    64,
   116,    65,     0,    64,     0,    65,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    66,    67,    68,     0,    66,    67,
    68,     0,     0,     0,   117,     3,     4,     5,     6,     0,
     0,     0,     7,     0,     9,    10,    11,    12,    13,    14,
    15,    16,     0,    17,    18,    19,     0,    20,    21,    22,
    23,    24,    25,    26,     0,    27,     0,     0,     0,    28,
     0,    29,     3,    86,     5,     6,     0,     0,     0,     7,
     0,     0,     0,    11,     0,     0,     0,     0,    16,     0,
    17,    18,    19,     3,    20,     5,     6,    23,     0,     0,
     7,     0,    27,     0,    11,     0,    28,     0,     0,    16,
     0,    17,    18,    19,     3,    20,    71,     6,    23,     0,
     0,     7,     0,    27,     0,    11,     0,    28,     0,     0,
    16,     0,    17,    18,    19,     3,    20,   125,     6,    23,
     0,     0,     7,     0,    27,     0,    11,     0,    28,     0,
     0,    16,     0,    17,    18,    19,     0,    20,     0,     0,
    23,     0,     0,     0,     0,    27,     0,     0,     0,    28
};

static const short yycheck[] = {     2,
     1,     3,    10,    10,     4,     5,     3,     3,    40,    40,
    10,    40,    12,    45,    45,     3,    45,     3,    11,     8,
    13,    40,    40,   138,    40,    27,    28,    35,    36,    37,
    37,    10,    45,    12,    40,    35,    36,    37,    40,    41,
   155,    40,    39,    39,    46,    47,    46,    49,    44,    51,
    52,    39,    54,    39,    57,    43,    35,    36,    37,    40,
    61,    41,    64,    65,    66,    67,    68,    40,    40,     8,
    40,     8,    41,    44,    41,    37,     4,     5,    80,    44,
    82,    45,    10,    41,    12,    24,    25,    26,    44,    92,
    91,    39,    31,    95,    96,     6,    46,     8,     9,    42,
   102,   103,    13,   105,     8,    41,    17,    35,    36,    37,
     3,    22,    45,    24,    25,    26,   118,    28,    23,    27,
    31,    46,    39,    41,     0,    36,   154,   118,   131,    40,
     7,   148,   134,    -1,    -1,    46,   138,   140,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,
    -1,    -1,    -1,   155,    -1,    -1,     0,     1,    -1,     3,
   161,   164,     6,     7,     8,     9,    -1,    -1,    -1,    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,    -1,
    24,    25,    26,    -1,    28,    29,    30,    31,    32,    33,
    34,    -1,    36,    -1,    -1,    39,    40,     1,    42,     3,
    -1,    -1,     6,     7,     8,     9,    -1,    -1,    -1,    13,
    -1,    15,    16,    17,    18,    19,    20,    21,    22,    -1,
    24,    25,    26,    -1,    28,    29,    30,    31,    32,    33,
    34,    -1,    36,    -1,    -1,    39,    40,    -1,    42,    43,
     1,    -1,     3,    -1,    -1,     6,     7,     8,     9,    -1,
    -1,    -1,    13,    -1,    15,    16,    17,    18,    19,    20,
    21,    22,    -1,    24,    25,    26,    -1,    28,    29,    30,
    31,    32,    33,    34,    -1,    36,    -1,    -1,    39,    40,
    -1,    42,    43,     1,    -1,     3,    -1,    -1,     6,     7,
     8,     9,    -1,    -1,    -1,    13,    -1,    15,    16,    17,
    18,    19,    20,    21,    22,    -1,    24,    25,    26,    -1,
    28,    29,    30,    31,    32,    33,    34,    -1,    36,    -1,
    -1,    39,    40,    -1,    42,    43,     1,    -1,     3,    -1,
    -1,     6,     7,     8,     9,    -1,    -1,    -1,    13,    -1,
    15,    16,    17,    18,    19,    20,    21,    22,    -1,    24,
    25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
    -1,    36,    -1,    -1,    39,    40,     1,    42,     3,    -1,
    -1,     6,     7,     8,     9,    -1,    -1,    -1,    13,    -1,
    15,    16,    17,    18,    19,    20,    21,    22,    -1,    24,
    25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
    -1,    36,    -1,    -1,    39,    40,     6,    42,     8,     9,
    -1,    -1,    -1,    13,    -1,    -1,    -1,    17,    -1,    -1,
    -1,    -1,    22,    -1,    24,    25,    26,    -1,    28,    -1,
    -1,    31,    -1,    -1,    -1,    -1,    36,    -1,     3,    -1,
    40,     6,     7,     8,     9,    -1,    46,    -1,    13,    -1,
    15,    16,    17,    18,    19,    20,    21,    22,    -1,    24,
    25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
    -1,    36,     4,     5,    -1,    40,    -1,    42,    10,    -1,
    12,     4,     5,    -1,     4,     5,    -1,    10,    -1,    12,
    10,    -1,    12,     4,     5,    -1,    -1,    -1,    -1,    10,
    -1,    12,    -1,    35,    36,    37,    -1,    -1,    -1,    41,
    -1,    -1,    35,    36,    37,    35,    36,    37,    41,    -1,
    -1,    41,    -1,     4,    35,    36,    37,     4,     5,    10,
    41,    12,    -1,    10,    -1,    12,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    35,    36,
    37,    -1,    -1,    -1,    41,     6,     7,     8,     9,    -1,
    -1,    -1,    13,    -1,    15,    16,    17,    18,    19,    20,
    21,    22,    -1,    24,    25,    26,    -1,    28,    29,    30,
    31,    32,    33,    34,    -1,    36,    -1,    -1,    -1,    40,
    -1,    42,     6,     7,     8,     9,    -1,    -1,    -1,    13,
    -1,    -1,    -1,    17,    -1,    -1,    -1,    -1,    22,    -1,
    24,    25,    26,     6,    28,     8,     9,    31,    -1,    -1,
    13,    -1,    36,    -1,    17,    -1,    40,    -1,    -1,    22,
    -1,    24,    25,    26,     6,    28,     8,     9,    31,    -1,
    -1,    13,    -1,    36,    -1,    17,    -1,    40,    -1,    -1,
    22,    -1,    24,    25,    26,     6,    28,     8,     9,    31,
    -1,    -1,    13,    -1,    36,    -1,    17,    -1,    40,    -1,
    -1,    22,    -1,    24,    25,    26,    -1,    28,    -1,    -1,
    31,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    40
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

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


#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__)
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

#line 169 "/usr/lib/bison.simple"
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
#line 106 "bc.y"
{
			      yyval.i_value = 0;
			      if (interactive)
				{
				  printf ("%s\n", BC_VERSION);
				  welcome ();
				}
			    ;
    break;}
case 3:
#line 117 "bc.y"
{ run_code (); ;
    break;}
case 4:
#line 119 "bc.y"
{ run_code (); ;
    break;}
case 5:
#line 121 "bc.y"
{
			      yyerrok;
			      init_gen ();
			    ;
    break;}
case 6:
#line 127 "bc.y"
{ yyval.i_value = 0; ;
    break;}
case 10:
#line 133 "bc.y"
{ yyval.i_value = 0; ;
    break;}
case 17:
#line 142 "bc.y"
{ yyval.i_value = yyvsp[0].i_value; ;
    break;}
case 18:
#line 145 "bc.y"
{ warranty (""); ;
    break;}
case 19:
#line 147 "bc.y"
{ limits (); ;
    break;}
case 20:
#line 149 "bc.y"
{
			      if (yyvsp[0].i_value & 2)
				warn ("comparison in expression");
			      if (yyvsp[0].i_value & 1)
				generate ("W");
			      else 
				generate ("p");
			    ;
    break;}
case 21:
#line 158 "bc.y"
{
			      yyval.i_value = 0;
			      generate ("w");
			      generate (yyvsp[0].s_value);
			      free (yyvsp[0].s_value);
			    ;
    break;}
case 22:
#line 165 "bc.y"
{
			      if (break_label == 0)
				yyerror ("Break outside a for/while");
			      else
				{
				  sprintf (genstr, "J%1d:", break_label);
				  generate (genstr);
				}
			    ;
    break;}
case 23:
#line 175 "bc.y"
{
			      warn ("Continue statement");
			      if (continue_label == 0)
				yyerror ("Continue outside a for");
			      else
				{
				  sprintf (genstr, "J%1d:", continue_label);
				  generate (genstr);
				}
			    ;
    break;}
case 24:
#line 186 "bc.y"
{ exit (0); ;
    break;}
case 25:
#line 188 "bc.y"
{ generate ("h"); ;
    break;}
case 26:
#line 190 "bc.y"
{ generate ("0R"); ;
    break;}
case 27:
#line 192 "bc.y"
{ generate ("R"); ;
    break;}
case 28:
#line 194 "bc.y"
{
			      yyvsp[0].i_value = break_label; 
			      break_label = next_label++;
			    ;
    break;}
case 29:
#line 199 "bc.y"
{
			      if (yyvsp[-1].i_value > 1)
				warn ("Comparison in first for expression");
			      yyvsp[-1].i_value = next_label++;
			      if (yyvsp[-1].i_value < 0)
				sprintf (genstr, "N%1d:", yyvsp[-1].i_value);
			      else
				sprintf (genstr, "pN%1d:", yyvsp[-1].i_value);
			      generate (genstr);
			    ;
    break;}
case 30:
#line 210 "bc.y"
{
			      if (yyvsp[-1].i_value < 0) generate ("1");
			      yyvsp[-1].i_value = next_label++;
			      sprintf (genstr, "B%1d:J%1d:", yyvsp[-1].i_value, break_label);
			      generate (genstr);
			      yyval.i_value = continue_label;
			      continue_label = next_label++;
			      sprintf (genstr, "N%1d:", continue_label);
			      generate (genstr);
			    ;
    break;}
case 31:
#line 221 "bc.y"
{
			      if (yyvsp[-1].i_value > 1)
				warn ("Comparison in third for expression");
			      if (yyvsp[-1].i_value < 0)
				sprintf (genstr, "J%1d:N%1d:", yyvsp[-7].i_value, yyvsp[-4].i_value);
			      else
				sprintf (genstr, "pJ%1d:N%1d:", yyvsp[-7].i_value, yyvsp[-4].i_value);
			      generate (genstr);
			    ;
    break;}
case 32:
#line 231 "bc.y"
{
			      sprintf (genstr, "J%1d:N%1d:",
				       continue_label, break_label);
			      generate (genstr);
			      break_label = yyvsp[-12].i_value;
			      continue_label = yyvsp[-4].i_value;
			    ;
    break;}
case 33:
#line 239 "bc.y"
{
			      yyvsp[-1].i_value = if_label;
			      if_label = next_label++;
			      sprintf (genstr, "Z%1d:", if_label);
			      generate (genstr);
			    ;
    break;}
case 34:
#line 246 "bc.y"
{
			      sprintf (genstr, "N%1d:", if_label); 
			      generate (genstr);
			      if_label = yyvsp[-4].i_value;
			    ;
    break;}
case 35:
#line 252 "bc.y"
{
			      yyvsp[0].i_value = next_label++;
			      sprintf (genstr, "N%1d:", yyvsp[0].i_value);
			      generate (genstr);
			    ;
    break;}
case 36:
#line 258 "bc.y"
{
			      yyvsp[0].i_value = break_label; 
			      break_label = next_label++;
			      sprintf (genstr, "Z%1d:", break_label);
			      generate (genstr);
			    ;
    break;}
case 37:
#line 265 "bc.y"
{
			      sprintf (genstr, "J%1d:N%1d:", yyvsp[-6].i_value, break_label);
			      generate (genstr);
			      break_label = yyvsp[-3].i_value;
			    ;
    break;}
case 38:
#line 271 "bc.y"
{ yyval.i_value = 0; ;
    break;}
case 39:
#line 273 "bc.y"
{  warn ("print statement"); ;
    break;}
case 43:
#line 280 "bc.y"
{
			      generate ("O");
			      generate (yyvsp[0].s_value);
			      free (yyvsp[0].s_value);
			    ;
    break;}
case 44:
#line 286 "bc.y"
{ generate ("P"); ;
    break;}
case 46:
#line 290 "bc.y"
{
			      warn ("else clause in if statement");
			      yyvsp[0].i_value = next_label++;
			      sprintf (genstr, "J%d:N%1d:", yyvsp[0].i_value, if_label); 
			      generate (genstr);
			      if_label = yyvsp[0].i_value;
			    ;
    break;}
case 48:
#line 300 "bc.y"
{
			      /* Check auto list against parameter list? */
			      check_params (yyvsp[-4].a_value,yyvsp[0].a_value);
			      sprintf (genstr, "F%d,%s.%s[", lookup(yyvsp[-6].s_value,FUNCT), 
				       arg_str (yyvsp[-4].a_value,TRUE), arg_str (yyvsp[0].a_value,TRUE));
			      generate (genstr);
			      free_args (yyvsp[-4].a_value);
			      free_args (yyvsp[0].a_value);
			      yyvsp[-7].i_value = next_label;
			      next_label = 0;
			    ;
    break;}
case 49:
#line 312 "bc.y"
{
			      generate ("0R]");
			      next_label = yyvsp[-11].i_value;
			    ;
    break;}
case 50:
#line 318 "bc.y"
{ yyval.a_value = NULL; ;
    break;}
case 52:
#line 322 "bc.y"
{ yyval.a_value = NULL; ;
    break;}
case 53:
#line 324 "bc.y"
{ yyval.a_value = yyvsp[-1].a_value; ;
    break;}
case 54:
#line 326 "bc.y"
{ yyval.a_value = yyvsp[-1].a_value; ;
    break;}
case 55:
#line 329 "bc.y"
{ yyval.a_value = nextarg (NULL, lookup (yyvsp[0].s_value,SIMPLE)); ;
    break;}
case 56:
#line 331 "bc.y"
{ yyval.a_value = nextarg (NULL, lookup (yyvsp[-2].s_value,ARRAY)); ;
    break;}
case 57:
#line 333 "bc.y"
{ yyval.a_value = nextarg (yyvsp[-2].a_value, lookup (yyvsp[0].s_value,SIMPLE)); ;
    break;}
case 58:
#line 335 "bc.y"
{ yyval.a_value = nextarg (yyvsp[-4].a_value, lookup (yyvsp[-2].s_value,ARRAY)); ;
    break;}
case 59:
#line 338 "bc.y"
{ yyval.a_value = NULL; ;
    break;}
case 61:
#line 342 "bc.y"
{
			      if (yyvsp[0].i_value > 1) warn ("comparison in argument");
			      yyval.a_value = nextarg (NULL,0);
			    ;
    break;}
case 62:
#line 347 "bc.y"
{
			      sprintf (genstr, "K%d:", -lookup (yyvsp[-2].s_value,ARRAY));
			      generate (genstr);
			      yyval.a_value = nextarg (NULL,1);
			    ;
    break;}
case 63:
#line 353 "bc.y"
{
			      if (yyvsp[0].i_value > 1) warn ("comparison in argument");
			      yyval.a_value = nextarg (yyvsp[-2].a_value,0);
			    ;
    break;}
case 64:
#line 358 "bc.y"
{
			      sprintf (genstr, "K%d:", -lookup (yyvsp[-2].s_value,ARRAY));
			      generate (genstr);
			      yyval.a_value = nextarg (yyvsp[-4].a_value,1);
			    ;
    break;}
case 65:
#line 365 "bc.y"
{
			      yyval.i_value = -1;
			      warn ("Missing expression in for statement");
			    ;
    break;}
case 67:
#line 372 "bc.y"
{
			      yyval.i_value = 0;
			      generate ("0");
			    ;
    break;}
case 68:
#line 377 "bc.y"
{
			      if (yyvsp[0].i_value > 1)
				warn ("comparison in return expresion");
			    ;
    break;}
case 69:
#line 383 "bc.y"
{
			      if (yyvsp[0].c_value != '=')
				{
				  if (yyvsp[-1].i_value < 0)
				    sprintf (genstr, "DL%d:", -yyvsp[-1].i_value);
				  else
				    sprintf (genstr, "l%d:", yyvsp[-1].i_value);
				  generate (genstr);
				}
			    ;
    break;}
case 70:
#line 394 "bc.y"
{
			      if (yyvsp[0].i_value > 1) warn("comparison in assignment");
			      if (yyvsp[-2].c_value != '=')
				{
				  sprintf (genstr, "%c", yyvsp[-2].c_value);
				  generate (genstr);
				}
			      if (yyvsp[-3].i_value < 0)
				sprintf (genstr, "S%d:", -yyvsp[-3].i_value);
			      else
				sprintf (genstr, "s%d:", yyvsp[-3].i_value);
			      generate (genstr);
			      yyval.i_value = 0;
			    ;
    break;}
case 71:
#line 410 "bc.y"
{
			      warn("&& operator");
			      yyvsp[0].i_value = next_label++;
			      sprintf (genstr, "DZ%d:p", yyvsp[0].i_value);
			      generate (genstr);
			    ;
    break;}
case 72:
#line 417 "bc.y"
{
			      sprintf (genstr, "DZ%d:p1N%d:", yyvsp[-2].i_value, yyvsp[-2].i_value);
			      generate (genstr);
			      yyval.i_value = yyvsp[-3].i_value | yyvsp[0].i_value;
			    ;
    break;}
case 73:
#line 423 "bc.y"
{
			      warn("|| operator");
			      yyvsp[0].i_value = next_label++;
			      sprintf (genstr, "B%d:", yyvsp[0].i_value);
			      generate (genstr);
			    ;
    break;}
case 74:
#line 430 "bc.y"
{
			      int tmplab;
			      tmplab = next_label++;
			      sprintf (genstr, "B%d:0J%d:N%d:1N%d:",
				       yyvsp[-2].i_value, tmplab, yyvsp[-2].i_value, tmplab);
			      generate (genstr);
			      yyval.i_value = yyvsp[-3].i_value | yyvsp[0].i_value;
			    ;
    break;}
case 75:
#line 439 "bc.y"
{
			      yyval.i_value = yyvsp[0].i_value;
			      warn("! operator");
			      generate ("!");
			    ;
    break;}
case 76:
#line 445 "bc.y"
{
			      yyval.i_value = 3;
			      switch (*(yyvsp[-1].s_value))
				{
				case '=':
				  generate ("=");
				  break;

				case '!':
				  generate ("#");
				  break;

				case '<':
				  if (yyvsp[-1].s_value[1] == '=')
				    generate ("{");
				  else
				    generate ("<");
				  break;

				case '>':
				  if (yyvsp[-1].s_value[1] == '=')
				    generate ("}");
				  else
				    generate (">");
				  break;
				}
			    ;
    break;}
case 77:
#line 473 "bc.y"
{
			      generate ("+");
			      yyval.i_value = yyvsp[-2].i_value | yyvsp[0].i_value;
			    ;
    break;}
case 78:
#line 478 "bc.y"
{
			      generate ("-");
			      yyval.i_value = yyvsp[-2].i_value | yyvsp[0].i_value;
			    ;
    break;}
case 79:
#line 483 "bc.y"
{
			      genstr[0] = yyvsp[-1].c_value;
			      genstr[1] = 0;
			      generate (genstr);
			      yyval.i_value = yyvsp[-2].i_value | yyvsp[0].i_value;
			    ;
    break;}
case 80:
#line 490 "bc.y"
{
			      generate ("^");
			      yyval.i_value = yyvsp[-2].i_value | yyvsp[0].i_value;
			    ;
    break;}
case 81:
#line 495 "bc.y"
{
			      generate ("n");
			      yyval.i_value = yyvsp[0].i_value;
			    ;
    break;}
case 82:
#line 500 "bc.y"
{
			      yyval.i_value = 1;
			      if (yyvsp[0].i_value < 0)
				sprintf (genstr, "L%d:", -yyvsp[0].i_value);
			      else
				sprintf (genstr, "l%d:", yyvsp[0].i_value);
			      generate (genstr);
			    ;
    break;}
case 83:
#line 509 "bc.y"
{
			      int len = strlen(yyvsp[0].s_value);
			      yyval.i_value = 1;
			      if (len == 1 && *yyvsp[0].s_value == '0')
				generate ("0");
			      else if (len == 1 && *yyvsp[0].s_value == '1')
				generate ("1");
			      else
				{
				  generate ("K");
				  generate (yyvsp[0].s_value);
				  generate (":");
				}
			      free (yyvsp[0].s_value);
			    ;
    break;}
case 84:
#line 525 "bc.y"
{ yyval.i_value = yyvsp[-1].i_value | 1; ;
    break;}
case 85:
#line 527 "bc.y"
{
			      yyval.i_value = 1;
			      if (yyvsp[-1].a_value != NULL)
				{ 
				  sprintf (genstr, "C%d,%s:",
					   lookup (yyvsp[-3].s_value,FUNCT),
					   arg_str (yyvsp[-1].a_value,FALSE));
				  free_args (yyvsp[-1].a_value);
				}
			      else
				{
				  sprintf (genstr, "C%d:", lookup (yyvsp[-3].s_value,FUNCT));
				}
			      generate (genstr);
			    ;
    break;}
case 86:
#line 543 "bc.y"
{
			      yyval.i_value = 1;
			      if (yyvsp[0].i_value < 0)
				{
				  if (yyvsp[-1].c_value == '+')
				    sprintf (genstr, "DA%d:L%d:", -yyvsp[0].i_value, -yyvsp[0].i_value);
				  else
				    sprintf (genstr, "DM%d:L%d:", -yyvsp[0].i_value, -yyvsp[0].i_value);
				}
			      else
				{
				  if (yyvsp[-1].c_value == '+')
				    sprintf (genstr, "i%d:l%d:", yyvsp[0].i_value, yyvsp[0].i_value);
				  else
				    sprintf (genstr, "d%d:l%d:", yyvsp[0].i_value, yyvsp[0].i_value);
				}
			      generate (genstr);
			    ;
    break;}
case 87:
#line 562 "bc.y"
{
			      yyval.i_value = 1;
			      if (yyvsp[-1].i_value < 0)
				{
				  sprintf (genstr, "DL%d:x", -yyvsp[-1].i_value);
				  generate (genstr); 
				  if (yyvsp[0].c_value == '+')
				    sprintf (genstr, "A%d:", -yyvsp[-1].i_value);
				  else
				      sprintf (genstr, "M%d:", -yyvsp[-1].i_value);
				}
			      else
				{
				  sprintf (genstr, "l%d:", yyvsp[-1].i_value);
				  generate (genstr);
				  if (yyvsp[0].c_value == '+')
				    sprintf (genstr, "i%d:", yyvsp[-1].i_value);
				  else
				    sprintf (genstr, "d%d:", yyvsp[-1].i_value);
				}
			      generate (genstr);
			    ;
    break;}
case 88:
#line 585 "bc.y"
{ generate ("cL"); yyval.i_value = 1;;
    break;}
case 89:
#line 587 "bc.y"
{ generate ("cR"); yyval.i_value = 1;;
    break;}
case 90:
#line 589 "bc.y"
{ generate ("cS"); yyval.i_value = 1;;
    break;}
case 91:
#line 591 "bc.y"
{
			      warn ("read function");
			      generate ("cI"); yyval.i_value = 1;
			    ;
    break;}
case 92:
#line 597 "bc.y"
{ yyval.i_value = lookup(yyvsp[0].s_value,SIMPLE); ;
    break;}
case 93:
#line 599 "bc.y"
{
			      if (yyvsp[-1].i_value > 1) warn("comparison in subscript");
			      yyval.i_value = lookup(yyvsp[-3].s_value,ARRAY);
			    ;
    break;}
case 94:
#line 604 "bc.y"
{ yyval.i_value = 0; ;
    break;}
case 95:
#line 606 "bc.y"
{ yyval.i_value = 1; ;
    break;}
case 96:
#line 608 "bc.y"
{ yyval.i_value = 2; ;
    break;}
case 97:
#line 610 "bc.y"
{ yyval.i_value = 3; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 440 "/usr/lib/bison.simple"

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
#line 612 "bc.y"

