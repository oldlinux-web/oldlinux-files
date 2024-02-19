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

#include <stdio.h>
#include <math.h>
#include "object.h"

free_arc(list)
F_arc	**list;
{
	F_arc	*a, *arc;

	for (a = *list; a != NULL;) {
	    arc = a;
	    a = a->next;
	    if (arc->for_arrow) free((char*)arc->for_arrow);
	    if (arc->back_arrow) free((char*)arc->back_arrow);
	    free((char*)arc);
	    } 
	*list = NULL;
	}

free_compound(list)
F_compound	**list;
{
	F_compound	*c, *compound;

	for (c = *list; c != NULL;) {
	    compound = c;
	    c = c->next;
	    free_arc(&compound->arcs);
	    free_compound(&compound->compounds);
	    free_ellipse(&compound->ellipses);
	    free_line(&compound->lines);
	    free_spline(&compound->splines);
	    free_text(&compound->texts);
	    free((char*)compound);
	    } 
	*list = NULL;
	}

free_ellipse(list)
F_ellipse	**list;
{
	F_ellipse	*e, *ellipse;

	for (e = *list; e != NULL;) {
	    ellipse = e;
	    e = e->next;
	    free((char*)ellipse);
	    } 
	*list = NULL;
	}

free_line(list)
F_line	**list;
{
	F_line	*l, *line;

	for (l = *list; l != NULL;) {
	    line = l;
	    l = l->next;
	    free_linestorage(line);
	    } 
	*list = NULL;
	}

free_text(list)
F_text	**list;
{
	F_text	*t, *text;

	for (t = *list; t != NULL;) {
	    text = t;
	    t = t->next;
	    cfree(text->cstring);
	    free((char*)text);
	    } 
	*list = NULL;
	}

free_spline(list)
F_spline	**list;
{
	F_spline	*s, *spline;

	for (s = *list; s != NULL;) {
	    spline = s;
	    s = s->next;
	    free_splinestorage(spline);
	    }
	*list = NULL;
	}

free_splinestorage(s)
F_spline      *s;
{
        F_point		*p, *q;
        F_control	*a, *b;

        for (p = s->points; p != NULL; p = q) {
            q = p->next;
            free((char*)p);
            }
        for (a = s->controls; a != NULL; a = b) {
            b = a->next;
            free((char*)a);
            }
	if (s->for_arrow) free((char*)s->for_arrow);
	if (s->back_arrow) free((char*)s->back_arrow);
        free((char*)s);
        }

free_linestorage(l)
F_line	*l;
{
	F_point	*p, *q;

	for (p = l->points; p != NULL; p = q) {
	    q = p->next;
	    free((char*)p);
	    }
	if (l->for_arrow) free((char*)l->for_arrow);
	if (l->back_arrow) free((char*)l->back_arrow);
	free((char*)l);
	}
