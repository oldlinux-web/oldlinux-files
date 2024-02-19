/* mv - move files		Author: Adri Koppes */

/* 4/25/87 - J. Paradis		Bug fixes for directory handling
 * 3/15/88 - P. Housel		More directory bug fixes
 * 1/21/89 - B. Evans		Allow owner to move non-writable file.
 *				Don't allow move to non-writable file.
 *				Delete target file in case of moving to dir.
 *				Requires making file readable before copy.
 *				Fix up mode after copy (cp uses the target
 *				mode, if any, and strips high bits).
 *				Use adequate size for dotdot buffer.
 * Jan  90 - B. Evans		The directory above /foo was calculated as
 *				"" instead of "/", so "mv /bin /bin0" left
 *				the old bin/. and bin/.. links in the
 *				current directory or nowhere.
 *
 *				BUGS.
 *				There are race conditions all over.
 *				Error messages are not specific.
 *				There are magic sizes 64 and 128 instead of
 *				sizes related to PATH_MAX.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>

struct stat st, st1;

main(argc, argv)
int argc;
char **argv;
{
  char *destdir;

  if (argc < 3) {
	std_err("Usage: mv file1 file2 or mv dir1 dir2 or mv file1 file2 ... dir\n");
	exit(1);
  }
  if (argc == 3) {
	if (stat(argv[1], &st)) {
		std_err("mv: ");
		std_err(argv[1]);
		std_err(" doesn't exist\n");
		exit(1);
	}
	move(argv[1], argv[2]);
  } else {
	destdir = argv[--argc];
	if (stat(destdir, &st)) {
		std_err("mv: target directory ");
		std_err(destdir);
		std_err(" doesn't exist\n");
		exit(1);
	}
	if ((st.st_mode & S_IFMT) != S_IFDIR) {
		std_err("mv: target ");
		std_err(destdir);
		std_err(" not a directory\n");
		exit(1);
	}
	while (--argc) move(*++argv, destdir);
  }
  exit(0);
}

move(old, new)
char *old, *new;
{
  int retval;
  char name[64];
  char parent[64];
  char *oldbase;
  int i;

  if ((oldbase = strrchr(old, '/')) == 0)
	oldbase = old;
  else
	++oldbase;

  /* It's too dangerous to fool with "." or ".." ! */
  if ((strcmp(oldbase, ".") == 0) || (strcmp(oldbase, "..") == 0)) {
	cant(old);
  }

  /* Don't move a non-existent file. */
  if (stat(old, &st) != 0) cant(old);

  /* If source is not writable, don't move it unless user owns it. */
  if (access(old, 2) != 0 && st.st_uid != getuid()) cant(old);

  /* If target is a directory, form its full name. The error of moving
   * a directory to itself is caught later. */
  if (stat(new, &st1) == 0 && (st1.st_mode & S_IFMT) == S_IFDIR) {
	if ((strlen(oldbase) + strlen(new) + 2) > 64) cant(old);
	strcpy(name, new);
	strcat(name, "/");
	strcat(name, oldbase);
	new = name;
  }

  /* If target exists, do various overwriting checks. */
  if (stat(new, &st1) == 0) {

	/* Don't move a file to itself. */
	if (st.st_dev == st1.st_dev && st.st_ino == st1.st_ino) cant(old);

	/* Don't move on top of a directory. */
	if ((st1.st_mode & S_IFMT) == S_IFDIR) cant(old);

	/* Don't move on top of a non-writable file. */
	if (access(new, 2) != 0) cant(old);
  }
  strcpy(parent, new);

  for (i = (strlen(parent) - 1); i > 0; i--) {
	if (parent[i] == '/') break;
  }

  if (i == 0) {
	if (parent[0] == '/')
		parent[1] = '\0';	/* parent of "/bin" is "/", not "." */
	else
		strcpy(parent, ".");
  } else {
	/* Null-terminate name at last slash */
	parent[i] = '\0';
  }

  /* Prevent moving a directory into its own subdirectory */
  if ((st.st_mode & S_IFMT) == S_IFDIR) {
	char lower[128];
	short int prevdev = -1;
	unsigned short previno;

	strcpy(lower, parent);
	while (1) {
		if (stat(lower, &st1) || (st1.st_dev == st.st_dev
					&& st1.st_ino == st.st_ino))
			cant(old);

		/* Stop at root */
		if (st1.st_dev == prevdev && st1.st_ino == previno) break;
		prevdev = st1.st_dev;
		previno = st1.st_ino;
		strcat(lower, "/..");
	}
  }

  /* If target exists, it is a file. Delete it so that following link()
   * works except across file systems, to avoid copying. */
  if (stat(new, &st1) == 0) unlink(new);

  if (link(old, new))
	if ((st.st_mode & S_IFMT) != S_IFDIR) {
		switch (fork()) {
		    case 0:
			if (!(st.st_mode & S_IRUSR))
				chmod(old, st.st_mode | S_IRUSR);
			setgid(getgid());
			setuid(getuid());
			execl("/bin/cp", "cp", old, new, (char *) 0);
			execl("/usr/bin/cp", "cp", old, new, (char *) 0);
			cant(old);
		    case -1:
			std_err("mv: can't fork\n");
			exit(1);
		    default:
			wait(&retval);
			if (!(st.st_mode & S_IRUSR)) chmod(old, st.st_mode);
			if (retval) cant(old);
			chmod(new, st.st_mode);
		}
	} else
		cant(old);

  /* If this was a directory that we moved, then we have * to update
   * its ".." entry (in case it was moved some- * where else in the
   * tree...) */
  if ((st.st_mode & S_IFMT) == S_IFDIR) {
	char dotdot[64 + 3];

	/* Unlink the ".." entry */
	strcpy(dotdot, new);
	strcat(dotdot, "/..");
	unlink(dotdot);

	/* Now link it to its parent */
	link(parent, dotdot);
  }
  utime(new, &st.st_atime);
  unlink(old);
}

cant(name)
char *name;
{
  std_err("mv: can't move ");
  std_err(name);
  std_err("\n");
  exit(1);
}
