/* traverse - traverse a tree		Author:	Gary Perlman */

#include <sys/types.h>
#include <sys/dir.h>
#include <stdio.h>

#define STANDALONE

#define	DIR	FILE
#define	MAXNAME (DIRSIZ+2)
#define	opendir(path) fopen (path, "r")
#define closedir(dirp) fclose (dirp)

struct direct *readdir(dirp)
DIR *dirp;
{
  static struct direct entry;
  if (dirp == NULL) return((struct direct *) NULL);
  for (;;) {
	if (fread(&entry, sizeof(struct direct), 1, dirp) == 0)
		return((struct direct *) NULL);
	if (entry.d_ino) return(&entry);
  }
}

char *strncpy();

char *namedir(entry)
struct direct *entry;
{
  static char name[MAXNAME];
  return(strncpy(name, entry->d_name, DIRSIZ));
}


#include <sys/stat.h>
#define	isdir(path) (stat(path, &buf) ? 0 : (buf.st_mode&S_IFMT)==S_IFDIR)

traverse(path, func)
char *path;
int (*func) ();
{
  DIR *dirp;
  struct direct *entry;
  struct stat buf;
  int filetype = isdir(path) ? 'd' : 'f';
  (*func) (path, filetype, 0);
  if (filetype == 'd') {
	if (chdir(path) == 0) {
		if (dirp = opendir(".")) {
			while (entry = readdir(dirp)) {
				char name[MAXNAME];
				(void) strcpy(name, namedir(entry));
				if (strcmp(name, ".") && strcmp(name, ".."))
					traverse(name, func);
			}
			(void) closedir(dirp);
		}
		(void) chdir("..");
	}
  }
  (*func) (path, filetype, 1);
}

#ifdef STANDALONE

static Indent = 0;
tryverse(file, type, pos)
char *file;
{
  int in;
  if (pos == 0) {
	for (in = 0; in < Indent; in++) putchar('\t');
	if (type == 'd') {
		printf("%s/\n", file);
		Indent++;
	} else
		puts(file);
  } else if (type == 'd')
	Indent--;
}

main(argc, argv)
char **argv;
{
  int tryverse();
  char *root = argc > 1 ? argv[1] : ".";
  traverse(root, tryverse);
}

#endif
