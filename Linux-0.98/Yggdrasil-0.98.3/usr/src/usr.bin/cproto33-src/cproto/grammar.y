/* $Id: grammar.y 3.5 92/04/11 19:27:34 cthuang Exp $
 *
 * yacc grammar for C function prototype generator
 * This was derived from the grammar in Appendix A of
 * "The C Programming Language" by Kernighan and Ritchie.
 */

%token <text> '(' '*'

%token
	/* identifiers that are not reserved words */
	T_IDENTIFIER T_TYPEDEF_NAME

	/* storage class */
	T_AUTO T_EXTERN T_REGISTER T_STATIC T_TYPEDEF
	/* This keyword included for compatibility with C++. */
	T_INLINE

	/* type specifiers */
	T_CHAR T_DOUBLE T_FLOAT T_INT T_VOID
	T_LONG T_SHORT T_SIGNED T_UNSIGNED
	T_ENUM T_STRUCT T_UNION

	/* type qualifiers */
	T_TYPE_QUALIFIER

	/* left brace */
	T_LBRACE
	/* all input to the matching right brace */
	T_MATCHRBRACE

	/* paired square brackets and everything between them: [ ... ] */
	T_BRACKETS

	/* three periods */
	T_ELLIPSIS

	/* constant expression or paired braces following an equal sign */
	T_INITIALIZER

	/* string literal */
	T_STRING_LITERAL

	/* asm */
	T_ASM
	/* ( "string literal" ) following asm keyword */
	T_ASMARG

%type <decl_spec> decl_specifiers decl_specifier
%type <decl_spec> storage_class type_specifier type_qualifier
%type <decl_spec> struct_or_union_specifier enum_specifier
%type <decl_list> init_declarator_list
%type <declarator> init_declarator declarator direct_declarator
%type <declarator> abs_declarator direct_abs_declarator
%type <param_list> parameter_type_list parameter_list
%type <parameter> parameter_declaration
%type <param_list> opt_identifier_list identifier_list
%type <text>
	struct_or_union pointer opt_type_qualifiers type_qualifier_list any_id
	T_BRACKETS
	T_IDENTIFIER T_TYPEDEF_NAME
	T_AUTO T_EXTERN T_REGISTER T_STATIC T_TYPEDEF T_INLINE
	T_CHAR T_DOUBLE T_FLOAT T_INT T_VOID T_LONG T_SHORT T_SIGNED T_UNSIGNED
	T_ENUM T_STRUCT T_UNION
	T_TYPE_QUALIFIER
	'(' '*'

%{
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
%}
%%

program
	: /* empty */
	| translation_unit
	;

translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

external_declaration
	: declaration
	| function_definition
	| function_definition ';'
	| linkage_specification
	| T_ASM T_ASMARG ';'
	| error ';'
	| error braces
	;

braces
	: T_LBRACE T_MATCHRBRACE
	;

linkage_specification
	: T_EXTERN T_STRING_LITERAL braces
	| T_EXTERN T_STRING_LITERAL declaration
	;

declaration
	: decl_specifiers ';'
	{
	    free_decl_spec(&$1);
	}
	| decl_specifiers init_declarator_list ';'
	{
	    check_untagged(&$1);
	    if (func_params != NULL) {
		set_param_types(func_params, &$1, &$2);
	    } else {
		gen_declarations(&$1, &$2);
		free_decl_list(&$2);
	    }
	    free_decl_spec(&$1);
	}
	| T_TYPEDEF decl_specifiers
	{
	    cur_decl_spec_flags = $2.flags;
	    free_decl_spec(&$2);
	}
	  opt_declarator_list ';'
	;

opt_declarator_list
	: /* empty */
	| declarator_list
	;

declarator_list
	: declarator
	{
	    new_symbol(typedef_names, $1->name, cur_decl_spec_flags);
	    free_declarator($1);
	}
	| declarator_list ',' declarator
	{
	    new_symbol(typedef_names, $3->name, cur_decl_spec_flags);
	    free_declarator($3);
	}
	;

function_definition
	: decl_specifiers declarator
	{
	    check_untagged(&$1);
	    if ($2->func_def == FUNC_NONE) {
		yyerror("syntax error");
		YYERROR;
	    }
	    func_params = &($2->head->params);
	    func_params->begin_comment = cur_file->begin_comment;
	    func_params->end_comment = cur_file->end_comment;
	}
	  opt_declaration_list T_LBRACE
	{
	    func_params = NULL;

	    if (cur_file->convert)
		gen_func_definition(&$1, $2);
	    gen_prototype(&$1, $2);
	    free_decl_spec(&$1);
	    free_declarator($2);
	}
	  T_MATCHRBRACE
	| declarator
	{
	    if ($1->func_def == FUNC_NONE) {
		yyerror("syntax error");
		YYERROR;
	    }
	    func_params = &($1->head->params);
	    func_params->begin_comment = cur_file->begin_comment;
	    func_params->end_comment = cur_file->end_comment;
	}
	  opt_declaration_list T_LBRACE
	{
	    DeclSpec decl_spec;

	    func_params = NULL;

	    new_decl_spec(&decl_spec, "int", $1->begin, DS_NONE);
	    if (cur_file->convert)
		gen_func_definition(&decl_spec, $1);
	    gen_prototype(&decl_spec, $1);
	    free_decl_spec(&decl_spec);
	    free_declarator($1);
	}
	  T_MATCHRBRACE
	;

opt_declaration_list
	: /* empty */
	| declaration_list
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

decl_specifiers
	: decl_specifier
	| decl_specifiers decl_specifier
	{
	    join_decl_specs(&$$, &$1, &$2);
	    free($1.text);
	    free($2.text);
	}
	;

decl_specifier
	: storage_class
	| type_specifier
	| type_qualifier
	;

storage_class
	: T_AUTO
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_NONE);
	}
	| T_EXTERN
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_EXTERN);
	}
	| T_REGISTER
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_NONE);
	}
	| T_STATIC
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_STATIC);
	}
	| T_INLINE
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_JUNK);
	}
	;

type_specifier
	: T_CHAR
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_CHAR);
	}
	| T_DOUBLE
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_NONE);
	}
	| T_FLOAT
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_FLOAT);
	}
	| T_INT
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_NONE);
	}
	| T_LONG
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_NONE);
	}
	| T_SHORT
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_SHORT);
	}
	| T_SIGNED
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_NONE);
	}
	| T_UNSIGNED
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_NONE);
	}
	| T_VOID
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_NONE);
	}
	| struct_or_union_specifier
	| enum_specifier
	;

type_qualifier
	: T_TYPE_QUALIFIER
	{
	    new_decl_spec(&$$, $1.text, $1.begin, DS_NONE);
	}
	| T_TYPEDEF_NAME
	{
	    /* A typedef name is actually a type specifier, but since the
	     * typedef symbol table also stores #define names, this production
	     * is here so the <pointer> nonterminal will scan #define names.
	     */
	    Symbol *s;
	    s = find_symbol(typedef_names, $1.text);
	    new_decl_spec(&$$, $1.text, $1.begin, s->flags);
	}
	;

struct_or_union_specifier
	: struct_or_union any_id braces
	{
	    sprintf(buf, "%s %s", $1.text, $2.text);
	    new_decl_spec(&$$, buf, $1.begin, DS_NONE);
	}
	| struct_or_union braces
	{
	    sprintf(buf, "%s {}", $1.text);
	    new_decl_spec(&$$, buf, $1.begin, DS_NONE);
	}
	| struct_or_union any_id
	{
	    sprintf(buf, "%s %s", $1.text, $2.text);
	    new_decl_spec(&$$, buf, $1.begin, DS_NONE);
	}
	;

struct_or_union
	: T_STRUCT
	| T_UNION
	;

init_declarator_list
	: init_declarator
	{
	    new_decl_list(&$$, $1);
	}
	| init_declarator_list ',' init_declarator
	{
	    add_decl_list(&$$, &$1, $3);
	}
	;

init_declarator
	: declarator
	{
	    if ($1->func_def != FUNC_NONE && func_params == NULL &&
		func_style == FUNC_TRADITIONAL && cur_file->convert) {
		gen_func_declarator($1);
		fputs(yytext, cur_file->tmp_file);
	    }
	    cur_declarator = $$;
	}
	| declarator '='
	{
	    if ($1->func_def != FUNC_NONE && func_params == NULL &&
		func_style == FUNC_TRADITIONAL && cur_file->convert) {
		gen_func_declarator($1);
		fputs(" =", cur_file->tmp_file);
	    }
	}
	  T_INITIALIZER
	;

enum_specifier
	: T_ENUM any_id braces
	{
	    sprintf(buf, "enum %s", $2.text);
	    new_decl_spec(&$$, buf, $1.begin, DS_NONE);
	}
	| T_ENUM braces
	{
	    new_decl_spec(&$$, "enum {}", $1.begin, DS_NONE);
	}
	| T_ENUM any_id
	{
	    sprintf(buf, "enum %s", $2.text);
	    new_decl_spec(&$$, buf, $1.begin, DS_NONE);
	}
	;

any_id
	: T_IDENTIFIER
	| T_TYPEDEF_NAME
	;

declarator
	: pointer direct_declarator
	{
	    $$ = $2;
	    sprintf(buf, "%s%s", $1.text, $$->text);
	    free($$->text);
	    $$->text = xstrdup(buf);
	    $$->begin = $1.begin;
	}
	| direct_declarator
	;

direct_declarator
	: T_IDENTIFIER
	{
	    $$ = new_declarator($1.text, $1.text, $1.begin);
	}
	| '(' declarator ')'
	{
	    $$ = $2;
	    sprintf(buf, "(%s)", $$->text);
	    free($$->text);
	    $$->text = xstrdup(buf);
	    $$->begin = $1.begin;
	}
	| direct_declarator T_BRACKETS
	{
	    $$ = $1;
	    sprintf(buf, "%s%s", $$->text, $2.text);
	    free($$->text);
	    $$->text = xstrdup(buf);
	}
	| direct_declarator '(' parameter_type_list ')'
	{
	    $$ = new_declarator("%s()", $1->name, $1->begin);
	    $$->params = $3;
	    $$->func_stack = $1;
	    $$->head = ($1->func_stack == NULL) ? $$ : $1->head;
	    $$->func_def = FUNC_ANSI;
	}
	| direct_declarator '(' opt_identifier_list ')'
	{
	    $$ = new_declarator("%s()", $1->name, $1->begin);
	    $$->params = $3;
	    $$->func_stack = $1;
	    $$->head = ($1->func_stack == NULL) ? $$ : $1->head;
	    $$->func_def = FUNC_TRADITIONAL;
	}
	;

pointer
	: '*' opt_type_qualifiers
	{
	    sprintf($$.text, "*%s", $2.text);
	    $$.begin = $1.begin;
	}
	| '*' opt_type_qualifiers pointer
	{
	    sprintf($$.text, "*%s%s", $2.text, $3.text);
	    $$.begin = $1.begin;
	}
	;

opt_type_qualifiers
	: /* empty */
	{
	    strcpy($$.text, "");
	    $$.begin = 0L;
	}
	| type_qualifier_list
	;

type_qualifier_list
	: type_qualifier
	{
	    strcpy($$.text, $1.text);
	    $$.begin = $1.begin;
	    free($1.text);
	}
	| type_qualifier_list type_qualifier
	{
	    sprintf($$.text, "%s %s ", $1.text, $2.text);
	    $$.begin = $1.begin;
	    free($2.text);
	}
	;

parameter_type_list
	: parameter_list
	| parameter_list ',' T_ELLIPSIS
	{
	    add_ident_list(&$$, &$1, "...");
	}
	;

parameter_list
	: parameter_declaration
	{
	    new_param_list(&$$, &$1);
	}
	| parameter_list ',' parameter_declaration
	{
	    add_param_list(&$$, &$1, &$3);
	}
	;

parameter_declaration
	: decl_specifiers declarator
	{
	    check_untagged(&$1);
	    new_parameter(&$$, &$1, $2);
	}
	| decl_specifiers abs_declarator
	{
	    check_untagged(&$1);
	    new_parameter(&$$, &$1, $2);
	}
	| decl_specifiers
	{
	    check_untagged(&$1);
	    new_parameter(&$$, &$1, NULL);
	}
	;

opt_identifier_list
	: /* empty */
	{
	    new_ident_list(&$$);
	}
	| identifier_list
	;

identifier_list
	: T_IDENTIFIER
	{
	    new_ident_list(&$$);
	    add_ident_list(&$$, &$$, $1.text);
	}
	| identifier_list ',' T_IDENTIFIER
	{
	    add_ident_list(&$$, &$1, $3.text);
	}
	;

abs_declarator
	: pointer
	{
	    $$ = new_declarator($1.text, "", $1.begin);
	}
	| pointer direct_abs_declarator
	{
	    $$ = $2;
	    sprintf(buf, "%s%s", $1.text, $$->text);
	    free($$->text);
	    $$->text = xstrdup(buf);
	    $$->begin = $1.begin;
	}
	| direct_abs_declarator
	;

direct_abs_declarator
	: '(' abs_declarator ')'
	{
	    $$ = $2;
	    sprintf(buf, "(%s)", $$->text);
	    free($$->text);
	    $$->text = xstrdup(buf);
	    $$->begin = $1.begin;
	}
	| direct_abs_declarator T_BRACKETS
	{
	    $$ = $1;
	    sprintf(buf, "%s%s", $$->text, $2.text);
	    free($$->text);
	    $$->text = xstrdup(buf);
	}
	| T_BRACKETS
	{
	    $$ = new_declarator($1.text, "", $1.begin);
	}
	| direct_abs_declarator '(' parameter_type_list ')'
	{
	    $$ = new_declarator("%s()", "", $1->begin);
	    $$->params = $3;
	    $$->func_stack = $1;
	    $$->head = ($1->func_stack == NULL) ? $$ : $1->head;
	    $$->func_def = FUNC_ANSI;
	}
	| direct_abs_declarator '(' ')'
	{
	    $$ = new_declarator("%s()", "", $1->begin);
	    $$->func_stack = $1;
	    $$->head = ($1->func_stack == NULL) ? $$ : $1->head;
	    $$->func_def = FUNC_ANSI;
	}
	| '(' parameter_type_list ')'
	{
	    Declarator *d;
	    
	    d = new_declarator("", "", $1.begin);
	    $$ = new_declarator("%s()", "", $1.begin);
	    $$->params = $2;
	    $$->func_stack = d;
	    $$->head = $$;
	    $$->func_def = FUNC_ANSI;
	}
	| '(' ')'
	{
	    Declarator *d;
	    
	    d = new_declarator("", "", $1.begin);
	    $$ = new_declarator("%s()", "", $1.begin);
	    $$->func_stack = d;
	    $$->head = $$;
	    $$->func_def = FUNC_ANSI;
	}
	;

%%
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
	new_symbol(type_qualifiers, keywords[i], DS_NONE);
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
