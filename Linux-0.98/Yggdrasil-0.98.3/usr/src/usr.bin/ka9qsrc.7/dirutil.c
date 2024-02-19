/* dirutil.c - MS-DOS directory reading routines
 *
 * Bdale Garbee, N3EUA, Dave Trulli, NN2Z, and Phil Karn, KA9Q
 * Directory sorting by Mike Chepponis, K3MC
 * adapted for ATARI ST & cleaned up by Rob Janssen, PE1CHL
 * adapted for Lattice C by Walter Doerr, DG2KK
 */

#include <stdio.h>

#ifndef LATTICE
#include <ctype.h>		/* DG2KK: force Lattice to use library func. */
#endif

#ifdef LATTICE
#define	S_IJHID		2
#define S_IJSYS		4
#define S_IJDIR		0x10
struct stat {
	char st_mode;			/* that's all we need */
};	

#else						/* not LATTICE */

#ifdef __TURBOC__
#include <dos.h>
#include <sys\stat.h>
#else						/* not LATTICE and not TURBOC */
#include <stat.h>
#endif
#endif

#ifdef MSDOS
#define  IS_ERROR	== -1		/* error return from MSDOS */
#endif

#ifdef __TURBOC__
#define ST_RDONLY	0x01		/* Read only attribute */
#define ST_HIDDEN	0x02		/* Hidden file */
#define ST_SYSTEM	0x04		/* System file */
#define ST_LABEL	0x08		/* Volume label */
#define ST_DIRECT	0x10		/* Directory */
#define ST_ARCH		0x20		/* Archive */

int dos(unsigned ah,
	unsigned bx,
	unsigned cx,
	void *dx,
	unsigned si,
	unsigned di)
{
	union REGS regs;

	regs.h.ah = ah;
	regs.x.bx = bx;
	regs.x.cx = cx;
	regs.x.dx = dx;
	regs.x.si = si;
	regs.x.di = di;

	intdos(&regs, &regs);

	if (regs.x.cflag) return -1;
	return 0;
}

#define bdos(x, y)	bdos(x, (unsigned) y, 0)

#endif  /* TURBOC */


#ifdef ATARI_ST
#define  IS_ERROR	< 0		/* error return from gemdos */
#include <osbind.h>			/* os interface defines */
#define  bdos		gemdos		/* Atari OS call */
#define  dos(a,b,c,d,e,f) gemdos(a,d,c) /* only valid for FIND func */
#define  st_attr	st_mode 	/* what's in a name? */
#define  ST_HIDDEN	S_IJHID 	/* Hidden from search */
#define  ST_SYSTEM	S_IJSYS 	/* System, hidden from search */
#define  ST_DIRECT	S_IJDIR 	/* Directory */
#endif

#include "global.h"

#ifndef FALSE
#define FALSE	(0)
#endif

#ifndef TRUE
#define TRUE	!(FALSE)
#endif

#define REGFILE (ST_HIDDEN|ST_SYSTEM|ST_DIRECT)
#define SET_DTA 	0x1a
#define FIND_FIRST	0x4e
#define FIND_NEXT	0x4f

struct dirent {
	char rsvd[21];
	char attr;
	short ftime;
	short fdate;
	long fsize;
	char fname[13];
};
#define NULLENT (struct dirent *)0

struct dirsort {
	struct dirsort *prev;
	struct dirsort *next;
	struct dirent *direntry;
};
#define NULLSORT (struct dirsort *)0

/* Create a directory listing in a temp file and return the resulting file
 * descriptor. If full == 1, give a full listing; else return just a list
 * of names.
 */
FILE *
dir(path,full)
char *path;
int full;
{
	FILE *fp,*tmpfile();

	if ((fp = tmpfile()) != NULLFILE)
	{
		getdir(path,full,fp);
		/* This should be rewind(), but Aztec doesn't have it */
#if	(ATARI_ST && MWC)
		vseek(fp,0L,0);
#else
		fseek(fp,0L,0);
#endif
	}
	return fp;
}

/* wildcard filename lookup */
filedir(name,times,ret_str)
char *name;
int times;
char *ret_str;
{
	register char *cp,*cp1;
	static struct dirent sbuf;

	bdos(SET_DTA,&sbuf);	/* Set disk transfer address */

	/* Find matching file */
	if(dos(times == 0 ? FIND_FIRST:FIND_NEXT,0,REGFILE,name,0,0) IS_ERROR) 
		sbuf.fname[0] = '\0';

	/* Copy result to output, forcing to lower case */
	for(cp = ret_str,cp1 = sbuf.fname; cp1 < &sbuf.fname[13] && *cp1 != '\0';)
		*cp++ = (char)tolower(*cp1++);	/* DG2KK: added (char) */
	*cp = '\0';
}

/* Change working directory */
docd(argc,argv)
int argc;
char *argv[];
{
	char dirname[128];
#ifdef MSDOS
	char *getcwd();
#endif

	if(argc > 1){
		if(chdir(argv[1]) IS_ERROR){
			printf("Can't change directory\n");
			return 1;
		}
	}

#ifdef MSDOS
	if(getcwd(dirname,0) != NULLCHAR){
		printf("\\%s\n",dirname);
	}
#endif
#ifdef ATARI_ST
	if (Dgetpath(dirname,0) == 0){
		printf("%c:%s%s\n",(char) Dgetdrv()+'A',
			(*dirname? "" : "\\"),dirname);
	}
#endif
	return 0;
}

/* List directory to console. [-/]w option selects "wide" format */
dodir(argc,argv)
int argc;
char *argv[];
{
	char *path;
	int full = 1;

	if (argc > 1 &&
	    (argv[1][0] == '-' || argv[1][0] == '/') && argv[1][1] == 'w')
	{
		full = -1;
		argv++;
		argc--;
	}

	if(argc >= 2){
		path = argv[1];
	} else {
		path = "*.*";
	}
	getdir(path,full,stdout);
	return 0;
}

/* do a directory list to the stream 
 * full = 0 -> short form, 1 is long, -1 is multi-column short
*/
static
getdir(path,full,file)
char *path;
int full;
FILE *file;
{
	struct dirent sbuf;
	struct stat statbuf;
	char *cp,*cp1;				/* !!!!!!! was: register */
	char dirtmp[20];
	int command = FIND_FIRST;
	int i = 0;
	int cflag = 0;
	int n = 0;
	char line_buf[50];		/* for long dirlist */
	
	struct dirsort *head, *here, *new;
	struct dirent *de;
	void dir_sort(), format_dir(), format_fname(), diskfree(), free_clist();
	int malloc_lost = FALSE;

	/* Root directory is a special case */
	if(path == NULLCHAR || *path == '\0' || strcmp(path,"\\") == 0)
		path = "\\*.*";

	/* If arg is a directory, append "\*.*" to it.
	 * This is tricky, since the "stat" system call actually
	 * calls the DOS "find matching file" function. The stat
	 * call therefore returns the attributes for the first matching
	 * entry in the directory. If the arg already ends in *.*,
	 * stat will match the . entry in the directory and indicate
	 * that the argument is a valid directory name. Hence the
	 * heuristic check for '*' in the file name. Kludge...
	 */
#ifdef __TURBOC__
	else if(stat(path,&statbuf) != -1
	 && (statbuf.st_mode & S_IFDIR)
	 && index(path,'*') == NULLCHAR) {
#else
	else if(index(path,'*') == NULLCHAR
	 && stat(path,&statbuf) != -1
	 && (statbuf.st_attr & ST_DIRECT)) {
#endif
		if((cp = malloc(strlen(path) + 10)) == NULLCHAR)
			return -1;
		sprintf(cp,"%s%c%s",path,'\\',"*.*");
		path = cp;
		cflag = 1;
	}
	head = NULLSORT;	/* No head of chain yet... */
	for(;;){
		bdos(SET_DTA,&sbuf);	/* Set disk transfer address */
		if(dos(command, 0, REGFILE, path, 0, 0) IS_ERROR)
			break;

		command = FIND_NEXT;	/* Got first one already... */
		if(sbuf.fname[0] != '.'){
			/* nuke "." and ".." */
			n++;	/* One more entry */
			new = (struct dirsort *) malloc(sizeof(struct dirsort));
			if(new == NULLSORT)
				malloc_lost = TRUE;
			de  = (struct dirent *)malloc(sizeof(struct dirent));
			if(de == NULLENT)
				malloc_lost = TRUE;
			if(malloc_lost){
				/* Clean up and call other routine */
				if(new)free(new);
				free_clist(head);
				return getdir_nosort(path,full,file);
			}
			*de = sbuf;	/* Copy contents of directory entry struct */

			/* Fix up names for easier sorting... pain! */
			strcpy(de->fname,"           ");	/* 11 blanks */
			cp  = sbuf.fname;
			cp1 = de->fname;

			do *cp1++ = *cp++; while (*cp && *cp != '.');

			if(*cp++){
				/* If there is an extension */
				cp1 = &(de->fname[8]);
				do *cp1++ = *cp++; while (*cp);
			}
			if(!(int)head){
				/* Make the first one */
				here = head = new;
				head->prev = head->next = NULLSORT;
			} else {
				/* Link on next one */
				new->next = NULLSORT;
				new->prev = here;
				here->next = new;
				here = new;
			}
			new->direntry = de;
		} /* IF on "." */
	} /* infinite FOR loop */

	if(head)
		dir_sort(head); 	/* Make a nice, sorted list */

	here = head;
	if(here)
	    if(full > 0){
		do {
			format_dir(line_buf,here->direntry);
			fprintf(file,"%s%s",line_buf,(i^=1) ? "   " : "\r\n");
		} while (here = here->next);
		if(i & 1)
			fprintf(file,"\r\n");
	    }
	    else {
		/* This is the short form */
		do {
			format_fname(dirtmp,here->direntry->fname,
					    here->direntry->attr);
			fprintf(file,"%-15s%s",dirtmp,((full && ++i % 5)?"":"\r\n"));
		} while (here = here->next);
		if(full && i % 5)
			fprintf(file,"\r\n");
	    }

	/* Give back all the memory we temporarily needed... */
	free_clist(head);

	if(full > 0){
		/* Provide additional information only on DIR */

		if (isalpha(*path) && path[1] == ':')	/* detect A: drivespec */
			diskfree(file,*path & 0x1f,n);
		else
			diskfree(file,0,n);
	}
	if(cflag)
		free(path);
	return 0;
} /* getdir */

static
getdir_nosort(path,full,file)
char *path;
int full;
FILE *file;
{
	struct dirent sbuf;
	struct stat statbuf;
	register char *cp;
	char dirtmp[20];
	int command = FIND_FIRST;
	int i = 0;
	int cflag = 0;

	char	line_buf[50];		/* for long dirlist */
	
	void format_fname(),format_dir(),diskfree();
	int n = 0;	/* Number of directory entries */
	
	/* Root directory is a special case */
	if(path == NULLCHAR || *path == '\0' || strcmp(path,"\\") == 0)
		path = "\\*.*";

	/* If arg is a directory, append "\*.*" to it.
	 * This is tricky, since the "stat" system call actually
	 * calls the DOS "find matching file" function. The stat
	 * call therefore returns the attributes for the first matching
	 * entry in the directory. If the arg already ends in *.*,
	 * stat will match the . entry in the directory and indicate
	 * that the argument is a valid directory name. Hence the
	 * heuristic check for '*' in the file name. Kludge...
	 */
#ifdef __TURBOC__
	else if(stat(path,&statbuf) != -1
	 && (statbuf.st_mode & S_IFDIR)
	 && index(path,'*') == NULLCHAR) {
#else
	else if(index(path,'*') == NULLCHAR
	 && stat(path,&statbuf) != -1
	 && (statbuf.st_attr & ST_DIRECT)) {
#endif
		if((cp = malloc(strlen(path) + 10)) == NULLCHAR)
			return -1;
		sprintf(cp,"%s%c%s",path,'\\',"*.*");
		path = cp;
		cflag = 1;
	}
	for(;;){
		bdos(SET_DTA,&sbuf);	/* Set disk transfer address */
		if(dos(command,0,REGFILE,path,0,0) IS_ERROR)
			break;

		command = FIND_NEXT;	/* Got first one already... */
		if(sbuf.fname[0] != '.'){	/* nuke "." and ".." */
			if(full > 0){
				n++;	/* Count 'em */
				format_dir(line_buf,&sbuf);
				fprintf(file,"%s%s",line_buf,(i^=1) ? "   " : "\n");
			} else	{	/* is short form */
				format_fname(dirtmp,sbuf.fname,sbuf.attr);
				fprintf(file,"%-15s%s",dirtmp,(full && (++i % 5)?"":"\n"));
			}
		}
	}
	if(full > 0){
		if(i)
			fprintf(file,"\n");

		if (isalpha(*path) && path[1] == ':')	/* detect A: drivespec */
			diskfree(file,*path & 0x1f,n);
		else
			diskfree(file,0,n);
	}
	else
		if(i % 5)
			fprintf(file,"\n");

	if(cflag)
		free(path);
	return 0;
}

static
void
diskfree (file,drv,nfiles)
FILE *file;
int drv;
int nfiles;

{
#ifdef MSDOS
	unsigned short ax,bx,cx,dx;
	void isfree();
#endif
#ifdef ATARI_ST
	struct { unsigned long di_free,di_many,di_ssize,di_spau; } disk_info;
#endif
	unsigned long free_bytes = 0, total_bytes = 0;
	char s_free[11], s_total[11];
	void commas();

	fflush(stdout); 		/* function takes a short while... */

#ifdef MSDOS
	/* Provide additional information  */
	ax = 0x3600;	/* AH = 36h, AL = 0 (AL not used) */
	dx = drv;	/* Default drive */
	isfree(&ax,&bx,&cx,&dx);

	if (ax != 0xffff)
	{
		free_bytes  = (unsigned long)ax * (unsigned long)cx;
		total_bytes = free_bytes * (unsigned long)dx;
		free_bytes *= (unsigned long)bx;
	}
#endif
#ifdef ATARI_ST
	if (Dfree(&disk_info,drv) == 0)
	{
		free_bytes  = disk_info.di_spau * disk_info.di_ssize;
		total_bytes = free_bytes * disk_info.di_many;
		free_bytes *= disk_info.di_free;
	}
#endif

	sprintf(s_free,"%ld",free_bytes);	commas(s_free);
	sprintf(s_total,"%ld",total_bytes);	commas(s_total);

	if(nfiles)
		fprintf(file,"%d",nfiles);
	else
		fprintf(file,"No");

	fprintf(file," file%s. %s bytes free. Disk size %s bytes.\r\n",
		      (nfiles==1? "":"s"),s_free,s_total);
}


/*
 * Return a string with commas every 3 positions.
 * If malloc() fails, return original string unmodified.
 * else the original string is replace with the string with commas.
 *
 * The caller must be sure that there is enough room for the resultant
 * string.
 *
 *
 * k3mc 4 Dec 87
 * pe1chl 3 Feb 88     europeans would use periods...
 */

#ifndef THSEP
#define THSEP ','
#endif
void
commas(dest)
char *dest;
{
	char *src, *core;	/* Place holder for malloc */
	unsigned cc;		/* The comma counter */
	unsigned len;
	
	len = strlen(dest);
	if( (core = src = (char *)malloc(len+1)) == NULLCHAR)
		return;

	strcpy(src,dest);	/* Make a copy, so we can muck around */
	cc = (len-1)%3 + 1;	/* Tells us when to insert a comma */
	
	while(*src != '\0'){
		*dest++ = *src++;
		if( ((--cc) == 0) && *src ){
			*dest++ = THSEP; cc = 3;
		}
	}
	free(core);
	*dest = '\0';
}
/*
 * This insertion sort adapted from "Writing Efficient Programs" by Jon Louis
 * Bentley, Prentice-Hall 1982, ISBN 0-13-070244-X (paperback) p. 65
 *
 * Run Time (sec) = K * N^2, where K = 21e-6 on my turbo XT clone (SI=2.6).
 * This could be improved to perhaps K * N * log2(N) using Quicksort, but,
 * as Bentley points out, this insertion sort is actually faster for small
 * values of N.  His "best" sorting algorithm uses an insertion sort/Quicksort
 * hybrid, with the "cutoff" value being about 30 elements.
 *
 * I have opted for the straight insertion sort because it is quite simple,
 * provably correct, and not a terrible performer when N < 1000, which is the
 * case for most directories.
 */
static
void
dir_sort(head)
struct dirsort *head;
{
	struct dirsort *here, *backtrack;
	struct dirent *de_temp;
	
	for(here = head->next; here != NULLSORT; here = here->next){
		backtrack = here;
		de_temp = here->direntry;
		while(backtrack->prev
		 && strcmp(de_temp->fname,backtrack->prev->direntry->fname)<0){
			backtrack->direntry = backtrack->prev->direntry;
			backtrack = backtrack->prev;
		}
		backtrack->direntry = de_temp;
	}
}

static
void
format_dir (line_buf,sbuf)
	char *line_buf;
	struct dirent *sbuf;

{
	char dirtmp[20];
	char cbuf[20],cbuf1[20];

	format_fname(dirtmp,sbuf->fname,sbuf->attr);

	sprintf(line_buf,"%-13s",dirtmp);
	if(sbuf->attr & ST_DIRECT)
		strcat(line_buf,"           ");/* 11 spaces */
	else {
		sprintf(cbuf,"%ld",sbuf->fsize);
		commas(cbuf);
		sprintf(cbuf1,"%10s ",cbuf);
		strcat(line_buf,cbuf1); /* Do filesize */
	}

	sprintf(cbuf,"%2d:%02d %2d/%02d/%02d",
	  (sbuf->ftime >> 11) & 0x1f,	 /* hour */
	  (sbuf->ftime >> 5) & 0x3f,	 /* minute */
	  (sbuf->fdate >> 5) & 0xf,	 /* month */
	  (sbuf->fdate ) & 0x1f,	 /* day */
	  (sbuf->fdate >> 9) + 80);	 /* year */

	strcat(line_buf,cbuf);
}

static
void
format_fname(dest,src,attr)
char	*dest, *src;
char	attr;
{
	char	*cp = src+8;
	int	loop_counter;

	for(loop_counter=0; loop_counter<8; loop_counter++){
		*dest++ = (char)tolower(*src++);	/* DG2KK:  (char) */
		if(*src == ' ')break;
	}
	if(cp[0] != ' ' || cp[1] != ' ' || cp[2] != ' '){ /* There is an extension */
		*dest++ = '.';
		for(loop_counter=0; loop_counter<3; loop_counter++){
		*dest++ = (char)tolower(*cp++);		/* DG2KK:  (char) */
			if(*cp == ' ')break;
		}
	}
	if(attr & ST_DIRECT)*dest++ = '\\';
	*dest = '\0';
}

static
void
free_clist(head)
struct dirsort *head;
{
	struct dirsort *here;
	
	here = head;
	if(here)do{
		free(here->direntry);
		if(head != here){
			free(head);
			head = here;
		}
	} while (here = here->next);
	if(head != here){
		free(head);
		head = here;
	}
}

#ifdef LATTICE		/* DG2KK: Lattice doesn't know a thing about stat */
int
stat(name,buf)
char *name;
struct stat *buf;
{
	static struct dirent sbuf;
	static struct stat statbuf;
	int error;

	gemdos(SET_DTA, &sbuf);

	error = gemdos(FIND_FIRST, name, REGFILE);	/* look for directories */
	if (error)
		return(-1);			/* caller insists on -1 */

	statbuf.st_attr = sbuf.attr;
	return 0;
}
#endif
