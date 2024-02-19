/* $Id: semantic.c 3.5 92/04/11 19:28:08 cthuang Exp $
 *
 * Semantic actions executed by the parser of the
 * C function prototype generator.
 */
#include <stdio.h>
#include "cproto.h"
#include "semantic.h"

/* Head function declarator in a prototype or function definition */
static Declarator *func_declarator;

/* Where the declarator appears */
static int where;

/* Output format to use */
static int format;

/* Initialize a new declaration specifier part.
 */
void
new_decl_spec (decl_spec, text, offset, flags)
DeclSpec *decl_spec;
char *text;
long offset;
int flags;
{
    decl_spec->text = xstrdup(text);
    decl_spec->begin = offset;
    decl_spec->flags = flags;
}

/* Free storage used by a declaration specifier part.
 */
void
free_decl_spec (decl_spec)
DeclSpec *decl_spec;
{
    free(decl_spec->text);
}

/* Create a new string by joining two strings with a space between them.
 * Return a pointer to the resultant string.
 * If out of memory, output an error message and exit.
 */
static char *
concat_string (a, b)
char *a, *b;
{
    char *result;

    result = xmalloc(strlen(a) + strlen(b) + 2);
    strcpy(result, a);
    strcat(result, " ");
    strcat(result, b);
    return result;
}

/* Append two declaration specifier parts together.
 */
void
join_decl_specs (result, a, b)
DeclSpec *result, *a, *b;
{
    result->text = concat_string(a->text, b->text);
    result->flags = a->flags | b->flags;
    result->begin = a->begin;
}

/* Output an error message if the declaration specifier is an untagged
 * struct, union or enum.
 */
void
check_untagged (decl_spec)
DeclSpec *decl_spec;
{
    if (strstr(decl_spec->text, "struct {}") != NULL) {
	put_error();
	fputs("untagged struct declaration\n", stderr);
    } else if (strstr(decl_spec->text, "union {}") != NULL) {
	put_error();
	fputs("untagged union declaration\n", stderr);
    } else if (strstr(decl_spec->text, "enum {}") != NULL) {
	put_error();
	fputs("untagged enum declaration\n", stderr);
    }
}

/* Allocate and initialize a declarator.
 */
Declarator *
new_declarator (text, name, offset)
char *text, *name;
long offset;
{
    Declarator *d;

    d = (Declarator *)xmalloc(sizeof(Declarator));
    d->text = xstrdup(text);
    d->name = xstrdup(name);
    d->begin = offset;
    d->begin_comment = d->end_comment = 0;
    d->func_def = FUNC_NONE;
    new_ident_list(&d->params);
    d->head = d;
    d->func_stack = NULL;
    return d;
}

/* Free storage used by a declarator.
 */
void
free_declarator (d)
Declarator *d;
{
    free(d->text);
    free(d->name);
    free_param_list(&(d->params));
    if (d->func_stack != NULL)
	free_declarator(d->func_stack);
    free(d);
}

/* Initialize a declarator list and add the given declarator to it.
 */
void
new_decl_list (decl_list, declarator)
DeclaratorList *decl_list;
Declarator *declarator;
{
    decl_list->first = decl_list->last = declarator;
    declarator->next = NULL;
}

/* Free storage used by the declarators in the declarator list.
 */
void
free_decl_list (decl_list)
DeclaratorList *decl_list;
{
    Declarator *d, *next;

    d = decl_list->first;
    while (d != NULL) {
	next = d->next;
	free_declarator(d);
	d = next;
    }
}

/* Add the declarator to the declarator list.
 */
void
add_decl_list (to, from, declarator)
DeclaratorList *to, *from;
Declarator *declarator;
{
    to->first = from->first;
    from->last->next = declarator;
    to->last = declarator;
    to->last->next = NULL;
}

/* Initialize the parameter structure.
 */
void
new_parameter (param, decl_spec, declarator)
Parameter *param;		/* parameter to be initialized */
DeclSpec *decl_spec;
Declarator *declarator;
{
    if (decl_spec == NULL) {
	new_decl_spec(&(param->decl_spec), "", 0L, DS_JUNK);
    } else {
	param->decl_spec = *decl_spec;
    }

    if (declarator == NULL) {
	declarator = new_declarator("", "", 0L);
    }
    param->declarator = declarator;

    param->comment = NULL;
}

/* Free the storage used by the parameter.
 */
void
free_parameter (param)
Parameter *param;
{
    free_decl_spec(&(param->decl_spec));
    free_declarator(param->declarator);
    if (param->comment != NULL)
	free(param->comment);
}

/* Return TRUE if the parameter is void.
 */
boolean
is_void_parameter (p)
Parameter *p;
{
    return p == NULL || (strcmp(p->decl_spec.text, "void") == 0 &&
	strlen(p->declarator->text) == 0);
}

/* Initialize a list of function parameters.
 */
void
new_param_list (param_list, param)
ParameterList *param_list;
Parameter *param;
{
    Parameter *p;

    p = (Parameter *)xmalloc(sizeof(Parameter));
    *p = *param;
    
    param_list->first = param_list->last = p;
    p->next = NULL;

    param_list->begin_comment = param_list->end_comment = 0;
    param_list->comment = NULL;
}

/* Free storage used by the elements in the function parameter list.
 */
void
free_param_list (param_list)
ParameterList *param_list;
{
    Parameter *p, *next;

    p = param_list->first;
    while (p != NULL) {
	next = p->next;
	free_parameter(p);
	free(p);
	p = next;
    }

    if (param_list->comment != NULL)
	free(param_list->comment);
}

/* Add the function parameter declaration to the list.
 */
void
add_param_list (to, from, param)
ParameterList *to, *from;
Parameter *param;
{
    Parameter *p;

    p = (Parameter *)xmalloc(sizeof(Parameter));
    *p = *param;

    to->first = from->first;
    from->last->next = p;
    to->last = p;
    p->next = NULL;
}

/* Initialize an empty list of function parameter names.
 */
void
new_ident_list (param_list)
ParameterList *param_list;
{
    param_list->first = param_list->last = NULL;
    param_list->begin_comment = param_list->end_comment = 0;
    param_list->comment = NULL;
}

/* Add an item to the list of function parameter declarations but set only
 * the parameter name field.
 */
void
add_ident_list (to, from, name)
ParameterList *to, *from;
char *name;
{
    Parameter *p;
    Declarator *declarator;

    p = (Parameter *)xmalloc(sizeof(Parameter));
    declarator = new_declarator(name, name, 0L);
    new_parameter(p, NULL, declarator);

    to->first = from->first;
    if (to->first == NULL) {
	to->first = p;
    } else {
	from->last->next = p;
    }
    to->last = p;
    p->next = NULL;
}

/* Search the list of parameters for a matching parameter name.
 * Return a pointer to the matching parameter or NULL if not found.
 */
static Parameter *
search_parameter_list (params, name)
ParameterList *params;
char *name;
{
    Parameter *p;

    for (p = params->first; p != NULL; p = p->next) {
	if (strcmp(p->declarator->name, name) == 0)
	    return p;
    }
    return (Parameter *)NULL;
}

/* For each name in the declarator list <declarators>, set the declaration
 * specifier part of the parameter in <params> having the same name.
 * This is also where we promote formal parameters.  Parameters of type
 * "char", "unsigned char", "short", or "unsigned short" are promoted to
 * "int".  Parameters of type "float" are promoted to "double".
 */
void
set_param_types (params, decl_spec, declarators)
ParameterList *params;
DeclSpec *decl_spec;
DeclaratorList *declarators;
{
    Declarator *d;
    Parameter *p;
    char *decl_spec_text;

    for (d = declarators->first; d != NULL; d = d->next) {
	/* Search the parameter list for a matching name. */
	p = search_parameter_list(params, d->name);
	if (p == NULL) {
	    put_error();
	    fprintf(stderr, "declared argument \"%s\" is missing\n", d->name);
	} else {
	    decl_spec_text = decl_spec->text;
	    if (promote_param && strcmp(d->text, d->name) == 0) {
		if (decl_spec->flags & (DS_CHAR | DS_SHORT))
		    decl_spec_text = "int";
		else if (decl_spec->flags & DS_FLOAT)
		    decl_spec_text = "double";
	    }
	    free(p->decl_spec.text);
	    p->decl_spec.text = xstrdup(decl_spec_text);

	    free_declarator(p->declarator);
	    p->declarator = d;
	}
    }
}

static void put_declarator();

/* Output a function parameter.
 */
static void
put_parameter (outf, p)
FILE *outf;
Parameter *p;
{
    fputs(p->decl_spec.text, outf);
    if (strlen(p->declarator->text) > 0) {
	if (strcmp(p->declarator->text, "...") != 0) {
	    if (proto_style != PROTO_ABSTRACT || proto_comments ||
	     where != FUNC_PROTO ||
	     strcmp(p->declarator->text, p->declarator->name) != 0)
		fputc(' ', outf);
	}
	put_declarator(outf, p->declarator);
    }
}

/* Output a parameter list.
 */
static void
put_param_list (outf, declarator)
FILE *outf;
Declarator *declarator;
{
    Parameter *p;
    int f;

    p = declarator->params.first;
    if (is_void_parameter(p)) {
	if (p != NULL || ((where == FUNC_PROTO || where == FUNC_DEF) &&
	 declarator == func_declarator))
	    fputs("void", outf);
    } else {
	f = (declarator == func_declarator) ? format : FMT_OTHER;

	if (where == FUNC_DEF && declarator->params.comment != NULL)
	    fputs(declarator->params.comment, outf);
		
	fputs(fmt[f].first_param_prefix, outf);
	put_parameter(outf, p);

	while (p->next != NULL) {
	    fputc(',', outf);
	    if (where == FUNC_DEF && p->comment != NULL)
		fputs(p->comment, outf);

	    p = p->next;
	    fputs(fmt[f].middle_param_prefix, outf);
	    put_parameter(outf, p);
	}
	if (where == FUNC_DEF && p->comment != NULL)
	    fputs(p->comment, outf);

	fputs(fmt[f].last_param_suffix, outf);
    }
}

/* Output function parameters.
 */
static void
put_parameters (outf, declarator)
FILE *outf;
Declarator *declarator;
{
    Parameter *p;

    if (where == FUNC_DEF && func_style == FUNC_TRADITIONAL) {

	/* Output parameter name list for traditional function definition. */
	p = declarator->params.first;

	/* Output paramter name list only for head function declarator. */
	if (!is_void_parameter(p) && declarator == func_declarator) {
	    fputs(fmt[format].first_param_prefix, outf);
	    fputs(p->declarator->name, outf);
	    p = p->next;
	    while (p != NULL && strcmp(p->declarator->text, "...") != 0) {
		fputc(',', outf);
		fputs(fmt[format].middle_param_prefix, outf);
		fputs(p->declarator->name, outf);
		p = p->next;
	    }
	    fputs(fmt[format].last_param_suffix, outf);
	}
    } else {

	/* Output parameter type list. */
	if (where == FUNC_PROTO && proto_style == PROTO_TRADITIONAL &&
	 declarator == func_declarator) {
	    if (proto_comments) {
		fputs("/*", outf);
		put_param_list(outf, declarator);
		fputs("*/", outf);
	    }
	} else {
	    put_param_list(outf, declarator);
	}
    }
}

/* Output a function declarator.
 */
static void
put_func_declarator (outf, declarator)
FILE *outf;
Declarator *declarator;
{
    char *s, *t, *decl_text;
    int f;

    /* Output declarator text before function declarator place holder. */
    if ((s = strstr(declarator->text, "%s")) == NULL)
	return;
    *s = '\0';
    fputs(declarator->text, outf);

    /* Substitute place holder with function declarator. */
    if (declarator->func_stack->func_def == FUNC_NONE) {

	decl_text = declarator->func_stack->text;
	if (strlen(declarator->name) == 0) {
	    fputs(decl_text, outf);
	} else {

	    /* Output the declarator text before the declarator name. */
	    if ((t = strstr(decl_text, declarator->name)) == NULL)
		return;
	    *t = '\0';
	    fputs(decl_text, outf);
	    *t = declarator->name[0];

	    /* Output the declarator prefix before the name. */
	    f = (declarator == func_declarator) ? format : FMT_OTHER;
	    if (strcmp(fmt[f].declarator_prefix, " ") != 0)
		fputs(fmt[f].declarator_prefix, outf);

	    /* Output the declarator name. */
	    if (where == FUNC_PROTO && proto_style == PROTO_ABSTRACT &&
	     declarator != func_declarator) {
		if (proto_comments) {
		    fputs("/*", outf);
		    fputs(declarator->name, outf);
		    fputs("*/", outf);
		}
	    } else {
		fputs(declarator->name, outf);
	    }

	    /* Output the remaining declarator text. */
	    fputs(t + strlen(declarator->name), outf);

	    /* Output the declarator suffix. */
	    fputs(fmt[f].declarator_suffix, outf);
	}
    } else {
	put_func_declarator(outf, declarator->func_stack);
    }
    *s = '%';
    s += 2;

    /* Output declarator text up to but before parameters place holder. */
    if ((t = strstr(s, "()")) == NULL)
	return;
    *t = '\0';
    fputs(s, outf);

    if (where == FUNC_PROTO && proto_style == PROTO_MACRO &&
     declarator == func_declarator) {
	fprintf(outf, " %s(", macro_name);
    }

    /* Substitute place holder with function parameters. */
    fputc(*t++ = '(', outf);
    put_parameters(outf, declarator);
    fputs(t, outf);

    if (where == FUNC_PROTO && proto_style == PROTO_MACRO &&
     declarator == func_declarator) {
	fputc(')', outf);
    }
}

/* Output a declarator.
 */
static void
put_declarator (outf, declarator)
FILE *outf;
Declarator *declarator;
{
    char *s;

    if (declarator->func_def == FUNC_NONE) {
	if (where == FUNC_PROTO && proto_style == PROTO_ABSTRACT &&
	 strlen(declarator->name) > 0) {
	    if ((s = strstr(declarator->text, declarator->name)) == NULL)
		return;
	    *s = '\0';
	    if (proto_comments) {
		fprintf(outf, "%s/*%s*/%s", declarator->text, declarator->name,
		 s + strlen(declarator->name));
	    } else {
		fprintf(outf, "%s%s", declarator->text,
		 s + strlen(declarator->name));
	    }
	    *s = declarator->name[0];
	} else {
	    fputs(declarator->text, outf);
	}
    } else {
	put_func_declarator(outf, declarator);
    }
}

/* Output a declaration specifier for an external declaration.
 */
static void
put_decl_spec (outf, decl_spec)
FILE *outf;
DeclSpec *decl_spec;
{
    if (extern_out && (decl_spec->flags & DS_STATIC) == 0) {
	if (strstr(decl_spec->text, "extern") == NULL) {
	    fputs("extern ", outf);
	}
    }
    fputs(decl_spec->text, outf);
    fputc(' ', outf);
}

/* Generate variable declarations.
 */
void
gen_declarations (decl_spec, decl_list)
DeclSpec *decl_spec;		/* declaration specifier */
DeclaratorList *decl_list;	/* list of declared variables */
{
    Declarator *d;

    if (!variables_out || (decl_spec->flags & (DS_EXTERN|DS_JUNK)))
	return;
    if (!static_out && (decl_spec->flags & DS_STATIC))
	return;

    func_declarator = NULL;
    where = FUNC_OTHER;
    format = FMT_OTHER;
    for (d = decl_list->first; d != NULL; d = d->next) {
	if (d->func_def == FUNC_NONE) {
	    fputs(fmt[FMT_PROTO].decl_spec_prefix, stdout);
	    put_decl_spec(stdout, decl_spec);
	    put_declarator(stdout, d);
	    fputs(";\n", stdout);
	}
    }
}

/* If a parameter name appears in the parameter list of a traditional style
 * function definition but is not declared in the parameter declarations,
 * then assign it the default type "int".
 */
static void
set_param_decl_spec (declarator)
Declarator *declarator;
{
    Parameter *p;

    for (p = declarator->params.first; p != NULL; p = p->next) {
	if (strlen(p->decl_spec.text) == 0 &&
	    strcmp(p->declarator->text, "...") != 0) {
	    free(p->decl_spec.text);
	    p->decl_spec.text = xstrdup("int");
	}
    }
}

/* Generate a function prototype.
 */
void
gen_prototype (decl_spec, declarator)
DeclSpec *decl_spec;
Declarator *declarator;
{
    if (proto_style == PROTO_NONE || (decl_spec->flags & DS_JUNK))
	return;
    if (!static_out && (decl_spec->flags & DS_STATIC))
	return;

    func_declarator = declarator->head;
    set_param_decl_spec(func_declarator);

    where = FUNC_PROTO;
    format = FMT_PROTO;
    fputs(fmt[format].decl_spec_prefix, stdout);
    put_decl_spec(stdout, decl_spec);
    put_func_declarator(stdout, declarator);
    fputs(";\n", stdout);
}

/* Generate a declarator for a function pointer declarator or prototype.
 */
void
gen_func_declarator (declarator)
Declarator *declarator;
{
    /* Go to the beginning of the function declarator in the temporary
     * file and overwrite it with the converted declarator.
     */
    fseek(cur_tmp_file(), declarator->begin, 0);
    func_declarator = NULL;
    where = FUNC_DEF;
    format = FMT_FUNC;
    put_func_declarator(cur_tmp_file(), declarator);
    cur_file_changed();
}

/* Generate a function definition head.
 */
void
gen_func_definition (decl_spec, declarator)
DeclSpec *decl_spec;
Declarator *declarator;
{
    Parameter *p;
    ParameterList *params;
    char *comment;
    int comment_len, n;

    /* Return if the function is already defined in the desired style. */
    if (declarator->func_def == func_style)
	return;

    /* Save the text between the function head and the function body.
     * Read the temporary file from after the last ) or ; to the
     * end of the file.
     */
    comment_len = (int)(ftell(cur_tmp_file()) - cur_begin_comment());
    comment = xmalloc(comment_len);
    fseek(cur_tmp_file(), cur_begin_comment(), 0);
    fread(comment, sizeof(char), comment_len, cur_tmp_file());

    func_declarator = declarator->head;
    format = FMT_FUNC;

    /* Save the text before the parameter declarations. */
    if (func_style == FUNC_ANSI) {
	params = &func_declarator->params;
	n = (int)(params->end_comment - params->begin_comment);
	if (n > 0) {
	    params->comment = xmalloc(n+1);
	    fseek(cur_tmp_file(), params->begin_comment, 0);
	    fread(params->comment, sizeof(char), n, cur_tmp_file());
	    params->comment[n] = '\0';
	    format = FMT_FUNC_COMMENT;
	}
    }

    /* Get the parameter comments. */
    for (p = func_declarator->params.first; p != NULL; p = p->next) {
	n = (int)(p->declarator->end_comment - p->declarator->begin_comment);
	if (n > 0) {
	    p->comment = xmalloc(n+1);
	    fseek(cur_tmp_file(), p->declarator->begin_comment, 0);
	    fread(p->comment, sizeof(char), n, cur_tmp_file());
	    p->comment[n] = '\0';
	    format = FMT_FUNC_COMMENT;
	}
    }

    set_param_decl_spec(func_declarator);

    /* Go to the beginning of the function head in the temporary file
     * and overwrite it with the converted function head.
     */
    fseek(cur_tmp_file(), decl_spec->begin, 0);

    /* Output declarator specifiers. */
    fputs(fmt[format].decl_spec_prefix, cur_tmp_file());
    fputs(decl_spec->text, cur_tmp_file());
    fputc(' ', cur_tmp_file());

    /* Output function declarator. */
    where = FUNC_DEF;
    put_func_declarator(cur_tmp_file(), declarator);

    if (func_style == FUNC_TRADITIONAL) {
	/* Output traditional style parameter declarations. */
	p = func_declarator->params.first;
	if (!is_void_parameter(p)) {
	    fputc('\n', cur_tmp_file());
	    put_parameter(cur_tmp_file(), p);
	    fputc(';', cur_tmp_file());
	    p = p->next;
	    while (p != NULL && strcmp(p->declarator->text, "...") != 0) {
		fputc('\n', cur_tmp_file());
		put_parameter(cur_tmp_file(), p);
		fputc(';', cur_tmp_file());
		p = p->next;
	    }
	}
    }

    /* Output text between function head and body. */
    fwrite(comment, sizeof(char), comment_len, cur_tmp_file());
    free(comment);

    cur_file_changed();
}
