/* cpdir - copy directory  	 	Author: Erik Baalbergen */

/* Use "cpdir [-v] src dst" to make a copy dst of directory src.
   Cpdir should behave like the UNIX shell command
  (cd src; tar cf - .) | (mkdir dst; cd dst; tar xf -)

   The -m "merge" flag enables you to copy into an existing directory.
   The -s "similar" flag preserves the full mode, uid, gid and times.
   The -v "verbose" flag enables you to see what's going on when running cpdir.

   Work yet to be done:

  - link checks, i.e. am I not overwriting a file/directory by itself?

  Please report bugs and suggestions to erikb@cs.vu.nl
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define MKDIR1 "/bin/mkdir"
#define MKDIR2 "/usr/bin/mkdir"

#define BUFSIZE 1024
#define PLEN	256
#define DIRSIZ	16

#define MAXLINKS 512

struct {
  unsigned short ino;
  unsigned short dev;
  char *path;
} links[MAXLINKS];
int nlinks = 0;

char *prog;
int vflag = 0;			/* verbose */
int mflag = 0;			/* force */
int sflag = 0;			/* similar */
char *strcpy();
char *malloc();

main(argc, argv)
char *argv[];
{
  int rc = 0;
  char *p, *s;
  struct stat st;

  prog = *argv++;
  if ((p = *argv) && *p == '-') {
	argv++;
	argc--;
	while (*++p) {
		switch (*p) {
		    case 'v':	vflag = 1;	break;
		    case 'm':	mflag = 1;	break;
		    case 's':	sflag = 1;	break;
		    default:
			fatal("illegal flag %s", p);
		}
	}
  }
  if (argc != 3) fatal("Usage: cpdir [-msv] source destination");
  s = *argv++;
  if (stat(s, &st) < 0) fatal("can't get file status of %s", s);
  if ((st.st_mode & S_IFMT) != S_IFDIR) fatal("%s is not a directory", s);
  cpdir(&st, s, *argv);
  exit(0);
}

cpdir(sp, s, d)
struct stat *sp;
char *s, *d;
{
  char spath[PLEN], dpath[PLEN];
  char ent[DIRSIZ + 1];
  register char *send = spath, *dend = dpath;
  int fd, n;
  struct stat st;
  static first = 1;
  static dev_t dev;
  static ino_t ino;

  if ((fd = open(s, O_RDONLY)) < 0) {
	nonfatal("can't read directory %s", s);
	return;
  }
  if (
      mflag == 0
      ||
      stat(d, &st) != 0
      ||
      (st.st_mode & S_IFMT) != S_IFDIR
	) {
	make_dir(d);
	if (sflag) similar(sp, d);
  }
  if (first) {
	stat(d, &st);
	dev = st.st_dev;
	ino = st.st_ino;
	first = 0;
  }
  if (sp->st_dev == dev && sp->st_ino == ino) {
	nonfatal("%s skipped to avoid an endless loop", s);
	return;
  }
  while (*send++ = *s++) {
  }
  while (*dend++ = *d++) {
  }
  send[-1] = '/';
  dend[-1] = '/';
  ent[DIRSIZ] = '\0';
  while ((n = read(fd, ent, DIRSIZ)) == DIRSIZ) {
	if (!((ent[0] == '\0' && ent[1] == '\0') || (ent[2] == '.') &&
	      (ent[3] == '\0' || (ent[3] == '.' && ent[4] == '\0'))
	      )) {
		strcpy(send, ent + 2);
		strcpy(dend, ent + 2);
		if (stat(spath, &st) < 0)
			fatal("can't get file status of %s", spath);
		if ((st.st_mode & S_IFMT) != S_IFDIR && st.st_nlink > 1)
			if (cplink(st, spath, dpath) == 1) continue;
		switch (st.st_mode & S_IFMT) {
		    case S_IFDIR:
			cpdir(&st, spath, dpath);
			break;
		    case S_IFREG:
			cp(&st, spath, dpath);
			break;
		    default:
			cpspec(&st, spath, dpath);
			break;
		}
	}
  }
  close(fd);
  if (n) fatal("error in reading directory %s", spath);
}

make_dir(s)
char *s;
{
  int pid, status;

  if (vflag) printf("mkdir %s\n", s);
  if ((pid = fork()) == 0) {
	execl(MKDIR1, "mkdir", s, (char *) 0);
	execl(MKDIR2, "mkdir", s, (char *) 0);
	fatal("can't execute %s or %s", MKDIR1, MKDIR2);
  }
  if (pid == -1) fatal("can't fork", prog);
  wait(&status);
  if (status) fatal("can't create %s", s);
}

cp(sp, s, d)
struct stat *sp;
char *s, *d;
{
  char buf[BUFSIZE];
  int sfd, dfd, n;

  if (vflag) printf("cp %s %s\n", s, d);
  if ((sfd = open(s, O_RDONLY)) < 0)
	nonfatal("can't read %s", s);
  else {
	if ((dfd = creat(d, sp->st_mode & 0777)) < 0)
		fatal("can't create %s", d);
	while ((n = read(sfd, buf, BUFSIZE)) > 0)
		if (write(dfd, buf, n) != n)
			fatal("error in writing file %s", d);
	close(sfd);
	close(dfd);
	if (n) fatal("error in reading file %s", s);
	if (sflag) similar(sp, d);
  }
}

similar(sp, d)
struct stat *sp;
char *d;
{
  time_t timep[2];

  chmod(d, sp->st_mode);
  chown(d, sp->st_uid, sp->st_gid);
  timep[0] = sp->st_atime;
  timep[1] = sp->st_mtime;
  utime(d, timep);
}

nonfatal(s, a)
char *s, *a;
{
  fprintf(stderr, "%s: ", prog);
  fprintf(stderr, s, a);
  fprintf(stderr, "\n");
}

fatal(s, a)
char *s, *a;
{
  nonfatal(s, a);
  exit(1);
}

cpspec(sp, s, d)
struct stat *sp;
char *s, *d;
{
  if (vflag) {
	printf("copy special file %s to %s.", s, d);
	printf(" Major/minor = %d/%d.",
	       sp->st_rdev >> 8, sp->st_rdev & 0177);
	printf(" Mode = %o.\n", sp->st_mode);
  }
  if (mknod(d, sp->st_mode, sp->st_rdev, 0) < 0) {
	perror("mknod");
	nonfatal("Cannot create special file %s.\n", d);
  }
  if (sflag) similar(sp, d);
}

cplink(st, spath, dpath)
struct stat st;
char *spath, *dpath;
{
  /* Handle files that are links. Returns 0 if file must be copied.
   * Returns 1 if file has been successfully linked. */
  int i;
  int linkent;

  linkent = -1;
  for (i = 0; i < nlinks; i++) {
	if (links[i].dev == st.st_dev
	    && links[i].ino == st.st_ino)
		linkent = i;
  }
  if (linkent >= 0) {		/* It's already in the link table *//* we
			 * must have copied it earlier. So just link
			 * to the saved dest path. 		Don't copy it
			 * twice. */
	if (vflag) printf("ln %s %s\n", links[linkent].path, dpath);
	if (link(links[linkent].path, dpath) < 0)
		fatal("Could not link to %s\n", dpath);
	return(1);		/* Don't try to copy it */
  } else {			/* Make an entry in the link table */
	if (nlinks >= MAXLINKS) fatal("Too many links at %s\n", dpath);
	links[nlinks].dev = st.st_dev;
	links[nlinks].ino = st.st_ino;
	links[nlinks].path = malloc(strlen(dpath) + 1);
	if (links[nlinks].path == NULL)
		fatal("No more memory at %s\n", dpath);
	strcpy(links[nlinks].path, dpath);
	nlinks++;
	/* Go ahead and copy it the first time */
	return(0);
  }
}
