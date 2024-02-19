/*
 * TransFig: Facility for Translating Fig code
 * Copyright (c) 1991 Micah Beck, Cornell University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Cornell University not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Cornell University makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * CORNELL UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL CORNELL UNIVERSITY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* 
 *	Fig2dev : General Fig code translation program
 *
*/
#if defined(hpux) || defined(SYSV)
#include <sys/types.h>
#endif
#include <sys/file.h>
#include <stdio.h>
#include <ctype.h>
#include "patchlevel.h"
#include "object.h"
#include "fig2dev.h"
#include "drivers.h"

extern int getopt();
extern char *optarg;
extern int optind;

#define DEFAULT_FONT_SIZE 11

struct driver *dev = NULL;

char		Usage[] = "Usage: %s [-L language] [-f font] [-s size] [-m scale] [input [output]]\n";
char		Err_badarg[] = "Argument -%c unkown to %s driver.";
char		Err_incomp[] = "Incomplete %s object at line %d.";
char		Err_mem[] = "Running out of memory.";

char		*prog;
char		*from = NULL, *to = NULL;
int		font_size = 0;
double		mag = 1.0;
FILE		*tfp = NULL;
int		llx = 0, lly = 0, urx = 0, ury = 0;

struct obj_rec {
	void (*gendev)();
	char *obj;
	int depth;
};

put_msg(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
char   *format, *arg1, *arg2, *arg3, *arg4, *arg5, *arg6, *arg7, *arg8;
{
	fprintf(stderr, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
	fprintf(stderr, "\n");
	}

get_args(argc, argv)
int	 argc;
char	*argv[];
{
  	int	c, i;
	double	atof();

	prog = *argv;
/* add :? */
	while ((c = getopt(argc, argv, "acd:f:l:L:m:Pp:s:S:vVwW?")) != EOF) {

	  /* generic option handling */
	  switch (c) {

		case 'V': 
			fprintf(stderr, "TransFig Version %s Patchlevel %s\n",
							VERSION, PATCHLEVEL);
			exit(0);
			break;

		case 'L':			/* set output language */
		    for (i=0; *drivers[i].name; i++) 
			if (!strcmp(optarg, drivers[i].name))
				dev = drivers[i].dev;
		    if (!dev) {
			fprintf(stderr,
				"Unknown graphics language %s\n", optarg);
			fprintf(stderr,"Known languages are:\n");
			/* display available languages - 23/01/90 */
			for (i=0; *drivers[i].name; i++)
				fprintf(stderr,"%s ",drivers[i].name);
			fprintf(stderr,"\n");
			exit(1);
		    }
		    break;

		case 's':			/* set default font size */
		    font_size = atoi(optarg);
		    break;

		case 'm':			/* set magnification */
		    mag = atof(optarg);
		    break;

		case '?':			/* usage 		*/
			fprintf(stderr,Usage,prog);
			exit(1);
	    }

	    /* pass options through to driver */
	    if (!dev) {
		fprintf(stderr, "No graphics language specified.\n");
		exit(1);
	    }
	    dev->option(c, optarg);
      	}
      	if (!dev) {
		fprintf(stderr, "No graphics language specified.\n");
		exit(1);
      	}

	/* default font size is scaled if not specified */
	if (!font_size) font_size = DEFAULT_FONT_SIZE*mag + 0.5;

	if (optind < argc) from = argv[optind++];  /*  from file  */
	if (optind < argc) to   = argv[optind];  /*  to file    */
}

main(argc, argv)
int	 argc;
char	*argv[];
{
	F_compound	objects;
	int		status;

	get_args(argc, argv);

	if (to == NULL)
	    tfp = stdout;
	else if ((tfp = fopen(to, "w")) == NULL) {
	    fprintf(stderr, "Couldn't open %s", to);
	    fprintf(stderr, Usage, prog);
	    exit(1);
	    }

	if (from)
	    status = read_fig(from, &objects);
	else	/* read from stdin */
	    status = readfp_fig(stdin, &objects);

	if (status != 0) {
	    if (from) read_fail_message(from, status);
	    exit(1);
	    }
	gendev_objects(&objects, dev);
	if (tfp != stdout) (void)fclose(tfp);
	exit(0);
	}

/* count primitive objects & create pointer array */
static int compound_dump(com, array, count, dev)
F_compound *com;
struct obj_rec *array;
int count;
struct driver *dev;
{
  	F_arc		*a;
	F_compound	*c;
	F_ellipse	*e;
	F_line		*l;
	F_spline	*s;
	F_text		*t;

	for (c = com->compounds; c != NULL; c = c->next)
	  count = compound_dump(c, array, count, dev);
	for (a = com->arcs; a != NULL; a = a->next) {
	  if (array) {
		array[count].gendev = dev->arc;
		array[count].obj = (char *)a;
		array[count].depth = a->depth;
	  }
	  count += 1;
	}
	for (e = com->ellipses; e != NULL; e = e->next) {
	  if (array) {
		array[count].gendev = dev->ellipse;
		array[count].obj = (char *)e;
		array[count].depth = e->depth;
	  }
	  count += 1;
	}
	for (l = com->lines; l != NULL; l = l->next) {
	  if (array) {
		array[count].gendev = dev->line;
		array[count].obj = (char *)l;
		array[count].depth = l->depth;
	  }
	  count += 1;
	}
	for (s = com->splines; s != NULL; s = s->next) {
	  if (array) {
		array[count].gendev = dev->spline;
		array[count].obj = (char *)s;
		array[count].depth = s->depth;
	  }
	  count += 1;
	}
	for (t = com->texts; t != NULL; t = t->next) {
	  if (array) {
		array[count].gendev = dev->text;
		array[count].obj = (char *)t;
		array[count].depth = t->depth;
	  }
	  count += 1;
	}
	return count;
}

gendev_objects(objects, dev)
F_compound	*objects;
struct driver *dev;
{
	F_arc		*a;
	F_compound	*c;
	F_ellipse	*e;
	F_line		*l;
	F_spline	*s;
	F_text		*t;

	int obj_count, rec_comp();
	struct obj_rec *rec_array, *r; 

	if (0 == (double)objects->nwcorner.x) {
	    fprintf(stderr, "Resolution is zero!! default to 80 ppi\n");
	    objects->nwcorner.x = 80;
	    }
	if (objects->nwcorner.y != 1 && objects->nwcorner.y != 2) {
	    fprintf(stderr, "Wrong coordinate system; cannot continue\n");
	    return;
	    }

	/* Compute bounding box of objects, supressing texts if indicated */
	compound_bound(objects, &llx, &lly, &urx, &ury, dev->text_include);

	/* dump object pointers to an array */
	obj_count = compound_dump(objects, 0, 0, dev);
	if (!obj_count) {
	    fprintf(stderr, "No object");
	    return;
	    }
	rec_array = (struct obj_rec *)malloc(obj_count*sizeof(struct obj_rec));
	(void)compound_dump(objects, rec_array, 0, dev);

	/* sort object array by depth */
	qsort(rec_array, obj_count, sizeof(struct obj_rec), rec_comp);

	/* generate header */
	(*dev->start)(objects);

	/* generate objects in sorted order */
	for (r = rec_array; r<rec_array+obj_count; r++)
		(*(r->gendev))(r->obj);

	/* generate trailer */
	(*dev->end)();
}

int rec_comp(r1, r2)
struct obj_rec *r1, *r2;
{
	return (r2->depth - r1->depth);
}

/* null operation */
void gendev_null() {};
