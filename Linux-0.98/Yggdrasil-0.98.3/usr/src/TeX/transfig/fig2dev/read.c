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
 *	FIG : Facility for Interactive Generation of figures
 *
 *	Copyright (c) 1985, 1988 by Supoj Sutanthavibul (supoj@sally.UTEXAS.EDU)
 *	January 1985.
 *	1st revision : August 1985.
 *	2nd revision : March 1988.
 *
 *	%W%	%G%
*/
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "alloc.h"
#include "object.h"

#if defined(hpux) || defined(SYSV)
#define bzero(s,n) memset((s),'\0',(n))
#endif

extern int            errno;

extern F_arrow		*make_arrow();
extern char		*calloc();

static F_ellipse	*read_ellipseobject();
static F_line		*read_lineobject();
static F_text		*read_textobject();
static F_spline		*read_splineobject();
static F_arc		*read_arcobject();
static F_compound	*read_compoundobject();

#define			FILL_CONVERT(f) \
				((v2_flag || (f) < WHITE_FILL) \
					? (f) : 21 - ((f)-1)*5)

#define			BUF_SIZE		1024

char			buf[BUF_SIZE];
int			line_no = 0;
int			num_object;
int			v2_flag;	/* Protocol V2.0 or V2.1 */
int			v21_flag;	/* Protocol V2.1 */

read_fail_message(file, err)
char	*file;
int	err;
{
	/* extern char	*sys_errlist[];     Linux declares this in <error.h> */

	if (err == 0)		/* Successful read */
	    return;
#if !defined(hpux) && !defined(SYSV)
	else if (err == ENAMETOOLONG)
	    put_msg("File name \"%s\" is too long", file);
#endif
	else if (err == ENOENT)
	    put_msg("File \"%s\" does not exist", file);
	else if (err == ENOTDIR)
	    put_msg("A name in the path \"%s\" is not a directory", file);
	else if (err == EACCES)
	    put_msg("Read access to file \"%s\" is blocked", file);
	else if (err == EISDIR)
	    put_msg("File \"%s\" is a directory", file);
	else if (err == -2) {
	    put_msg("File \"%s\" is empty", file);
	    }
	else if (err == -1) {
	    /* Format error; relevant error message is already delivered */
	    }
	else
	    put_msg("File \"%s\" is not accessable; %s", file, sys_errlist[err]);
	}

/**********************************************************
Read_fig returns :

     0 : successful read.
    -1 : File is in incorrect format
    -2 : File is empty
err_no : if file can not be read for various reasons

The resolution (ppi) and the cooridnate system (coord_sys) are
stored in obj->nwcorner.x and obj->nwcorner.x respectively.
**********************************************************/

read_fig(file_name, obj)
char		*file_name;
F_compound	*obj;
{
	FILE		*fp;

	if ((fp = fopen(file_name, "r")) == NULL)
	    return(errno);
	else
	    return(readfp_fig(fp, obj));
	}

readfp_fig(fp, obj)
FILE	*fp;
F_compound	*obj;
{
	char		c;
	int		status;

	num_object = 0;
	c = fgetc(fp);
	if (feof(fp)) return(-2);
	ungetc(c, fp);
	bzero((char*)obj, COMOBJ_SIZE);
	if (c == '#')
	    status = read_objects(fp, obj);
	else
	    status = read_1_3_objects(fp, obj);
	(void)fclose(fp);
	return(status);
	}
	
int
read_objects(fp, obj)
FILE		*fp;
F_compound	*obj;
{
	F_ellipse	*e, *le = NULL;
	F_line		*l, *ll = NULL;
	F_text		*t, *lt = NULL;
	F_spline	*s, *ls = NULL;
	F_arc		*a, *la = NULL;
	F_compound	*c, *lc = NULL;
	int		object, ppi, coord_sys;

	bzero((char*)obj, COMOBJ_SIZE);
	(void)fgets(buf, BUF_SIZE, fp);	/* get the version line */

	v2_flag = (!strncmp(buf, "#FIG 2", 6));
	v21_flag = (!strncmp(buf, "#FIG 2.1", 8));

	line_no++;
	if (get_line(fp) < 0) {
	    put_msg("File is truncated");
	    return(-1);
	    }
	if (2 != sscanf(buf,"%d%d\n", &ppi, &coord_sys)) {
	    put_msg("Incomplete data at line %d", line_no);
	    return(-1);
	    }

	obj->nwcorner.x = ppi;
	obj->nwcorner.y = coord_sys;
	while (get_line(fp) > 0) {
	    if (1 != sscanf(buf, "%d", &object)) {
		put_msg("Incorrect format at line %d", line_no);
		return(-1);
		}
	    switch (object) {
		case O_POLYLINE :
		    if ((l = read_lineobject(fp)) == NULL) return(-1);
		    if (ll)
			ll = (ll->next = l);
		    else 
			ll = obj->lines = l;
		    num_object++;
		    break;
		case O_SPLINE :
		    if ((s = read_splineobject(fp)) == NULL) return(-1);
		    if (ls)
			ls = (ls->next = s);
		    else 
			ls = obj->splines = s;
		    num_object++;
		    break;
		case O_ELLIPSE :
		    if ((e = read_ellipseobject()) == NULL) return(-1);
		    if (le)
			le = (le->next = e);
		    else 
			le = obj->ellipses = e;
		    num_object++;
		    break;
		case O_ARC :
		    if ((a = read_arcobject(fp)) == NULL) return(-1);
		    if (la)
			la = (la->next = a);
		    else 
			la = obj->arcs = a;
		    num_object++;
		    break;
		case O_TEXT :
		    if ((t = read_textobject(fp)) == NULL) return(-1);
		    if (lt)
			lt = (lt->next = t);
		    else 
			lt = obj->texts = t;
		    num_object++;
		    break;
		case O_COMPOUND :
		    if ((c = read_compoundobject(fp)) == NULL) return(-1);
		    if (lc)
			lc = (lc->next = c);
		    else 
			lc = obj->compounds = c;
		    num_object++;
		    break;
		default :
		    put_msg("Incorrect object code at line %d", line_no);
		    return(-1);
		} /*  switch */
	    } /*  while */
	if (feof(fp))
	    return(0);
	else
	    return(errno);
	} /*  read_objects */

static F_arc *
read_arcobject(fp)
FILE	*fp;
{
	F_arc	*a;
	int	n, fa, ba;
	int	type, style;
	double	thickness, wid, ht;

	if (NULL == (Arc_malloc(a))) {
	    put_msg(Err_mem);
	    return(NULL);
	    }
	a->pen = 0;
	a->area_fill = 0;
	a->for_arrow = NULL;
	a->back_arrow = NULL;
	a->next = NULL;
	n = sscanf(buf, "%*d%d%d%d%d%d%d%d%lf%d%d%d%lf%lf%d%d%d%d%d%d\n",
		&a->type, &a->style, &a->thickness, 
		&a->color, &a->depth, &a->pen, &a->area_fill, 
		&a->style_val, &a->direction, &fa, &ba,
		&a->center.x, &a->center.y, 
		&a->point[0].x, &a->point[0].y, 
		&a->point[1].x, &a->point[1].y, 
		&a->point[2].x, &a->point[2].y);
	if (n != 19) {
	    put_msg(Err_incomp, "arc", line_no);
	    free((char*)a);
	    return(NULL);
	    }
	a->area_fill = FILL_CONVERT(a->area_fill);
	skip_comment(fp);
	if (fa) {
	    line_no++;
	    if (5 != fscanf(fp, "%d%d%lf%lf%lf", &type, &style, &thickness, &wid, &ht)) {
		fprintf(stderr, Err_incomp, "arc", line_no);
		return(NULL);
		}
	    skip_line(fp);
	    a->for_arrow = make_arrow(type, style, thickness, wid, ht);
	    skip_comment(fp);
	    }
	skip_comment(fp);
	if (ba) {
	    line_no++;
	    if (5 != fscanf(fp, "%d%d%lf%lf%lf", &type, &style, &thickness, &wid, &ht)) {
		fprintf(stderr, Err_incomp, "arc", line_no);
		return(NULL);
		}
	    skip_line(fp);
	    a->back_arrow = make_arrow(type, style, thickness, wid, ht);
	    }
	return(a);
	}

static F_compound *
read_compoundobject(fp)
FILE	*fp;
{
	F_arc		*a, *la = NULL;
	F_ellipse	*e, *le = NULL;
	F_line		*l, *ll = NULL;
	F_spline	*s, *ls = NULL;
	F_text		*t, *lt = NULL;
	F_compound	*com, *c, *lc = NULL;
	int		n, object;

	Compound_malloc(com);
	com->arcs = NULL;
	com->ellipses = NULL;
	com->lines = NULL;
	com->splines = NULL;
	com->texts = NULL;
	com->compounds = NULL;
	com->next = NULL;
	n = sscanf(buf, "%*d%d%d%d%d\n", &com->nwcorner.x, &com->nwcorner.y,
		&com->secorner.x, &com->secorner.y);
	if (4 != n) {
	    put_msg(Err_incomp, "compound", line_no);
	    free((char*)com);
	    return(NULL);
	    }
	while (get_line(fp) > 0) {
	    if (1 != sscanf(buf, "%d", &object)) {
		put_msg(Err_incomp, "compound", line_no);
		free_compound(&com);
		return(NULL);
		}
	    switch (object) {
		case O_POLYLINE :
		    if ((l = read_lineobject(fp)) == NULL) { 
			free_line(&l);
			return(NULL);
			}
		    if (ll)
			ll = (ll->next = l);
		    else 
			ll = com->lines = l;
		    break;
		case O_SPLINE :
		    if ((s = read_splineobject(fp)) == NULL) { 
			free_spline(&s);
			return(NULL);
			}
		    if (ls)
			ls = (ls->next = s);
		    else 
			ls = com->splines = s;
		    break;
		case O_ELLIPSE :
		    if ((e = read_ellipseobject()) == NULL) { 
			free_ellipse(&e);
			return(NULL);
			}
		    if (le)
			le = (le->next = e);
		    else 
			le = com->ellipses = e;
		    break;
		case O_ARC :
		    if ((a = read_arcobject(fp)) == NULL) { 
			free_arc(&a);
			return(NULL);
			}
		    if (la)
			la = (la->next = a);
		    else 
			la = com->arcs = a;
		    break;
		case O_TEXT :
		    if ((t = read_textobject(fp)) == NULL) { 
			free_text(&t);
			return(NULL);
			}
		    if (lt)
			lt = (lt->next = t);
		    else 
			lt = com->texts = t;
		    break;
		case O_COMPOUND :
		    if ((c = read_compoundobject(fp)) == NULL) { 
			free_compound(&c);
			return(NULL);
			}
		    if (lc)
			lc = (lc->next = c);
		    else 
			lc = com->compounds = c;
		    break;
		case O_END_COMPOUND :
		    return(com);
		default :
		    put_msg("Wrong object code at line %d", line_no);
		    return(NULL);
		} /*  switch */
	    }
	if (feof(fp))
	    return(com);
	else
	    return(NULL);
	}

static F_ellipse *
read_ellipseobject()
{
	F_ellipse	*e;
	int		n;

	Ellipse_malloc(e);
	e->area_fill = 0;
	e->pen = 0;
	e->next = NULL;
	n = sscanf(buf, "%*d%d%d%d%d%d%d%d%lf%d%lf%d%d%d%d%d%d%d%d\n",
		&e->type, &e->style, &e->thickness,
		&e->color, &e->depth, &e->pen, &e->area_fill,
		&e->style_val, &e->direction, &e->angle,
		&e->center.x, &e->center.y, 
		&e->radiuses.x, &e->radiuses.y, 
		&e->start.x, &e->start.y, 
		&e->end.x, &e->end.y);
	if (n != 18) {
	    put_msg(Err_incomp, "ellipse", line_no);
	    free((char*)e);
	    return(NULL);
	    }
	e->area_fill = FILL_CONVERT(e->area_fill);
	return(e);
	}

static F_line *
read_lineobject(fp)
FILE	*fp;
{
	F_line	*l;
	F_point	*p, *q;
	int	n, x, y, fa, ba;
	int	type, style, radius_flag;
	double	thickness, wid, ht;

	Line_malloc(l);
	l->points = NULL;
	l->pen = 0;
	l->area_fill = 0;
	l->for_arrow = NULL;
	l->back_arrow = NULL;
	l->next = NULL;

	sscanf(buf,"%*d%d",&l->type);	/* get the line type */

	radius_flag = v21_flag || (v2_flag && l->type == T_ARC_BOX);
	if (radius_flag)
	    {
	    n = sscanf(buf, "%*d%d%d%d%d%d%d%d%lf%d%d%d",
	    &l->type, &l->style, &l->thickness, &l->color,
	    &l->depth, &l->pen, &l->area_fill, &l->style_val, &l->radius, &fa, &ba);
	    }
	/* old format uses pen for radius of arc-box corners */
	else
	    {
	    n = sscanf(buf, "%*d%d%d%d%d%d%d%d%lf%d%d",
		&l->type, &l->style, &l->thickness, &l->color,
		&l->depth, &l->pen, &l->area_fill, &l->style_val, &fa, &ba);
	    if (l->type == T_ARC_BOX)
		{
		l->radius = (int) l->pen;
		l->pen = 0;
		}
	    else
		l->radius = 0;
	    }
	if ((!radius_flag && n!=10) || (radius_flag && n!=11)) {
	    put_msg(Err_incomp, "line", line_no);
	    free((char*)l);
	    return(NULL);
	    }
	l->area_fill = FILL_CONVERT(l->area_fill);
	skip_comment(fp);
	if (fa) {
	    line_no++;
	    if (5 != fscanf(fp, "%d%d%lf%lf%lf", &type, &style, &thickness, &wid, &ht)) {
		fprintf(stderr, Err_incomp, "line", line_no);
		return(NULL);
		}
	    skip_line(fp);
	    l->for_arrow = make_arrow(type, style, thickness, wid, ht);
	    skip_comment(fp);
	    }
	if (ba) {
	    line_no++;
	    if (5 != fscanf(fp, "%d%d%lf%lf%lf", &type, &style, &thickness, &wid, &ht)) {
		fprintf(stderr, Err_incomp, "line", line_no);
		return(NULL);
		}
	    skip_line(fp);
	    l->back_arrow = make_arrow(type, style, thickness, wid, ht);
	    skip_comment(fp);
	    }
    	if (l->type == T_EPS_BOX) {
		line_no++;
		Eps_malloc(l->eps);
		if (l->eps  == NULL) {
		    free((char *) l);
		    return (NULL);
		}
		if (2 != fscanf(fp, "%d %s", &l->eps->flipped, l->eps->file)) {
	    		put_msg(Err_incomp,
				"Encapsulated Postscript", line_no);
	    		fprintf(stderr, Err_incomp,
				"Encapsulated Postscript", line_no);
	    	return (NULL);
		}
		read_epsf(l->eps);
    	} else
		l->eps = NULL;

	if (NULL == (l->points = Point_malloc(p))) {
	    put_msg(Err_mem);
	    return(NULL);
	    }
	p->next = NULL;
	if (fscanf(fp, "%d%d", &p->x, &p->y) != 2) {
	    put_msg(Err_incomp, "line", line_no);
	    free_linestorage(l);
	    return(NULL);
	    }
	for (;;) {
	    if (fscanf(fp, "%d%d", &x, &y) != 2) {
		put_msg(Err_incomp, "line", line_no);
		free_linestorage(l);
		return(NULL);
		}
	    if (x == 9999) break;
	    if (NULL == (Point_malloc(q))) {
		put_msg(Err_mem);
		free_linestorage(l);
		return(NULL);
		}
	    q->x = x;
	    q->y = y;
	    q->next = NULL;
	    p->next = q;
	    p = q;
	    }
	skip_line(fp);
	return(l);
	}

static F_spline *
read_splineobject(fp)
FILE	*fp;
{
	F_spline	*s;
	F_point		*p, *q;
	F_control	*cp, *cq;
	int		c, n, x, y, fa, ba;
	int		type, style;
	double		thickness, wid, ht;
	double		lx, ly, rx, ry;

	Spline_malloc(s);
	s->points = NULL;
	s->controls = NULL;
	s->pen = 0;
	s->area_fill = 0;
	s->for_arrow = NULL;
	s->back_arrow = NULL;
	s->next = NULL;

	n = sscanf(buf, "%*d%d%d%d%d%d%d%d%lf%d%d",
	    	&s->type, &s->style, &s->thickness, &s->color,
		&s->depth, &s->pen, &s->area_fill, &s->style_val, &fa, &ba);
	if (n != 10) {
	    put_msg(Err_incomp, "spline", line_no);
	    free((char*)s);
	    return(NULL);
	    }
	s->area_fill = FILL_CONVERT(s->area_fill);
	skip_comment(fp);
	if (fa) {
	    line_no++;
	    if (5 != fscanf(fp, "%d%d%lf%lf%lf", &type, &style, &thickness, &wid, &ht)) {
		fprintf(stderr, Err_incomp, "spline", line_no);
		return(NULL);
		}
	    skip_line(fp);
	    s->for_arrow = make_arrow(type, style, thickness, wid, ht);
	    skip_comment(fp);
	    }
	if (ba) {
	    line_no++;
	    if (5 != fscanf(fp, "%d%d%lf%lf%lf", &type, &style, &thickness, &wid, &ht)) {
		fprintf(stderr, Err_incomp, "spline", line_no);
		return(NULL);
		}
	    skip_line(fp);
	    s->back_arrow = make_arrow(type, style, thickness, wid, ht);
	    skip_comment(fp);
	    }

	/* Read points */
	if ((n = fscanf(fp, "%d%d", &x, &y)) != 2) {
	    put_msg(Err_incomp, "spline", line_no);
	    free_splinestorage(s);
	    return(NULL);
	    };
	if (NULL == (s->points = Point_malloc(p))) {
	    put_msg(Err_mem);
	    free_splinestorage(s);
	    return(NULL);
	    }
	p->x = x; p->y = y;
	for (c = 1;;) {
	    if (fscanf(fp, "%d%d", &x, &y) != 2) {
		put_msg(Err_incomp, "spline", line_no);
		p->next = NULL;
		free_splinestorage(s);
		return(NULL);
		};
	    if (x == 9999) break;
	    if (NULL == (Point_malloc(q))) {
		put_msg(Err_mem);
		free_splinestorage(s);
		return(NULL);
		}
	    q->x = x;
	    q->y = y;
	    p->next = q;
	    p = q;
	    c++;
	    }
	p->next = NULL;
	skip_line(fp);

	if (normal_spline(s)) return(s);

	skip_comment(fp);
	/* Read controls */
	if ((n = fscanf(fp, "%lf%lf%lf%lf", &lx, &ly, &rx, &ry)) != 4) {
	    put_msg(Err_incomp, "spline", line_no);
	    free_splinestorage(s);
	    return(NULL);
	    };
	if (NULL == (s->controls = Control_malloc(cp))) {
	    put_msg(Err_mem);
	    free_splinestorage(s);
	    return(NULL);
	    }
	cp->lx = lx; cp->ly = ly;
	cp->rx = rx; cp->ry = ry;
	while (--c) {
	    if (fscanf(fp, "%lf%lf%lf%lf", &lx, &ly, &rx, &ry) != 4) {
		put_msg(Err_incomp, "spline", line_no);
		cp->next = NULL;
		free_splinestorage(s);
		return(NULL);
		};
	    if (NULL == (Control_malloc(cq))) {
		put_msg(Err_mem);
		cp->next = NULL;
		free_splinestorage(s);
		return(NULL);
		}
	    cq->lx = lx; cq->ly = ly;
	    cq->rx = rx; cq->ry = ry;
	    cp->next = cq;
	    cp = cq;
	    }
	cp->next = NULL;

	skip_line(fp);
	return(s);
	}

static F_text *
read_textobject(fp)
FILE	*fp;
{
	F_text	*t;
	int	n, ignore = 0;
	char	s[BUF_SIZE], s_temp[BUF_SIZE], junk[2];

	Text_malloc(t);
	t->font = 0;
	t->size = 0.0;
	t->next = NULL;
	/* The text object is terminated by a CONTROL-A, so we read
	   everything up to the CONTROL-A and then read that character.
	   If we do not find the CONTROL-A on this line then this must
	   be a multi-line text object and we will have to read more. */
	n = sscanf(buf,"%*d%d%d%lf%d%d%d%lf%d%lf%lf%d%d%[^\1]%[\1]",
		&t->type, &t->font, &t->size, &t->pen,
		&t->color, &t->depth, &t->angle,
		&t->flags, &t->height, &t->length, 
		&t->base_x, &t->base_y, s, junk);
	if ((n != 14) && (n != 13)) {
	  put_msg(Err_incomp, "text", line_no);
	  free((char*)t);
/* 	  return(NULL); */
	}
	if (n == 13) {
	  /* Read in the remainder of the text object. */
	  do {
	    fgets(buf, BUF_SIZE, fp);
	    line_no++;  /* As is done in get_line */
	    n = sscanf(buf,"%[^\1]%[\1]", s_temp, junk);
	    /* Safety check */
	    if (strlen(s)+1 + strlen(s_temp)+1 > BUF_SIZE) {
	      /* Too many characters.  Ignore the rest. */
	      ignore = 1;
	    }
	    if (!ignore)
	      strcat(s, s_temp);
	  } while (n == 1);
	}
	if (strlen(s) == 0) (void)strcpy(s, " ");
	t->cstring = (char*)calloc((unsigned)(strlen(s)), sizeof(char));
	if (NULL == t->cstring) {
	    put_msg(Err_mem);
	    free((char*)t);
	    return(NULL);
	    }
	(void)strcpy(t->cstring, s+1);

	if (!v21_flag && (t->font == 0 || t->font == DEFAULT))
		t->flags = ((t->flags != DEFAULT) ? t->flags : 0)
				| SPECIAL_TEXT;

	if (v2_flag && !v21_flag && !special_text(t)) 
		t->flags = ((t->flags != DEFAULT) ? t->flags : 0)
				| PSFONT_TEXT;

	return(t);
      }

get_line(fp)
FILE	*fp;
{
	while (1) {
	    if (NULL == fgets(buf, BUF_SIZE, fp)) {
		return(-1);
		}
	    line_no++;
	    if (*buf != '\n' && *buf != '#') return(1);
			/* Skip empty and comment lines */
	    }
	}

skip_comment(fp)
FILE	*fp;
{
	char c;

	while ((c = fgetc(fp)) == '#') skip_line(fp);
	if (c != '#') ungetc(c, fp);
	}

skip_line(fp)
FILE	*fp;
{
	while (fgetc(fp) != '\n') {
	    if (feof(fp)) return;
	    }
	}

read_epsf(eps)
    F_eps          *eps;
{
    int             nbitmap;
    int             bitmapz;
    char           *cp;
    unsigned char  *mp;
    int             n;
    int             flag;
    char            buf[300];
    int             llx, lly, urx, ury;
    FILE           *epsf;
    register unsigned char *last;

    epsf = fopen(eps->file, "r");
    if (epsf == NULL) {
	put_msg("Cannot open file: %s", eps->file);
	return 0;
    }
    while (fgets(buf, 300, epsf) != NULL) {
	lower(buf);
	if (!strncmp(buf, "%%boundingbox", 13)) {
	    if (sscanf(buf, "%%%%boundingbox: %d %d %d %d",
		       &llx, &lly, &urx, &ury) < 4) {
		put_msg("Bad EPS bitmap file: %s", eps->file);
		fclose(epsf);
		return 0;
	    }
	    break;
	}
    }

    eps->hw_ratio = (float) (ury - lly) / (float) (urx - llx);

    eps->bitmap = NULL;
    eps->bit_size.x = 0;
    eps->bit_size.y = 0;

    eps->pix_flipped = 0;
    eps->pix_rotation = 0;
    eps->pix_width = 0;
    eps->pix_height = 0;

    if (ury - lly <= 0 || urx - llx <= 0) {
	put_msg("Bad values in EPS bitmap bounding box");
    }
    bitmapz = 0;

    /* look for a preview bitmap */
    while (fgets(buf, 300, epsf) != NULL) {
	lower(buf);
	if (!strncmp(buf, "%%beginpreview", 14)) {
	    sscanf(buf, "%%%%beginpreview: %d %d %d",
		   &eps->bit_size.x, &eps->bit_size.y, &bitmapz);
	    break;
	}
    }

    if (eps->bit_size.x > 0 && eps->bit_size.y > 0 && bitmapz == 1) {
	nbitmap = (eps->bit_size.x + 7) / 8 * eps->bit_size.y;
	eps->bitmap = (unsigned char *) malloc(nbitmap);
	if (eps->bitmap == NULL)
	    fprintf(stderr, "could not allocate %d bytes of memory\n", nbitmap);
    }
    /* read for a preview bitmap */
    if (eps->bitmap != NULL) {
	mp = eps->bitmap;
	bzero(mp, nbitmap);	/* init bitmap to zero */
	last = eps->bitmap + nbitmap;
	flag = 1;
	while (fgets(buf, 300, epsf) != NULL && mp < last) {
	    lower(buf);
	    if (!strncmp(buf, "%%endpreview", 12) ||
		!strncmp(buf, "%%endimage", 10))
		break;
	    cp = buf;
	    if (*cp != '%')
		break;
	    cp++;
	    while (*cp != '\0') {
		if (isxdigit(*cp)) {
		    n = hex(*cp);
		    if (flag) {
			flag = 0;
			*mp = n << 4;
		    } else {
			flag = 1;
			*mp = *mp + n;
			mp++;
			if (mp >= last)
			    break;
		    }
		}
		cp++;
	    }
	}
    }
    fclose(epsf);
    return 1;
}

int
hex(c)
    char            c;
{
    if (isdigit(c))
	return (c - 48);
    else
	return (c - 87);
}

lower(buf)
    char           *buf;
{
    while (*buf) {
	if (isupper(*buf))
	    *buf = (char) tolower(*buf);
	buf++;
    }
}
