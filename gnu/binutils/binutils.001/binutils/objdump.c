/*
 * objdump
 * 
 * dump information about an object file.  Until there is other documentation,
 * refer to the manual page dump(1) in the system 5 program's reference manual
 */
#include <stdio.h>

#ifndef COFF_ENCAPSULATE
#include "a.out.gnu.h"
#else
#include "a.out.encap.h"
#endif

char *malloc();
int nsyms;
struct nlist *symtbl;
char *strtbl;
int strsize;

read_symbols (execp, f)
struct exec *execp;
FILE *f;
{
	int i;
	struct nlist *sp;
	if (symtbl)
		return;
	nsyms = execp->a_syms / sizeof (struct nlist);
	if (nsyms == 0)
		return;

	symtbl = (struct nlist *)malloc (nsyms * sizeof (struct nlist));
	if (symtbl == NULL) {
		fprintf (stderr, "can't malloc for %d symbols\n",
			 nsyms);
		exit (1);
	}

	fseek (f, N_STROFF(*execp), 0);
	if (fread ((char *)&strsize, sizeof strsize, 1, f) != 1) {
		fprintf (stderr, "can't get string table size\n");
		exit (1);
	}
	strtbl = malloc (strsize);
	if (strtbl == NULL) {
		fprintf (stderr, "can't malloc %d bytes for string table\n");
		exit (1);
	}
	fseek (f, N_STROFF (*execp), 0);
	if (fread (strtbl, 1, strsize, f) != strsize) {
		fprintf (stderr, "error reading string table\n");
		exit (1);
	}

	fseek (f, N_SYMOFF (*execp), 0);
	if (fread ((char *)symtbl, sizeof (struct nlist), nsyms, f) != nsyms) {
		fprintf (stderr, "error reading symbol table\n");
		exit (1);
	}

	for (i = 0, sp = symtbl; i < nsyms; i++, sp++) {
		if (sp->n_un.n_strx < 0 || sp->n_un.n_strx > strsize)
			sp->n_un.n_name = "<bad string table index>";
		else
			sp->n_un.n_name = strtbl + sp->n_un.n_strx;
	}
}

free_symbols ()
{
	if (symtbl)
		free (symtbl);
	symtbl = NULL;
	if (strtbl)
		free (strtbl);
	strtbl = NULL;
}


usage ()
{
	fprintf (stderr, "usage: dump [-h] [-n] [-r] [-t] obj ...\n");
	exit (1);
}

int hflag;
int nflag;
int rflag;
int tflag;

main (argc, argv)
char **argv;
{
	int c;
	extern char *optarg;
	extern int optind;
	int seenflag = 0;

	while ((c = getopt (argc, argv, "hnrt")) != EOF) {
		seenflag = 1;
		switch (c) {
		case 't': tflag = 1; break;
		case 'r': rflag = 1; break;
		case 'n': nflag = 1; break;
		case 'h': hflag = 1; break;
		default:
			usage ();
		}
	}

	if (seenflag == 0 || optind == argc)
		usage ();

	while (optind < argc)
		doit (argv[optind++]);
}

doit (name)
char *name;
{
	FILE *f;
	struct exec exec;
	printf ("%s:\n", name);
	f = fopen (name, "r");
	if (f == NULL) {
		fprintf (stderr, "can't open %s\n", name);
		return;
	}
#ifdef HEADER_SEEK
	HEADER_SEEK (f);
#endif
	if (fread ((char *)&exec, sizeof exec, 1, f) != 1) {
		fprintf (stderr, "can't read header for %s\n", name);
		return;
	}

	if (N_BADMAG (exec)) {
		fprintf (stderr, "%s is not an object file\n", name);
		return;
	}

	if (hflag)
		dump_header (&exec);

	if (nflag)
		dump_nstuff (&exec);

	if (tflag)
		dump_sym (&exec, f);

	if (rflag)
		dump_reloc (&exec, f);

	free_symbols ();

}

dump_header (execp)
struct exec *execp;
{
	int x;

	printf ("magic: 0x%x (%o) ", execp->a_magic, execp->a_magic & 0xffff);
#ifdef COFF_ENCAPSULATE
	printf ("machtype: %d ", execp->a_machtype);
	printf ("flags: 0x%x ", execp->a_flags);
#endif
	printf ("text 0x%x ", execp->a_text);
	printf ("data 0x%x ", execp->a_data);
	printf ("bss 0x%x\n", execp->a_bss);
	printf ("nsyms %d", execp->a_syms / sizeof (struct nlist));
	x = execp->a_syms % sizeof (struct nlist);
	if (x) 
		printf (" (+ %d bytes)", x);
	printf (" entry 0x%x ", execp->a_entry);
	printf ("trsize 0x%x ", execp->a_trsize);
	printf ("drsize 0x%x\n", execp->a_drsize);
}

dump_nstuff (execp)
struct exec *execp;
{
	printf ("N_BADMAG %d\n", N_BADMAG (*execp));
	printf ("N_TXTOFF 0x%x\n", N_TXTOFF (*execp));
	printf ("N_SYMOFF 0x%x\n", N_SYMOFF (*execp));
	printf ("N_STROFF 0x%x\n", N_STROFF (*execp));
	printf ("N_TXTADDR 0x%x\n", N_TXTADDR (*execp));
	printf ("N_DATADDR 0x%x\n", N_DATADDR (*execp));
}

dump_sym (execp, f)
struct exec *execp;
FILE *f;
{
	int i;
	struct nlist *sp;

	read_symbols (execp, f);
	if (nsyms == 0) {
		printf ("no symbols\n");
		return;
	}

	printf ("%3s: %4s %5s %4s %8s\n",
		"#", "type", "other", "desc", "val");
	for (i = 0, sp = symtbl; i < nsyms; i++, sp++) {
		printf ("%3d: %4x %5x %4x %8x %s\n",
			i, sp->n_type, sp->n_other, sp->n_desc, sp->n_value,
			sp->n_un.n_name);
	}
}

dump_reloc (execp, f)
struct exec *execp;
FILE *f;
{
	read_symbols (execp, f);
	if (execp->a_trsize) {
		printf ("text reloc\n");
		dump_reloc1 (execp, f, N_TRELOFF (*execp), execp->a_trsize);
	}
	if (execp->a_drsize) {
		printf ("data reloc\n");
		dump_reloc1 (execp, f, N_DRELOFF (*execp), execp->a_drsize);
	}
}

dump_reloc1 (execp, f, off, size)
struct exec *execp;
FILE *f;
{
	int nreloc;
	struct relocation_info reloc;
	int i;

	nreloc = size / sizeof (struct relocation_info);

	printf ("%3s: %3s %8s %4s\n", "#", "len", "adr", "sym");
	fseek (f, off, 0);
	for (i = 0; i < nreloc; i++) {
		if (fread ((char *)&reloc, sizeof reloc, 1, f) != 1) {
			fprintf (stderr, "error reading reloc\n");
			return;
		}
		printf ("%3d: %3d %8x ", i, 1 << reloc.r_length,
			reloc.r_address);
		
		if (reloc.r_extern) {
			printf ("%4d ", reloc.r_symbolnum);
			if (reloc.r_symbolnum < nsyms)
				printf ("%s ",
					symtbl[reloc.r_symbolnum].n_un.n_name);
		} else {
			printf ("     ");
			switch (reloc.r_symbolnum & ~N_EXT) {
			case N_TEXT: printf (".text "); break;
			case N_DATA: printf (".data "); break;
			case N_BSS: printf (".bss "); break;
			case N_ABS: printf (".abs "); break;
			default: printf ("base %x ", reloc.r_symbolnum); break;
			}
		}
		if (reloc.r_pcrel) printf ("PCREL ");
#if 0
		if (reloc.r_pad) printf ("PAD %x ", reloc.r_pad);
#endif
		printf ("\n");
	}
}

		
