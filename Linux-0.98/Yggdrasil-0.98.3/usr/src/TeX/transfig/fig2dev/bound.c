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
#include "pi.h"
#include "object.h"

#define		Ninety_deg		M_PI_2
#define		One_eighty_deg		M_PI
#define		Two_seventy_deg		(M_PI + M_PI_2)
#define		Three_sixty_deg		(M_PI + M_PI)
#define		round(x)		((int) ((x) + ((x >= 0)? 0.5: -0.5)))
#define		half(z1 ,z2)		((z1+z2)/2.0)
#define		max(a, b)		(((a) > (b)) ? (a) : (b))
#define		min(a, b)		(((a) < (b)) ? (a) : (b))

arc_bound(arc, xmin, ymin, xmax, ymax)
F_arc	*arc;
int	*xmin, *ymin, *xmax, *ymax;
{
	double	alpha, beta;
	double	dx, dy, radius;
	int	bx, by, sx, sy;

	dx = arc->point[0].x - arc->center.x;
	dy = arc->center.y - arc->point[0].y;
	alpha = atan2(dy, dx);
	if (alpha < 0.0) alpha += Three_sixty_deg;
	/* compute_angle returns value between 0 to 2PI */
	
	radius = hypot(dx, dy);

	dx = arc->point[2].x - arc->center.x;
	dy = arc->center.y - arc->point[2].y;
	beta = atan2(dy, dx);
	if (beta < 0.0) beta += Three_sixty_deg;

	bx = max(arc->point[0].x, arc->point[1].x);
	bx = max(arc->point[2].x, bx);
	by = max(arc->point[0].y, arc->point[1].y);
	by = max(arc->point[2].y, by);
	sx = min(arc->point[0].x, arc->point[1].x);
	sx = min(arc->point[2].x, sx);
	sy = min(arc->point[0].y, arc->point[1].y);
	sy = min(arc->point[2].y, sy);

	if (arc->direction == 1) { /* counter clockwise */
	    if (alpha > beta) {
		if (alpha <= 0 || 0 <= beta)
		    bx = (int)(arc->center.x + radius + 1.0);
		if (alpha <= Ninety_deg || Ninety_deg <= beta)
		    sy = (int)(arc->center.y - radius - 1.0);
		if (alpha <= One_eighty_deg || One_eighty_deg <= beta)
		    sx = (int)(arc->center.x - radius - 1.0);
		if (alpha <= Two_seventy_deg || Two_seventy_deg <= beta)
		    by = (int)(arc->center.y + radius + 1.0);
		}
	    else {
		if (0 <= beta && alpha <= 0)
		    bx = (int)(arc->center.x + radius + 1.0);
		if (Ninety_deg <= beta && alpha <= Ninety_deg)
		    sy = (int)(arc->center.y - radius - 1.0);
		if (One_eighty_deg <= beta && alpha <= One_eighty_deg)
		    sx = (int)(arc->center.x - radius - 1.0);
		if (Two_seventy_deg <= beta && alpha <= Two_seventy_deg)
		    by = (int)(arc->center.y + radius + 1.0);
		}
	    }
	else {	/* clockwise	*/
	    if (alpha > beta) {
		if (beta <= 0 && 0 <= alpha)
		    bx = (int)(arc->center.x + radius + 1.0);
		if (beta <= Ninety_deg && Ninety_deg <= alpha)
		    sy = (int)(arc->center.y - radius - 1.0);
		if (beta <= One_eighty_deg && One_eighty_deg <= alpha)
		    sx = (int)(arc->center.x - radius - 1.0);
		if (beta <= Two_seventy_deg && Two_seventy_deg <= alpha)
		    by = (int)(arc->center.y + radius + 1.0);
		}
	    else {
		if (0 <= alpha || beta <= 0)
		    bx = (int)(arc->center.x + radius + 1.0);
		if (Ninety_deg <= alpha || beta <= Ninety_deg)
		    sy = (int)(arc->center.y - radius - 1.0);
		if (One_eighty_deg <= alpha || beta <= One_eighty_deg)
		    sx = (int)(arc->center.x - radius - 1.0);
		if (Two_seventy_deg <= alpha || beta <= Two_seventy_deg)
		    by = (int)(arc->center.y + radius + 1.0);
		}
	    }
	*xmax = bx; *ymax = by;
	*xmin = sx; *ymin = sy;
	}

compound_bound(compound, xmin, ymin, xmax, ymax, include)
F_compound	*compound;
int		*xmin, *ymin, *xmax, *ymax;
int		include;
{
	F_arc		*a;
	F_ellipse	*e;
	F_compound	*c;
	F_spline	*s;
	F_line		*l;
	F_text		*t;
	int		bx, by, sx, sy, first = 1;
	int		llx, lly, urx, ury;

   while(compound != NULL)
   {
	for (a = compound->arcs; a != NULL; a = a->next) {
	    arc_bound(a, &sx, &sy, &bx, &by);
	    if (first) {
		first = 0;
		llx = sx; lly = sy;
		urx = bx; ury = by;
		}
	    else {
		llx = min(llx, sx); lly = min(lly, sy);
		urx = max(urx, bx); ury = max(ury, by);
		}
	    }

	for (c = compound->compounds; c != NULL; c = c->next) {
	    compound_bound(c, &sx, &sy, &bx, &by);
	    if (first) {
		first = 0;
		llx = sx; lly = sy;
		urx = bx; ury = by;
		}
	    else {
		llx = min(llx, sx); lly = min(lly, sy);
		urx = max(urx, bx); ury = max(ury, by);
		}
	    }

	for (e = compound->ellipses; e != NULL; e = e->next) {
	    ellipse_bound(e, &sx, &sy, &bx, &by);
	    if (first) {
		first = 0;
		llx = sx; lly = sy;
		urx = bx; ury = by;
		}
	    else {
		llx = min(llx, sx); lly = min(lly, sy);
		urx = max(urx, bx); ury = max(ury, by);
		}
	    }

	for (l = compound->lines; l != NULL; l = l->next) {
	    line_bound(l, &sx, &sy, &bx, &by);
	    if (first) {
		first = 0;
		llx = sx; lly = sy;
		urx = bx; ury = by;
		}
	    else {
		llx = min(llx, sx); lly = min(lly, sy);
		urx = max(urx, bx); ury = max(ury, by);
		}
	    }

	for (s = compound->splines; s != NULL; s = s->next) {
	    spline_bound(s, &sx, &sy, &bx, &by);
	    if (first) {
		first = 0;
		llx = sx; lly = sy;
		urx = bx; ury = by;
		}
	    else {
		llx = min(llx, sx); lly = min(lly, sy);
		urx = max(urx, bx); ury = max(ury, by);
		}
	    }

	for (t = compound->texts; t != NULL; t = t->next) {
	    text_bound(t, &sx, &sy, &bx, &by, include);
	    if (first) {
		first = 0;
		llx = sx; lly = sy;
		urx = bx; ury = by;
		}
	    else {
		llx = min(llx, sx); lly = min(lly, sy);
		urx = max(urx, bx); ury = max(ury, by);
		}
	    }
        compound = compound->next;
     }

	*xmin = llx; *ymin = lly;
	*xmax = urx; *ymax = ury;
	}

ellipse_bound(e, xmin, ymin, xmax, ymax)
F_ellipse	*e;
int		*xmin, *ymin, *xmax, *ymax;
{
	*xmin = e->center.x - e->radiuses.x;
	*ymin = e->center.y - e->radiuses.y;
	*xmax = e->center.x + e->radiuses.x;
	*ymax = e->center.y + e->radiuses.y;
	}

line_bound(l, xmin, ymin, xmax, ymax)
F_line	*l;
int	*xmin, *ymin, *xmax, *ymax;
{
	points_bound(l->points, xmin, ymin, xmax, ymax);
	}

spline_bound(s, xmin, ymin, xmax, ymax)
F_spline	*s;
int		*xmin, *ymin, *xmax, *ymax;
{
	if (int_spline(s)) {
	    int_spline_bound(s, xmin, ymin, xmax, ymax);
	    }
	else {
	    normal_spline_bound(s, xmin, ymin, xmax, ymax);
	    }
	}

int_spline_bound(s, xmin, ymin, xmax, ymax)
F_spline	*s;
int		*xmin, *ymin, *xmax, *ymax;
{
	F_point		*p1, *p2;
	F_control	*cp1, *cp2;
	double		x0, y0, x1, y1, x2, y2, x3, y3, sx1, sy1, sx2, sy2;
	double		tx, ty, tx1, ty1, tx2, ty2;
	double		sx, sy, bx, by;

	p1 = s->points;
	sx = bx = p1->x;
	sy = by = p1->y;
	cp1 = s->controls;
	for (p2 = p1->next, cp2 = cp1->next; p2 != NULL;
		p1 = p2, cp1 = cp2, p2 = p2->next, cp2 = cp2->next) {
	    x0 = p1->x; y0 = p1->y;
	    x1 = cp1->rx; y1 = cp1->ry;
	    x2 = cp2->lx; y2 = cp2->ly;
	    x3 = p2->x; y3 = p2->y;
	    tx = half(x1, x2); ty = half(y1, y2);
	    sx1 = half(x0, x1); sy1 = half(y0, y1);
	    sx2 = half(sx1, tx); sy2 = half(sy1, ty);
	    tx2 = half(x2, x3); ty2 = half(y2, y3);
	    tx1 = half(tx2, tx); ty1 = half(ty2, ty);

	    sx = min(x0, sx); sy = min(y0, sy);
	    sx = min(sx1, sx); sy = min(sy1, sy);
	    sx = min(sx2, sx); sy = min(sy2, sy);
	    sx = min(tx1, sx); sy = min(ty1, sy);
	    sx = min(tx2, sx); sy = min(ty2, sy);
	    sx = min(x3, sx); sy = min(y3, sy);

	    bx = max(x0, bx); by = max(y0, by);
	    bx = max(sx1, bx); by = max(sy1, by);
	    bx = max(sx2, bx); by = max(sy2, by);
	    bx = max(tx1, bx); by = max(ty1, by);
	    bx = max(tx2, bx); by = max(ty2, by);
	    bx = max(x3, bx); by = max(y3, by);
	    }
	*xmin = round(sx);
	*ymin = round(sy);
	*xmax = round(bx);
	*ymax = round(by);
	}

normal_spline_bound(s, xmin, ymin, xmax, ymax)
F_spline	*s;
int		*xmin, *ymin, *xmax, *ymax;
{
	F_point	*p;
	double	cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4;
	double	x1, y1, x2, y2, sx, sy, bx, by;
	double	px, py, qx, qy;

	p = s->points;
	x1 = p->x;  y1 = p->y;
	p = p->next;
	x2 = p->x;  y2 = p->y;
	cx1 = (x1 + x2) / 2.0;   cy1 = (y1 + y2) / 2.0;
	cx2 = (cx1 + x2) / 2.0;  cy2 = (cy1 + y2) / 2.0;
	if (closed_spline(s)) {
	    x1 = (cx1 + x1) / 2.0;
	    y1 = (cy1 + y1) / 2.0;
	    }
	sx = min(x1, cx2); sy = min(y1, cy2);
	bx = max(x1, cx2); by = max(y1, cy2);

	for (p = p->next; p != NULL; p = p->next) {
	    x1 = x2;  y1 = y2;
	    x2 = p->x;  y2 = p->y;
	    cx4 = (x1 + x2) / 2.0; cy4 = (y1 + y2) / 2.0;
	    cx3 = (x1 + cx4) / 2.0; cy3 = (y1 + cy4) / 2.0;
	    cx2 = (cx4 + x2) / 2.0;  cy2 = (cy4 + y2) / 2.0;

	    px = min(cx2, cx3); py = min(cy2, cy3);
	    qx = max(cx2, cx3); qy = max(cy2, cy3);

	    sx = min(sx, px); sy = min(sy, py);
	    bx = max(bx, qx); by = max(by, qy);
	    }
	if (closed_spline(s)) {
	    *xmin = round(sx); *ymin = round(sy);
	    *xmax = round(bx); *ymax = round(by);
	    }
	else {
	    *xmin = round(min(sx, x2)); *ymin = round(min(sy, y2));
	    *xmax = round(max(bx, x2)); *ymax = round(max(by, y2));
	    }
	}

double rot_x(x,y,angle) 
double x,y,angle;
{
    return(x*cos(-angle)-y*sin(-angle));
}

double rot_y(x,y,angle)
double x,y,angle;
{
 return(x*sin(-angle)+y*cos(-angle));
}


text_bound(t, xmin, ymin, xmax, ymax, include)
F_text	*t;
int	*xmin, *ymin, *xmax, *ymax;
int	include;
{
    double dx1, dx2, dx3, dx4, dy1, dy2, dy3, dy4;

	if (t->type == T_CENTER_JUSTIFIED) {
	    dx1 = (t->length/2);     dy1 = 0.0;
	    dx2 = -(t->length/2);    dy2 = 0.0;
	    dx3 = (t->length/2);     dy3 = -t->height;
	    dx4 = -(t->length/2);    dy4 = -t->height;
	} else if (t->type == T_RIGHT_JUSTIFIED) {
	    dx1 = 0.0;               dy1 = 0.0;
	    dx2 = -t->length;        dy2 = 0.0;
	    dx3 = 0.0;               dy3 = -t->height;
	    dx4 = -t->length;        dy4 = -t->height;
	} else {
	    dx1 = (include ? t->length : 0); dy1 = 0.0;
	    dx2 = 0.0;                       dy2 = 0.0;
	    dx3 = (include ? t->length : 0); dy3 = -t->height;
	    dx4 = 0.0;                       dy4 = -t->height;
	}
    *xmax= t->base_x +
           max( max( rot_x(dx1,dy1,t->angle), rot_x(dx2,dy2,t->angle) ), 
	        max( rot_x(dx3,dy3,t->angle), rot_x(dx4,dy4,t->angle) ) );
    *ymax= t->base_y + 
           max( max( rot_y(dx1,dy1,t->angle), rot_y(dx2,dy2,t->angle) ), 
	        max( rot_y(dx3,dy3,t->angle), rot_y(dx4,dy4,t->angle) ) );

    *xmin= t->base_x + 
           min( min( rot_x(dx1,dy1,t->angle), rot_x(dx2,dy2,t->angle) ), 
	        min( rot_x(dx3,dy3,t->angle), rot_x(dx4,dy4,t->angle) ) );
    *ymin= t->base_y + 
           min( min( rot_y(dx1,dy1,t->angle), rot_y(dx2,dy2,t->angle) ), 
	        min( rot_y(dx3,dy3,t->angle), rot_y(dx4,dy4,t->angle) ) );
	}

points_bound(points, xmin, ymin, xmax, ymax)
F_point	*points;
int	*xmin, *ymin, *xmax, *ymax;
{
	int	bx, by, sx, sy;
	F_point	*p;

	bx = sx = points->x; by = sy = points->y;
	for (p = points->next; p != NULL; p = p->next) {
	    sx = min(sx, p->x); sy = min(sy, p->y);
	    bx = max(bx, p->x); by = max(by, p->y);
	    }
	*xmin = sx; *ymin = sy;
	*xmax = bx; *ymax = by;
	}

control_points_bound(cps, xmin, ymin, xmax, ymax)
F_control	*cps;
int		*xmin, *ymin, *xmax, *ymax;
{
	F_control	*c;
	double		bx, by, sx, sy;

	bx = sx = cps->lx;
	by = sy = cps->ly;
	sx = min(sx, cps->rx); sy = min(sy, cps->ry);
	bx = max(bx, cps->rx); by = max(by, cps->ry);
	for (c = cps->next; c != NULL; c = c->next) {
	    sx = min(sx, c->lx); sy = min(sy, c->ly);
	    bx = max(bx, c->lx); by = max(by, c->ly);
	    sx = min(sx, c->rx); sy = min(sy, c->ry);
	    bx = max(bx, c->rx); by = max(by, c->ry);
	    }
	*xmin = round(sx); *ymin = round(sy);
	*xmax = round(bx); *ymax = round(by);
	}
