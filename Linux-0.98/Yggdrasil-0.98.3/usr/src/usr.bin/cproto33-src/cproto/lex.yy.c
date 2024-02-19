# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
/* $Id: lex.l 3.6 92/04/11 19:27:25 cthuang Exp $
 *
 * Lexical analyzer for C function prototype generator
 */

static int curly;	/* number of curly brace nesting levels */
static int ly_count;	/* number of occurances of %% */

/* information about the current input file */
typedef struct {
    char *base_name;		/* base input file name */
    char *file_name;		/* current file name */
    FILE *file; 		/* input file */
    unsigned line_num;		/* current line number in input file */
    FILE *tmp_file;		/* temporary file */
    long begin_comment; 	/* tmp file offset after last written ) or ; */
    long end_comment;		/* tmp file offset after last comment */
    boolean convert;		/* if TRUE, convert function definitions */
    boolean changed;		/* TRUE if conversion done in this file */
#ifdef FLEX_SCANNER
    YY_BUFFER_STATE buffer;	/* flex scanner state */
#endif
} IncludeStack;

static int inc_depth;				/* include nesting level */
static IncludeStack inc_stack[MAX_INC_DEPTH];	/* stack of included files */
static IncludeStack *cur_file;			/* current file */
static SymbolTable *included_files;		/* files already included */

static void save_text();
static void save_text_offset();
static void get_comment();
static void get_cpp_directive();
static void do_include();

#ifdef yywrap
#undef yywrap
#endif
static int yywrap();
# define CPP1 2
# define INIT1 4
# define INIT2 6
# define CURLY 8
# define LEXYACC 10
# define ASM 12
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
		{ save_text(); cur_file->line_num++;
			    cur_declarator = NULL; }
break;
case 2:
		{ save_text(); get_comment(); }
break;
case 3:
		save_text();
break;
case 4:
	{ save_text(); if (++ly_count >= 2) BEGIN INITIAL; }
break;
case 5:
	{ save_text(); BEGIN INITIAL; }
break;
case 6:
save_text();
break;
case 7:
	{ save_text(); BEGIN LEXYACC; }
break;
case 8:
	{ save_text(); BEGIN CPP1; }
break;
case 9:
{
			    save_text();
			    sscanf(yytext, "define %s", buf);
			    get_cpp_directive();
			    new_symbol(typedef_names, buf, DS_EXTERN);
			}
break;
case 10:
    {
			    save_text();
			    sscanf(yytext, "include \"%[^\"]\"", buf);
			    get_cpp_directive();
			    do_include(buf, FALSE);
			}
break;
case 11:
{
			    save_text();
			    sscanf(yytext, "include <%[^>]>", buf);
			    get_cpp_directive();
			    do_include(buf, TRUE);
			}
break;
case 12:
 {
			    save_text();
			    sscanf(yytext, "%d \"%[^\"]\"", &cur_file->line_num,
				cur_file->file_name);
			    cur_file->line_num--;
			    BEGIN INITIAL;
			}
break;
case 13:
	{
			    save_text();
			    sscanf(yytext, "%d ", &cur_file->line_num);
			    cur_file->line_num--;
			    BEGIN INITIAL;
			}
break;
case 14:
		{ save_text(); get_cpp_directive(); }
break;
case 15:
	{ save_text_offset(); return '('; }
break;
case 16:
	{
			    save_text();
			    if (cur_file->convert)
				cur_file->begin_comment =
				 ftell(cur_file->tmp_file);
			    return ')';
			}
break;
case 17:
	{ save_text_offset(); return '*'; }
break;
case 18:
	{ save_text(); return ','; }
break;
case 19:
	{
			    save_text();
			    if (cur_file->convert)
				cur_file->begin_comment =
				 ftell(cur_file->tmp_file);
			    return ';';
			}
break;
case 20:
	{ save_text(); return T_ELLIPSIS; }
break;
case 21:
{ save_text(); return T_STRING_LITERAL; }
break;
case 22:
	{ save_text(); BEGIN ASM; return T_ASM; }
break;
case 23:
	save_text();
break;
case 24:
	{ save_text(); BEGIN INITIAL; return T_ASMARG; }
break;
case 25:
	save_text();
break;
case 26:
	{ save_text_offset(); return T_AUTO; }
break;
case 27:
	{ save_text_offset(); return T_EXTERN; }
break;
case 28:
{ save_text_offset(); return T_REGISTER; }
break;
case 29:
	{ save_text_offset(); return T_STATIC; }
break;
case 30:
{ save_text_offset(); return T_TYPEDEF; }
break;
case 31:
	{ save_text_offset(); return T_INLINE; }
break;
case 32:
	{ save_text_offset(); return T_CHAR; }
break;
case 33:
	{ save_text_offset(); return T_DOUBLE; }
break;
case 34:
	{ save_text_offset(); return T_FLOAT; }
break;
case 35:
	{ save_text_offset(); return T_INT; }
break;
case 36:
	{ save_text_offset(); return T_VOID; }
break;
case 37:
	{ save_text_offset(); return T_LONG; }
break;
case 38:
	{ save_text_offset(); return T_SHORT; }
break;
case 39:
	{ save_text_offset(); return T_SIGNED; }
break;
case 40:
{ save_text_offset(); return T_UNSIGNED; }
break;
case 41:
	{ save_text_offset(); return T_ENUM; }
break;
case 42:
	{ save_text_offset(); return T_STRUCT; }
break;
case 43:
	{ save_text_offset(); return T_UNION; }
break;
case 44:
	{
			    save_text_offset();
			    if (find_symbol(type_qualifiers, yytext) != NULL)
				return T_TYPE_QUALIFIER;
			    else if (find_symbol(typedef_names, yytext) != NULL)
				return T_TYPEDEF_NAME;
			    else
				return T_IDENTIFIER;
			}
break;
case 45:
{
			    /* This can't handle the case where a comment
			     * containing a ] appears between the brackets.
			     */
			    int i;

			    save_text_offset();
			    for (i = 0; i < yyleng; ++i) {
				if (yytext[i] == '\n')
				    cur_file->line_num++;
			    }
			    return T_BRACKETS;
			}
break;
case 46:
	{ save_text(); BEGIN INIT1; return '='; }
break;
case 47:
	{ save_text(); curly = 1; BEGIN INIT2; }
break;
case 48:
	{
			    unput(yytext[yyleng-1]);
			    BEGIN INITIAL;
			    return T_INITIALIZER;
			}
break;
case 49:
save_text();
break;
case 50:
	{ save_text(); ++curly; }
break;
case 51:
	{
			    save_text();
			    if (--curly == 0) {
				BEGIN INITIAL;
				return T_INITIALIZER;
			    }
			}
break;
case 52:
save_text();
break;
case 53:
	{
			    save_text();
			    curly = 1;
			    BEGIN CURLY;
			    return T_LBRACE;
			}
break;
case 54:
	{ save_text(); ++curly; }
break;
case 55:
	{
			    save_text();
			    if (--curly == 0) {
				BEGIN INITIAL;
				return T_MATCHRBRACE;
			    }
			}
break;
case 56:
save_text();
break;
case 57:
	save_text();
break;
case 58:
		{
			    save_text();
			    put_error();
			    fprintf(stderr, "bad character '%c'\n", yytext[0]);
			}
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

/* Save the matched text in the temporary file.
 */
static void
save_text ()
{
    if (cur_file->convert) {
	fputs(yytext, cur_file->tmp_file);
    }
}

/* Record the current position in the temporary file and write the matched text
 * to the file.
 */
static void
save_text_offset ()
{
    strcpy(yylval.text.text, yytext);
    if (cur_file->convert) {
	yylval.text.begin = ftell(cur_file->tmp_file);
	fputs(yytext, cur_file->tmp_file);
    }
}

/* Scan to end of comment.
 */
static void
get_comment ()
{
    int c, lastc = '\0';

    while ((c = input()) != 0) {
	if (cur_file->convert)
	    fputc(c, cur_file->tmp_file);
	switch (c) {
	case '\n':
	    cur_file->line_num++;
	    break;
	case '/':
	    if (lastc == '*') {
		if (cur_file->convert) {
		    if (func_params && cur_declarator) {
			cur_declarator->begin_comment = cur_file->begin_comment;
			cur_file->begin_comment = ftell(cur_file->tmp_file);
			cur_declarator->end_comment = cur_file->begin_comment;
			cur_declarator = NULL;
		    } else {
			cur_file->end_comment = ftell(cur_file->tmp_file);
		    }
		}
		return;
	    }
	    break;
	}
	lastc = c;
    }
}

/* Scan rest of preprocessor statement.
 */
static void
get_cpp_directive ()
{
    int c, lastc = '\0';

    while ((c = input()) != 0) {
	if (cur_file->convert)
	    fputc(c, cur_file->tmp_file);
	switch (c) {
	case '\n':
	    cur_file->line_num++;
	    if (lastc != '\\') {
		BEGIN INITIAL;
		return;
	    }
	    break;
	case '*':
	    if (lastc == '/')
		get_comment();
	    break;
	}
	lastc = c;
    }
}

/* Return a pointer to the current file name.
 */
char *
cur_file_name ()
{
    return cur_file->file_name;
}

/* Return the current line number.
 */
unsigned
cur_line_num ()
{
    return cur_file->line_num;
}

/* Return the current temporary output file.
 */
FILE *
cur_tmp_file ()
{
    return cur_file->tmp_file;
}

/* Set the modify flag for the current file.
 */
void
cur_file_changed ()
{
    cur_file->changed = TRUE;
}

/* Return the temporary file offset of beginning of the current comment.
 */
long
cur_begin_comment ()
{
    return cur_file->begin_comment;
}

/* Push a file onto the include stack.	The stream yyin must already
 * point to the file.
 */
static void
include_file (name, convert)
char *name;		/* file name */
boolean convert;	/* if TRUE, convert function definitions */
{
    ++inc_depth;
    cur_file = inc_stack + inc_depth;
    cur_file->file = yyin;
    cur_file->base_name = xstrdup(name);
    cur_file->file_name = strcpy(xmalloc(MAX_TEXT_SIZE), name);
    cur_file->line_num = 1;
    cur_file->convert = convert;
    cur_file->changed = FALSE;

#ifdef FLEX_SCANNER
    cur_file->buffer = yy_create_buffer(yyin, YY_BUF_SIZE);
    yy_switch_to_buffer(cur_file->buffer);
#endif

    if (convert) {
	cur_file->begin_comment = cur_file->end_comment = 0;
	cur_file->tmp_file = tmpfile();
	if (cur_file->tmp_file == NULL) {
	    fprintf(stderr, "%s: cannot create temporary file %s\n", progname);
	    cur_file->convert = FALSE;
	}
    }
}

#define BLOCK_SIZE 2048

/* Copy converted C source from the temporary file to the output stream.
 */
static void
put_file (outf)
FILE *outf;
{
    char block[BLOCK_SIZE];
    long filesize;
    unsigned nread, count;

    filesize = ftell(cur_file->tmp_file);
    fseek(cur_file->tmp_file, 0L, 0);
    while (filesize > 0) {
	count = (filesize < BLOCK_SIZE) ? (unsigned)filesize : BLOCK_SIZE;
	nread = fread(block, sizeof(char), count, cur_file->tmp_file);
	if (nread == 0)
	    break;
	fwrite(block, sizeof(char), nread, outf);
	filesize -= nread;
    }
}

/* Remove the top of the include stack.
 */
void
pop_file ()
{
    FILE *outf;

    if (yyin != stdin)
	fclose(yyin);

    if (cur_file->convert) {
	if (cur_file->changed) {
	    if (yyin == stdin) {
		put_file(stdout);
	    } else if ((outf = fopen(cur_file->base_name, "w")) != NULL) {
		put_file(outf);
		fclose(outf);
	    } else {
		fprintf(stderr, "%s: cannot create file %s\n", progname,
		    cur_file->base_name);
	    }
	}

	fclose(cur_file->tmp_file);
    }
    free(cur_file->base_name);
    free(cur_file->file_name);

#ifdef FLEX_SCANNER
    yy_delete_buffer(YY_CURRENT_BUFFER);
#endif

    if (--inc_depth >= 0) {
	cur_file = inc_stack + inc_depth;
	yyin = cur_file->file;

#ifdef FLEX_SCANNER
	yy_switch_to_buffer(cur_file->buffer);
#endif
    }
}

/* Process include directive.
 */
static void
do_include (filename, stdinc)
char *filename; 	/* file to include */
boolean stdinc; 	/* TRUE if file name specified with angle brackets */
{
    char path[MAX_TEXT_SIZE];
    int i;
    FILE *fp;

    if (inc_depth >= MAX_INC_DEPTH-1) {
	put_error();
	fprintf(stderr, "includes too deeply nested\n");
	return;
    }

    sprintf(path, stdinc ? "<%s>" : "\"%s\"", filename);
    if (find_symbol(included_files, path) != NULL)
	return;
    new_symbol(included_files, path, 0);

    for (i = stdinc != 0; i < num_inc_dir; ++i) {
	if (strlen(inc_dir[i]) == 0) {
	    strcpy(path, filename);
	} else {
	    sprintf(path, "%s/%s", inc_dir[i], filename);
	}
	if ((fp = fopen(path, "r")) != NULL) {
	    yyin = fp;
	    include_file(path, func_style != FUNC_NONE && !stdinc);
	    return;
	}
    }

    if (!quiet) {
	put_error();
	fprintf(stderr, "cannot read file %s\n", filename);
    }
}

/* When the end of the current input file is reached, pop a
 * nested include file.
 */
static int
yywrap ()
{
    if (inc_depth > 0) {
	pop_file();
	return 0;
    } else {
	return 1;
    }
}
int yyvstop[] = {
0,

58,
0,

57,
58,
0,

1,
0,

58,
0,

8,
58,
0,

15,
58,
0,

16,
58,
0,

17,
58,
0,

18,
58,
0,

58,
0,

58,
0,

19,
58,
0,

46,
58,
0,

44,
58,
0,

58,
0,

44,
58,
0,

44,
58,
0,

44,
58,
0,

44,
58,
0,

44,
58,
0,

44,
58,
0,

44,
58,
0,

44,
58,
0,

44,
58,
0,

44,
58,
0,

44,
58,
0,

44,
58,
0,

53,
58,
0,

58,
0,

14,
58,
0,

14,
57,
58,
0,

14,
58,
0,

14,
58,
-13,
0,

14,
58,
0,

14,
58,
0,

49,
58,
0,

49,
57,
58,
0,

49,
58,
0,

49,
58,
0,

48,
49,
58,
0,

49,
58,
0,

49,
58,
0,

47,
49,
58,
0,

52,
58,
0,

52,
57,
58,
0,

52,
58,
0,

52,
58,
0,

52,
58,
0,

52,
58,
0,

50,
52,
58,
0,

51,
52,
58,
0,

56,
58,
0,

56,
57,
58,
0,

56,
58,
0,

56,
58,
0,

56,
58,
0,

56,
58,
0,

54,
56,
58,
0,

55,
56,
58,
0,

6,
58,
0,

6,
57,
58,
0,

6,
58,
0,

6,
58,
0,

6,
58,
0,

6,
58,
0,

6,
58,
0,

25,
58,
0,

25,
57,
58,
0,

25,
58,
0,

25,
58,
0,

23,
25,
58,
0,

24,
25,
58,
0,

25,
58,
0,

25,
58,
0,

57,
0,

21,
0,

8,
0,

2,
0,

-3,
0,

44,
0,

45,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

7,
0,

-13,
0,

-13,
0,

13,
0,

-13,
0,

49,
0,

52,
0,

56,
0,

6,
0,

4,
0,

5,
0,

25,
0,

21,
0,

20,
0,

3,
0,

22,
44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

35,
44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

-12,
-13,
0,

49,
0,

49,
0,

52,
0,

52,
0,

56,
0,

56,
0,

6,
0,

6,
0,

25,
0,

25,
0,

26,
44,
0,

32,
44,
0,

44,
0,

41,
44,
0,

44,
0,

44,
0,

44,
0,

37,
44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

36,
44,
0,

12,
13,
0,

44,
0,

44,
0,

34,
44,
0,

44,
0,

44,
0,

38,
44,
0,

44,
0,

44,
0,

44,
0,

44,
0,

43,
44,
0,

44,
0,

33,
44,
0,

27,
44,
0,

31,
44,
0,

44,
0,

39,
44,
0,

29,
44,
0,

42,
44,
0,

44,
0,

44,
0,

44,
0,

30,
44,
0,

44,
0,

28,
44,
0,

40,
44,
0,

9,
0,

10,
0,

11,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,15,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,16,	1,17,	
0,0,	1,16,	16,89,	19,93,	
0,0,	16,89,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	1,18,	
1,19,	16,89,	19,93,	0,0,	
1,15,	1,20,	1,21,	1,22,	
12,80,	1,23,	0,0,	1,24,	
1,25,	1,15,	3,44,	24,94,	
4,46,	0,0,	12,78,	0,0,	
0,0,	0,0,	3,45,	3,17,	
1,26,	3,45,	1,27,	1,15,	
25,95,	2,19,	1,28,	2,43,	
0,0,	25,96,	2,20,	2,21,	
2,22,	0,0,	0,0,	6,55,	
2,24,	2,25,	0,0,	0,0,	
0,0,	8,62,	0,0,	3,44,	
80,143,	0,0,	0,0,	90,0,	
3,44,	2,26,	0,0,	2,27,	
1,29,	3,44,	1,15,	94,151,	
3,46,	3,47,	1,30,	12,79,	
1,31,	1,32,	1,33,	1,34,	
0,0,	4,48,	1,35,	31,102,	
0,0,	1,36,	4,49,	3,44,	
32,103,	33,104,	3,44,	1,37,	
1,38,	1,39,	1,40,	1,41,	
6,56,	2,29,	34,106,	33,105,	
1,42,	5,50,	8,63,	2,30,	
35,107,	2,31,	2,32,	2,33,	
2,34,	5,51,	5,17,	2,35,	
5,51,	30,100,	2,36,	30,101,	
36,108,	37,109,	3,44,	39,113,	
2,37,	2,38,	2,39,	2,40,	
2,41,	3,48,	40,114,	6,57,	
41,115,	2,42,	3,49,	14,85,	
14,86,	8,64,	5,52,	8,65,	
43,116,	48,121,	14,87,	5,53,	
49,122,	7,58,	38,110,	38,111,	
5,54,	90,92,	80,144,	5,55,	
5,50,	7,59,	7,17,	100,153,	
7,59,	9,66,	38,112,	101,154,	
92,0,	102,155,	103,156,	104,157,	
105,158,	9,67,	5,50,	106,159,	
9,67,	5,50,	108,162,	107,160,	
109,163,	110,164,	111,165,	113,168,	
112,166,	115,171,	7,60,	107,161,	
117,117,	121,173,	122,174,	7,61,	
92,150,	154,185,	123,0,	14,88,	
7,58,	114,169,	9,68,	7,62,	
7,58,	112,167,	118,172,	9,69,	
5,56,	5,50,	125,0,	114,170,	
9,66,	126,0,	127,0,	9,70,	
9,66,	128,0,	7,58,	131,0,	
118,117,	7,58,	130,0,	132,0,	
10,66,	155,186,	133,0,	135,0,	
156,187,	157,188,	9,66,	117,117,	
10,67,	9,66,	125,175,	10,67,	
136,0,	137,0,	138,0,	5,57,	
141,0,	140,0,	145,0,	127,176,	
147,0,	142,0,	130,177,	158,189,	
7,63,	7,58,	159,190,	135,179,	
132,178,	148,0,	92,92,	149,0,	
150,0,	10,68,	160,191,	162,192,	
9,71,	9,66,	10,69,	163,193,	
11,74,	140,181,	137,180,	10,66,	
147,183,	164,194,	10,70,	10,66,	
11,75,	11,17,	142,182,	11,75,	
165,195,	166,196,	167,197,	7,64,	
123,125,	7,65,	168,198,	169,199,	
149,184,	10,66,	170,200,	171,201,	
10,66,	173,203,	174,204,	9,72,	
125,125,	9,73,	187,205,	126,127,	
127,127,	11,76,	189,206,	128,130,	
190,207,	131,132,	11,77,	191,208,	
130,130,	132,132,	175,0,	11,74,	
133,135,	135,135,	11,78,	11,74,	
193,209,	176,0,	177,0,	10,71,	
10,66,	178,0,	136,137,	137,137,	
138,140,	13,81,	141,142,	140,140,	
145,147,	11,74,	147,147,	142,142,	
11,74,	13,82,	13,17,	194,210,	
13,82,	18,90,	195,211,	148,149,	
196,212,	149,149,	150,92,	197,213,	
198,214,	18,90,	18,0,	199,215,	
18,90,	179,0,	10,72,	180,0,	
10,73,	200,216,	203,217,	204,218,	
205,219,	206,220,	13,83,	11,79,	
11,74,	208,221,	209,222,	13,84,	
13,85,	13,86,	211,223,	212,224,	
13,81,	213,225,	18,91,	13,87,	
13,81,	214,226,	216,227,	18,90,	
217,228,	218,229,	222,230,	181,0,	
18,90,	182,0,	183,0,	226,231,	
18,90,	227,232,	13,81,	228,233,	
229,234,	13,81,	230,235,	232,236,	
175,125,	240,242,	241,243,	0,0,	
0,0,	0,0,	18,90,	176,127,	
177,130,	18,90,	0,0,	178,132,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	228,233,	0,0,	
184,0,	0,0,	0,0,	0,0,	
13,88,	13,81,	28,97,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	28,97,	28,97,	28,97,	
18,92,	18,90,	0,0,	179,135,	
0,0,	180,137,	0,0,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	181,140,	0,0,	182,142,	
183,147,	28,97,	0,0,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	28,97,	28,97,	28,97,	
28,97,	29,98,	184,149,	0,0,	
0,0,	0,0,	0,0,	47,117,	
0,0,	29,98,	29,98,	0,0,	
29,98,	52,123,	0,0,	47,118,	
47,119,	0,0,	47,117,	0,0,	
53,126,	52,123,	52,0,	0,0,	
52,123,	0,0,	0,0,	0,0,	
53,126,	53,0,	0,0,	53,126,	
234,234,	0,0,	29,98,	0,0,	
0,0,	0,0,	0,0,	29,98,	
47,117,	0,0,	0,0,	0,0,	
29,98,	47,117,	52,124,	0,0,	
29,98,	0,0,	47,117,	52,123,	
0,0,	53,126,	47,120,	234,234,	
52,123,	234,238,	53,124,	0,0,	
52,123,	0,0,	29,98,	53,126,	
0,0,	29,98,	0,0,	53,126,	
47,117,	0,0,	0,0,	47,117,	
0,0,	0,0,	52,123,	0,0,	
0,0,	52,123,	0,0,	0,0,	
56,124,	53,126,	0,0,	234,239,	
53,126,	0,0,	60,128,	0,0,	
56,124,	56,0,	0,0,	56,124,	
61,131,	29,99,	60,128,	60,0,	
0,0,	60,128,	0,0,	47,117,	
61,131,	61,0,	0,0,	61,131,	
52,125,	52,123,	0,0,	0,0,	
0,0,	0,0,	0,0,	53,127,	
53,126,	56,124,	0,0,	0,0,	
0,0,	0,0,	56,124,	60,129,	
0,0,	0,0,	0,0,	56,124,	
60,128,	61,131,	0,0,	56,124,	
0,0,	60,128,	61,129,	0,0,	
0,0,	60,128,	0,0,	61,131,	
0,0,	0,0,	0,0,	61,131,	
0,0,	56,124,	0,0,	63,129,	
56,124,	0,0,	0,0,	60,128,	
0,0,	68,133,	60,128,	63,129,	
63,0,	61,131,	63,129,	69,136,	
61,131,	68,133,	68,0,	0,0,	
68,133,	0,0,	71,134,	69,136,	
69,0,	0,0,	69,136,	0,0,	
0,0,	0,0,	71,134,	71,0,	
56,124,	71,134,	0,0,	0,0,	
63,129,	60,130,	60,128,	0,0,	
0,0,	63,129,	68,134,	61,132,	
61,131,	0,0,	63,129,	68,133,	
69,136,	0,0,	63,129,	0,0,	
68,133,	69,134,	0,0,	71,134,	
68,133,	0,0,	69,136,	0,0,	
71,134,	0,0,	69,136,	0,0,	
63,129,	71,134,	76,138,	63,129,	
0,0,	71,134,	68,133,	0,0,	
0,0,	68,133,	76,138,	76,0,	
69,136,	76,138,	0,0,	69,136,	
0,0,	0,0,	77,141,	71,134,	
0,0,	0,0,	71,134,	0,0,	
0,0,	0,0,	77,141,	77,0,	
0,0,	77,141,	0,0,	63,129,	
0,0,	0,0,	0,0,	76,139,	
68,135,	68,133,	0,0,	0,0,	
76,138,	0,0,	69,137,	69,136,	
0,0,	76,138,	0,0,	0,0,	
0,0,	76,138,	71,134,	77,141,	
0,0,	0,0,	0,0,	0,0,	
77,139,	0,0,	0,0,	79,139,	
0,0,	77,141,	0,0,	76,138,	
0,0,	77,141,	76,138,	79,139,	
79,0,	0,0,	79,139,	0,0,	
0,0,	0,0,	0,0,	83,145,	
0,0,	0,0,	0,0,	77,141,	
0,0,	0,0,	77,141,	83,145,	
83,0,	0,0,	83,145,	0,0,	
0,0,	0,0,	0,0,	0,0,	
79,139,	76,140,	76,138,	0,0,	
0,0,	79,139,	0,0,	0,0,	
0,0,	0,0,	79,139,	0,0,	
0,0,	0,0,	79,139,	0,0,	
83,146,	77,142,	77,141,	0,0,	
0,0,	83,145,	0,0,	0,0,	
84,148,	0,0,	83,145,	0,0,	
79,139,	0,0,	83,145,	79,139,	
84,148,	84,0,	0,0,	84,148,	
0,0,	0,0,	0,0,	0,0,	
88,146,	0,0,	0,0,	0,0,	
83,145,	0,0,	0,0,	83,145,	
88,146,	88,0,	0,0,	88,146,	
0,0,	0,0,	0,0,	0,0,	
0,0,	84,148,	0,0,	79,139,	
0,0,	0,0,	84,146,	0,0,	
0,0,	0,0,	0,0,	84,148,	
0,0,	0,0,	0,0,	84,148,	
0,0,	88,146,	83,147,	83,145,	
0,0,	0,0,	88,146,	0,0,	
0,0,	96,96,	0,0,	88,146,	
0,0,	84,148,	0,0,	88,146,	
84,148,	96,96,	96,152,	0,0,	
96,96,	0,0,	0,0,	0,0,	
0,0,	172,172,	0,0,	0,0,	
0,0,	88,146,	0,0,	0,0,	
88,146,	172,172,	172,202,	0,0,	
172,172,	0,0,	0,0,	0,0,	
0,0,	0,0,	96,96,	84,149,	
84,148,	0,0,	0,0,	96,96,	
0,0,	0,0,	0,0,	0,0,	
96,96,	0,0,	0,0,	0,0,	
96,96,	0,0,	172,172,	0,0,	
88,146,	0,0,	233,233,	172,172,	
0,0,	0,0,	0,0,	0,0,	
172,172,	0,0,	96,96,	0,0,	
172,172,	96,96,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	233,233,	172,172,	0,0,	
0,0,	172,172,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	96,96,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	172,172,	233,237,	233,237,	
233,237,	233,237,	233,237,	233,237,	
233,237,	233,237,	233,237,	233,237,	
233,237,	233,237,	233,237,	233,237,	
233,237,	233,237,	233,237,	233,237,	
233,237,	233,237,	233,237,	233,237,	
233,237,	233,237,	233,237,	233,237,	
0,0,	0,0,	0,0,	0,0,	
233,237,	0,0,	233,237,	233,237,	
233,237,	233,237,	233,237,	233,237,	
233,237,	233,237,	233,237,	233,237,	
233,237,	233,237,	233,237,	233,237,	
233,237,	233,237,	233,237,	233,237,	
233,237,	233,237,	233,237,	233,237,	
233,237,	233,237,	233,237,	233,237,	
237,237,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	237,237,	
237,237,	237,237,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	0,0,	
0,0,	0,0,	0,0,	237,237,	
0,0,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	237,237,	
237,237,	237,237,	237,237,	238,240,	
0,0,	0,0,	0,0,	0,0,	
0,0,	239,241,	0,0,	238,240,	
238,240,	0,0,	238,240,	0,0,	
0,0,	239,241,	239,241,	0,0,	
239,241,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
238,0,	0,0,	0,0,	0,0,	
0,0,	238,240,	239,241,	0,0,	
0,0,	0,0,	238,240,	239,241,	
0,0,	0,0,	238,240,	0,0,	
239,241,	0,0,	0,0,	0,0,	
239,241,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
238,240,	0,0,	0,0,	238,240,	
0,0,	0,0,	239,0,	0,0,	
0,0,	239,241,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	238,240,	
0,0,	0,0,	0,0,	0,0,	
0,0,	239,241,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-30,	yysvec+1,	0,	
yycrank+-49,	0,		0,	
yycrank+-5,	yysvec+3,	0,	
yycrank+-124,	0,		0,	
yycrank+-28,	yysvec+5,	0,	
yycrank+-164,	0,		0,	
yycrank+-34,	yysvec+7,	0,	
yycrank+-176,	yysvec+7,	0,	
yycrank+-231,	yysvec+7,	0,	
yycrank+-271,	0,		0,	
yycrank+-7,	yysvec+11,	0,	
yycrank+-328,	0,		0,	
yycrank+-115,	yysvec+13,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+5,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+-340,	0,		yyvstop+8,
yycrank+6,	0,		yyvstop+10,
yycrank+0,	0,		yyvstop+13,
yycrank+0,	0,		yyvstop+16,
yycrank+0,	0,		yyvstop+19,
yycrank+0,	0,		yyvstop+22,
yycrank+5,	0,		yyvstop+25,
yycrank+22,	0,		yyvstop+27,
yycrank+0,	0,		yyvstop+29,
yycrank+0,	0,		yyvstop+32,
yycrank+374,	0,		yyvstop+35,
yycrank+-496,	0,		yyvstop+38,
yycrank+22,	yysvec+28,	yyvstop+40,
yycrank+3,	yysvec+28,	yyvstop+43,
yycrank+1,	yysvec+28,	yyvstop+46,
yycrank+3,	yysvec+28,	yyvstop+49,
yycrank+14,	yysvec+28,	yyvstop+52,
yycrank+18,	yysvec+28,	yyvstop+55,
yycrank+29,	yysvec+28,	yyvstop+58,
yycrank+40,	yysvec+28,	yyvstop+61,
yycrank+62,	yysvec+28,	yyvstop+64,
yycrank+22,	yysvec+28,	yyvstop+67,
yycrank+40,	yysvec+28,	yyvstop+70,
yycrank+41,	yysvec+28,	yyvstop+73,
yycrank+0,	0,		yyvstop+76,
yycrank+35,	0,		yyvstop+79,
yycrank+0,	0,		yyvstop+81,
yycrank+0,	yysvec+16,	yyvstop+84,
yycrank+0,	yysvec+25,	yyvstop+88,
yycrank+-502,	0,		yyvstop+91,
yycrank+60,	0,		yyvstop+95,
yycrank+54,	0,		yyvstop+98,
yycrank+0,	0,		yyvstop+101,
yycrank+0,	yysvec+16,	yyvstop+104,
yycrank+-508,	0,		yyvstop+108,
yycrank+-515,	0,		yyvstop+111,
yycrank+0,	0,		yyvstop+114,
yycrank+0,	yysvec+25,	yyvstop+118,
yycrank+-575,	0,		yyvstop+121,
yycrank+0,	0,		yyvstop+124,
yycrank+0,	0,		yyvstop+128,
yycrank+0,	yysvec+16,	yyvstop+131,
yycrank+-581,	0,		yyvstop+135,
yycrank+-587,	0,		yyvstop+138,
yycrank+0,	yysvec+25,	yyvstop+141,
yycrank+-638,	0,		yyvstop+144,
yycrank+0,	0,		yyvstop+147,
yycrank+0,	0,		yyvstop+151,
yycrank+0,	0,		yyvstop+155,
yycrank+0,	yysvec+16,	yyvstop+158,
yycrank+-644,	0,		yyvstop+162,
yycrank+-650,	0,		yyvstop+165,
yycrank+0,	yysvec+25,	yyvstop+168,
yycrank+-657,	0,		yyvstop+171,
yycrank+0,	0,		yyvstop+174,
yycrank+0,	0,		yyvstop+178,
yycrank+0,	0,		yyvstop+182,
yycrank+0,	yysvec+16,	yyvstop+185,
yycrank+-701,	0,		yyvstop+189,
yycrank+-717,	0,		yyvstop+192,
yycrank+0,	yysvec+25,	yyvstop+195,
yycrank+-758,	0,		yyvstop+198,
yycrank+47,	0,		yyvstop+201,
yycrank+0,	0,		yyvstop+204,
yycrank+0,	yysvec+16,	yyvstop+207,
yycrank+-774,	0,		yyvstop+211,
yycrank+-815,	0,		yyvstop+214,
yycrank+0,	0,		yyvstop+217,
yycrank+0,	0,		yyvstop+221,
yycrank+0,	yysvec+25,	yyvstop+225,
yycrank+-831,	0,		yyvstop+228,
yycrank+0,	yysvec+16,	yyvstop+231,
yycrank+-77,	yysvec+18,	0,	
yycrank+0,	0,		yyvstop+233,
yycrank+-170,	yysvec+18,	0,	
yycrank+0,	yysvec+19,	yyvstop+235,
yycrank+49,	0,		0,	
yycrank+0,	0,		yyvstop+237,
yycrank+-872,	0,		yyvstop+239,
yycrank+0,	yysvec+28,	yyvstop+241,
yycrank+0,	yysvec+29,	0,	
yycrank+0,	0,		yyvstop+243,
yycrank+66,	yysvec+28,	yyvstop+245,
yycrank+63,	yysvec+28,	yyvstop+247,
yycrank+84,	yysvec+28,	yyvstop+249,
yycrank+65,	yysvec+28,	yyvstop+251,
yycrank+66,	yysvec+28,	yyvstop+253,
yycrank+68,	yysvec+28,	yyvstop+255,
yycrank+76,	yysvec+28,	yyvstop+257,
yycrank+83,	yysvec+28,	yyvstop+259,
yycrank+80,	yysvec+28,	yyvstop+261,
yycrank+89,	yysvec+28,	yyvstop+263,
yycrank+82,	yysvec+28,	yyvstop+265,
yycrank+91,	yysvec+28,	yyvstop+267,
yycrank+99,	yysvec+28,	yyvstop+269,
yycrank+83,	yysvec+28,	yyvstop+271,
yycrank+104,	yysvec+28,	yyvstop+273,
yycrank+92,	yysvec+28,	yyvstop+275,
yycrank+0,	0,		yyvstop+277,
yycrank+-191,	yysvec+47,	yyvstop+279,
yycrank+-180,	yysvec+47,	yyvstop+281,
yycrank+0,	0,		yyvstop+283,
yycrank+0,	yysvec+47,	yyvstop+285,
yycrank+99,	0,		0,	
yycrank+103,	0,		0,	
yycrank+-196,	yysvec+52,	0,	
yycrank+0,	0,		yyvstop+287,
yycrank+-208,	yysvec+52,	0,	
yycrank+-211,	yysvec+53,	0,	
yycrank+-212,	yysvec+53,	0,	
yycrank+-215,	yysvec+60,	0,	
yycrank+0,	0,		yyvstop+289,
yycrank+-220,	yysvec+60,	0,	
yycrank+-217,	yysvec+61,	0,	
yycrank+-221,	yysvec+61,	0,	
yycrank+-224,	yysvec+68,	0,	
yycrank+0,	0,		yyvstop+291,
yycrank+-225,	yysvec+68,	0,	
yycrank+-234,	yysvec+69,	0,	
yycrank+-235,	yysvec+69,	0,	
yycrank+-236,	yysvec+76,	0,	
yycrank+0,	0,		yyvstop+293,
yycrank+-239,	yysvec+76,	0,	
yycrank+-238,	yysvec+77,	0,	
yycrank+-243,	yysvec+77,	0,	
yycrank+0,	0,		yyvstop+295,
yycrank+0,	0,		yyvstop+297,
yycrank+-240,	yysvec+83,	0,	
yycrank+0,	0,		yyvstop+299,
yycrank+-242,	yysvec+83,	0,	
yycrank+-251,	yysvec+84,	0,	
yycrank+-253,	yysvec+84,	0,	
yycrank+-254,	yysvec+18,	yyvstop+301,
yycrank+0,	0,		yyvstop+303,
yycrank+0,	0,		yyvstop+305,
yycrank+0,	yysvec+28,	yyvstop+307,
yycrank+94,	yysvec+28,	yyvstop+310,
yycrank+119,	yysvec+28,	yyvstop+312,
yycrank+138,	yysvec+28,	yyvstop+314,
yycrank+128,	yysvec+28,	yyvstop+316,
yycrank+154,	yysvec+28,	yyvstop+318,
yycrank+161,	yysvec+28,	yyvstop+320,
yycrank+161,	yysvec+28,	yyvstop+322,
yycrank+0,	yysvec+28,	yyvstop+324,
yycrank+164,	yysvec+28,	yyvstop+327,
yycrank+166,	yysvec+28,	yyvstop+329,
yycrank+163,	yysvec+28,	yyvstop+331,
yycrank+174,	yysvec+28,	yyvstop+333,
yycrank+169,	yysvec+28,	yyvstop+335,
yycrank+169,	yysvec+28,	yyvstop+337,
yycrank+189,	yysvec+28,	yyvstop+339,
yycrank+180,	yysvec+28,	yyvstop+341,
yycrank+189,	yysvec+28,	yyvstop+343,
yycrank+195,	yysvec+28,	yyvstop+345,
yycrank+-888,	0,		yyvstop+347,
yycrank+192,	0,		0,	
yycrank+190,	0,		0,	
yycrank+-304,	yysvec+52,	yyvstop+350,
yycrank+-311,	yysvec+53,	yyvstop+352,
yycrank+-312,	yysvec+60,	yyvstop+354,
yycrank+-315,	yysvec+61,	yyvstop+356,
yycrank+-343,	yysvec+68,	yyvstop+358,
yycrank+-345,	yysvec+69,	yyvstop+360,
yycrank+-373,	yysvec+76,	yyvstop+362,
yycrank+-375,	yysvec+77,	yyvstop+364,
yycrank+-376,	yysvec+83,	yyvstop+366,
yycrank+-406,	yysvec+84,	yyvstop+368,
yycrank+0,	yysvec+28,	yyvstop+370,
yycrank+0,	yysvec+28,	yyvstop+373,
yycrank+194,	yysvec+28,	yyvstop+376,
yycrank+0,	yysvec+28,	yyvstop+378,
yycrank+192,	yysvec+28,	yyvstop+381,
yycrank+192,	yysvec+28,	yyvstop+383,
yycrank+201,	yysvec+28,	yyvstop+385,
yycrank+0,	yysvec+28,	yyvstop+387,
yycrank+205,	yysvec+28,	yyvstop+390,
yycrank+223,	yysvec+28,	yyvstop+392,
yycrank+241,	yysvec+28,	yyvstop+394,
yycrank+239,	yysvec+28,	yyvstop+396,
yycrank+248,	yysvec+28,	yyvstop+398,
yycrank+248,	yysvec+28,	yyvstop+400,
yycrank+241,	yysvec+28,	yyvstop+402,
yycrank+254,	yysvec+28,	yyvstop+404,
yycrank+0,	yysvec+28,	yyvstop+406,
yycrank+0,	0,		yyvstop+409,
yycrank+248,	0,		0,	
yycrank+242,	0,		0,	
yycrank+259,	yysvec+28,	yyvstop+412,
yycrank+251,	yysvec+28,	yyvstop+414,
yycrank+0,	yysvec+28,	yyvstop+416,
yycrank+264,	yysvec+28,	yyvstop+419,
yycrank+250,	yysvec+28,	yyvstop+421,
yycrank+0,	yysvec+28,	yyvstop+423,
yycrank+270,	yysvec+28,	yyvstop+426,
yycrank+272,	yysvec+28,	yyvstop+428,
yycrank+257,	yysvec+28,	yyvstop+430,
yycrank+276,	yysvec+28,	yyvstop+432,
yycrank+0,	yysvec+28,	yyvstop+434,
yycrank+268,	yysvec+28,	yyvstop+437,
yycrank+279,	0,		0,	
yycrank+281,	0,		0,	
yycrank+0,	yysvec+28,	yyvstop+439,
yycrank+0,	yysvec+28,	yyvstop+442,
yycrank+0,	yysvec+28,	yyvstop+445,
yycrank+281,	yysvec+28,	yyvstop+448,
yycrank+0,	yysvec+28,	yyvstop+450,
yycrank+0,	yysvec+28,	yyvstop+453,
yycrank+0,	yysvec+28,	yyvstop+456,
yycrank+285,	yysvec+28,	yyvstop+459,
yycrank+288,	yysvec+28,	yyvstop+461,
yycrank+382,	0,		0,	
yycrank+291,	0,		0,	
yycrank+280,	yysvec+28,	yyvstop+463,
yycrank+0,	yysvec+28,	yyvstop+465,
yycrank+295,	yysvec+28,	yyvstop+468,
yycrank+917,	0,		0,	
yycrank+519,	0,		0,	
yycrank+0,	yysvec+28,	yyvstop+470,
yycrank+0,	yysvec+28,	yyvstop+473,
yycrank+992,	0,		yyvstop+476,
yycrank+-1114,	0,		0,	
yycrank+-1120,	0,		0,	
yycrank+-363,	yysvec+238,	0,	
yycrank+-336,	yysvec+239,	0,	
yycrank+0,	0,		yyvstop+478,
yycrank+0,	0,		yyvstop+480,
0,	0,	0};
struct yywork *yytop = yycrank+1213;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,014 ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,'"' ,01  ,01  ,01  ,01  ,047 ,
01  ,01  ,01  ,01  ,',' ,01  ,01  ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,01  ,',' ,01  ,01  ,'>' ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,']' ,01  ,'A' ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,1,0,0,0,0,
0,0,0,0,1,1,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
#ifndef lint
static	char ncform_sccsid[] = "@(#)ncform 1.6 88/02/08 SMI"; /* from S5R2 1.2 */
#endif

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
