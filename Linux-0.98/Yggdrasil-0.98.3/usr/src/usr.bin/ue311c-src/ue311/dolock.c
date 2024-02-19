/*	DOLOCK.C:	Machine specific code for File Locking
			for MicroEMACS
			(C)Copyright 1987 by Daniel M Lawrence
*/

#include	<stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include	"elang.h"

#if	WMCS
/*	file locking for WMCS */

#include "sys$disk/sysincl.sys/sysequ.h"
#include <stdio.h>
#include <ctype.h>

char msg[] = TEXT35;
/*           "another user" */

char *dolock(fname)
char *fname;
{
	int lun,status;
	status = _open(fname,OPREADACC|OPWRITEACC|OPWRITELOCK,-1,&lun);
	if(status == 133 || status == 0 ) return(NULL);
	return(msg);
}

char *undolock(fname)
char *fname;
{
	int i,j,k,lun,status;
	char xname[95],c;
	
	for(lun=4; _getfnam(lun,xname) == 0; lun++) {
		for(i=0;i<strlen(xname);i++)	{
			k = i;
			for(j=0;j<strlen(fname);j++)  {
				c = fname[j];
				if(islower(c)) c = toupper(c);
				if(c == xname[k]) { ++k; continue; }
				if(c == '\0') break;
				break;
				}
			if(j == strlen(fname)) {
				_close(lun,0);
				return(NULL);
				}
			}
	}
	return(NULL);
}
#endif

#if	FILOCK && (MSDOS || OS2 || SUN || USG || V7 || BSD)
#if	OS2 || (MSDOS && MSC)
#include	<sys/types.h>
#endif
#include	<sys/stat.h>
#include	<errno.h>
#if	MSDOS && TURBO
#include	<dir.h>
#endif
#if	SUN
#include	<sys/dir.h>
#include	<signal.h>
#endif

#if	OS2 == 0
#if	ZTC
extern volatile int errno;
#else
extern int errno;
#if	MSC == 0
#endif
#endif

#endif
#define LOCKDIR "_xlk"
#define LOCKMSG "LOCK ERROR -- "
#define	LOCKDEBUG	FALSE

/*	dolock:	Generic non-UNIX file locking mechinism	*/

/**********************
 *
 * dolock -- lock the file fname
 *
 * if successful, returns NULL 
 * if file locked, returns username of person locking the file
 * if other error, returns "LOCK ERROR: explanation"
 *
 *********************/

char *parse_name(filespec)		/* get name component of filespec */

char *filespec;

{
	char *rname;

	rname = &filespec[strlen(filespec) - 1];
	while (rname >= filespec) {
		if (*rname == DIRSEPCHAR || *rname == DRIVESEPCHAR) {
			++rname;
			break;
		}
		--rname;
	}

	if (rname >= filespec)
		return(rname);
	else
		return(filespec);
}

char *parse_path(filespec)

char *filespec;

{
	static char rbuff[NFILEN];
	char *rname;

	/* make a copy we can mung */
	strcpy(rbuff,filespec);

	/* starting from the end */
	rname = &rbuff[strlen(rbuff)-1];

	/* scan for a directory separator */
	while (rname >= rbuff) {
		if (*rname == DIRSEPCHAR)
			break;
		--rname;
	}

	/* no path here! */
	if (rname < rbuff)
		return("");


	*(rname) = '\0';	/* point just beyond slash */

	/* no skip beyond any drive spec */
	rname = rbuff;
	while (*rname) {
		if (*rname == DRIVESEPCHAR)
			return(++rname);
		++rname;
	}

	/* none to be found, return the whole string */
	return(rbuff);
}

char *parse_drive(filespec)

char *filespec;

{
	static char rbuff[NFILEN];
	char *rname;

	/* search for a drive specifier */
	strcpy(rbuff,filespec);
	rname = rbuff;
	while (*rname) {
		if (*rname == DRIVESEPCHAR) {
			*(++rname) = 0;
			return(rbuff);
		}
		++rname;
	}

	/* no drive letter/name */
	return("");
}

char *dolock(filespec)

char *filespec;		/* full file spec of file to lock */

{
	struct stat sb;			/* stat buffer for info on files/dirs */
	FILE *fp;			/* ptr to lock file */
	long proc_id;			/* process id from lock file */
	char filename[NFILEN];		/* name of file to lock */
	char pathname[NFILEN];		/* path leading to file to lock */
	char drivename[NFILEN];		/* drive for file to lock */
	char lockpath[NFILEN];		/* lock directory name */
	char lockfile[NFILEN];		/* lock file name */
	char buf[NSTRING];		/* input buffer */
	char host[NSTRING];		/* current host name */
	static char result[NSTRING];	/* error return string */
	char *getenv(); 

	/* separate filespec into components */
	strcpy(filename, parse_name(filespec));
	strcpy(pathname, parse_path(filespec));
	strcpy(drivename, parse_drive(filespec));
	if (pathname[0] == 0)
		strcpy(pathname, ".");

	/* merge the drive into the pathname */
	strcat(drivename, pathname);
	strcpy(pathname, drivename);

#if	LOCKDEBUG
	printf("Locking [%s] [%s]\n", pathname, filename); tgetc();
#endif

#if	MSDOS == 0 && OS2 == 0
	/* check to see if we can access the path */
	if (stat(pathname, &sb) != 0) {
#if	LOCKDEBUG
		printf("stat() = %u   errno = %u\n", stat(pathname, &sb), errno); tgetc();
#endif
		strcpy(result, LOCKMSG);
		strcat(result, "Path not found");
		return(result);
	}
	if ((sb.st_mode & S_IFDIR) == 0) {
		strcpy(result, LOCKMSG);
		strcat(result, "Illegal Path");
		return(result);
	}
#endif

	/* create the lock directory if it does not exist */
	strcpy(lockpath, pathname);
	strcat(lockpath, DIRSEPSTR);
	strcat(lockpath, LOCKDIR);
#if	LOCKDEBUG
	printf("Lockdir [%s]\n", lockpath); tgetc();
#endif
#if	MSDOS || OS2
	if (TRUE) {
#else
	if (stat(lockpath, &sb) != 0) {
#endif

		/* create it! */
#if	LOCKDEBUG
		printf("MKDIR(%s)\n", lockpath); tgetc();
#endif
#if	MSDOS || OS2
		mkdir(lockpath);
#else
		if (mkdir(lockpath, 0777) != 0) {
			strcpy(result, LOCKMSG);
			switch (errno) {

			    case EACCES:
				strcat(result, "Permission Denied");
				break;

			    case ENOENT:
				strcat(result, "No such file or directory");
				break;
			}
			return(result);
		}
#endif
	}

	/* check for the existance of this lockfile */
	strcpy(lockfile, lockpath);
	strcat(lockfile, DIRSEPSTR);
	strcat(lockfile, filename);
#if	LOCKDEBUG
	printf("Lockfile [%s]\n", lockfile); tgetc();
#endif

	if (stat(lockfile, &sb) != 0) {

		/* create the lock file */
		fp = fopen(lockfile, "w");
		if (fp == (FILE *)NULL) {
			strcpy(result, LOCKMSG);
			strcat(result, "Can not open lock file");
			return(result);
		}

		/* and output the info needed */
#if	MSDOS || OS2
		fprintf(fp, "0\n");		/* process ID */
#endif
#if	SUN
		fprintf(fp, "%u\n", getpid());
#endif

		/* user name */
		if (getenv("USER"))
			fprintf(fp, "%s\n", getenv("USER"));
		else
			fprintf(fp, "<unknown>\n");

		/* host name */
		if (getenv("HOST"))
			fprintf(fp, "%s\n", getenv("HOST"));
		else {
#if	MSDOS || OS2
			fprintf(fp, "<unknown>\n");
#endif
#if	SUN
			gethostname(buf, NFILEN);
			fprintf(fp, "%s\n", buf);
#endif
		}

		/* time... */
		fprintf(fp, "%s\n", timeset());

		fclose(fp);
		return(NULL);

	} else {

		/* get the existing lock info */
		fp = fopen(lockfile, "r");
		if (fp == (FILE *)NULL) {
			strcpy(result, LOCKMSG);
			strcat(result, "Can not read lock file");
			return(result);
		}

		/* get the process id */
		fgets(buf, NSTRING, fp);
		proc_id = asc_int(buf);

		/* get the user name */
		fgets(result, NSTRING, fp);
		result[strlen(result)-1] = 0;

		/* get the host name */
		strcat(result, "@");
		fgets(buf, NSTRING, fp);
		buf[strlen(buf)-1] = 0;
		strcat(result, buf);

#if	SUN
		/* is it the current host? */
		gethostname(host, NFILEN);
		if (strcmp(buf, host) == 0) {

			/* see if the process is dead already */
			if (kill(proc_id, 0) != 0 && errno == ESRCH) {

				/* kill the lock file and retry the lock */
				fclose(fp);
				unlink(lockfile);
				return(dolock(filespec));
			}
		}
#endif

		/* get the time */
		strcat(result, " at ");
		fgets(buf, NSTRING, fp);
		buf[strlen(buf)-1] = 0;
		strcat(result, buf);
		fclose(fp);
		return(result);
	}
}

/**********************
 *
 * undolock -- unlock the file fname
 *
 * if successful, returns NULL 
 * if other error, returns "LOCK ERROR: explanation"
 *
 *********************/

char *undolock(filespec)

char *filespec;		/* filespec to unlock */

{
	char filename[NFILEN];		/* name of file to lock */
	char pathname[NFILEN];		/* path leading to file to lock */
	char drivename[NFILEN];		/* drive for file to lock */
	char lockpath[NFILEN];		/* lock directory name */
	char lockfile[NFILEN];		/* lock file name */
	static char result[NSTRING];	/* error return string */

	/* separate filespec into components */
	strcpy(filename, parse_name(filespec));
	strcpy(pathname, parse_path(filespec));
	strcpy(drivename, parse_drive(filespec));
	if (pathname[0] == 0)
		strcpy(pathname, ".");

	/* merge the drive into the pathname */
	strcat(drivename, pathname);
	strcpy(pathname, drivename);

#if	LOCKDEBUG
	printf("\nUnLocking [%s] [%s]\n", pathname, filename); tgetc();
#endif

	/* create the lock directory if it does not exist */
	strcpy(lockpath, pathname);
	strcat(lockpath, DIRSEPSTR);
	strcat(lockpath, LOCKDIR);
#if	LOCKDEBUG
	printf("Lockdir [%s]\n", lockpath); tgetc();
#endif
	/* check for the existance of this lockfile */
	strcpy(lockfile, lockpath);
	strcat(lockfile, DIRSEPSTR);
	strcat(lockfile, filename);
#if	LOCKDEBUG
	printf("Lockfile [%s]\n", lockfile); tgetc();
#endif
	if (unlink(lockfile)) { 
		strcat(result, "could not remove lock file"); 
		return(result); 
	} else {
		rmdir(lockpath);   /* this will work only if dir is empty */
  	  	return(NULL);
	}
}

#else
dolhello()
{
}
#endif

