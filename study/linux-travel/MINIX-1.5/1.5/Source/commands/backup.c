/* backup - backup a directory		Author: Andy Tanenbaum */

/* This program recursively backs up a directory.  It has two typical uses:
 *
 * 1. Backing up a directory to 1 or more diskettes
 * 2. Backing up RAM disk to a shadow directory on hard disk
 *
 * The backup directory or medium may be empty, in which case, the entire
 * source directory is copied, or it may contain an old backup, in which
 * case only those files that are new or out of date are copied.  In this
 * respect, 'backup' resembles 'make', except that no 'makefile' is needed.
 * The backed up copy may optionally be compressed to save space.
 *
 * The following flags exist:
 *
 *	-d  At the top level, only back up directories (not loose files)
 *	-j  Don't copy junk: *.o, *.Z, *.bak, *.log, a.out, and core
 *	-m  If ENOSPC encountered, ask for another diskette
 *	-n  No directories, only loose files are backed up
 *	-s  Don't copy *.s files
 *      -t  set creation date of target-file equal to cdate of source-file
 *	-v  Verbose (announce what is being done)
 *	-z  Compress the backed up files
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <utime.h>

#define COPY_SIZE 4096
#define MAX_ENTRIES 512
#define DIR_ENT_SIZE 16
#define NAME_SIZE 14
#define MAX_PATH 256
#define NONFATAL 0
#define FATAL 1
#define NO_SAVINGS 512		/* compress can return code 2 */
#define OUT_OF_SPACE 2

typedef unsigned short unshort;

struct dir_buf {		/* list of the src directory */
  unshort ino;
  char name[NAME_SIZE];
} dir_buf[MAX_ENTRIES];

struct sorted {
  int mode;			/* file mode */
  char *namep;			/* pointer to name in dir_buf */
  long acctime;			/* time of last access */
  long modtime;			/* time of last modification */
} sorted[MAX_ENTRIES];

char copybuf[COPY_SIZE];

unshort dflag, jflag, mflag, nflag, rflag, sflag, tflag, vflag, zflag;

extern int errno;
extern char *environ;

main(argc, argv)
int argc;
char *argv[];
{
  int i, ct, n, m, fd;
  char *dir1, *dir2, *cp, c;

  /* Get the flags. */
  sync();
  if (argc < 3 || argc > 4) usage();
  if (argc == 4) {
	cp = argv[1];
	if (*cp++ != '-') usage();
	while ((c = *cp++) != '\0') {
		switch (c) {
		    case 'd':	dflag++;	break;
		    case 'j':	jflag++;	break;
		    case 'm':	mflag++;	break;
		    case 'n':	nflag++;	break;
		    case 's':	sflag++;	break;
		    case 'r':	rflag++;	break;
		    case 't':	tflag++;	break;
		    case 'v':	vflag++;	break;
		    case 'z':	zflag++;	break;
		    default:	usage();
		}
	}
	dir1 = argv[2];
	dir2 = argv[3];
  } else {
	dir1 = argv[1];
	dir2 = argv[2];
  }

  /* Read in the source directory */

  fd = open(dir1, O_RDONLY);
  if (fd < 0) error(FATAL, "cannot open ", dir1, "");
  ct = read(fd, &dir_buf[0], MAX_ENTRIES * DIR_ENT_SIZE);
  close(fd);
  if (ct == MAX_ENTRIES * DIR_ENT_SIZE)
	error(FATAL, "directory ", dir1, " is too large");

  /* Create the target directory. */
  maketarget(dir2);

  /* Stat all the entries. */
  n = ct / DIR_ENT_SIZE;
  m = stat_all(dir1, n);

  /* Remove non-entries and sort what's left. */
  sort_dir(m);

  /* Process each of the m entries one at a time. */
  process(m, dir1, dir2);
  exit(0);
}


maketarget(dir2)
char *dir2;
{
/* The target directory is created if it does not already exist. */

  char *p, c, dbuf[MAX_PATH];

  if (access(dir2, 6) == 0)
	return;			/* if target exists, we're done */
  if (make_dir(dir2) == 0) return;	/* we just made it */

  /* We have to try creating all the higher level directories. */
  strcpy(dbuf, dir2);
  p = dbuf + 1;
  while (1) {
	while (*p != '/' && *p != '\0') p++;
	c = *p;			/* either / or \0 */
	*p = 0;
	make_dir(dbuf);
	if (c == '\0') return;
	*p = c;
	p++;
  }
}

int make_dir(dir)
char *dir;
{
/* Create a directory. */
  int pid, status;

  if ((pid = fork()) < 0)
	error(FATAL, "cannot fork off mkdir to create ", dir, "");
  if (pid > 0) {
	/* Parent process waits for child (mkdir). */
	wait(&status);
	return(status);
  } else {
	/* Child process executes mkdir */
	close(2);		/* don't want mkdir's error messages */
	execle("/bin/mkdir", "mkdir", dir, (char *) 0, environ);
	execle("/usr/bin/mkdir", "mkdir", dir, (char *) 0, environ);
	error(FATAL, "cannot execute mkdir", "", "");
  }
}


int stat_all(dir1, n)
char *dir1;
int n;
{
/* Stat all the directory entries.  By doing this all at once, the disk
 * head can stay in the inode area.
 */

  int i, j;
  char cbuf[MAX_PATH];
  struct stat s;
  struct dir_buf *dp;
  struct sorted *sp;

  for (i = 0; i < n; i++) {
	/* Mark "." and ".." as null entries, as well as unstatable ones. */
	if (strcmp(dir_buf[i].name, ".") == 0) dir_buf[i].ino = 0;
	if (strcmp(dir_buf[i].name, "..") == 0) dir_buf[i].ino = 0;
	if (dir_buf[i].ino == 0) continue;

	/* Stat the file. */
	strcpy(cbuf, dir1);
	strncat(cbuf, "/", 1);
	strncat(cbuf, dir_buf[i].name, NAME_SIZE);
	if (stat(cbuf, &s) < 0) {
		error(NONFATAL, "cannot stat ", cbuf, "");
		dir_buf[i].ino = 0;	/* mark as unusable */
		continue;
	}
	sorted[i].mode = s.st_mode;
	sorted[i].acctime = s.st_atime;
	sorted[i].modtime = s.st_mtime;
	sorted[i].namep = dir_buf[i].name;
  }

  /* Squeeze out all the entries who ino field is 0. */
  j = 0;
  for (i = 0; i < n; i++) {
	if (dir_buf[i].ino != 0) {
		sorted[j] = sorted[i];
		j++;
	}
  }
  return(j);
}


int sort_dir(m)
int m;
{
/* Sort the directory using bubble sort. */

  struct sorted *sp1, *sp2;

  for (sp1 = &sorted[0]; sp1 < &sorted[m - 1]; sp1++) {
	for (sp2 = sp1 + 1; sp2 < &sorted[m]; sp2++) {
		if (strncmp(sp1->namep, sp2->namep, NAME_SIZE) > 0)
			swap(sp1, sp2);
	}
  }
}


process(m, dir1, dir2)
int m;
char *dir1, *dir2;
{
/* Process each entry in sorted[].  If it is a regular file, stat the target
 * file.  The the source is newer, copy it.  If the entry is a directory,
 * recursively call the entire program to process the directory.
 */

  int er, fmode, res, namlen;
  struct sorted *sp;
  struct stat s;
  char cbuf[MAX_PATH];

  for (sp = &sorted[0]; sp < &sorted[m]; sp++) {
	fmode = sp->mode & S_IFMT;
	if (fmode == S_IFREG) {
		/* Regular file.  Construct target name and stat it. */
		strcpy(cbuf, dir2);
		strncat(cbuf, "/", 1);
		strncat(cbuf, sp->namep, NAME_SIZE);
		namlen = strlen(sp->namep);
		if (sp->namep[namlen - 2] != '.' || sp->namep[namlen - 1] != 'Z')
			if (zflag && (namlen <= (NAME_SIZE - 2)))
				strncat(cbuf, ".Z", 2);
		er = stat(cbuf, &s);
		if (er < 0 || sp->modtime > s.st_mtime) {
			res = copy(dir1, sp, cbuf);
		} else {
			res = NONFATAL;
		}

		/* Check status of the copy. */
		if (res == OUT_OF_SPACE) {
			printf("Out of space while copying to %s\n", cbuf);
			/* We ran out of space copying a regular file. */
			if (mflag == 0)
				error(FATAL,"Disk full. Backup aborted","","");

			/* If -m, ask for new diskette and continue. */
			newdisk(dir2);
			sp--;
			continue;
		}
	} else if (fmode == S_IFDIR) {
		/* Directory.  Execute this program recursively. */
		copydir(dir1, dir2, sp->namep);
	} else if (fmode == S_IFBLK || fmode == S_IFCHR) {
		/* Special file. */
		strncpy(cbuf, sp->namep, NAME_SIZE);
		printf("%s is special file.  Not backed up.\n", cbuf);
	}
  }
}




swap(sp1, sp2)
struct sorted *sp1, *sp2;
{
/* Swap two directory entries. */

  struct sorted d;

  d = *sp1;
  *sp1 = *sp2;
  *sp2 = d;
}


int copy(dir1, sp, cbuf2)
struct sorted *sp;
char *dir1, *cbuf2;
{
/* Copy a regular file. */

  int fd1, fd2, nr, nw, res, n, namlen;
  char cbuf1[MAX_PATH], *p;

  /* If the -j or -s flags were given, suppress certain files. */
  p = sp->namep;
  n = strlen(p);
  if (jflag) {
	if (strcmp(p, "a.out") == 0) return(0);
	if (strcmp(p, "core") == 0) return (0);
	if (strcmp(p + n - 2, ".o") == 0) return (0);
	if (strcmp(p + n - 2, ".Z") == 0) return (0);
	if (strcmp(p + n - 4, ".bak") == 0) return (0);
	if (strcmp(p + n - 4, ".log") == 0) return (0);
  }
  if (sflag) {
	if (strcmp(p + n - 2, ".s") == 0) return(0);
  }
  res = 0;
  if (dflag) return(0);		/* backup -d means only directories */
  strcpy(cbuf1, dir1);
  strncat(cbuf1, "/", 1);
  strncat(cbuf1, sp->namep, NAME_SIZE);	/* cbuf1 = source file name */

  /* At this point, cbuf1 contains the source file name, cbuf2 the target. */
  fd1 = open(cbuf1, O_RDONLY);
  if (fd1 < 0) {
	error(NONFATAL, "cannot open ", cbuf1, "");
	return(res);
  }
  fd2 = creat(cbuf2, (sp->mode | S_IWUSR) & 07777);
  if (fd2 < 0) {
	if (errno == ENFILE) {
		close(fd1);
		return(OUT_OF_SPACE);
	}
	error(NONFATAL, "cannot create ", cbuf2, "");
	close(fd1);
	return(res);
  }

  /* Both files are now open.  Do the copying. */
  namlen = strlen(sp->namep);
  if (sp->namep[namlen - 2] != '.' || sp->namep[namlen - 1] != 'Z')
	if (zflag && (namlen <= (NAME_SIZE - 2))) {
		close(fd1);
		close(fd2);
		res = zcopy(cbuf1, cbuf2);
		if (tflag) utime(cbuf2, (struct utimbuf *) & (sp->acctime));
		if (res != 0) unlink(cbuf2);	/* if error, get rid of the
					 * corpse */
		if (vflag && res == 0) printf("Backing up %s\n", cbuf1);
		return(res);
	}
  while (1) {
	nr = read(fd1, copybuf, COPY_SIZE);
	if (nr == 0) break;
	if (nr < 0) {
		error(NONFATAL, "read error on ", cbuf1, "");
		res = EIO;
		break;
	}
	nw = write(fd2, copybuf, nr);
	if (nw < 0) {
		if (errno == ENOSPC) {
			/* Device is full. */
			res = OUT_OF_SPACE;
			break;
		}

		/* True write error. */
		error(NONFATAL, "write error on ", cbuf2, "");
		res = EIO;
		break;
	}
  }
  if (res == 0) {
	if (vflag) printf("Backing up %s\n", cbuf1);
  } else {
	unlink(cbuf2);
  }
  close(fd1);
  close(fd2);
  if (tflag) utime(cbuf2, (struct utimbuf *) & (sp->acctime));
  return(res);
}


int zcopy(src, targ)
char *src, *targ;
{

  int pid, status, res, s;

  if ((pid = fork()) < 0) error(FATAL, "cannot fork", "", "");
  if (pid > 0) {
	wait(&status);

	/* Error codes 0 and 2 are ok, assume others mean disk is full. */
	res = (status == 0 || status == NO_SAVINGS ? 0 : OUT_OF_SPACE);
	return(res);
  } else {
	/* Child must execute compress. */
	close(1);
	s = open(targ, O_RDWR);
	if (s < 0) error(FATAL, "cannot write on ", "targ", "");
	execle("/bin/compress", "compress", "-fc", src, (char *) 0, environ);
	execle("/usr/bin/compress", "compress", "-fc", src, (char *)0,environ);
	error(FATAL, "cannot exec compress", "", "");
  }
}


copydir(dir1, dir2, namep)
char *dir1, *dir2, *namep;
{
/* Copy a directory. */

  int pid, res, status;
  char fbuf[20], d1buf[MAX_PATH], d2buf[MAX_PATH];

  res = 0;
  if (nflag) return(res);	/* backup -n means no directories */

  /* Handle directory copy by forking off 'backup' ! */
  strcpy(fbuf, "-r");
  if (jflag) strcat(fbuf, "j");
  if (mflag) strcat(fbuf, "m");
  if (sflag) strcat(fbuf, "s");
  if (tflag) strcat(fbuf, "t");
  if (vflag) strcat(fbuf, "v");
  if (zflag) strcat(fbuf, "z");
  strcpy(d1buf, dir1);
  strcat(d1buf, "/", 1);
  strncat(d1buf, namep, NAME_SIZE);
  strcpy(d2buf, dir2);
  strcat(d2buf, "/", 2);
  strncat(d2buf, namep, NAME_SIZE);

  if ((pid = fork()) < 0) error(FATAL, "cannot fork", "", "");
  if (pid > 0) {
	wait(&status);
	return;
  } else {
	execle("backup", "backup", fbuf, d1buf, d2buf, (char *) 0, environ);
	execle("/bin/backup", "backup", fbuf, d1buf, d2buf, (char *)0,environ);
	execle("/usr/bin/backup","backup",fbuf,d1buf,d2buf,(char *)0,environ);
	error(FATAL, "cannot recursively exec backup", "", "");
  }
}

newdisk(dir)
char *dir;
{
/* Ask for a new diskette. A big problem is that this program does not
 * know which device is being used and where it is mounted on.  As an
 * emergency solution, fork off a shell and ask the user to do the work.
 */

  int pid, status;

  printf("\nDiskette full. Please do the following:\n");
  printf("   1. Unmount the diskette using /etc/umount\n");
  printf("   2. Physically replace the diskette by the next one.\n");
  printf("   3. Mount the new diskette using /etc/mount\n");
  printf("   4. Type CTRL-D to return to the backup program\n");

  if ((pid = fork()) < 0) error(FATAL, "cannot fork", "", "");
  if (pid > 0) {
	wait(&status);
	maketarget(dir);	/* make the directory */
  } else {
	execle("/bin/sh", "sh", "-i", (char *) 0, environ);
	execle("/usr/bin/sh", "sh", "-i", (char *) 0, environ);
	error(FATAL, "cannot execute shell to ask for new diskette", "", "");
  }
}

usage()
{
  error(2, "Usage: backup [-djmnstvz] dir1 dir2", "", "");
}


error(type, s1, s2, s3)
int type;
char *s1, *s2, *s3;
{
  fprintf(stderr, "backup: %s%s%s\n", s1, s2, s3);

  if (type == NONFATAL)
	return;
  else
	exit(type);
}
