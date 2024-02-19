/* $Id: symbol.c 3.1 92/03/03 10:43:52 cthuang Exp $
 *
 * Implements a symbol table abstract data type.
 */
#include <stdio.h>
#include "cproto.h"
#include "symbol.h"


/* Create a symbol table.
 * Return a pointer to the symbol table or NULL if an error occurs.
 */
SymbolTable *
new_symbol_table ()
{
    SymbolTable *symtab;
    int i;

    if ((symtab = (SymbolTable *)xmalloc(sizeof(SymbolTable))) != NULL) {
	for (i = 0; i < SYM_MAX_HASH; ++i)
	    symtab->bucket[i] = NULL;
    }
    return symtab;
}


/* Free the memory allocated to the symbol table.
 */
void
free_symbol_table (symtab)
SymbolTable *symtab;
{
    int i;
    Symbol *sym, *next;

    for (i = 0; i < SYM_MAX_HASH; ++i) {
	sym = symtab->bucket[i];
	while (sym != NULL) {
	    next = sym->next;
	    free(sym->name);
	    free(sym);
	    sym = next;
	}
    }
}


/* This is a simple hash function mapping a symbol name to a hash bucket. */

static unsigned
hash (name)
char *name;
{
    char *s;
    unsigned h;

    h = 0;
    s = name;
    while (*s != '\0')
	h = (h << 1) ^ *s++;
    return h % SYM_MAX_HASH;
}


/* Search the list of symbols <list> for the symbol <name>.
 * Return a pointer to the symbol or NULL if not found.
 */
static Symbol *
search_symbol_list (list, name)
Symbol *list;
char *name;
{
    Symbol *sym;

    for (sym = list; sym != NULL; sym = sym->next) {
	if (strcmp(sym->name, name) == 0)
	    return sym;
    }
    return NULL;
}


/* Look for symbol <name> in symbol table <symtab>.
 * Return a pointer to the symbol or NULL if not found.
 */
Symbol *
find_symbol (symtab, name)
SymbolTable *symtab;
char *name;
{
    return search_symbol_list(symtab->bucket[hash(name)], name);
}


/* If the symbol <name> does not already exist in symbol table <symtab>,
 * then add the symbol to the symbol table.
 * Return a pointer to the symbol or NULL on an error.
 */
Symbol *
new_symbol (symtab, name, flags)
SymbolTable *symtab;	/* symbol table */
char *name;		/* symbol name */
int flags;		/* symbol attributes */
{
    Symbol *sym;
    int i;

    if ((sym = find_symbol(symtab, name)) == NULL) {
	if ((sym = (Symbol *)xmalloc(sizeof(Symbol))) != NULL) {
	    sym->name = xstrdup(name);
	    sym->flags = flags;
	    i = hash(name);
	    sym->next = symtab->bucket[i];
	    symtab->bucket[i] = sym;
	}
    }
    return sym;
}
