/* $Id: cproto.h 3.4 92/04/04 13:59:08 cthuang Exp $
 *
 * Declarations for C function prototype generator
 */
#include "config.h"

/* Boolean type */
typedef char boolean;
#define FALSE	0
#define TRUE	1

/* Source file text */
typedef struct text {
    char text[MAX_TEXT_SIZE];	/* source text */
    long begin; 		/* offset in temporary file */
} Text;

/* This is a list of function parameters. */
typedef struct parameter_list {
    struct parameter *first;	/* pointer to first parameter in list */
    struct parameter *last;	/* pointer to last parameter in list */  
    long begin_comment; 	/* begin offset of comment */
    long end_comment;		/* end offset of comment */
    char *comment;		/* comment at start of parameter list */
} ParameterList;

/* Declaration specifier flags */
#define DS_EXTERN	0	/* default: external declaration */
#define DS_STATIC	1	/* visible only in current file */
#define DS_CHAR 	2	/* "char" type specifier in declaration */
#define DS_SHORT	4	/* "short" type specifier in declaration */
#define DS_FLOAT	8	/* "float" type specifier in declaration */
#define DS_JUNK 	16	/* we're not interested in this declaration */

/* This structure stores information about a declaration specifier. */
typedef struct decl_spec {
    unsigned short flags;	/* flags defined above */
    char *text; 		/* source text */
    long begin; 		/* offset in temporary file */
} DeclSpec;

/* Styles of function definitions */
#define FUNC_NONE		0	/* not a function definition */
#define FUNC_TRADITIONAL	1	/* traditional style */
#define FUNC_ANSI		2	/* ANSI style */
typedef int FuncDefStyle;

/* This structure stores information about a declarator. */
typedef struct declarator {
    char *name; 			/* name of variable or function */
    char *text; 			/* source text */
    long begin; 			/* offset in temporary file */
    long begin_comment; 		/* begin offset of comment */
    long end_comment;			/* end offset of comment */
    FuncDefStyle func_def;		/* style of function definition */
    ParameterList params;		/* function parameters */
    struct declarator *head;		/* head function declarator */
    struct declarator *func_stack;	/* stack of function declarators */
    struct declarator *next;		/* next declarator in list */
} Declarator;

/* This is a list of declarators. */
typedef struct declarator_list {
    Declarator *first;		/* pointer to first declarator in list */
    Declarator *last;		/* pointer to last declarator in list */  
} DeclaratorList;

/* This structure stores information about a function parameter. */
typedef struct parameter {
    struct parameter *next;	/* next parameter in list */
    DeclSpec decl_spec;
    Declarator *declarator;
    char *comment;		/* comment following the parameter */
} Parameter;

/* parser stack entry type */
typedef union {
    Text text;
    DeclSpec decl_spec;
    Parameter parameter;
    ParameterList param_list;
    Declarator *declarator;
    DeclaratorList decl_list;
} YYSTYPE;

/* Prototype styles */
#define PROTO_NONE		0	/* do not output any prototypes */
#define PROTO_TRADITIONAL	1	/* comment out parameters */
#define PROTO_ABSTRACT		2	/* comment out parameter names */
#define PROTO_ANSI		3	/* ANSI C prototype */
#define PROTO_MACRO		4	/* macro around parameters */
typedef int PrototypeStyle;

/* The role of a function declarator */
#define FUNC_OTHER	0	/* miscellaneous declaration */
#define FUNC_PROTO	1	/* prototype */
#define FUNC_DEF	2	/* function definition */
typedef int FuncDeclRole;

/* Prototype/function definition output formats */
#define FMT_OTHER		0	/* miscellaneous */
#define FMT_PROTO		1	/* prototype */
#define FMT_FUNC		2	/* function definition */
#define FMT_FUNC_COMMENT	3	/* func. def. with parameter comments */
typedef int FuncFormatType;

/* Prototype/function definition output format */
typedef struct {
    char *decl_spec_prefix;	/* output before declaration specifier */
    char *declarator_prefix;	/* output before declarator name */
    char *declarator_suffix;	/* output before '(' of parameter list */
    char *first_param_prefix;	/* output before first parameter */
    char *middle_param_prefix;	/* output before each subsequent parameter */
    char *last_param_suffix;	/* output after last parameter */
} FuncFormat;

/* Program options */
extern boolean extern_out;
extern boolean static_out;
extern boolean variables_out;
extern boolean promote_param;
extern PrototypeStyle proto_style;
extern FuncDefStyle func_style;
extern boolean define_macro;
extern char *macro_name;
extern boolean proto_comments;
extern int num_inc_dir;
extern char *inc_dir[];
extern FuncFormat fmt[4];

/* Global declarations */
extern char progname[];

extern char *xmalloc(), *xstrdup(), *trim_path_sep();
extern void put_error();
extern void init_parser(), process_file(), pop_file();
extern char *cur_file_name();
extern unsigned cur_line_num();
extern FILE *cur_tmp_file();
extern void cur_file_changed();
extern long cur_begin_comment();
