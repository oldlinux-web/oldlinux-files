/*
 *	Stat.c		Dump out inode info in nice form.   pur-ee!rsk
 *			Original version by someone at Purdue in the days of v6;
 *			this one by Rsk for modern times. (4.2, V)
 *
 *			Bug fix for setuid bit misplacement, and modifications
 *			for System V by Bill Stoll, whuxlm!wws.
 *
 *			Masscomp (and system V) mods by Stan Barber, neuro1!sob.
 *
 *			Bug fix for setuid bit and flag mods, and for
 *			misplaced include of time.h, okstate!zap.
 *			Note: I left SINCE as a compile-time option; it
 *			probably shouldn't even be there. ---Rsk
 *
 *			(void)'s added to make lint happy, pur-ee!rsk.
 *
 *			Still doesn't run under 2.9; to be fixed soon.
 *
 *
 *	Vanilla version is for system V.
 * 	Define BSD42 for 4.2bsd version.
 *	Define SINCE for "elapsed time" since inode times.
 *	Define MASSCOMP for those machines; this implies system V.
 */
/*
 *	1992-03-03 bergt@informatik.tu-chemnitz.de: ported the stuff
 *		   to Linux 0.12 and add's unknown uid's and gid's
 */

#ifdef MASSCOMP
#undef BSD42
#endif MASSCOMP

#include	<stdio.h>
#include	<sys/time.h>
#include	<sys/types.h>
#include	<ctype.h>
#include	<sys/stat.h>
#include	<pwd.h>
#include	<grp.h>
#include	<strings.h>
#include	<stdlib.h>

#define	FAIL	-1		/* Failure return code from call */	
#define OKAY	0		/* Success return code from call */

struct 	stat	Sbuf;		/* for return values from stat() call */
char	*ctime();		/* Time conversion	*/
struct	passwd	*pwent;		/* User structure */
struct	group	*grent;		/* Group structure */

char	Mode[10];	/* File protection mode */

#define	LBUFSIZ	256	/* Length for symbolic link translation buffer */
char	Lbuf[LBUFSIZ];	/* Symbolic link translation buffer */

main(argc, argv)
int argc;
char *argv[];
{
	int i;

	i = 1;

	if(argc == 1) {
		(void) fprintf(stderr,"Usage: stat file1 [file2 ...]\n");
		exit(1);
	}

	do {

		(void) stat_it(argv[i]);

		if(  (argc > 1) && (i < (argc-1)) )
			(void) printf("\n");
	}
	while(++i < argc);

	exit(0);
}

/*
 * stat_it() - Actually stat and format results from file.
 *		exit -    OKAY if no problems encountered
 *			  FAIL if couldn't open or other nastiness
 *
 */
stat_it(filename)
char	*filename;
{
	int	count;

#ifdef BSD42
	if( lstat(filename,&Sbuf) == FAIL) {
		(void) fprintf(stderr,"Can't lstat %s\n",filename); 
		return(FAIL);
	}
#else BSD42
	if( stat(filename,&Sbuf) == FAIL) {
		(void) fprintf(stderr,"Can't stat %s\n",filename); 
		return(FAIL);
	}
#endif BSD42

#ifdef BSD42
	if( (Sbuf.st_mode & S_IFMT) == S_IFLNK) {
		if( (count=readlink(filename,Lbuf,LBUFSIZ)) == FAIL) {
			(void) fprintf(stderr,"Can't readlink %s\n", filename);
			return(FAIL);
		}
		if( count < LBUFSIZ)
			Lbuf[count] = '\0';
		(void) printf("  File: \"%s\" -> \"%s\"\n",filename,Lbuf);
	}
	else
#endif BSD42
		(void) printf("  File: \"%s\"\n", filename);

	(void) printf("  Size: %-10d", Sbuf.st_size);
#ifdef BSD42
 	(void) printf("   Allocated Blocks: %-10ld", Sbuf.st_blocks);
#endif BSD42

	(void) printf("   Filetype: ");
	switch( Sbuf.st_mode & S_IFMT) {
		case	S_IFDIR:	(void) printf("Directory\n");
					break;
		case	S_IFCHR:	(void) printf("Character Device\n");
					break;
		case	S_IFBLK:	(void) printf("Block Device\n");
					break;
		case	S_IFREG:	(void) printf("Regular File\n");
					break;
#ifdef BSD42
		case	S_IFLNK:	(void) printf("Symbolic Link\n");
					break;
		case	S_IFSOCK:	(void) printf("Socket\n");
					break;
#else BSD42
		case	S_IFIFO:	(void) printf("Fifo File\n");
					break;
#endif BSD42
#ifdef MASSCOMP
		case	S_IFCTG:	(void) printf("Contiguous File\n");
					break;
#endif MASSCOMP
		default		:	(void) printf("Unknown\n");
	}

	(void) strcpy(Mode,"----------");
	if(Sbuf.st_mode & (S_IXUSR>>6))		/* Other execute */
		Mode[9] = 'x';
	if(Sbuf.st_mode & (S_IWUSR>>6))	/* Other write */
		Mode[8] = 'w';
	if(Sbuf.st_mode & (S_IRUSR>>6))		/* Other read */
		Mode[7] = 'r';
	if(Sbuf.st_mode & (S_IXUSR>>3))		/* Group execute */
		Mode[6] = 'x';
	if(Sbuf.st_mode & (S_IWUSR>>3))	/* Group write */
		Mode[5] = 'w';
	if(Sbuf.st_mode & (S_IRUSR>>3))		/* Group read */
		Mode[4] = 'r';
	if(Sbuf.st_mode & S_IXUSR)		/* User execute */
		Mode[3] = 'x';
	if(Sbuf.st_mode & S_IWUSR)		/* User write */
		Mode[2] = 'w';
	if(Sbuf.st_mode & S_IRUSR)		/* User read */
		Mode[1] = 'r';
	if(Sbuf.st_mode & S_ISVTX)		/* Sticky bit */
		Mode[9] = 't';
	if(Sbuf.st_mode & S_ISGID)		/* Set group id */
		Mode[6] = 's';
	if(Sbuf.st_mode & S_ISUID)		/* Set user id */
		Mode[3] = 's';
	switch( Sbuf.st_mode & S_IFMT) {
		case	S_IFDIR:	Mode[0] = 'd';
					break;
		case	S_IFCHR:	Mode[0] = 'c';
					break;
		case	S_IFBLK:	Mode[0] = 'b';
					break;
		case	S_IFREG:	Mode[0] = '-';
					break;
#ifdef BSD42
		case	S_IFLNK:	Mode[0] = 'l';
					break;
		case	S_IFSOCK:	Mode[0] = 's';
					break;
#else BSD42
		case	S_IFIFO:	Mode[0] = 'f';
					break;
#endif BSD42
#ifdef MASSCOMP
		case	S_IFCTG:	Mode[0] = 'C';
					break;
#endif MASSCOMP
		default		:	Mode[0] = '?';
	}
	(void) printf("  Mode: (%04o/%s)", Sbuf.st_mode&07777,Mode);


	(void) setpwent();
	if( (pwent = getpwuid(Sbuf.st_uid)) == NULL) {
		pwent = (PTR) malloc(sizeof(struct passwd));
		(void) strcpy(pwent->pw_name, "???");
	}
	(void) printf("         Uid: (%5d/%8s)", Sbuf.st_uid, pwent->pw_name);

	(void) setgrent();
	if( (grent = getgrgid(Sbuf.st_gid)) == NULL) {
		grent = (PTR) malloc(sizeof(struct passwd));
		(void) strcpy(grent->gr_name, "???");
	}
	(void) printf("  Gid: (%5d/%8s)\n", Sbuf.st_gid, grent->gr_name);


	(void) printf("Device: %-5d", Sbuf.st_dev);
	(void) printf("  Inode: %-10d", Sbuf.st_ino);
	(void) printf("Links: %-5d", Sbuf.st_nlink);

	/* Only meaningful if file is device */

	if(  ( (Sbuf.st_mode & S_IFMT) == S_IFCHR)
		|| ( (Sbuf.st_mode & S_IFMT) == S_IFBLK) )
		(void) printf("     Device type: %d\n",Sbuf.st_rdev);
	else
		(void) printf("\n");

	/* The %.24s strips the newline from the ctime() string */

#ifdef SINCE
	(void) printf("Access: %.24s",ctime(&Sbuf.st_atime));
	(void) tsince(Sbuf.st_atime);
	(void) printf("Modify: %.24s",ctime(&Sbuf.st_mtime));
	(void) tsince(Sbuf.st_mtime);
	(void) printf("Change: %.24s",ctime(&Sbuf.st_ctime));
	(void) tsince(Sbuf.st_ctime);
#else SINCE
	(void) printf("Access: %s",ctime(&Sbuf.st_atime));
	(void) printf("Modify: %s",ctime(&Sbuf.st_mtime));
	(void) printf("Change: %s",ctime(&Sbuf.st_ctime));
#endif SINCE

/*
* 	Should I put this in somewhere?  No.
*
* 	printf("Optimal Blocksize: %ld\n", Sbuf.st_blksize);
*/

	return(OKAY);
}

#ifdef SINCE
tsince(time_sec)
long	time_sec;
{
	long	time_buf;
	long	d_since;	/* days elapsed since time */
	long	h_since;	/* hours elapsed since time */
	long	m_since;	/* minutes elapsed since time */
	long	s_since;	/* seconds elapsed since time */

	(void) time(&time_buf);

	if(time_sec > time_buf) {
		(void) fprintf(stderr,"Time going backwards\n");
		exit(1);
	}

	s_since = time_buf - time_sec;
	d_since = s_since / 86400l ;
	s_since -= d_since * 86400l ;
	h_since = s_since / 3600l ;
	s_since -= h_since * 3600l ;
	m_since = s_since / 60l ;
	s_since -= m_since * 60l ;

	(void) printf("(%05ld.%02ld:%02ld:%02ld)\n",d_since,h_since,m_since,s_since);

	return(OKAY);
}
#endif SINCE
