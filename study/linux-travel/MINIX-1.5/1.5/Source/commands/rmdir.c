/* rmdir - remove a directory		Author: Adri Koppes

/* (modified by Paul Polderman)
 * (modified by Bjarne Steinsbo)	Fixed "rmdir ../anything"
 *					Modified style to standard Minix
 *					Added some comments.
 */

#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <limits.h>
#include <string.h>

int error = 0;

main(argc, argv)
register int argc;
register char **argv;
{
  if (argc < 2) {
	prints("Usage: rmdir dir ...\n");
	exit(1);
  }
  signal(SIGHUP, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  while (--argc) remove(*++argv);
  if (error) exit(1);
}


remove(dirname)
char *dirname;
{
  struct direct d;		/* buffer for reading directory */
  struct stat s, cwd;		/* buffers for `stat' call */
  int fd = 0;
  int sl = 0;
  int n;
  char dots[PATH_MAX];		/* scratch buffer for dirname */
  register char *p;

  /* Is the path name too long ? Check once and for all. */
  if (strlen(dirname) > PATH_MAX - 3) {	/* Need to append `/..' */
	stderr2("path name too long : ", dirname);
	std_err("\n");
	error++;
	return;
  }

  /* Does the file exist ? */
  if (stat(dirname, &s)) {
	stderr2(dirname, " doesn't exist\n");
	error++;
	return;
  }

  /* Is it a directory ? */
  if ((s.st_mode & S_IFMT) != S_IFDIR) {
	stderr2(dirname, " not a directory\n");
	error++;
	return;
  }

  /* If path ends in /., fix it (e.g., /usr/ast/. ==> /usr/ast). */
  while (1) {
	n = strlen(dirname);
	if (n > 2 && dirname[n-2] == '/' && dirname[n-1] == '.') 
		dirname[n-2] = 0;
	else
		break;
  }
  
  /* Are we trying to remove "." or ".." ? */
  if (p = strrchr(dirname, '/'))
	p++;
  else
	p = dirname;
  if (strcmp(p, ".") == 0 || strcmp(p, "..") == 0) {
	stderr2(dirname, " will not remove \".\" or \"..\"\n");
	error++;
	return;
  }

  /* Write permission in parent directory ? */
  strcpy(dots, dirname);
  while (dirname[fd])
	if (dirname[fd++] == '/') sl = fd;
  dots[sl] = '\0';
  if (*dots == '\0') strcpy(dots, ".");
  if (access(dots, 2)) {
	stderr2(dirname, " no permission\n");
	error++;
	return;
  }

  /* Are we trying to remove current directory ? */
  stat(".", &cwd);
  if ((s.st_ino == cwd.st_ino) && (s.st_dev == cwd.st_dev)) {
	std_err("rmdir: can't remove current directory\n");
	error++;
	return;
  }

  /* Is it possible to open the directory ? */
  if ((fd = open(dirname, O_RDONLY)) < 0) {
	stderr2("can't read ", dirname);
	std_err("\n");
	error++;
	return;
  }

  /* Is the directory empty ? (except "." and "..") */
  while(read(fd, (char *) &d, sizeof(struct direct)) == sizeof(struct direct)){
	if (d.d_ino != 0) {
		if (strcmp(d.d_name, ".") && strcmp(d.d_name, "..")) {
			stderr2(dirname, " not empty\n");
			close(fd);
			error++;
			return;
		}
	}
  }
  close(fd);

  /* Will the path name be invalidated when dirname/. or dirname/.. is
   * unlinked ? In that case, fix the path-name ! */
  strcpy(dots, dirname);
  patch_path(dots);

  /* OK, let's do the rmdir. */
  if (rmdir(dots) != 0) {
	stderr2("can't remove ", dots);
	std_err("\n");
	error++;
	return;
  }
}

stderr2(s1, s2)
char *s1, *s2;
{
  std_err("rmdir: ");
  std_err(s1);
  std_err(s2);
}

/* With s pointing to the first char in the next part of the pathname,
 * check if this part is empty (/), dot (./) or dotdot (../)
 */
#define IS_EMPTY(s)  (*(s) == '/')
#define IS_DOT(s)    (*(s) == '.' && *((s)+1) == '/')
#define IS_DOTDOT(s) (*(s) == '.' && *((s)+1) == '.' && *((s)+2) == '/')

patch_path(dir)
char *dir;
{
/* Check if the path name will be invalidated when `dirname/..' and
 * `dirname/.' is later unlinked. Return a (possibly) patched path.
 * Do this by cleaning the path up, i.e. removing unnecessary parts
 * of the path. `anypath/anything/../' , `anypath/./' and `anypath//'
 * are all considered equal to `anypath/' . This assumption will break
 * when symbolic links are (are they ?) introduced. Don't remove those
 * `../' parts that are essential to the path name.
 */

  register char *p;
  char *last;
  int level = 0;

  if (*dir == '/')		/* absolute ? */
	last = dir + 1;
  else
	last = dir;

  p = last;
  while (*p != '\0') {		/* clean up the path name */
	if (IS_EMPTY(p)) {	/* reduce `//' to `/' */
		StrCpy(p, p + 1);
		continue;
	}
	if (IS_DOT(p)) {	/* reduce `/./'  to `/' */
		StrCpy(p, p + 2);
		continue;
	}
	if (IS_DOTDOT(p)) {	/* reduce `/anything/../' to `/' */
		if (level > 0) {/* is it possible to reduce? */
			--level;
			StrCpy(last, p + 3);
			p = last;
			last -= 2;
			while (*last != '/' && last > dir) --last;
			if (*last == '/') last++;
		} else
			last = p += 3;
		continue;
	}
	last = p;
	level++;
	while (*p != '\0' && *p++ != '/');	/* get next part of path
						 * name */
  }
}

StrCpy(s, t)
register char *s, *t;
{
/* Overlapping copies are implemetation-dependent in strcpy, so we'll
 * use our own version.
 */

  while (*s++ = *t++)		/* do nothing */
	;
}
