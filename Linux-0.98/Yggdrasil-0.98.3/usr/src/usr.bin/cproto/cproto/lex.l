%{
/* $Id: lex.l 3.5 92/04/04 13:58:53 cthuang Exp $
 *
 * Lexical analyzer for C function prototype generator
 */

static int curly;	/* number of curly brace nesting levels */
static int ly_count;	/* number of occurances of %% */

/* information about the current input file */
typedef struct {
    char *file_name;		/* input file name */
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
%}

WS		[ \t]
LETTER		[A-Za-z_]
DIGIT		[0-9]
ID		{LETTER}({LETTER}|{DIGIT})*
STRING		\"(\\\"|[^"\n])*\"
QUOTED		({STRING}|\'(\\\'|[^'\n])*\'|\\.)

%s CPP1 INIT1 INIT2 CURLY LEXYACC ASM
%%

\n			{ save_text(); cur_file->line_num++;
			    cur_declarator = NULL; }

"/*"			{ save_text(); get_comment(); }
"//".*$ 		save_text();

<LEXYACC>^"%%"		{ save_text(); if (++ly_count >= 2) BEGIN INITIAL; }
<LEXYACC>^"%{"		{ save_text(); BEGIN INITIAL; }
<LEXYACC>{QUOTED}|.	save_text();
<INITIAL>^"%}"		{ save_text(); BEGIN LEXYACC; }

<INITIAL>#{WS}* 	{ save_text(); BEGIN CPP1; }

<CPP1>define{WS}+{ID}	{
			    save_text();
			    sscanf(yytext, "define %s", buf);
			    get_cpp_directive();
			    new_symbol(typedef_names, buf, DS_EXTERN);
			}

<CPP1>include{WS}*\"[^"]+\"     {
			    save_text();
			    sscanf(yytext, "include \"%[^\"]\"", buf);
			    get_cpp_directive();
			    do_include(buf, FALSE);
			}
<CPP1>include{WS}*\<[^>]+\>	{
			    save_text();
			    sscanf(yytext, "include <%[^>]>", buf);
			    get_cpp_directive();
			    do_include(buf, TRUE);
			}

<CPP1>[0-9]+{WS}+\".*$  {
			    save_text();
			    sscanf(yytext, "%d \"%[^\"]\"", &cur_file->line_num,
				cur_file->file_name);
			    cur_file->line_num--;
			    BEGIN INITIAL;
			}
<CPP1>[0-9]+.*$ 	{
			    save_text();
			    sscanf(yytext, "%d ", &cur_file->line_num);
			    cur_file->line_num--;
			    BEGIN INITIAL;
			}

<CPP1>. 		{ save_text(); get_cpp_directive(); }

<INITIAL>"("		{ save_text_offset(); return '('; }
<INITIAL>")"		{
			    save_text();
			    if (cur_file->convert)
				cur_file->begin_comment =
				 ftell(cur_file->tmp_file);
			    return ')';
			}
<INITIAL>"*"		{ save_text_offset(); return '*'; }
<INITIAL>","		{ save_text(); return ','; }
<INITIAL>";"		{
			    save_text();
			    if (cur_file->convert)
				cur_file->begin_comment =
				 ftell(cur_file->tmp_file);
			    return ';';
			}
<INITIAL>"..."		{ save_text(); return T_ELLIPSIS; }
<INITIAL>{STRING}	{ save_text(); return T_STRING_LITERAL; }

<INITIAL>asm		{ save_text(); BEGIN ASM; return T_ASM; }
<ASM>"("		save_text();
<ASM>")"		{ save_text(); BEGIN INITIAL; return T_ASMARG; }
<ASM>{QUOTED}|. 	save_text();

<INITIAL>auto		{ save_text_offset(); return T_AUTO; }
<INITIAL>extern 	{ save_text_offset(); return T_EXTERN; }
<INITIAL>register	{ save_text_offset(); return T_REGISTER; }
<INITIAL>static 	{ save_text_offset(); return T_STATIC; }
<INITIAL>typedef	{ save_text_offset(); return T_TYPEDEF; }
<INITIAL>inline 	{ save_text_offset(); return T_INLINE; }
<INITIAL>char		{ save_text_offset(); return T_CHAR; }
<INITIAL>double 	{ save_text_offset(); return T_DOUBLE; }
<INITIAL>float		{ save_text_offset(); return T_FLOAT; }
<INITIAL>int		{ save_text_offset(); return T_INT; }
<INITIAL>void		{ save_text_offset(); return T_VOID; }
<INITIAL>long		{ save_text_offset(); return T_LONG; }
<INITIAL>short		{ save_text_offset(); return T_SHORT; }
<INITIAL>signed 	{ save_text_offset(); return T_SIGNED; }
<INITIAL>unsigned	{ save_text_offset(); return T_UNSIGNED; }
<INITIAL>enum		{ save_text_offset(); return T_ENUM; }
<INITIAL>struct 	{ save_text_offset(); return T_STRUCT; }
<INITIAL>union		{ save_text_offset(); return T_UNION; }

<INITIAL>{ID}		{
			    save_text_offset();
			    if (find_symbol(type_qualifiers, yytext) != NULL)
				return T_TYPE_QUALIFIER;
			    else if (find_symbol(typedef_names, yytext) != NULL)
				return T_TYPEDEF_NAME;
			    else
				return T_IDENTIFIER;
			}

<INITIAL>\[[^\]]*\]	{
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

<INITIAL>"="		{ save_text(); BEGIN INIT1; return '='; }
<INIT1>"{"		{ save_text(); curly = 1; BEGIN INIT2; }
<INIT1>[,;]		{
			    unput(yytext[yyleng-1]);
			    BEGIN INITIAL;
			    return T_INITIALIZER;
			}
<INIT1>{QUOTED}|.	save_text();

<INIT2>"{"		{ save_text(); ++curly; }
<INIT2>"}"		{
			    save_text();
			    if (--curly == 0) {
				BEGIN INITIAL;
				return T_INITIALIZER;
			    }
			}
<INIT2>{QUOTED}|.	save_text();

<INITIAL>"{"		{
			    save_text();
			    curly = 1;
			    BEGIN CURLY;
			    return T_LBRACE;
			}
<CURLY>"{"		{ save_text(); ++curly; }
<CURLY>"}"		{
			    save_text();
			    if (--curly == 0) {
				BEGIN INITIAL;
				return T_MATCHRBRACE;
			    }
			}
<CURLY>{QUOTED}|.	save_text();

[ \t\f]+		save_text();
.			{
			    save_text();
			    put_error();
			    fprintf(stderr, "bad character '%c'\n", yytext[0]);
			}
%%

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
	    } else if ((outf = fopen(cur_file->file_name, "w")) != NULL) {
		put_file(outf);
		fclose(outf);
	    } else {
		fprintf(stderr, "%s: cannot create file %s\n", progname,
		    cur_file->file_name);
	    }
	}

	fclose(cur_file->tmp_file);
    }
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

    put_error();
    fprintf(stderr, "cannot read file %s\n", filename);
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
