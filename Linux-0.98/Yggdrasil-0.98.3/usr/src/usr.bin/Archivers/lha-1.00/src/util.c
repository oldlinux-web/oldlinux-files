/*
 * util.c - part of LHa for UNIX
 *	Feb 26 1992 modified by Masaru Oki
 *	Mar  4 1992 modified by Masaru Oki
 *		#ifndef USESTRCASECMP added.
 *	Mar 31 1992 modified by Masaru Oki
 *		#ifdef NOMEMSET added.
 */

#include <stdio.h>
#include <errno.h>
#ifdef sony_news
#include <sys/param.h>
#endif
#if defined(__STDC__) || defined(NEWSOS)
#include <stdlib.h>
#endif
#include "lharc.h"

#define BUFFERSIZE 2048
#ifndef NULL
#define NULL (char *)0
#endif

extern unsigned short crc;
extern int quiet;

long copyfile(f1, f2, size, crc_flg)	/* return: size of source file */
FILE *f1;
FILE *f2;
long size;
int crc_flg;	/* 0: no crc, 1: crc check, 2: extract, 3: append */
{
	unsigned short xsize;
	char *buf;
	long rsize = 0;

	if ((buf = (char *)malloc(BUFFERSIZE)) == NULL)
		fatal_error("virtual memory exhausted.\n");
	crc = 0;
	if ((crc_flg == 2 || crc_flg) && text_mode)
		init_code_cache();
	while (size > 0) {
		/* read */
		if (crc_flg == 3 && text_mode) {
			xsize = fread_txt(buf, BUFFERSIZE, f1);
			if (xsize == 0)
				break;
			if (ferror(f1)) {
				fatal_error("file read error\n");
			}
		}
		else {
			xsize = (size > BUFFERSIZE) ? BUFFERSIZE : size;
			if (fread(buf, 1, xsize, f1) != xsize) {
				fatal_error("file read error\n");
			}
		}
		/* write */
		if (f2) {
			if (crc_flg == 2 && text_mode) {
				if (fwrite_txt(buf, xsize, f2)) {
					fatal_error("file write error\n");
				}
			}
			else {
				if (fwrite(buf, 1, xsize, f2) != xsize) {
					fatal_error("file write error\n");
				}
			}
		}
		/* calculate crc */
		if (crc_flg) {
			calccrc(buf, xsize);
		}
		rsize += xsize;
		if (crc_flg != 3 || !text_mode)
			size -= xsize;
	}
	free(buf);
	return rsize;
}

int
encode_stored_crc (ifp, ofp, size, original_size_var, write_size_var)
FILE *ifp, *ofp;
long size;
long *original_size_var;
long *write_size_var;
{
	int save_quiet;

	save_quiet = quiet;
	quiet = 1;
	size = copyfile (ifp,ofp,size,3);
	*original_size_var = *write_size_var = size;
	quiet = save_quiet;
	return crc;
}

/***************************************
	convert path delimiter
****************************************
	returns *filename
***************************************/
unsigned char *
convdelim(path, delim)
unsigned char *path;
unsigned char delim;
{
  unsigned char c;
  unsigned char *p;
#ifdef MULTIBYTE_CHAR
  int kflg;

  kflg = 0;
#endif
  for (p = path; (c = *p) != 0; p++) {
#ifdef MULTIBYTE_CHAR
    if (kflg) {
      kflg = 0;
    } else if (iskanji(c)) {
      kflg = 1;
    } else
#endif
      if (c == '\\' || c == DELIM || c == DELIM2)
	{
	  *p = delim;
	  path = p + 1;
	}
  }
  return path;
}

/* If TRUE, archive file name is msdos SFX file name. */
boolean
archive_is_msdos_sfx1 (name)
     char *name;
{
  int len = strlen (name);

  return ((len >= 4) &&
	  (strucmp (".COM",name + len - 4) == 0 ||
	   strucmp (".EXE",name + len - 4) == 0)) ||
	 ((len >= 2) &&
	  (strucmp (".x",name + len - 2) == 0));
}

/* skip SFX header */
boolean
skip_msdos_sfx1_code (fp)
     FILE *fp;
{
  unsigned char buffer[4096];
  unsigned char *p, *q;
  int n;

  n = fread (buffer, sizeof (char), 4096, fp);

  for (p = buffer + 2, q = buffer + n - 5; p < q; p ++)
    {
      /* found "-l??-" keyword (as METHOD type string) */
      if (p[0] == '-' && p[1] == 'l' && p[4] == '-')
	{
	  /* size and checksum validate check */
	  if (p[-2] > 20 && p[-1] == calc_sum (p, p[-2]))
	    {
	      fseek (fp, ((p - 2) - buffer) - n, SEEK_CUR);
	      return TRUE;
	    }
	}
    }

  fseek (fp, -n, SEEK_CUR);
  return FALSE;
}

/*
 * strdup(3)
 */

#ifdef NOSTRDUP
char *strdup ( buf )
char *buf;
{
	char *p;

	if ((p = (char *)malloc(strlen(buf) + 1)) == NULL) return NULL;
	strcpy( p, buf );
	return p;
}
#endif

/*
 *	memmove( char *dst , char *src , size_t cnt )
 */

#if defined(NOBSTRING) && !defined(__STDC__)
void *
memmove( dst , src , cnt )
register char *dst , *src;
register int cnt;
{
	if ( dst == src ) return dst;
	if ( src > dst )
	{
		while ( --cnt >=0 ) *dst++ = *src++;
	}
	else
	{
		dst += cnt; src += cnt;
		while ( --cnt >=0 ) *--dst = *--src;
	}
	return dst;
}
#endif

/* rename - change the name of file
 * 91.11.02 by Tomohiro Ishikawa (ishikawa@gaia.cow.melco.CO.JP)
 * 92.01.20 little modified (added #ifdef) by Masaru Oki
 * 92.01.28 added mkdir() and rmdir() by Tomohiro Ishikawa
 */

#if defined(NOFTRUNCATE) && !defined(_MINIX)

int
rename(from, to)
char *from, *to;
{
	struct	stat s1,s2;
	extern	int errno;

	if (stat(from, &s1) < 0)
		return(-1);
	    /* is 'FROM' file a directory? */
	if ((s1.st_mode & S_IFMT) == S_IFDIR){
		errno = ENOTDIR;
		return(-1);
	}
	if (stat(to, &s2) >= 0) { /* 'TO' exists! */
		/* is 'TO' file a directory? */
		if ((s2.st_mode & S_IFMT) == S_IFDIR){
			errno=EISDIR;
			return(-1);
		}
		if (unlink(to) < 0)
			return(-1);
	}
	if (link(from, to) < 0)
		return(-1);
	if (unlink(from) < 0)
		return(-1);
	return(0);
}
#endif /* NOFTRUNCATE */

#ifdef	NOMKDIR
#ifndef	MKDIRPATH
#define	MKDIRPATH	"/bin/mkdir"
#endif
#ifndef	RMDIRPATH
#define	RMDIRPATH	"/bin/rmdir"
#endif
int
rmdir(path)
char *path;
{
	int	stat,rtn=0;
	char	*cmdname;
	if ( (cmdname=(char *)malloc(strlen(RMDIRPATH)+1+strlen(path)+1))
		== 0)
		return(-1);
	strcpy(cmdname,RMDIRPATH);
	*(cmdname+strlen(RMDIRPATH))=' ';
	strcpy(cmdname+strlen(RMDIRPATH)+1,path);
	if ((stat = system(cmdname))<0)
		rtn=-1;	/* fork or exec error */
	else if(stat){		/* RMDIR command error */
		errno = EIO;
		rtn=-1;
	}
	free(cmdname);
	return(rtn);
}
int
mkdir(path,mode)
char	*path;
int	mode;
{
	int child, stat;
	char	*cmdname,*cmdpath=MKDIRPATH;
	if ( (cmdname=(char *)strrchr(cmdpath,'/'))==(char *)0)
		cmdname=cmdpath;
	if ((child = fork())<0)
		return(-1);	/* fork error */
	else if(child) {	/* parent process */
		while (child != wait(&stat))	/* ignore signals */
			continue;
	}
	else{			/* child process */
		int	maskvalue;
		maskvalue = umask(0);	/* get current umask() value */
		umask(maskvalue | (0777 & ~mode)); /* set it! */
		execl(cmdpath, cmdname, path, (char *)0);
		/* never come here except execl is error */
		return(-1);
	}
	if (stat != 0) {
		errno = EIO;	/* cannot get error num. */
		return(-1);
	}
	return(0);
}
#endif

/*
 * strucmp
 *	modified: Oct 29 1991 by Masaru Oki
 */

#ifndef USESTRCASECMP
static int my_toupper(n)
  register int n;
{
  if(n >= 'a' && n <= 'z') return n & (~('a'-'A'));
  return n;
}

int strucmp(s, t)
register char *s, *t;
{
	while (my_toupper(*s++) == my_toupper(*t++))
		if ( !*s || !*t ) break;
	if ( !*s && !*t ) return 0;
	return 1;
}
#endif

#ifdef NOMEMSET
/* Public Domain memset(3) */
char *
memset (s, c, n)
char *s;
int c, n;
{
  char *p = s;
  while(n--) *p++ = (char)c;
  return s;
}
#endif
