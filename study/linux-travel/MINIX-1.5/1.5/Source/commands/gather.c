/* gather - collect files for mailing	Author: Andy Tanenbaum */

/* It sometimes happens that one needs to mail a large directory full of
 * files to someone.  This program can be used to collect these files into
 * shar archives, compress and uuencode them. The interesting property that
 * it has is that it makes sure that none of the archives are too big, and
 * that no files are split over two archives.
 *
 * Syntax: gather [-s source_dir] [-d dest_dir] [-b max_arch_size] [-f file]
 *
 *	-s source directory	(where are the files to be sent)
 *	-d destination dir	(where should the archives be put)
 *	-b bytes		(maximum size of the archives; default 60K)
 *	-f file			(use file_00.uue etc as archive names)
 *
 * Examples:
 *	gather				# make 60K archives in this dir
 *	gather  -d mailings -b 50000	# make 50K archives in mailings
 *
 * Note:
 *	The maximum size given by -b (default 60000 bytes) is only an
 *	approximation, since it is hard to tell how big the final file
 *	will be after shar'ing, compressing, and uue'ing.  A heuristic
 *	is used.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>

#define DEFAULT        60000	/* default archive size */
#define MAX_DIR_ENT      512	/* how many directory entries allowed */
#define HEAP_SIZE      20000	/* storage size for all file names */
#define NAME_SIZE       4096	/* storage size for current command */
#define BASE_SIZE          7	/* max number of chars in basename */
#define PATH_MAX         512	/* largest path name */
#define NUMERATOR       138L	/* heuristic parameter */
#define DENOMINATOR     100L	/* heuristic parameter */

char heap[HEAP_SIZE + 2];	/* dir entries stored here */
char names[NAME_SIZE];		/* file name lists constructed here */
char work[NAME_SIZE];		/* scratch buffer */
char base_name[BASE_SIZE + 1];	/* base name to use for the archives */
char target[PATH_MAX];		/* storage for target file names */

struct dir_ent {
  char *file_name;
  long file_size;
} dir_ent[MAX_DIR_ENT];

long atol(), heuristic();
char *getcwd();

main(argc, argv)
int argc;
char *argv[];
{
/* Parse the command and get ready. */

  int i, counter, l, s, nonlocal;
  char *p, num[3];
  struct dirent *d;
  DIR *dirp;
  struct stat stbuf;
  int first;			/* first entry not used yet */
  int limit;			/* number of files in src_dir */
  char *src_dir = ".";		/* pointer to source directory */
  char *dst_dir = ".";		/* pointer to destination directory */
  char *file = "";		/* name to use */
  long max_bytes = DEFAULT;	/* max archive size (approx.) */
  long cutoff;			/* max cumulative input size */

  if (argc > 9) usage();
  i = 1;
  while (i < argc) {
	/* Examine the i-th argument. */
	p = argv[i];
	if (*p != '-') usage();
	switch (*(p + 1)) {
	    case 's':	src_dir = argv[i + 1];	break;
	    case 'd':	dst_dir = argv[i + 1];	break;
	    case 'f':	file = argv[i + 1];	break;

	    case 'b':
		max_bytes = atol(argv[i + 1]);
		if (max_bytes <= 0) {
			fprintf(stderr, "gather: bad -b value\n");
			exit(1);
		}
		break;

	    default:
		fprintf(stderr, "gather: unknown flag %s\n", p);
		exit(1);
	}
	i += 2;
  }

  /* Determine the basename. */
  get_basename(src_dir, file);

  /* Open the source directory. */
  i = 0;
  p = heap;
  if ((dirp = opendir(src_dir)) == (DIR *) NULL) {
	fprintf(stderr, "gather: cannot open %s\n", src_dir);
	exit(2);
  }

  /* Read in all the file names. */
  while (1) {
	d = readdir(dirp);
	if (d == (struct dirent *) NULL) break;
	l = strlen(d->d_name);
	if (p + l >= &heap[HEAP_SIZE] || i >= MAX_DIR_ENT) {
		fprintf(stderr, "gather: %s is too large\n", src_dir);
		exit(2);
	}
	strcpy(work, src_dir);
	strcat(work, "/");
	strcat(work, d->d_name);
	stat(work, &stbuf);
	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) continue;
	dir_ent[i].file_name = p;
	strcpy(p, d->d_name);
	dir_ent[i].file_size = stbuf.st_size;
	p += l + 1;
	i++;
  }
  limit = i;
  closedir(dirp);

  /* Sort the names. */
  sort_dir(limit);

  /* Figure out when to stop reading files. */
  cutoff = heuristic(max_bytes);

  /* Collect files into archives. */
  first = 0;
  counter = 0;
  while (first < limit) {
	first = collect(first, limit, cutoff);
	num[0] = '0' + (counter / 10);
	num[1] = '0' + (counter % 10);
	num[2] = 0;

	/* Construct full path of compressed target. */
	target[0] = 0;
	if (strcmp(dst_dir, ".") != 0) {
		strcpy(target, dst_dir);
		strcat(target, "/");
	}
	strcat(target, base_name);
	strcat(target, "_");
	strcat(target, num);
	strcat(target, ".Z");

	/* (cd src; shar file ... | compress -fc) >dir/base.00.Z */
	nonlocal = strcmp(src_dir, ".");
	work[0] = 0;
	if (nonlocal) {
		strcat(work, "(cd ");
		strcat(work, src_dir);
		strcat(work, "; ");
	}
	strcat(work, "shar ");
	strcat(work, names);
	strcat(work, " | compress -fc ");
	if (nonlocal) strcat(work, ")");
	strcat(work, " >");
	strcat(work, target);
	s = system(work);
	if (s < 0) {
		fprintf(stderr, "gather: shar command failed\n");
		exit(2);
	}

	/* Uue dir/base.00.Z */
	strcpy(work, "uue ");
	strcat(work, target);
	strcat(work, "\n");
	s = system(work);
	if (s < 0) {
		fprintf(stderr, "gather: uue command failed\n");
		exit(2);
	}

	/* Unlink dir/base.00.Z */
	unlink(target);

	counter++;
  }
}

int collect(first, limit, cutoff)
int first;
int limit;
long cutoff;
{
/* See how many files will fit in an archive. */

  int nr_files;
  long cum_size, size;
  struct dir_ent *p, *endp;

  names[0] = 0;
  p = &dir_ent[first];
  endp = &dir_ent[limit];
  nr_files = 0;
  cum_size = 0;

  while (p < endp) {
	size = p->file_size;
	if (size > cutoff) {
		fprintf(stderr, "gather: %s is too big\n", p->file_name);
		exit(2);
	}

	/* First peek to see if next file fits.  If not, maybe some
	 * other file can be used instead.  Swap them. */
	if (cum_size + size > cutoff) fudge(p, endp, cutoff - cum_size);

	/* If it fails now, there is no file that will fit. */
	size = p->file_size;
	if (cum_size + size > cutoff) return(p - dir_ent);
	strcat(names, p->file_name);
	strcat(names, " ");
	cum_size += size;
	p++;
  }
  return(p - dir_ent);
}

long heuristic(m)
long m;
{
  /* The basic algorithm is to collect files up to some limit, and put
   * them in an archive.  It is tricky to determine how many files to
   * collect, because they will be shar'ed, compressed and uue'ed.
   * Thus we need a heuristic for guessing how to relate the total size
   * of the input files to the size of the final uue archive.  This
   * heuristic is contained in this procedure.  It takes the desired
   * final size as input and produces the file cutoff as output. */

  return((NUMERATOR * m) / DENOMINATOR);
}



sort_dir(limit)
int limit;			/* how many entries in dir_ent */
{
/* Sort the directory using bubble sort. */

  struct dir_ent *p, *q;

  for (p = &dir_ent[0]; p < &dir_ent[limit - 1]; p++) {
	for (q = p + 1; q < &dir_ent[limit]; q++) {
		if (strcmp(p->file_name, q->file_name) > 0) swap(p, q);
	}
  }
}

swap(p, q)
struct dir_ent *p, *q;
{
  /* Exchange two entries. */

  char *cp;
  long l;

  cp = p->file_name;
  l = p->file_size;
  p->file_name = q->file_name;
  p->file_size = q->file_size;
  q->file_name = cp;
  q->file_size = l;
}


fudge(p, endp, size)
struct dir_ent *p, *endp;
long size;
{
/* Look for a file that will fit (i.e., <= size). This fudging gives a more
 * uniform distribution, and reduces the number of files needed.
 */

  register struct dir_ent *q;

  for (q = p + 1; q < endp; q++) {
	if (q->file_size <= size) {
		swap(p, q);
		return;
	}
  }
}


get_basename(s, file)
char *s;
char *file;
{
/* Determine the basename and copy it to base_name. */

  int fd, n;
  char *p, *q;

  if (*file != 0) p = file;
  else if (strcmp(s, ".") == 0) {
	if (getcwd(work, NAME_SIZE) == (char *) NULL) {
		fprintf(stderr, "gather: could not get name of working dir\n");
		exit(2);
	}
	p = work;
  } else {
	p = s;
  }

  q = p + strlen(p) - 1;
  if (*q == '\n') {
	*q = 0;
	q--;
  }
  while (1) {
	if (q < p || *q == '/') break;
	q--;
  }
  strncpy(base_name, q + 1, BASE_SIZE);
}

usage()
{
  fprintf(stderr, "Usage: gather [-b bytes] [-s src_dir] [-d dst_dir] [-f file]\n");
  exit(1);
}
