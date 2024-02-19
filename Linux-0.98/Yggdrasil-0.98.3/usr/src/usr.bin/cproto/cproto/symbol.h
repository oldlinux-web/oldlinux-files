/* $Id: symbol.h 3.3 92/03/14 11:57:48 cthuang Exp $
 *
 * A symbol table is a collection of string identifiers stored in a
 * hash table.
 */
#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct symbol {
    struct symbol *next;	/* next symbol in list */
    char *name; 		/* name of symbol */
    unsigned short flags;	/* symbol attributes */
} Symbol;

/* The hash table length should be a prime number. */
#define SYM_MAX_HASH 251

typedef struct symbol_table {
    Symbol *bucket[SYM_MAX_HASH];	/* hash buckets */
} SymbolTable;

extern SymbolTable *new_symbol_table(); /* Create symbol table */
extern void free_symbol_table();	/* Destroy symbol table */
extern Symbol *find_symbol();		/* Lookup symbol name */
extern Symbol *new_symbol();		/* Define new symbol */

#endif
