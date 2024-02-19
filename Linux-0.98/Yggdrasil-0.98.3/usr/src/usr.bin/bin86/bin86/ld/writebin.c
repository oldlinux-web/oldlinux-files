/*
 *   bin86/ld/writebin.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

extern long text_base_address;
#define btextoffset text_base_address
static long bdataoffset;
#define page_size() 4096

/* writebin.c - write binary file for linker */

#ifdef A_OUT_H
# include A_OUT_H
#else
# ifdef BSD_A_OUT
#  include "bsd-a.out.h"
#  define A_MINHDR 0
#  define C_EXT N_EXT
#  define C_STAT 0
#  define n_was_name n_un.n_name
#  define n_was_numaux n_other
#  define n_was_other n_numaux
#  define n_was_sclass n_type
#  define n_was_strx n_un.n_strx
#  define n_was_type n_desc
# else /* not BSD_A_OUT */
#  include "a.out.h"		/* maybe local copy of <a.out.h> for X-link */
#  define n_was_name n_name
#  define n_was_numaux n_numaux
#  define n_was_other n_other
#  define n_was_sclass n_sclass
#  define n_was_strx n_value
#  define n_was_type n_type
# endif /* BSD_A_OUT */
#endif

#include "const.h"
#include "obj.h"
#include "type.h"
#undef EXTERN
#include "globvar.h"

#ifdef EDOS
# define FILEHEADERLENGTH 0
#endif
#ifdef MINIX
# ifdef BSD_A_OUT
#  define FILEHEADERLENGTH 48
# else
#  define FILEHEADERLENGTH A_MINHDR
				/* part of header not counted in offsets */
# endif
#endif
#define DPSEG 2

#define CM_MASK 0xC0
#define MODIFY_MASK 0x3F
#define S_MASK 0x04
#define OF_MASK 0x03

#define CM_SPECIAL 0
#define CM_ABSOLUTE 0x40
#define CM_OFFSET_RELOC 0x80
#define CM_SYMBOL_RELOC 0xC0

#define CM_EOT 0
#define CM_BYTE_SIZE 1
#define CM_WORD_SIZE 2
#define CM_LONG_SIZE 3
#define CM_1_SKIP 17
#define CM_2_SKIP 18
#define CM_4_SKIP 19
#define CM_0_SEG 32

#define ABS_TEXT_MAX 64

#define offsetof(struc, mem) ((int) &((struc *) 0)->mem)
#define memsizeof(struc, mem) sizeof(((struc *) 0)->mem)

PRIVATE bool_t bits32;		/* nonzero for 32-bit executable */
PRIVATE offset_t combase[NSEG];	/* bases of common parts of segments */
PRIVATE offset_t comsz[NSEG];	/* sizes of common parts of segments */
PRIVATE char curseg;		/* current segment, 0 to $F */
PRIVATE offset_t edataoffset;	/* end of data */
PRIVATE offset_t endoffset;	/* end of bss */
PRIVATE offset_t etextoffset;	/* end of text */
PRIVATE offset_t etextpadoff;	/* end of padded text */
#ifdef BSD_A_OUT
PRIVATE unsigned ndreloc;	/* number of data relocations */
#endif
PRIVATE unsigned nsym;		/* number of symbols written */
#ifdef BSD_A_OUT
PRIVATE unsigned ntreloc;	/* number of text relocations */
PRIVATE bool_t reloc_output;	/* nonzero to leave reloc info in output */
#endif
PRIVATE unsigned relocsize;	/* current relocation size 1, 2 or 4 */
PRIVATE offset_t segadj[NSEG];	/* adjusts (file offset - seg offset) */
				/* depends on zero init */
PRIVATE offset_t segbase[NSEG];	/* bases of data parts of segments */
PRIVATE char segboundary[9] = "__seg0DH";
				/* name of seg boundary __seg0DL to __segfCH */
PRIVATE offset_t segpos[NSEG];	/* segment positions for current module */
PRIVATE offset_t segsz[NSEG];	/* sizes of data parts of segments */
				/* depends on zero init */
PRIVATE bool_t sepid;		/* nonzero for separate I & D */
PRIVATE bool_t stripflag;	/* nonzero to strip symbols */
PRIVATE offset_t spos;		/* position in current seg */
PRIVATE bool_t uzp;		/* nonzero for unmapped zero page */

#ifdef EDOS
FORWARD unsigned binheaderlength P((char *commandname));
FORWARD char *idconvert P((struct entrylist *elptr, char *commandname));
#endif
FORWARD void linkmod P((struct modstruct *modptr));
FORWARD void linkrefs P((struct modstruct *modptr));
FORWARD void padmod P((struct modstruct *modptr));
FORWARD void setsym P((char *name, offset_t value));
FORWARD void symres P((char *name));
FORWARD void setseg P((unsigned newseg));
FORWARD void skip P((unsigned countsize));
#ifdef EDOS
FORWARD void writeheader P((char *commandname));
#else
FORWARD void writeheader P((void));
#endif
FORWARD void writenulls P((offset_t count));

/* link all symbols connected to entry symbols */

PUBLIC void linksyms(argreloc_output)
bool_pt argreloc_output;
{
    char needlink;
    struct entrylist *elptr;
    struct modstruct *modptr;
    struct symstruct *symptr;

#ifdef BSD_A_OUT
    reloc_output = argreloc_output;
    if (argreloc_output)
    {
	if (modfirst->modnext != NULL)
	    fatalerror("relocatable output only works for one input file");
	for (modptr = modfirst; modptr != NULL; modptr = modptr->modnext)
	    modptr->loadflag = TRUE;
	return;
    }
#endif
    if ((symptr = findsym("_main")) != NULL)
	entrysym(symptr);
    do
    {
	if ((elptr = entryfirst) == NULL)
	    fatalerror("no start symbol");
	for (modptr = modfirst; modptr != NULL; modptr = modptr->modnext)
	    modptr->loadflag = FALSE;
	for (; elptr != NULL; elptr = elptr->elnext)
	    linkrefs(elptr->elsymptr->modptr);
	if ((symptr = findsym("start")) != NULL ||
	    (symptr = findsym("crtso")) != NULL)
	    linkrefs(symptr->modptr);
	needlink = FALSE;
	{
	    struct redlist *prlptr;
	    struct redlist *rlptr;

	    for (rlptr = redfirst; rlptr != NULL;
		 rlptr = (prlptr = rlptr)->rlnext)
		if (rlptr->rlmodptr->loadflag &&
		    rlptr->rlmodptr->class > rlptr->rlsymptr->modptr->class)
		{
		    rlptr->rlsymptr->modptr = rlptr->rlmodptr;
		    rlptr->rlsymptr->value = rlptr->rlvalue;
		    if (rlptr == redfirst)
			redfirst = rlptr->rlnext;
		    else
			prlptr->rlnext = rlptr->rlnext;
		    needlink = TRUE;
		}
	}
    }
    while (needlink);
}

/* write binary file */

PUBLIC void writebin(outfilename, argsepid, argbits32, argstripflag, arguzp)
char *outfilename;
bool_pt argsepid;
bool_pt argbits32;
bool_pt argstripflag;
bool_pt arguzp;
{
    char buf4[4];
#ifdef EDOS
    char *commandname;
#endif
    char *cptr;
    struct nlist extsym;
    flags_t flags;
    struct modstruct *modptr;
    char seg;
    unsigned sizecount;
    offset_t tempoffset;
    struct symstruct *symptr;

    sepid = argsepid;
    bits32 = argbits32;
    stripflag = argstripflag;
#ifdef BSD_A_OUT
    uzp = arguzp && !reloc_output;
#else
    uzp = arguzp;
#endif
    if (uzp)
    {
	if (btextoffset == 0)
	    btextoffset = page_size();
	if (bdataoffset == 0 && sepid)
	    bdataoffset = page_size();
    }
#ifdef EDOS
    commandname = stralloc(outfilename);
    if ((cptr = strchr(commandname, ':')) != NULL)
	commandname = cptr + 1;
    if ((cptr = strrchr(commandname, '.')) != NULL)
	*cptr = 0;
#endif

    /* reserve special symbols use curseg to pass parameter to symres() */
    for (curseg = 0; curseg < NSEG; ++curseg)
    {
	segboundary[5] = hexdigit[curseg];	/* to __segX?H */
	segboundary[6] = 'D';
	symres(segboundary);	/* __segXDH */
	segboundary[7] = 'L';
	symres(segboundary);	/* __segXDL */
	segboundary[6] = 'C';
	symres(segboundary);	/* __segXCL */
	segboundary[7] = 'H';
	symres(segboundary);	/* __segXCH */
    }
#ifdef EDOS
    curseg = 0;			/* data seg, s.b. variable */
#else
    curseg = 3;
#endif
    symres("__edata");
    symres("__end");
    curseg = 0;			/* text seg, s.b. variable */
    symres("__etext");

    /* calculate segment and common sizes (sum over loaded modules) */
    /* use zero init of segsz[] */
    /* also relocate symbols relative to starts of their segments */
    for (modptr = modfirst; modptr != NULL; modptr = modptr->modnext)
	if (modptr->loadflag)
	{
	    register struct symstruct **symparray;
	    register struct symstruct *symptr;

	    for (symparray = modptr->symparray;
		 (symptr = *symparray) != NULL; ++symparray)
		if (symptr->modptr == modptr && !(symptr->flags & A_MASK))
		{
		    if (!(symptr->flags & (I_MASK | SA_MASK)))
		    {
			/* relocate by offset of module in segment later */
			/* relocate by offset of segment in memory special */
			/* symbols get relocated improperly */
			symptr->value += segsz[symptr->flags & SEGM_MASK];
		    }
		    else if (symptr->value == 0)
		    {
#ifdef BSD_A_OUT
			if (!reloc_output)
#endif
			    undefined(symptr->name);
		    }
		    else
		    {
#ifdef BSD_A_OUT
#if 0
			if (!reloc_output)
#else	
			if (!reloc_output || !(symptr->flags & I_MASK))
#endif			
#endif
			{
			    tempoffset = roundup(symptr->value, 4, offset_t);
			    /* temp kludge quad alignment for 386 */
			    symptr->value = comsz[seg = symptr->flags & SEGM_MASK];
			    comsz[seg] += tempoffset;
			}
			if (!(symptr->flags & SA_MASK))
			    symptr->flags |= C_MASK;
		    }
		}
	    for (seg = 0, cptr = modptr->segsize; seg < NSEG; ++seg)
	    {
		segsz[seg] += cntooffset(cptr,
			  sizecount = segsizecount((unsigned) seg, modptr));
#ifndef EDOS

		/* adjust sizes to even to get quad boundaries */
		/* this should be specifiable dynamically */
		segsz[seg] = roundup(segsz[seg], 4, offset_t);
		comsz[seg] = roundup(comsz[seg], 4, offset_t);
#endif
		cptr += sizecount;
	    }
	}

    /* calculate seg positions now their sizes are known */
    /* temp use fixed order 0D 0C 1D 1C 2D 2C ... */
    /* assume seg 0 is text and rest are data */
#ifdef EDOS
    if (btextoffset == 0)
	btextoffset = binheaderlength(commandname);
#endif
    segpos[0] = segbase[0] = spos = btextoffset;
    combase[0] = segbase[0] + segsz[0];
    segadj[1] = segadj[0] = -btextoffset;
    etextpadoff = etextoffset = combase[0] + comsz[0];
    if (sepid)
    {
	etextpadoff = roundup(etextoffset, 0x10, offset_t);
	segadj[1] += etextpadoff - bdataoffset;
    }
    else if (bdataoffset == 0)
	bdataoffset = etextpadoff;
    segpos[1] = segbase[1] = edataoffset = bdataoffset;
    combase[1] = segbase[1] + segsz[1];
    for (seg = 2; seg < NSEG; ++seg)
    {
	segpos[seg] = segbase[seg] = combase[seg - 1] + comsz[seg - 1];
	if (seg == DPSEG)
	{
	    /* temporarily have fixed DP seg */
	    /* adjust if nec so it only spans 1 page */
	    tempoffset = segsz[seg] + comsz[seg];
	    if (tempoffset > 0x100)
		fatalerror("direct page segment too large");
	    if ((((segbase[seg] + tempoffset) ^ segbase[seg]) & ~0xFF) != 0)
		segpos[seg] = segbase[seg] = (segbase[seg] + 0xFF) & ~0xFF;
	}
	combase[seg] = segbase[seg] + segsz[seg];
	segadj[seg] = segadj[seg - 1];
    }

    /* relocate symbols by offsets of segments in memory */
    for (modptr = modfirst; modptr != NULL; modptr = modptr->modnext)
	if (modptr->loadflag)
	{
	    register struct symstruct **symparray;
	    register struct symstruct *symptr;

	    for (symparray = modptr->symparray;
		 (symptr = *symparray) != NULL; ++symparray)
		if (symptr->modptr == modptr && !(symptr->flags & A_MASK))
		{
		    if (symptr->flags & (C_MASK | SA_MASK))
		    {
#ifdef BSD_A_OUT
#if 0
			if (!reloc_output)
#else	
			if (!reloc_output || !(symptr->flags & I_MASK))
#endif			
#endif
			    symptr->value += combase[symptr->flags & SEGM_MASK];
		    }
		    else
#ifdef BSD_A_OUT
		    if (!reloc_output || !(symptr->flags & I_MASK))
#endif
			symptr->value += segbase[symptr->flags & SEGM_MASK];
		}
	}

    /* adjust special symbols */
    for (seg = 0; seg < NSEG; ++seg)
    {
	if (segsz[seg] != 0)
	    /* only count data of nonzero length */
	    edataoffset = segbase[seg] + segsz[seg];
	segboundary[5] = hexdigit[seg];		/* to __segX?H */
	segboundary[6] = 'D';
	setsym(segboundary, (tempoffset = segbase[seg]) + segsz[seg]);
						/* __segXDH */
	segboundary[7] = 'L';
	setsym(segboundary, tempoffset);	/* __segXDL */
	segboundary[6] = 'C';
	setsym(segboundary, tempoffset = combase[seg]);
						/* __segXCL */
	segboundary[7] = 'H';
	setsym(segboundary, tempoffset + comsz[seg]);
						/* __segXCH */
    }
    setsym("__etext", etextoffset);
    setsym("__edata", edataoffset);
    setsym("__end", endoffset = combase[NSEG - 1] + comsz[NSEG - 1]);

    openout(outfilename);
#ifdef BSD_A_OUT
    if (reloc_output)
	seektrel(FILEHEADERLENGTH
		 + (long) (etextpadoff - btextoffset)
		 + (long) (edataoffset - bdataoffset));
#endif
#ifdef EDOS
    writeheader(commandname);
#else
    writeheader();
#endif
    for (modptr = modfirst; modptr != NULL; modptr = modptr->modnext)
	if (modptr->loadflag)
	{
	    linkmod(modptr);
	    padmod(modptr);
	}

    /* dump symbol table */
#ifdef MINIX
    if (!stripflag)
    {
#ifdef BSD_A_OUT
	unsigned stringoff;
#endif

	seekout(FILEHEADERLENGTH
		+ (long) (etextpadoff - btextoffset)
		+ (long) (edataoffset - bdataoffset)
#ifdef BSD_A_OUT
		+ ((long) ndreloc + ntreloc)
		  * (long) sizeof(struct relocation_info)
#endif
		);
	extsym.n_was_numaux = extsym.n_was_type = 0;
#ifdef BSD_A_OUT
	stringoff = 4;
#endif
	for (modptr = modfirst; modptr != NULL; modptr = modptr->modnext)
	    if (modptr->loadflag)
	    {
		register struct symstruct **symparray;
		register struct symstruct *symptr;

		for (symparray = modptr->symparray;
		     (symptr = *symparray) != NULL; ++symparray)
		    if (symptr->modptr == modptr)
		    {
#ifdef BSD_A_OUT
			offtocn((char *) &extsym.n_was_strx,
				(offset_t) stringoff, 4);
#else
			strncpy((char *) &extsym.n_was_name, symptr->name,
				sizeof extsym.n_was_name);
#endif
			u4cn((char *) &extsym.n_value, (u4_t) symptr->value,
			     sizeof extsym.n_value);
			if ((flags = symptr->flags) & A_MASK)
			    extsym.n_was_sclass = N_ABS;
			else if (flags & (E_MASK | I_MASK))
			    extsym.n_was_sclass = C_EXT;
			else
			    extsym.n_was_sclass = C_STAT;
			if (!(flags & I_MASK) ||
#ifdef BSD_A_OUT
			     !reloc_output &&
#endif
			     flags & C_MASK)
			    switch (flags & (A_MASK | SEGM_MASK))
			    {
			    case 0:
				extsym.n_was_sclass |= N_TEXT;
			    case A_MASK:
				break;
			    default:
				if (flags & (C_MASK | SA_MASK))
				    extsym.n_was_sclass |= N_BSS;
				else
				    extsym.n_was_sclass |= N_DATA;
				break;
			    }
			writeout((char *) &extsym, sizeof extsym);
			++nsym;
#ifdef BSD_A_OUT
			stringoff += strlen(symptr->name) + 1;
#endif
		    }
	    }
#ifdef BSD_A_OUT
	offtocn((char *) &extsym.n_was_strx, (offset_t) stringoff, 4);
	writeout((char *) &extsym.n_was_strx, 4);
	for (modptr = modfirst; modptr != NULL; modptr = modptr->modnext)
	    if (modptr->loadflag)
	    {
		register struct symstruct **symparray;
		register struct symstruct *symptr;

		for (symparray = modptr->symparray;
		     (symptr = *symparray) != NULL; ++symparray)
		    if (symptr->modptr == modptr)
			writeout(symptr->name, strlen(symptr->name) + 1);
	    }
#endif
	seekout((long) offsetof(struct exec, a_syms));
	u4cn(buf4, (u4_t) nsym * sizeof extsym,
	     memsizeof(struct exec, a_syms));
	writeout(buf4, memsizeof(struct exec, a_syms));
#ifdef BSD_A_OUT
	seekout((long) offsetof(struct exec, a_trsize));
	u4cn(buf4, (u4_t) ntreloc * sizeof(struct relocation_info),
	     memsizeof(struct exec, a_trsize));
	writeout(buf4, memsizeof(struct exec, a_trsize));
	seekout((long) offsetof(struct exec, a_drsize));
	u4cn(buf4, (u4_t) ndreloc * sizeof(struct relocation_info),
	     memsizeof(struct exec, a_drsize));
	writeout(buf4, memsizeof(struct exec, a_drsize));
#endif
    }
#endif /* MINIX */
    closeout();
#ifdef BSD_A_OUT
    if (!reloc_output)
#endif
	executable();
}

#ifdef EDOS

PRIVATE unsigned binheaderlength(commandname)
char *commandname;
{
    unsigned count;
    char *name;
    struct entrylist *elptr;
    struct symstruct *startptr;

    count = 2 + 2 + 1;		/* len len nul */
    startptr = findsym("start");
    for (elptr = entryfirst; elptr != NULL; elptr = elptr->elnext)
    {
	name = idconvert(elptr, commandname);
	count += strlen(name) + 1 + 2 + 1;	/* nul off flg */
	ourfree(name);
	if (startptr != NULL)
	    count += 6;		/* LBSR $xxxx and LBRA $xxxx */
    }
    return count;
}

/* convert name of symbol (entry) list element to a Basic identifier */
/* new name is built in storage obtained from stralloc() */
/* the special name  _main  is converted to the command name first */
/* copy upper case and numerals, convert lower case to upper, ignore rest */

PRIVATE char *idconvert(elptr, commandname)
struct entrylist *elptr;
char *commandname;
{
    char *name;
    char *newname;

    if (strcmp(name = elptr->elsymptr->name, "_main") == 0)
	name = commandname;
    newname = stralloc(name);
    {
	register char *t;
	register char *s;

	t = newname;
	s = name;
	do
	{
	    if (*s >= '0' && *s <= '9' || *s >= 'A' && *s <= 'Z')
		*t++ = *s;
	    if (*s >= 'a' && *s <= 'z')
		*t++ = *s + ('A' - 'a');
	}
	while (*s++);
	*t = 0;
    }
    if (*newname < 'A')		/* numeral or null */
	fatalerror("bad entry name");
    return newname;
}

#endif /* EDOS */

PRIVATE void linkmod(modptr)
struct modstruct *modptr;
{
    char buf[ABS_TEXT_MAX];
    int command;
    unsigned char modify;
    offset_t offset;
    int symbolnum;
    struct symstruct **symparray;
    struct symstruct *symptr;

    setseg(0);
    relocsize = 2;
    symparray = modptr->symparray;
    openin(modptr->filename);	/* does nothing if already open */
    seekin((long) modptr->textoffset);
    while (TRUE)
    {
	if ((command = readchar()) < 0)
	    prematureeof();
	modify = command & MODIFY_MASK;
	switch (command & CM_MASK)
	{
	case CM_SPECIAL:
	    switch (modify)
	    {
	    case CM_EOT:
		segpos[curseg] = spos;
		return;
	    case CM_BYTE_SIZE:
		relocsize = 1;
		break;
	    case CM_WORD_SIZE:
		relocsize = 2;
		break;
	    case CM_LONG_SIZE:
#ifdef LONG_OFFSETS
		relocsize = 4;
		break;
#else
		fatalerror("relocation by long offsets not implemented");
#endif
	    case CM_1_SKIP:
		skip(1);
		break;
	    case CM_2_SKIP:
		skip(2);
		break;
	    case CM_4_SKIP:
		skip(4);
		break;
	    default:
		if ((modify -= CM_0_SEG) >= NSEG)
		    inputerror("bad data in");
		setseg((unsigned) modify);
		break;
	    }
	    break;
	case CM_ABSOLUTE:
	    if (modify == 0)
		modify = ABS_TEXT_MAX;
	    readin(buf, (unsigned) modify);
	    writeout(buf, (unsigned) modify);
	    spos += (int) modify;
	    break;
	case CM_OFFSET_RELOC:
	    offset = readsize(relocsize);
	    if (modify & R_MASK)
		offset -= (spos + relocsize);
	    offtocn(buf, segbase[modify & SEGM_MASK] + offset, relocsize);
	    writeout(buf, relocsize);
#ifdef BSD_A_OUT
	    if (reloc_output)
	    {
		u4_t bitfield;

		if (curseg == 0)
		{
		    ++ntreloc;
		    offtocn(buf, spos, 4);
		    writetrel(buf, 4);
		}
		else
		{
		    ++ndreloc;
		    offtocn(buf, spos - segbase[1], 4);
		    writedrel(buf, 4);
		}
		if ((modify & SEGM_MASK) == 0)
		    bitfield = N_TEXT;
		else
		    bitfield = N_DATA;
		if (modify & R_MASK)
		    bitfield |= 1L << 24;
		if (relocsize == 2)
		    bitfield |= 1L << 25;
		else if (relocsize == 4)
		    bitfield |= 1L << 26;
		u4cn(buf, bitfield, 4);
		if (curseg == 0)
		    writetrel(buf, 4);
		else
		    writedrel(buf, 4);
	    }
#endif /* BSD_A_OUT */
	    spos += relocsize;
	    break;
	case CM_SYMBOL_RELOC:
	    symptr = symparray[symbolnum = readconvsize((unsigned)
					    (modify & S_MASK ? 2 : 1))];
	    offset = readconvsize((unsigned) modify & OF_MASK);
	    if (modify & R_MASK)
		offset -= (spos + relocsize);
#ifdef BSD_A_OUT
	    if (!reloc_output || !(symptr->flags & I_MASK))
#endif
		offset += symptr->value;	    
	    offtocn(buf, offset, relocsize);
	    writeout(buf, relocsize);
#ifdef BSD_A_OUT
	    if (reloc_output)
	    {
		u4_t bitfield;

		if (curseg == 0)
		{
		    ++ntreloc;
		    offtocn(buf, spos, 4);
		    writetrel(buf, 4);
		}
		else
		{
		    ++ndreloc;
		    offtocn(buf, spos - segbase[1], 4);
		    writedrel(buf, 4);
		}
		if (symptr->flags & I_MASK)
		    bitfield = (1L << 27) | symbolnum;
		else if ((symptr->flags & SEGM_MASK) == 0)
		    bitfield = N_TEXT;
		else if (symptr->flags & (C_MASK | SA_MASK))
		    bitfield = N_BSS;
		else
		    bitfield = N_DATA;
		if (modify & R_MASK)
		    bitfield |= 1L << 24;
		if (relocsize == 2)
		    bitfield |= 1L << 25;
		else if (relocsize == 4)
		    bitfield |= 1L << 26;
		u4cn(buf, bitfield, 4);
		if (curseg == 0)
		    writetrel(buf, 4);
		else
		    writedrel(buf, 4);
	    }
#endif /* BSD_A_OUT */
	    spos += relocsize;
	}
    }
}

PRIVATE void linkrefs(modptr)
struct modstruct *modptr;
{
    register struct symstruct **symparray;
    register struct symstruct *symptr;

    modptr->loadflag = TRUE;
    for (symparray = modptr->symparray;
	 (symptr = *symparray) != NULL; ++symparray)
	if (symptr->modptr->loadflag == FALSE)
	    linkrefs(symptr->modptr);
}

PRIVATE void padmod(modptr)
struct modstruct *modptr;
{
    offset_t count;
    char seg;
    offset_t size;
    unsigned sizecount;
    char *sizeptr;

    for (seg = 0, sizeptr = modptr->segsize; seg < NSEG; ++seg)
    {
	size = cntooffset(sizeptr,
			  sizecount = segsizecount((unsigned) seg, modptr));
	sizeptr += sizecount;
	if ((count = segpos[seg] - segbase[seg]) != size)
	    size_error(seg, count, size);

	/* pad to quad boundary */
	/* not padding in-between common areas which sometimes get into file */
	if ((size = roundup(segpos[seg], 4, offset_t) - segpos[seg]) != 0)
	{
	    setseg(seg);
	    writenulls(size);
	    segpos[seg] = spos;
	}
	segbase[seg] = segpos[seg];
    }
}

PRIVATE void setsym(name, value)
char *name;
offset_t value;
{
    struct symstruct *symptr;

#ifdef BSD_A_OUT
    if (!reloc_output)
#endif
	if ((symptr = findsym(name)) != NULL)
	    symptr->value = value;
}

PRIVATE void symres(name)
register char *name;
{
    register struct symstruct *symptr;

    if ((symptr = findsym(name)) != NULL)
    {
	if ((symptr->flags & SEGM_MASK) == SEGM_MASK)
	    symptr->flags &= ~SEGM_MASK | curseg;
	if (symptr->flags != (I_MASK | curseg) || symptr->value != 0)
	    reserved(name);
#ifdef BSD_A_OUT
	if (!reloc_output)
#endif
	    symptr->flags = E_MASK | curseg;	/* show defined, not common */
    }
}

/* set new segment */

PRIVATE void setseg(newseg)
unsigned newseg;
{
    if (newseg != curseg)
    {
	segpos[curseg] = spos;
	spos = segpos[curseg = newseg];
	seekout(FILEHEADERLENGTH + (long) spos + (long) segadj[curseg]);
    }
}

PRIVATE void skip(countsize)
unsigned countsize;
{
    writenulls((offset_t) readsize(countsize));
}

#ifdef EDOS

PRIVATE void writeheader(commandname)
char *commandname;
{
    char buf[MAX_OFFSET_SIZE];
    offset_t offset;
    unsigned headlength;
    char *name;
    struct entrylist *elptr;
    struct symstruct *startptr;

    headlength = binheaderlength(commandname);
    for (elptr = entryfirst; elptr != NULL; elptr = elptr->elnext)
	headlength -= 6;
    offset = headlength;
    startptr = findsym("start");
    offtocn(buf, edataoffset, 2);
    writeout(buf, 2);
    writechar(0xFF);		/* dummy data length 0xFFFF takes everything */
    writechar(0xFF);
    for (elptr = entryfirst; elptr != NULL; elptr = elptr->elnext)
    {
	name = idconvert(elptr, commandname);
	writeout(name, (unsigned) strlen(name) + 1);
	ourfree(name);
	offtocn(buf, startptr == NULL ? elptr->elsymptr->value : offset, 2);
	writeout(buf, 2);
	writechar(0x82);	/* 8 = set flags from here, 2 = cmd line */
	offset += 6;		/* LBSR $xxxx and LBRA $xxxx */
    }
    writechar(0);
    if (startptr != NULL)
    {
	offset = headlength + 3;	/* over 1st LBSR */
	for (elptr = entryfirst; elptr != NULL; elptr = elptr->elnext)
	{
	    writechar(0x17);	/* LBSR */
	    offtocn(buf, startptr->value - offset, 2);
	    writeout(buf, 2);
	    writechar(0x16);	/* LBRA */
	    offtocn(buf, elptr->elsymptr->value - offset - 3, 2);
	    writeout(buf, 2);
	    offset += 6;
	}
    }
}

#endif /* EDOS */

#ifdef MINIX

PRIVATE void writeheader()
{
    struct exec header;

    memset(&header, 0, sizeof header);
    header.a_magic[0] = A_MAGIC0;
    header.a_magic[1] = A_MAGIC1;
#ifdef BSD_A_OUT
    if (!reloc_output)
#endif
    {
	header.a_flags = sepid ? A_SEP : A_EXEC;
	if (uzp)
	    header.a_flags |= A_UZP;
    }
#ifdef BSD_A_OUT
    header.a_cpu = (bits32 || reloc_output) ? A_I80386 : A_I8086;
#else
    header.a_cpu = bits32 ? A_I80386 : A_I8086;
#endif
    header.a_hdrlen = FILEHEADERLENGTH;
    offtocn((char *) &header.a_text, etextpadoff - btextoffset,
	    sizeof header.a_text);
    offtocn((char *) &header.a_data, edataoffset - bdataoffset,
	    sizeof header.a_data);
    offtocn((char *) &header.a_bss, endoffset - edataoffset,
	    sizeof header.a_bss);
#ifdef BSD_A_OUT
    if (!reloc_output)
#endif
    {
	if (uzp)
	    offtocn((char *) &header.a_entry, page_size(),
		    sizeof header.a_entry);
	offtocn((char *) &header.a_total, (offset_t)
    	    (endoffset < 0x00010000L ? 0x00010000L : endoffset + 0x0008000L),
		sizeof header.a_total);
    }
    writeout((char *) &header, FILEHEADERLENGTH);
}

#endif /* MINIX */

PRIVATE void writenulls(count)
offset_t count;
{
    spos += count;
    while (count--)
	writechar(0);
}
