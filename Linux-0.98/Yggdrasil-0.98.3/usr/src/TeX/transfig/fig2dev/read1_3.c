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

/*******************************************************************/
/***************       Read version 1.3 format       ***************/
/*******************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "alloc.h"
#include "object.h"

/*******    Fig 1.3 subtype of objects    *******/
#define			DRAW_ELLIPSE_BY_RAD 	1
#define			DRAW_ELLIPSE_BY_DIA 	2
#define			DRAW_CIRCLE_BY_RAD 	3
#define			DRAW_CIRCLE_BY_DIA 	4
#define			DRAW_CIRCULAR_ARC	5
#define			DRAW_POLYLINE		6
#define			DRAW_BOX		7
#define			DRAW_POLYGON		8
#define			DRAW_TEXT		9
#define			DRAW_SPLINE		10
#define			DRAW_CLOSEDSPLINE	11
#define			DRAW_COMPOUND		13

extern F_arrow		*forward_arrow(), *backward_arrow();
extern int		figure_modified;
extern int		errno;
/* extern char		*sys_errlist[];	  Linux declares this in <error.h> */
extern int		sys_nerr, errno;

static F_ellipse	*read_ellipseobject();
static F_line		*read_lineobject();
static F_text		*read_textobject();
static F_spline		*read_splineobject();
static F_arc		*read_arcobject();
static F_compound	*read_compoundobject();

extern int		line_no;
extern int		num_object;

int
read_1_3_objects(fp, obj)
FILE		*fp;
F_compound	*obj;
{
	F_ellipse	*e, *le = NULL;
	F_line		*l, *ll = NULL;
	F_text		*t, *lt = NULL;
	F_spline	*s, *ls = NULL;
	F_arc		*a, *la = NULL;
	F_compound	*c, *lc = NULL;
	int		 n;
	int		 object, pixperinch, canvaswid, canvasht, coord_sys;

	n = fscanf(fp,"%d%d%d%d\n", &pixperinch, &coord_sys, &canvaswid, &canvasht);
	if (n != 4) {
	    put_msg("Incorrect format in the first line in input file");
	    return(-1);
	    }
	obj->nwcorner.x = pixperinch;
	obj->nwcorner.y = coord_sys;
	while (fscanf(fp, "%d", &object) == 1) {
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
		    if ((e = read_ellipseobject(fp)) == NULL) return(-1);
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
		default:
		    put_msg("Incorrect object code %d", object);
		    return(-1);
		} /*  switch */
	    } /*  while */
	if (feof(fp))
	    return(0);
	else
	    return(errno);
	}

static F_arc *
read_arcobject(fp)
FILE	*fp;
{
	F_arc	*a;
	int	f, b, h, w, n;

	Arc_malloc(a);
	a->type = T_3_POINTS_ARC;
      	a->color = BLACK_COLOR;
	a->depth = 0;
	a->pen = 0;
	a->for_arrow = NULL;
	a->back_arrow = NULL;
	a->next = NULL;
	n = fscanf(fp, " %d %d %d %lf %d %d %d %d %d %lf %lf %d %d %d %d %d %d\n",
		&a->type, &a->style, &a->thickness, 
		&a->style_val, &a->direction, &f, &b,
		&h, &w, &a->center.x, &a->center.y, 
		&a->point[0].x, &a->point[0].y, 
		&a->point[1].x, &a->point[1].y, 
		&a->point[2].x, &a->point[2].y);
	if (n != 17) {
	    put_msg("incomplete arc data");
	    free((char*)a);
	    return(NULL);
	    }
	if (f) {
	    a->for_arrow = forward_arrow();
	    a->for_arrow->wid = w;
	    a->for_arrow->ht = h;
	    }
	if (b) {
	    a->back_arrow = backward_arrow();
	    a->back_arrow->wid = w;
	    a->back_arrow->ht = h;
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
	int		 n, object;

	Compound_malloc(com);
	com->arcs = NULL;
	com->ellipses = NULL;
	com->lines = NULL;
	com->splines = NULL;
	com->texts = NULL;
	com->compounds = NULL;
	com->next = NULL;
	n = fscanf(fp, " %d %d %d %d\n", &com->nwcorner.x, &com->nwcorner.y,
		&com->secorner.x, &com->secorner.y);
	if (n != 4) {
	    put_msg("Incorrect compound object format");
	    return(NULL);
	    }
	while (fscanf(fp, "%d", &object) == 1) {
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
		    if ((e = read_ellipseobject(fp)) == NULL) { 
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
		} /*  switch */
	    }
	if (feof(fp))
	    return(com);
	else {
	    put_msg("Format error: %s", sys_errlist[errno]);
	    return(NULL);
	    }
	}

static F_ellipse *
read_ellipseobject(fp)
FILE	*fp;
{
	F_ellipse	*e;
	int		n, t;

	Ellipse_malloc(e);
      	e->color = BLACK_COLOR;
	e->angle = 0.0;
	e->depth = 0;
	e->pen = 0;
	e->area_fill = 0;
	e->next = NULL;
	n = fscanf(fp," %d %d %d %lf %d %d %d %d %d %d %d %d %d\n", 
		&t, &e->style,
		&e->thickness, &e->style_val, &e->direction, 
		&e->center.x, &e->center.y, 
		&e->radiuses.x, &e->radiuses.y, 
		&e->start.x, &e->start.y, 
		&e->end.x, &e->end.y);
	if (n != 13) {
	    put_msg("incomplete ellipse data");
	    free((char*)e);
	    return(NULL);
	    }
	if (t == DRAW_ELLIPSE_BY_RAD)
	    e->type = T_ELLIPSE_BY_RAD;
	else if (t == DRAW_ELLIPSE_BY_DIA)
	    e->type = T_ELLIPSE_BY_DIA;
	else if (t == DRAW_CIRCLE_BY_RAD)
	    e->type = T_CIRCLE_BY_RAD;
	else
	    e->type = T_CIRCLE_BY_DIA;
	return(e);
	}

static F_line *
read_lineobject(fp)
FILE			*fp;
{
	F_line	*l;
	F_point	*p, *q;
	int	f, b, h, w, n, t, x, y;

	Line_malloc(l);
      	l->color = BLACK_COLOR;
	l->depth = 0;
	l->pen = 0;
	l->area_fill = 0;
	l->for_arrow = NULL;
	l->back_arrow = NULL;
	l->next = NULL;
	l->points = Point_malloc(p);
	n = fscanf(fp, " %d %d %d %lf %d %d %d %d %d %d", &t, 
		&l->style, &l->thickness, &l->style_val,
		&f, &b, &h, &w, &p->x, &p->y);
	if (n != 10) {
	    put_msg("incomplete line data");
	    free((char*)l);
	    return(NULL);
	    }
	if (t == DRAW_POLYLINE)
	    l->type = T_POLYLINE;
	else if (t == DRAW_POLYGON)
	    l->type = T_POLYGON;
	else
	    l->type = T_BOX;
	if (f) {
	    l->for_arrow = forward_arrow();
	    l->for_arrow->wid = w;
	    l->for_arrow->ht = h;
	    }
	if (b) {
	    l->back_arrow = backward_arrow();
	    l->back_arrow->wid = w;
	    l->back_arrow->ht = h;
	    }
	for (;;) {
	    if (fscanf(fp, " %d %d", &x, &y) != 2) {
		put_msg("incomplete line object");
		free_linestorage(l);
		return(NULL);
		}
	    if (x == 9999) break;
	    Point_malloc(q);
	    q->x = x;
	    q->y = y;
	    q->next = NULL;
	    p->next = q;
	    p = q;
	    }
	return(l);
	}

static F_spline *
read_splineobject(fp)
FILE	*fp;
{
	F_spline	*s;
	F_point		*p, *q;
	int		f, b, h, w, n, t, x, y;

	Spline_malloc(s);
      	s->color = BLACK_COLOR;
	s->depth = 0;
	s->pen = 0;
	s->area_fill = 0;
	s->for_arrow = NULL;
	s->back_arrow = NULL;
	s->controls = NULL;
	s->next = NULL;
	s->points = Point_malloc(p);
	n = fscanf(fp, " %d %d %d %lf %d %d %d %d %d %d", 
	    	&t, &s->style, &s->thickness, &s->style_val,
	    	&f, &b,
	    	&h, &w, &p->x, &p->y);
	if (n != 10) {
	    put_msg("incomplete spline data");
	    free((char*)s);
	    return(NULL);
	    }
	if (t == DRAW_CLOSEDSPLINE)
	    s->type = T_CLOSED_NORMAL;
	else
	    s->type = T_OPEN_NORMAL;
	if (f) {
	    s->for_arrow = forward_arrow();
	    s->for_arrow->wid = w;
	    s->for_arrow->ht = h;
	    }
	if (b) {
	    s->back_arrow = backward_arrow();
	    s->back_arrow->wid = w;
	    s->back_arrow->ht = h;
	    }
	for (;;) {
	    if (fscanf(fp, " %d %d", &x, &y) != 2) {
		put_msg("incomplete spline object");
		free_splinestorage(s);
		return(NULL);
		};
	    if (x == 9999) break;
	    Point_malloc(q);
	    q->x = x;
	    q->y = y;
	    q->next = NULL;
	    p->next = q;
	    p = q;
	    }
	return(s);
	}

static F_text *
read_textobject(fp)
FILE	*fp;
{
	F_text	*t;
	int	n;
	char	buf[128];

	Text_malloc(t);
	t->type = T_LEFT_JUSTIFIED;
	t->flags = 0;
      	t->color = BLACK_COLOR;
	t->depth = 0;
	t->pen = 0;
	t->angle = 0.0;
	t->next = NULL;
	n = fscanf(fp," %d %d %d %d %d %d %d %[^\n]", &t->font, 
		&t->size, &t->flags, &t->height, &t->length, 
		&t->base_x, &t->base_y, buf);
	if (n != 8) {
	    put_msg("incomplete text data");
	    free((char*)t);
	    return(NULL);
	    }
	t->cstring = (char *) calloc((unsigned)(strlen(buf)+1), sizeof(char));
	if (t->cstring == NULL) {
	    put_msg(Err_mem);
	    free((char*) t);
	    return(NULL);
	    }
	(void)strcpy(t->cstring, buf);
	if (t->size == 0) t->size = 18;
	return(t);
	}
