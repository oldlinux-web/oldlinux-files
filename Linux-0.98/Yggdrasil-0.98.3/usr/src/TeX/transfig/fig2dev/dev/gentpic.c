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
 *	gentpic : TPIC driver for fig2dev
 *
 *	Author: Conrad Kwok, UC Davis, 12/88
 *      Modified: Richard Auletta, George Mason Univ., 6/21/89
 *		Added code comments are marked with "rja".
 *      	Added: Support for native pic arrowheads.
 *      	Added: Support for arrowheads at both ends of lines, arc, splines.
 *	Modified: Modified from pic to tpic. Baron Grey, UCLA. 10/2/90.
 *
 *		This driver supports TeX's virtual font mechanism. Any Fig
 *		font whose ordinal number is greater than 6 is treated as
 *		a TeX virtual PostScript font. Virtual PostScript font
 *		names are defined in tpicfonts.h (you should define
 *		corresponding names in Fig in file change.c). Customize to
 *		suit your virtual font availability or your name preferences.
 *
 *		There are no restrictions on font sizes in this driver, but
 *		keep in mind that TeX only supports fonts up to 2048 points!
 *		Since PostScript fonts are scaleable, virtual PS fonts can have
 *		any point size within the limits imposed by TeX or by your
 *		PostScript interpreter. If you are using
 *		standard predefined LaTeX fonts (assumed to be the first
 *		6 positions in Fig's ordinal font numbering scheme), font
 *		sizes are mapped into the nearest valid normally-available
 *		LaTeX font sizes -- look at tpicfonts.h to see how this is
 *		done; you can modify it for fonts at your site.
 *
 *		Additional features of this driver are:
 *		1) Support for all Fig features except text at nonzero
 *		   angles.  (In fact, any limitations of this driver are
 *		   actually tpic limitations.)
 *		2) Boxes with rounded corners are supported in two ways.
 *		   If (when) tpic is extended to use the "radius" attribute
 *		   with a "box" specification, the driver will generate the
 *		   appropriate box specification. Otherwise, it creates
 *		   rounded-corner boxes from line and arc segments using
 *		   existing tpic primitives. In the former case, rounded-
 *		   corner boxes can be shaded; in the latter, they can't.
 *		   Define TPIC_ARC_BOX for the first case (the default);
 *		   undefine it otherwise.
 *		3) Supports Fig's ability to texture (shade) objects.
 *		   Bitmapped PostScript texture patterns are supported;
 *		   they are defined in "tpicfonts.h". The tpic ".tx"
 *		   primitive is used for this. Your dvi-to-PostScript
 *		   driver must be adapted to support them.
 *		4) If an object's line "thickness" is 0, it is given tpic's
 *		   `invis' attribute. That is, tpic will treat it as an
 *		   invisible object in the normal tpic sense.
 *		5) This driver supports tpic 2.0. It will work with tpic 1.0,
 *		   but some object attributes will not be available.
 *		6) The default line thickness is 8 milli inches (in tpic
 *		   units). This corresponds to 1 pixel width in Fig. The
 *		   formula is: line_thickness = Fig_line_thickness *
 *		   default_tpic_line_thickness.
 *		7) The default font size is (assumed to be) 10 points!
 */

#include <stdio.h>
#include <math.h>
#include "object.h"
#include "fig2dev.h"
#include "tpicfonts.h"

/*
 * Define TPIC_ARC_BOX if your tpic supports rounded-corner boxes
 * via a "radius" attribute of a box specification.
 */
#define TPIC_ARC_BOX

#define			TOP	10.5	/* top of page is 10.5 inch */
static double		ppi;
static int		CONV = 0;
static int		line_width = 8;	/* milli-inches */
static int		vfont = 0; /* true if using a virtual TeX font */

void gentpic_ctl_spline(), gentpic_itp_spline();
void gentpic_open_spline(), gentpic_closed_spline();
void gentpic_spline(), gentpic_ellipse(), gentpic_text();
void gentpic_arc(), gentpic_line(), newline();

void gentpic_option(opt, optarg)
char opt, *optarg;
{
	switch (opt) {

	case 'f':		/* set default text font */
	        {   int i;

		    for ( i = 1; i <= MAX_TPICFONT; i++ )
			if ( !strcmp(optarg, texfontnames[i]) ) break;

		    if ( i > MAX_FONT + 1 )
			    vfont = 1; /* assume a virtual font */
		}
		
		texfontnames[0] = optarg;
		break;

	case 's':
	case 'm':
	case 'L':
		break;

 	default:
		put_msg(Err_badarg, opt, "tpic");
		exit(1);
	}
}

static double convy(a)
double	a;
{
	return((double)(CONV ? TOP-a : a));
}

void gentpic_start(objects)
F_compound	*objects;
{
	int		coord_system;

	ppi = objects->nwcorner.x/mag;
	coord_system = objects->nwcorner.y;
	if (coord_system == 2) CONV = 1;

	fprintf(tfp, ".PS\n");	/* PIC preamble */

}

void gentpic_end()
{
  	fprintf(tfp, ".PE\n");				/* PIC ending */
}

/*
 * tpic's line thickness is given in milli-inches with a default of 8
 * milli-inches. We simply multiply this default with the Fig pixel width.
 */
static set_linewidth(w)
int	w;
{
	static int	cur_thickness = -1;

	if (w == 0) return;
	if (w != cur_thickness) {
	    cur_thickness = w;
	    fprintf(tfp, ".ps %d\n", cur_thickness * line_width);
	    }
	}

static set_style(s, v)
int	s;
float	v;
{
	static float	style_val = -1;

	if (s == DASH_LINE || s == DOTTED_LINE) {
	    if (v == style_val || v == 0.0) return;
	    style_val = v;
	    fprintf(tfp, "dashwid = %.3fi\n", style_val/ppi);
	    }
	}

static set_baseline(b)
int	b;
{
	static int	cur_baseline = -1;

	if (b != cur_baseline) {
		fprintf(tfp, ".baseline %d\n", b);
		cur_baseline = b;
	}
}

static set_texture(cur_texture)
int	cur_texture;
{
	/*
	 * This applies only to bitmapped texture patterns defined in
	 * tpicfonts.h. See set_fill() below for normal shading.
	 */
	if (cur_texture <= BLACK_FILL || cur_texture > MAXPATTERNS + BLACK_FILL)
		return;
	fprintf(tfp, ".tx 16 %s\n",
		texture_patterns[cur_texture - BLACK_FILL - 1]);
}

/*
 * Set the shade for filling an object. Your dvi-to-postscript driver must
 * distinguish between a "shaded" attribute with or without a parameter.
 * If there is no parameter, then it should do a bitmap texture fill;
 * otherwise, it should do a normal gray-scale fill. Note that the gray-
 * scale fill parameter is wired for fig2.X (the constant 0.05).
 */
static set_fill(cur_fill)
int	cur_fill;
{
	if(cur_fill < WHITE_FILL ||
	   cur_fill > BLACK_FILL + MAXPATTERNS)
		return;
	switch(cur_fill) {
	case BLACK_FILL:
		fprintf(tfp, " black");
		break;
	case WHITE_FILL:
		fprintf(tfp, " white");
		break;
	default:
		if (cur_fill > BLACK_FILL)
			fprintf (tfp, " shaded");
		else
			fprintf(tfp, " shaded %0.3f", (cur_fill-1)*0.05);
		break;
	}
}

#ifdef TPIC_ARC_BOX
void gentpic_line(l)
F_line	*l;
{
	F_point		*p, *q;
	int	llx, lly, urx, ury;

	set_linewidth(l->thickness);
	set_style(l->style, l->style_val);
	set_texture(l->area_fill);
	p = l->points;
	q = p->next;
	if (q == NULL) { /* A single point line */
	    fprintf(tfp, "line from %.3f,%.3f to %.3f,%.3f",
			p->x/ppi, convy(p->y/ppi), p->x/ppi, convy(p->y/ppi));
	    newline();
	    return;
	    }

	if (l->type == T_BOX || l->type == T_ARC_BOX) {
		llx = urx = p->x;
		lly = ury = p->y;
		while (q != NULL) {
			p = q;
			if (p->x < llx) {
				llx = p->x;
			} else if (p->x > urx) {
				urx = p->x;
			}
			if (p->y < lly) {
				lly = p->y;
			} else if (p->y > ury) {
				ury = p->y;
			}
			q = q->next;
		}
		fprintf(tfp, "box height %.3f width %.3f", (ury-lly)/ppi,
			(urx-llx)/ppi);
		switch(l->style) {
		case SOLID_LINE:
			break;
		case DASH_LINE:
			fprintf(tfp, " dashed");
			break;
		case DOTTED_LINE:
			fprintf(tfp, " dotted");
			break;
		default:
			put_msg("Program error! No other line styles allowed.\n");
			return;
		}
		if (l->thickness == 0)
			fprintf(tfp, " invis");
		fprintf(tfp, " radius %.3f", l->radius/ppi);
		set_fill(l->area_fill);
		fprintf(tfp, " with .nw at %.3f,%.3f", llx/ppi, convy(lly/ppi));
		newline();
		return;
	}

	if (l->style == DASH_LINE && l->style_val > 0.0)
	    fprintf(tfp, "line dashed");
	else if (l->style == DOTTED_LINE && l->style_val > 0.0)
	    fprintf(tfp, "line dotted");
	else
	    fprintf(tfp, "line");

	/*rja: Place arrowheads or lack there of on the line*/
	if ((l->for_arrow) && (l->back_arrow))
	    fprintf(tfp, " <-> from");
	else if (l->back_arrow)
	    fprintf(tfp, " <- from");
	else if (l->for_arrow)
	    fprintf(tfp, " -> from");
        else
	    fprintf(tfp, " from ");

	fprintf(tfp, " %.3f,%.3f to", p->x/ppi, convy(p->y/ppi));
	while (q->next != NULL) {
	    p = q;
	    q = q->next;
	    fprintf(tfp, " %.3f,%.3f to", p->x/ppi, convy(p->y/ppi));
	    }
	fprintf(tfp, " %.3f,%.3f", q->x/ppi, convy(q->y/ppi));
	if (l->thickness == 0)
		fprintf(tfp, " invis");
	if (l->type == T_POLYGON)
		set_fill(l->area_fill);
	newline();
	}

#else
void gentpic_line(l)
F_line	*l;
{
	F_point		*p, *q;
	int		radius = l->radius;
	char		attr[80];

	attr[0] = '\0';
	set_linewidth(l->thickness);
	set_style(l->style, l->style_val);
	set_texture(l->area_fill);
	p = l->points;
	q = p->next;
	if (q == NULL) { /* A single point line */
	    fprintf(tfp, "line from %.3f,%.3f to %.3f,%.3f",
			p->x/ppi, convy(p->y/ppi), p->x/ppi, convy(p->y/ppi));
	    newline();
	    return;
	    }

	if (l->style == DASH_LINE && l->style_val > 0.0)
		strcat(attr, "dashed");
	else if (l->style == DOTTED_LINE && l->style_val > 0.0)
		strcat(attr, "dotted");
	if (l->thickness == 0)
		strcat(attr, " invis");

	if (radius > 0) {	/* T_ARC_BOX */
                register int xmin,xmax,ymin,ymax;

                xmin = xmax = p->x;
                ymin = ymax = p->y;
                while (p->next != NULL) { /* find lower left and upper right corners */
                        p=p->next;
                        if (xmin > p->x)
                                xmin = p->x;
                        else if (xmax < p->x)
                                xmax = p->x;
                        if (ymin > p->y)
                                ymin = p->y;
                        else if (ymax < p->y)
                                ymax = p->y;
		}
                fprintf(tfp, "line %s from  %.3f,%.3f to %.3f, %.3f\n", attr,
			(xmin+radius)/ppi, convy(ymin/ppi),
			(xmax-radius)/ppi, convy(ymin/ppi));
                fprintf(tfp, "arc cw %s from %.3f, %.3f to %.3f,%.3f radius %.3f\n",
			attr, (xmax-radius)/ppi, convy(ymin/ppi),
			(xmax/ppi), convy((ymin+radius)/ppi), radius/ppi);
                fprintf(tfp, "line %s from  %.3f,%.3f to %.3f, %.3f\n", attr,
			xmax/ppi, convy((ymin+radius)/ppi),
			xmax/ppi, convy((ymax-radius)/ppi));
                fprintf(tfp, "arc cw %s from %.3f, %.3f to %.3f,%.3f radius %.3f\n",
			attr, xmax/ppi, convy((ymax-radius)/ppi),
			(xmax-radius)/ppi, convy(ymax/ppi), radius/ppi);
                fprintf(tfp, "line %s from  %.3f,%.3f to %.3f, %.3f\n", attr,
			(xmax-radius)/ppi, convy(ymax/ppi),
			(xmin+radius)/ppi, convy(ymax/ppi));
                fprintf(tfp, "arc cw %s from %.3f, %.3f to %.3f,%.3f radius %.3f\n",
			attr, (xmin+radius)/ppi, convy(ymax/ppi),
			xmin/ppi, convy((ymax-radius)/ppi), radius/ppi);
                fprintf(tfp, "line %s from  %.3f,%.3f to %.3f, %.3f\n", attr,
			xmin/ppi, convy((ymax-radius)/ppi),
			xmin/ppi, convy((ymin+radius)/ppi));
                fprintf(tfp, "arc cw %s from %.3f, %.3f to %.3f,%.3f radius %.3f",
			attr, xmin/ppi, convy((ymin+radius)/ppi),
			(xmin+radius)/ppi, convy(ymin/ppi), radius/ppi);
	} else {
		/*rja: Place arrowheads or lack there of on the line*/
		fprintf(tfp, "line %s", attr);
		if ((l->for_arrow) && (l->back_arrow))
			fprintf(tfp, " <-> from");
		else if (l->back_arrow)
			fprintf(tfp, " <- from");
		else if (l->for_arrow)
			fprintf(tfp, " -> from");
		else
			fprintf(tfp, " from ");

		fprintf(tfp, " %.3f,%.3f to", p->x/ppi, convy(p->y/ppi));
		while (q->next != NULL) {
			p = q;
			q = q->next;
			fprintf(tfp, " %.3f,%.3f to", p->x/ppi, convy(p->y/ppi));
		}
		fprintf(tfp, " %.3f,%.3f", q->x/ppi, convy(q->y/ppi));
	}
	if (l->type != T_POLYLINE)
		set_fill(l->area_fill);
	newline();
	}
#endif

void gentpic_spline(s)
F_spline	*s;
{
	if (int_spline(s))
	    gentpic_itp_spline(s);
	else
	    gentpic_ctl_spline(s);
	}

void gentpic_ctl_spline(s)
F_spline	*s;
{
	if (closed_spline(s))
	    gentpic_closed_spline(s);
	else
	    gentpic_open_spline(s);
	}

void gentpic_open_spline(s)
F_spline	*s;
{
	double		x1, y1, x2, y2;
	F_point		*p, *q;

	p = s->points;
	x1 = p->x/ppi; y1 = convy(p->y/ppi);
	p = p->next;
	x2 = p->x/ppi; y2 = convy(p->y/ppi);

	set_style(s->style, s->style_val);
	set_linewidth(s->thickness);

	if (p->next == NULL) {
	    fprintf(tfp, "line");
	    if (s->style == DASH_LINE && s->style_val > 0.0)
		    fprintf(tfp, " dashed");
	    else if (s->style == DOTTED_LINE && s->style_val > 0.0)
		    fprintf(tfp, " dotted");


           /*rja: Attach arrowhead as required */
	    if ((s->for_arrow) && (s->back_arrow))
	       fprintf(tfp, " <->");
	    else if (s->back_arrow)
	       fprintf(tfp, " <-");
	    else if (s->for_arrow)
	       fprintf(tfp, " ->");

	    fprintf(tfp, " from %.3f,%.3f to %.3f,%.3f", x1, y1, x2, y2);
	    newline();
	    return;
	    }

	fprintf(tfp, "spline"); 
	if (s->style == DASH_LINE && s->style_val > 0.0)
	    fprintf(tfp, " dashed");
	else if (s->style == DOTTED_LINE && s->style_val > 0.0)
	    fprintf(tfp, " dotted");


           /*rja: Attach arrowhead as required */
	    if ((s->for_arrow) && (s->back_arrow))
	       fprintf(tfp, " <->");
	    else if (s->back_arrow)
	       fprintf(tfp, " <-");
	    else if (s->for_arrow)
	       fprintf(tfp, " ->");

	fprintf(tfp, " from %.3f,%.3f to %.3f,%.3f", x1, y1, x2, y2);

	for (q = p->next; q->next != NULL; p = q, q = q->next)
	    fprintf(tfp, " to %.3f,%.3f", q->x/ppi, convy(q->y/ppi));
	fprintf(tfp, " to %.3f,%.3f", (x2=q->x/ppi), (y2=convy(q->y/ppi)));

	newline();
	}

void gentpic_ellipse(e)
F_ellipse	*e;
{
	set_linewidth(e->thickness);
	set_texture(e->area_fill);
	set_style(e->style, e->style_val);

	fprintf(tfp, "ellipse");

	if (e->style == DASH_LINE && e->style_val > 0.0)
		fprintf(tfp, " dashed");
	else if (e->style == DOTTED_LINE && e->style_val > 0.0)
		fprintf(tfp, " dotted");

	fprintf(tfp, " at %.3f,%.3f wid %.3f ht %.3f",
		e->center.x/ppi, convy(e->center.y/ppi),
		2 * e->radiuses.x/ppi, 2 * e->radiuses.y/ppi);
	if (e->thickness == 0)
		fprintf(tfp, " invis");
	set_fill(e->area_fill);
	newline();
	}

/*
Text is displayed on the screen with the base line starting at
(base_x, base_y); some characters extend below this line.
Pic displays the center of the height of text at the given
coordinate. HT_OFFSET is used to compensate all the above factors
so text position in fig should be at the same position on
the screen as on the hard copy.

THIS IS A HACK. tpic should be modified to put text with its
baseline at the given coordinates as does fig -- Baron.
*/
#define			HT_OFFSET	(0.25 / 72.0)

void gentpic_text(t)
F_text	*t;
{
	float	y;
        char	*tpos;
	int	virtual_font = 0;

	/*
	 * If a font size is specified and the current text font size
	 * is the default, then use the specified font size.
	 */
	if (t->size == 0 && font_size)
		t->size = font_size;

	/*
	 * tpic is informed of the baseline spacing here. Not particularly
	 * useful in this version of the driver.
	 */
	set_baseline((int)t->size);

	unpsfont(t);

	if (t->font > MAX_FONT+1 || vfont)
		virtual_font = 1;	/* must be a virtual font */

	if (virtual_font)
		/*
		 * NOTE. Virtual fonts are defined dynamically using
		 * TeX's font definition machinery.
		 */
		fprintf(tfp, "\"\\font\\fig%s=%s at %dpt \\fig%s ",
			TEXFONT(t->font), TEXFONT(t->font), t->size,
			TEXFONT(t->font));
	else
#ifdef FST
		fprintf(tfp, "\"\\%spt\\%s ", TEXFONTMAG(t), TEXFONT(t->font));
#else
		fprintf(tfp, "\"\\%s%s ", TEXFONTMAG(t), TEXFONT(t->font));
#endif

        switch (t->type) {
        case T_LEFT_JUSTIFIED:
        case DEFAULT:
            tpos = "ljust";
            break;
        case T_CENTER_JUSTIFIED:
            tpos = "";
            break;
        case T_RIGHT_JUSTIFIED:
            tpos = "rjust";
            break;
        default:
            fprintf(stderr, "unknown text position type\n");
            exit(1);
        }    
	y = convy(t->base_y/ppi) + (TEXFONTMAGINT(t)
		* HT_OFFSET);

	fprintf(tfp, "%s\" at %.3f,%.3f %s",
		t->cstring, t->base_x/ppi, y, tpos);
	newline();
}

void gentpic_arc(a)
F_arc	*a;
{
	double		x, y;
	double		cx, cy, sx, sy, ex, ey;

	cx = a->center.x/ppi; cy = convy(a->center.y/ppi);
	sx = a->point[0].x/ppi; sy = convy(a->point[0].y/ppi);
	ex = a->point[2].x/ppi; ey = convy(a->point[2].y/ppi);

	set_texture(a->area_fill);
	set_linewidth(a->thickness);
	set_style(a->style, a->style_val);

	fprintf(tfp, "arc");

	if (a->style == DASH_LINE && a->style_val > 0.0)
		fprintf(tfp, " dashed");
	else if (a->style == DOTTED_LINE && a->style_val > 0.0)
		fprintf(tfp, " dotted");

	/*rja: Attach arrowhead as required */
	if ((a->for_arrow) && (a->back_arrow))
		fprintf(tfp, " <->");
	else if (a->back_arrow)
		fprintf(tfp, " <-");
	else if (a->for_arrow)
		fprintf(tfp, " ->");


	if (a->direction)
	    fprintf(tfp, " at %.3f,%.3f from %.3f,%.3f to %.3f,%.3f",
			cx, cy, sx, sy, ex, ey);
	else
	    fprintf(tfp, " at %.3f,%.3f from %.3f,%.3f to %.3f,%.3f cw",
			cx, cy, sx, sy, ex, ey);
	if (a->thickness == 0)
		fprintf(tfp, " invis");
	set_fill(a->area_fill);
	newline();

	}

static arc_tangent(x1, y1, x2, y2, direction, x, y)
double	x1, y1, x2, y2, *x, *y;
int	direction;
{
	if (direction) { /* counter clockwise  */
	    *x = x2 + (y2 - y1);
	    *y = y2 - (x2 - x1);
	    }
	else {
	    *x = x2 - (y2 - y1);
	    *y = y2 + (x2 - x1);
	    }
	}

#define		THRESHOLD	.05	/* inch */

static quadratic_spline(a1, b1, a2, b2, a3, b3, a4, b4)
double	a1, b1, a2, b2, a3, b3, a4, b4;
{
	double	x1, y1, x4, y4;
	double	xmid, ymid;

	x1 = a1; y1 = b1;
	x4 = a4; y4 = b4;

	xmid = (a2 + a3) / 2;
	ymid = (b2 + b3) / 2;
	if (fabs(x1 - xmid) < THRESHOLD && fabs(y1 - ymid) < THRESHOLD) {
	    fprintf(tfp, "\tto %.3f,%.3f\\\n", xmid, ymid);
	    }
	else {
	    quadratic_spline(x1, y1, ((x1+a2)/2), ((y1+b2)/2),
			((3*a2+a3)/4), ((3*b2+b3)/4), xmid, ymid);
	    }

	if (fabs(xmid - x4) < THRESHOLD && fabs(ymid - y4) < THRESHOLD) {
	    fprintf(tfp, "\tto %.3f,%.3f\\\n", x4, y4);
	    }
	else {
	    quadratic_spline(xmid, ymid, ((a2+3*a3)/4), ((b2+3*b3)/4),
			((a3+x4)/2), ((b3+y4)/2), x4, y4);
	    }
	}

void gentpic_closed_spline(s)
F_spline	*s;
{
	F_point	*p;
	double	cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4;
	double	x1, y1, x2, y2;

	p = s->points;
	x1 = p->x/ppi;  y1 = convy(p->y/ppi);
	p = p->next;
	x2 = p->x/ppi;  y2 = convy(p->y/ppi);
	cx1 = (x1 + x2) / 2;      cy1 = (y1 + y2) / 2;
	cx2 = (x1 + 3 * x2) / 4;  cy2 = (y1 + 3 * y2) / 4;

	set_linewidth(s->thickness);
	set_texture(s->area_fill); /* probably won't work! */
	set_style(s->style, s->style_val);

	for (p = p->next; p != NULL; p = p->next) {
	    fprintf(tfp, "line from %.3f,%.3f ", cx1, cy1);
	    x1 = x2;  y1 = y2;
	    x2 = p->x/ppi;  y2 = convy(p->y/ppi);
	    cx3 = (3 * x1 + x2) / 4;  cy3 = (3 * y1 + y2) / 4;
	    cx4 = (x1 + x2) / 2;      cy4 = (y1 + y2) / 2;
	    quadratic_spline(cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4);
	    newline();
	    /* fprintf(tfp, "\n"); */
	    cx1 = cx4;  cy1 = cy4;
	    cx2 = (x1 + 3 * x2) / 4;  cy2 = (y1 + 3 * y2) / 4;
	    }
	x1 = x2;  y1 = y2;
	p = s->points->next;
	x2 = p->x/ppi;  y2 = convy(p->y/ppi);
	cx3 = (3 * x1 + x2) / 4;  cy3 = (3 * y1 + y2) / 4;
	cx4 = (x1 + x2) / 2;      cy4 = (y1 + y2) / 2;
	fprintf(tfp, "line from %.3f,%.3f ", cx1, cy1);
	quadratic_spline(cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4);
	if (s->thickness == 0)
		fprintf(tfp, " invis");
	set_fill(s->area_fill);
	newline();
	/* fprintf(tfp, "\n"); */
	}

void gentpic_itp_spline(s)
F_spline	*s;
{
	F_point		*p1, *p2, *pfirst;
	F_control	*cp1, *cp2;
	double		x1, x2, y1, y2;

	set_style(s->style, s->style_val);
	set_texture(s->area_fill); /* probably won't work! */
	set_linewidth(s->thickness);

	p1 = s->points;
	cp1 = s->controls;
	cp2 = cp1->next;
	x2 = p1->x/ppi; y2 = convy(p1->y/ppi);

         pfirst = p1->next;/*save first to test in loop*/
	for (p2 = p1->next, cp2 = cp1->next; p2 != NULL;
		p1 = p2, cp1 = cp2, p2 = p2->next, cp2 = cp2->next) {

	    fprintf(tfp, "line ");

           /*rja: Attach arrowhead as required */

	    if ((s->back_arrow) && (p2 == pfirst))
	       fprintf(tfp, " <- ");
	    else if ((s->for_arrow) && (p2->next == NULL))
	       fprintf(tfp, " -> ");

	    fprintf(tfp, " from %.3f,%.3f ", x2, y2);

	    x1 = x2; y1 = y2;
	    x2 = p2->x/ppi; y2 = convy(p2->y/ppi);
	    bezier_spline(x1, y1, (double)cp1->rx/ppi, convy(cp1->ry/ppi),
		(double)cp2->lx/ppi, convy(cp2->ly/ppi), x2, y2);
	    /* fprintf(tfp, "\n"); */
	    newline();
	    }

	}

static bezier_spline(a0, b0, a1, b1, a2, b2, a3, b3)
double	a0, b0, a1, b1, a2, b2, a3, b3;
{
	double	x0, y0, x3, y3;
	double	sx1, sy1, sx2, sy2, tx, ty, tx1, ty1, tx2, ty2, xmid, ymid;

	x0 = a0; y0 = b0;
	x3 = a3; y3 = b3;
	if (fabs(x0 - x3) < THRESHOLD && fabs(y0 - y3) < THRESHOLD) {
	    fprintf(tfp, "\tto %.3f,%.3f\\\n", x3, y3);
	    }
	else {
	    tx = (a1 + a2) / 2;		ty = (b1 + b2) / 2;
	    sx1 = (x0 + a1) / 2;	sy1 = (y0 + b1) / 2;
	    sx2 = (sx1 + tx) / 2;	sy2 = (sy1 + ty) / 2;
	    tx2 = (a2 + x3) / 2;	ty2 = (b2 + y3) / 2;
	    tx1 = (tx2 + tx) / 2;	ty1 = (ty2 + ty) / 2;
	    xmid = (sx2 + tx1) / 2;	ymid = (sy2 + ty1) / 2;

	    bezier_spline(x0, y0, sx1, sy1, sx2, sy2, xmid, ymid);
	    bezier_spline(xmid, ymid, tx1, ty1, tx2, ty2, x3, y3);
	    }
	}

static void
newline()
{
	/*
	 * A vestige from another version of this driver.
	 */
	fprintf(tfp, "\n");
}

struct driver dev_tpic = {
     	gentpic_option,
	gentpic_start,
	gentpic_arc,
	gentpic_ellipse,
	gentpic_line,
	gentpic_spline,
	gentpic_text,
	gentpic_end,
	INCLUDE_TEXT
};

