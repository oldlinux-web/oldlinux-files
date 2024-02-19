/* $Id: semantic.h 3.2 92/03/06 00:54:38 cthuang Exp $
 *
 * Declarations of semantic action routines
 */

extern void new_decl_spec(/*
    DeclSpec *decl_spec, char *text, long offset, int flags*/);
extern void free_decl_spec(/*
    DeclSpec *decl_spec*/);
extern void join_decl_specs(/*
    DeclSpec *result, DeclSpec *a, DeclSpec *b*/);
extern void check_untagged(/*
    DeclSpec *decl_spec*/);
extern Declarator *new_declarator(/*
    char *text, char *name, long offset*/);
extern void free_declarator(/*
    Declarator *d*/);
extern void new_decl_list(/*
    DeclaratorList *decl_list, Declarator *declarator*/);
extern void free_decl_list(/*
    DeclaratorList *decl_list*/);
extern void add_decl_list(/*
    DeclaratorList *to, DeclaratorList *from, Declarator *declarator*/);
extern void new_parameter(/*
    Parameter *param, DeclSpec *decl_spec, Declarator *declarator*/);
extern void free_parameter(/*
    Parameter *param*/);
extern boolean is_void_parameter(/*
    Parameter *p*/);
extern void new_param_list(/*
    ParameterList *param_list, Parameter *param*/);
extern void free_param_list(/*
    ParameterList *param_list*/);
extern void add_param_list(/*
    ParameterList *to, ParameterList *from, Parameter *param*/);
extern void new_ident_list(/*
    ParameterList *param_list*/);
extern void add_ident_list(/*
    ParameterList *to, ParameterList *from, char *name*/);
extern void set_param_types(/*
    ParameterList *params, DeclSpec *decl_spec, DeclaratorList *declarators*/);
extern void gen_declarations(/*
    DeclSpec *decl_spec, DeclaratorList *decl_list*/);
extern void gen_prototype(/*
    DeclSpec *decl_spec, Declarator *declarator*/);
extern void gen_func_declarator(/*
    Declarator *declarator*/);
extern void gen_func_definition(/*
    DeclSpec *decl_spec, Declarator *declarator*/);
