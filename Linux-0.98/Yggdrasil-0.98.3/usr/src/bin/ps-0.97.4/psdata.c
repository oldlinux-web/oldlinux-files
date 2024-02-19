/*
 * psdata.c	- get info from psdatabase
 *
 * Copyright (c) 1992 Branko Lankester
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef VERSION_CHECK
#include <linux/utsname.h>
#define utsname	new_utsname
#endif
#include "psdata.h"

/*
 * define NO_UPDATE if you want to use only the psdatabase
 * for symbol info, code in update_db.c is then not needed.
 * Open psdatabase with open_psdb().
 *
 * Without NO_UPDATE defined the program can specify a different 
 * file for namelist, and it can update/create the psdatabase.
 * Open the systemfile with open_sys(sysfile, update), if sysfile
 * is NULL the psdatabase will still be used, if it exists.
 */

static read_tbl();
char *xmalloc();
unsigned long k_addr();

struct tbl_s vars, fncs;

int psdb = -1;
struct psdb_hdr db_hdr;

char *swappath = SWAPPATH;

#ifdef DEBUG
int Debug = 1;
#endif

open_psdb()
{
#ifdef VERSION_CHECK
    struct utsname uts;
#endif

    if ((psdb = open(PSDATABASE, O_RDONLY)) == -1)
	return -1;
    if (read(psdb, (char *) &db_hdr, sizeof(db_hdr)) != sizeof(db_hdr))
	return -1;
    
    if (strncmp(db_hdr.magic, PS_MAGIC, sizeof(db_hdr.magic))) {
	fprintf(stderr, "invalid psdatabase\n");
	return -1;
    }
    swappath = db_hdr.swap_path;
    
    if (Debug > 1) {
	fprintf(stderr, "psdatabase is from %s  %8.8s %-8.8s\n",
		db_hdr.sys_path, db_hdr.uts_release, db_hdr.uts_version);
	fprintf(stderr, "swap file: %s\n", db_hdr.swap_path);
	fprintf(stderr, "%d bss/data symbols\n%d text symbols\n",
		db_hdr.vars.nsym, db_hdr.fncs.nsym);
    }
#ifdef VERSION_CHECK
    kmemread(&uts, k_addr("_thisname"), sizeof uts);
    if (strncmp(uts.release, db_hdr.uts_release, sizeof db_hdr.uts_release) ||
	strncmp(uts.version, db_hdr.uts_version, sizeof db_hdr.uts_version)) {
	fprintf(stderr, "psdatabase is out of date, run 'ps -U'\n\n\
NOTE: You may have to recompile \"ps\" for this version of Linux\n\n");
	return -1;
    }
#endif
    return(0);
}

close_psdb()
{
    if (psdb != -1)
	close(psdb);
    psdb = -1;
}

/*
 * get address of data symbol
 */
unsigned long
k_addr(sym)
char *sym;
{
    struct sym_s *p, key;

    if (vars.tbl == NULL)
#ifndef NO_UPDATE
	if (psdb == -1) make_vartbl(); else
#endif
	    read_tbl(&db_hdr.vars, &vars);

    key.name = sym - vars.strings;
    p = (struct sym_s *) bsearch(&key, vars.tbl, vars.nsym,
				sizeof(struct sym_s), varcmp);
    if (Debug && p == NULL)
	fprintf(stderr, "symbol '%s' not found\n", sym);
    return(p ? p->addr : -1);
}


/*
 * find the name of a function on the value of a saved instruction ptr
 */
char *
find_func(eip)
unsigned long eip;
{
    int n;
    struct sym_s *p;
    char *s;

    if (fncs.tbl == NULL)
#ifndef NO_UPDATE
	if (psdb == -1) make_fnctbl(); else
#endif
	    read_tbl(&db_hdr.fncs, &fncs);

    p = fncs.tbl;
    n = fncs.nsym;
    while (n) {
	int i = n / 2;
	if (p[i].addr < eip) {
	    p = &p[i+1];
	    if (p->addr > eip) {
		--p;
		break;
	    }
	    --n;
	}
	n /= 2;
    }
    s = p->name + fncs.strings;
    return(*s == '_' ? s+1 : s);
}

static
read_tbl(dbtbl, tbl)
struct dbtbl_s *dbtbl;
struct tbl_s *tbl;
{
    lseek(psdb, dbtbl->off, SEEK_SET);
    tbl->tbl = (struct sym_s *) xmalloc(dbtbl->size);
    if (read(psdb, (char *) tbl->tbl, dbtbl->size) != dbtbl->size) {
	perror(PSDATABASE);
	exit(1);
    }
    tbl->nsym = dbtbl->nsym;
    tbl->strings = (char *) (tbl->tbl + tbl->nsym);
    if (Debug == 3) {
	fprintf(stderr, "read table from psdatabase\n");
	dump_tbl(tbl);
    }
    return(0);
}


/*
 * vars are sorted on name
 */
varcmp(p1, p2)
struct sym_s *p1, *p2;
{
    return(strcmp(vars.strings + p1->name, vars.strings + p2->name));
}


dump_tbl(tbl)
struct tbl_s *tbl;
{
    int i;
    if (tbl == &vars)
	fprintf(stderr, "vars table:\n");
    if (tbl == &fncs)
	fprintf(stderr, "fncs table:\n");
    fprintf(stderr, "nsym = %d   (tbl @0x%x, strings @0x%x)\n",
		tbl->nsym, tbl->tbl, tbl->strings);
    for (i = 0; i<tbl->nsym; ++i)
	fprintf(stderr, "%8x   %s\n", tbl->tbl[i].addr,
		    tbl->strings + tbl->tbl[i].name);
}


/*
 * misc stuff needed
 */

char *
xmalloc(size)
{
    char *p;

    if (size == 0) ++size;
    if ((p = (char *) malloc(size)) == NULL) {
	perror("xmalloc");
	exit(1);
    }
    return(p);
}



char *kmem_path = "/dev/kmem";
int kmemfd = -1;

kmemread(buf, addr, size)
char *buf;
unsigned long addr;
int size;
{
    int n;

    if (kmemfd == -1 && (kmemfd = open(kmem_path, O_RDONLY)) == -1) {
	perror(kmem_path);
	exit(1);
    }

    if (addr == (unsigned) -1) {	/* symbol not found */
	fprintf(stderr, "trying to read invalid address\n");
	exit(1);
    }
    if (lseek(kmemfd, addr, SEEK_SET) == -1) {
	perror("lseek kmem");
	exit(1);
    }
    if ((n = read(kmemfd, buf, size)) != size) {
	fprintf(stderr, "error reading kmem (offset = 0x%x)\n", addr);
	if (n == -1)
	    perror("");
	exit(1);
    }
}

unsigned long
get_kword(addr)
unsigned long addr;
{
    unsigned long w;

    kmemread(&w, addr, sizeof w);
    return(w);
}



#define	MEM_PATH	"/dev/mem"
int memfd = -1;

memread(buf, addr, size)
char *buf;
unsigned long addr;
int size;
{
    int n;

    if (memfd == -1 && (memfd = open(MEM_PATH, O_RDONLY)) == -1) {
	perror(MEM_PATH);
	exit(1);
    }

    lseek(memfd, addr, SEEK_SET);
    if ((n = read(memfd, buf, size)) == -1) {
	perror("cannot read mem");
	exit(1);
    }
    return(n);
}
