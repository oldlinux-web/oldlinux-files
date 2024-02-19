/*
 *   bin86/ld/dumps.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* dumps.c - print data about symbols and modules for linker */

#include "const.h"
#include "obj.h"
#include "type.h"
#include "globvar.h"

/* print list of modules and whether they are loaded */

PUBLIC void dumpmods()
{
    struct modstruct *modptr;

    for (modptr = modfirst; modptr != NULL; modptr = modptr->modnext)
    {
	putstr(modptr->loadflag ? "L " : "  ");
	putbstr(20, modptr->modname);
	putbyte('\n');
    }
}

/* print data about symbols (in loaded modules only) */

PUBLIC void dumpsyms()
{
    flags_t flags;
    struct modstruct *modptr;
    struct symstruct **symparray;
    struct symstruct *symptr;
    char uflag;

    for (modptr = modfirst; modptr != NULL; modptr = modptr->modnext)
	if (modptr->loadflag)
	{
	    for (symparray = modptr->symparray;
		 (symptr = *symparray) != NULL; ++symparray)
		if (symptr->modptr == modptr)
		{
		    uflag = FALSE;
		    if (((flags = symptr->flags) & (C_MASK | I_MASK)) == I_MASK)
			uflag = TRUE;
		    putbstr(20, uflag ? "" : modptr->modname);
		    putstr("  ");
		    putbstr(20, symptr->name);
		    putstr("  ");
		    putbyte(hexdigit[flags & SEGM_MASK]);
		    putstr("  ");
		    if (uflag)
			putstr("        ");
		    else
#ifdef LONG_OFFSETS
			put08lx(symptr->value);
#else
			put08x(symptr->value);
#endif
		    putstr(flags & A_MASK ? "  A" : "  R");
		    if (uflag)
			putstr(" U");
		    if (flags & C_MASK)
			putstr(" C");
		    if (flags & N_MASK)
			putstr(" N");
		    putbyte('\n');
		}
	}
}
