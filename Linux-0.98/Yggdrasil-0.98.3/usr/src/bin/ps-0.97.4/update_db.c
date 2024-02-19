/*
 * update_db.c	- create/update psdatabase
 *
 * Copyright (c) 1992 Branko Lankester
 *
 */
#include <stdio.h>
#include <a.out.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#ifdef VERSION_CHECK
#include <linux/utsname.h>
#define	utsname	new_utsname
#endif
#include "psdata.h"

#ifdef SYS_PATH
#undef SYS_PATH
#endif /* SYS_PATH */
#define	SYS_PATH	"/usr/src/linux/tools/system"


static update_psdb();
static read_nlist();
static write_tbl();
static addrcmp();

static struct nlist *namelist;
static int nsym;
static char *strings;
static int stringsize;


/*
 * Open systemfile, if update is non zero the psdatabase will be
 * updated from sysfile. If sysfile is NULL the system file that
 * was used to create the existing database will be used for
 * updating.
 */
open_sys(sysfile, update)
char *sysfile;
{

    if (sysfile == NULL || *sysfile == '\0') {
	if (open_psdb() == -1) {
#if 0
	    if (!update)
		return -1;
#else
	    read_nlist(SYS_PATH);
#endif
	    sysfile = SYS_PATH;
	} else
	    sysfile = db_hdr.sys_path;
    } else
	read_nlist(sysfile);

    if (update)
	update_psdb(sysfile);

    return(0);
}


static
update_psdb(syspath)
char *syspath;
{
    int fd, sysfd;
#ifdef VERSION_CHECK
    struct utsname uts;
#endif

    if (namelist == NULL)
	read_nlist(syspath);
    
    close_psdb();

    if ((fd = open(PSDATABASE, O_RDWR|O_TRUNC|O_CREAT, 0666)) == -1) {
	perror(PSDATABASE);
	exit(1);
    }
    if (*syspath != '/') {
	memset(db_hdr.sys_path, 0, sizeof db_hdr.sys_path);
	if (getcwd(db_hdr.sys_path, sizeof db_hdr.sys_path - 2) != -1)
	    strcat(db_hdr.sys_path, "/");
	strncat(db_hdr.sys_path, syspath, sizeof db_hdr.sys_path -
				strlen(db_hdr.sys_path) - 1);
    } else
	strncpy(db_hdr.sys_path, syspath, sizeof db_hdr.sys_path);
    strncpy(db_hdr.magic, PS_MAGIC, sizeof db_hdr.magic);

    strncpy(db_hdr.swap_path, swappath, sizeof db_hdr.swap_path);


    make_vartbl();
    make_fnctbl();

    write(fd, (char *) &db_hdr, sizeof db_hdr);
    write_tbl(fd, &db_hdr.vars, &vars);
    write_tbl(fd, &db_hdr.fncs, &fncs);

#ifdef VERSION_CHECK
    if ((sysfd = open(syspath, O_RDONLY)) == -1) {
	perror(syspath);
	exit(1);
    }
    lseek(sysfd, k_addr("_thisname") + 1024, SEEK_SET);
    read(sysfd, (char *) &uts, sizeof(struct utsname));
    close(sysfd);
    strncpy(db_hdr.uts_release, uts.release, sizeof db_hdr.uts_release);
    strncpy(db_hdr.uts_version, uts.version, sizeof db_hdr.uts_version);
#endif

    lseek(fd, 0L, SEEK_SET);
    write(fd, (char *) &db_hdr, sizeof db_hdr);
    close(fd);

    free(namelist);
    namelist = NULL;
    return(0);
}

static
read_nlist(systemfile)
char *systemfile;
{
    int fd;
    struct exec hdr;
    unsigned symsize, size;

    if ((fd = open(systemfile, O_RDONLY)) < 0) {
	perror(systemfile);
	exit(1);
    }
    if (read(fd, (char *) &hdr, sizeof(hdr)) != sizeof(hdr)) {
	perror(systemfile);
	exit(1);
    }
    if (N_BADMAG(hdr)) {
	fprintf(stderr, "%s: bad magic number\n", systemfile);
	exit(1);
    }
    if (N_STROFF(hdr) == 0) {
	fprintf(stderr, "%s has no symbols\n", systemfile);
	exit(1);
    }
    lseek(fd, N_STROFF(hdr), SEEK_SET);
    read(fd, (char *) &stringsize, sizeof(stringsize));
    symsize = N_STROFF(hdr) - N_SYMOFF(hdr);
    size = symsize + stringsize;
    namelist = (struct nlist *) xmalloc(size);
    lseek(fd, N_SYMOFF(hdr), SEEK_SET);
    if (read(fd, (char *) namelist, size) != size) {
	perror(systemfile);
	exit(1);
    }
    close(fd);

    strings = ((char *) namelist) + symsize;
    nsym = symsize / sizeof(struct nlist);
    if (Debug > 1)
	fprintf(stderr, "read %d symbols from %s\n", nsym, systemfile);
    return(0);
}


/*
 * make list of all text symbols, sorted on address for easy
 * lookup of wait channel.
 */
make_fnctbl()
{
    int i;
    struct sym_s *vp, *fp;

    fp= fncs.tbl= (struct sym_s *) xmalloc(nsym * sizeof(struct sym_s));
    fncs.strings = strings;
    for (i = 0; i < nsym; ++i) {
	if ((namelist[i].n_type & ~N_EXT) == N_TEXT && 
		!strchr(strings + namelist[i].n_un.n_strx, '.')) {
	    fp->addr = namelist[i].n_value;
	    fp->name = namelist[i].n_un.n_strx;
	    ++fp;
	}
    }
    fncs.nsym = fp - fncs.tbl;

    if (Debug > 1)
	fprintf(stderr, "%d text symbols\n", fncs.nsym);
    qsort(fncs.tbl, fncs.nsym, sizeof(struct sym_s), addrcmp);
    if (Debug > 1)
	for (i = 1; i<fncs.nsym; ++i)
	    if (fncs.tbl[i].addr == fncs.tbl[i-1].addr)
		printf("text symbols %s and %s both have address %x\n",
		    	strings + fncs.tbl[i-1].name,
			strings + fncs.tbl[i].name, fncs.tbl[i].addr);
    if (Debug == 3)
	dump_tbl(&fncs);
    return(0);
}


make_vartbl()
{
    int i;
    struct sym_s *vp;

    vp= vars.tbl= (struct sym_s *) xmalloc(nsym * sizeof(struct sym_s));
    vars.strings = strings;
    for (i = 0; i < nsym; ++i) {
	int typ = namelist[i].n_type & ~N_EXT;

	if (typ == N_DATA || typ == N_BSS) {
	    vp->addr = namelist[i].n_value;
	    vp->name = namelist[i].n_un.n_strx;
	    ++vp;
	}
    }
    vars.nsym = vp - vars.tbl;

    if (Debug > 1)
	fprintf(stderr, "%d data/bss symbols\n", vars.nsym);

    qsort(vars.tbl, vars.nsym, sizeof(struct sym_s), varcmp);
    if (Debug == 3)
	dump_tbl(&vars);
    return(0);
}


/*
 * write table tbl to descriptor fd, header structure dbtdl is updated
 */
static
write_tbl(fd, dbtbl, tbl)
int fd;
struct dbtbl_s *dbtbl;
struct tbl_s *tbl;
{
    int i;
    struct sym_s *p;
    char *s;
    int strsize, symsize;

    dbtbl->off = lseek(fd, 0L, SEEK_CUR);
    s= tbl->strings= xmalloc(stringsize);
    for (i = tbl->nsym, p = tbl->tbl; i--; ) {
	strcpy(s, strings + p->name);
	p->name = s - tbl->strings;
	++p;
	s += strlen(s) + 1;
    }
    symsize = tbl->nsym * sizeof(struct sym_s);
    if (write(fd, (char *) tbl->tbl, symsize) != symsize)
	return -1;
    strsize = (s - tbl->strings + 3) & ~3;
    if (write(fd, tbl->strings, strsize) != strsize)
	return -1;
    
    dbtbl->size = strsize + symsize;
    dbtbl->nsym = tbl->nsym;

    return(0);
}

/*
 * fncs are sorted on address
 */
static int
addrcmp(p1, p2)
struct sym_s *p1, *p2;
{
    return(p1->addr - p2->addr);
}
