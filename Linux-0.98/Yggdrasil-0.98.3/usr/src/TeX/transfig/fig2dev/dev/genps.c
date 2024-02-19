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
 *	genps.c: PostScript driver for fig2dev
 *
 *      Modified by Herbert Bauer to support ISO-Characters,
 *      multiple page output, color mode etc.
 *      heb@regent.e-technik.tu-muenchen.de
 *
 *	Modified by Eric Picheral to support the whole set of ISO-Latin-1
*/

#if defined(hpux) || defined(SYSV)
#include <sys/types.h>
#endif
#include <sys/file.h>
#include <stdio.h>
#include <math.h>
#include <pwd.h>
#include "pi.h"
#include "object.h"
#include "fig2dev.h"
#include "psfonts.h"
#ifdef SYSV
#include <string.h>
#else
#include <strings.h>
#endif
#include <time.h>

#ifdef A4
#define		PAGE_WIDTH		595	/* points; 21cm */
#define		PAGE_HEIGHT		842	/* points; 29.7cm */
#else
#define		PAGE_WIDTH		612	/* points; 8.5" */
#define		PAGE_HEIGHT		792	/* points; 11" */
#endif
#define		TRUE			1
#define		FALSE			0
#define		POINT_PER_INCH		72
#define		ULIMIT_FONT_SIZE	300
#define 	MAXCOLORS 		8

int		pagewidth = PAGE_WIDTH;
int		pageheight = PAGE_HEIGHT;
int		coord_system;
int		show_page = 0;
int		cur_thickness;
int		center = 0;
int		landscape = 0;
int		pages;
int             iso_encoding = 0;
int		no_obj = 0;
int		multi_page = FALSE;

extern int 	v2_flag, v21_flag;

#define GRAYVAL(F)	((F) <= 21 ? ((F)-1)/20.0 : 1.0)

#define		BEGIN_PROLOG	"\
/$F2psDict 64 dict def \n\
$F2psDict begin\n\
$F2psDict /mtrx matrix put\n\
/l {lineto} bind def\n\
/m {moveto} bind def\n\
/s {stroke} bind def\n\
/n {newpath} bind def\n\
/gs {gsave} bind def\n\
/gr {grestore} bind def\n\
/clp {closepath} bind def\n\
/graycol {dup dup currentrgbcolor 4 -2 roll mul 4 -2 roll mul\n\
4 -2 roll mul setrgbcolor} bind def\n\
/col-1 {} def\n\
/col0 {0 0 0 setrgbcolor} bind def\n\
/col1 {0 0 1 setrgbcolor} bind def\n\
/col2 {0 1 0 setrgbcolor} bind def\n\
/col3 {0 1 1 setrgbcolor} bind def\n\
/col4 {1 0 0 setrgbcolor} bind def\n\
/col5 {1 0 1 setrgbcolor} bind def\n\
/col6 {1 1 0 setrgbcolor} bind def\n\
/col7 {1 1 1 setrgbcolor} bind def\n\
"

#define		SPECIAL_CHAR	"\
/reencdict 12 dict def /ReEncode { reencdict begin\n\
/newcodesandnames exch def /newfontname exch def /basefontname exch def\n\
/basefontdict basefontname findfont def /newfont basefontdict maxlength dict def\n\
basefontdict { exch dup /FID ne { dup /Encoding eq\n\
{ exch dup length array copy newfont 3 1 roll put }\n\
{ exch newfont 3 1 roll put } ifelse } { pop pop } ifelse } forall\n\
newfont /FontName newfontname put newcodesandnames aload pop\n\
128 1 255 { newfont /Encoding get exch /.notdef put } for\n\
newcodesandnames length 2 idiv { newfont /Encoding get 3 1 roll put } repeat\n\
newfontname newfont definefont pop end } def\n\
/isovec [ \n\
8#200 /grave 8#201 /acute 8#202 /circumflex 8#203 /tilde\n\
8#204 /macron 8#205 /breve 8#206 /dotaccent 8#207 /dieresis\n\
8#210 /ring 8#211 /cedilla 8#212 /hungarumlaut 8#213 /ogonek 8#214 /caron\n\
8#220 /dotlessi 8#240 /space 8#241 /exclamdown 8#242 /cent 8#243 /sterling\n\
8#244 /currency 8#245 /yen 8#246 /brokenbar 8#247 /section 8#250 /dieresis\n\
8#251 /copyright 8#252 /ordfeminine 8#253 /guillemotleft 8#254 /logicalnot\n\
8#255 /endash 8#256 /registered 8#257 /macron 8#260 /degree 8#261 /plusminus\n\
8#262 /twosuperior 8#263 /threesuperior 8#264 /acute 8#265 /mu 8#266 /paragraph\n\
8#267 /periodcentered 8#270 /cedilla 8#271 /onesuperior 8#272 /ordmasculine\n\
8#273 /guillemotright 8#274 /onequarter 8#275 /onehalf \n\
8#276 /threequarters 8#277 /questiondown 8#300 /Agrave 8#301 /Aacute\n\
8#302 /Acircumflex 8#303 /Atilde 8#304 /Adieresis 8#305 /Aring\n\
8#306 /AE 8#307 /Ccedilla 8#310 /Egrave 8#311 /Eacute\n\
8#312 /Ecircumflex 8#313 /Edieresis 8#314 /Igrave 8#315 /Iacute\n\
8#316 /Icircumflex 8#317 /Idieresis 8#320 /Eth 8#321 /Ntilde 8#322 /Ograve\n\
8#323 /Oacute 8#324 /Ocircumflex 8#325 /Otilde 8#326 /Odieresis 8#327 /multiply\n\
8#330 /Oslash 8#331 /Ugrave 8#332 /Uacute 8#333 /Ucircumflex\n\
8#334 /Udieresis 8#335 /Yacute 8#336 /Thorn 8#337 /germandbls 8#340 /agrave\n\
8#341 /aacute 8#342 /acircumflex 8#343 /atilde 8#344 /adieresis 8#345 /aring\n\
8#346 /ae 8#347 /ccedilla 8#350 /egrave 8#351 /eacute\n\
8#352 /ecircumflex 8#353 /edieresis 8#354 /igrave 8#355 /iacute\n\
8#356 /icircumflex 8#357 /idieresis 8#360 /eth 8#361 /ntilde 8#362 /ograve\n\
8#363 /oacute 8#364 /ocircumflex 8#365 /otilde 8#366 /odieresis 8#367 /divide\n\
8#370 /oslash 8#371 /ugrave 8#372 /uacute 8#373 /ucircumflex\n\
8#374 /udieresis 8#375 /yacute 8#376 /thorn 8#377 /ydieresis \
] def\n\
"

#define		ELLIPSE_PS	" \
/DrawEllipse {\n\
	/endangle exch def\n\
	/startangle exch def\n\
	/yrad exch def\n\
	/xrad exch def\n\
	/y exch def\n\
	/x exch def\n\
	/savematrix mtrx currentmatrix def\n\
	x y translate xrad yrad scale 0 0 1 startangle endangle arc\n\
	savematrix setmatrix\n\
	} def\n\
"
/* The original PostScript definition for adding a spline section to the
 * current path uses recursive bisection.  The following definition using the
 * curveto operator is more efficient since it executes at compiled rather
 * than interpreted code speed.  The Bezier control points are 2/3 of the way
 * from z1 (and z3) to z2.
 *
 * ---Rene Llames, 21 July 1988.
 */
#define		SPLINE_PS	" \
/DrawSplineSection {\n\
	/y3 exch def\n\
	/x3 exch def\n\
	/y2 exch def\n\
	/x2 exch def\n\
	/y1 exch def\n\
	/x1 exch def\n\
	/xa x1 x2 x1 sub 0.666667 mul add def\n\
	/ya y1 y2 y1 sub 0.666667 mul add def\n\
	/xb x3 x2 x3 sub 0.666667 mul add def\n\
	/yb y3 y2 y3 sub 0.666667 mul add def\n\
	x1 y1 lineto\n\
	xa ya xb yb x3 y3 curveto\n\
	} def\n\
"
#define		END_PROLOG	"\
	end\n\
/$F2psBegin {$F2psDict begin /$F2psEnteredState save def} def\n\
/$F2psEnd {$F2psEnteredState restore end} def\n\
%%EndProlog\n\
"

static double		tx, scalex, scaley;
static double		dx, dy, origx, origy;

void genps_option(opt, optarg)
char opt;
char *optarg;
{
	int i;

	switch (opt) {

	case 'f':
		for ( i = 1; i <= MAX_PSFONT + 1; i++ )
			if ( !strcmp(optarg, PSfontnames[i]) ) break;

		if ( i > MAX_PSFONT + 1 )
			fprintf(stderr,
			    "warning: non-standard font name %s\n", optarg);

	    	psfontnames[0] = psfontnames[1] = optarg;
	    	PSfontnames[0] = PSfontnames[1] = optarg;
	    	break;

	case 'c':
	    	center = 1;
		break;

	case 's':
		if (font_size <= 0 || font_size > ULIMIT_FONT_SIZE) {
			fprintf(stderr,
				"warning: font size %d out of bounds\n", font_size);
		}
		break;

	case 'P':
		show_page = 1;
		break;

      	case 'm':
      	case 'L':
		break;

      	case 'l':
		landscape = 1;
		break;

	default:
		put_msg(Err_badarg, opt, "ps");
		exit(1);
		break;
	}
}

void genps_start(objects)
F_compound	*objects;
{
	char		host[256];
	struct passwd	*who;
	long		when;
	extern char	*strstr();
	extern long	time();
	int		itmp;

	coord_system = objects->nwcorner.y;
	scalex = scaley = mag * POINT_PER_INCH / (double)objects->nwcorner.x;
	/* convert to point unit */
	llx = (int)ceil(llx * scalex); lly = (int)ceil(lly * scaley);
	urx = (int)ceil(urx * scalex); ury = (int)ceil(ury * scaley);


	if (landscape)
	{
	   itmp = pageheight; pageheight = pagewidth; pagewidth = itmp;
	   itmp = llx; llx = lly; lly = itmp;
	   itmp = urx; urx = ury; ury = itmp;
	}
	if (show_page)
	{
	   if (center)
	   {
              if (landscape)
              {
                 origx = (pageheight - urx - llx)/2.0;
                 origy = (pagewidth - ury - lly)/2.0;
              }
              else
              {
                 origx = (pagewidth - urx - llx)/2.0;
                 origy = (pageheight + ury + lly)/2.0;
              }
	   }
	   else
	   {
	      origx = 0.0;
	      origy = landscape ? 0.0 : pageheight;
	   }
	}
	else
	{
	   origx = -llx;
	   origy = landscape ? -lly : ury;
	}

	if (coord_system == 2) scaley = -scaley;

	fprintf(tfp, "%%!\n");	/* PostScript magic strings */
	who = getpwuid(getuid());
	if (-1 == gethostname(host, sizeof(host)))
	    (void)strcpy(host, "unknown-host!?!?");
	(void) time(&when);
	fprintf(tfp, "%%%%Title: %s\n", ((from) ? from : "stdin"));
	fprintf(tfp, "%%%%Creator: %s\n", prog);
	fprintf(tfp, "%%%%CreationDate: %s", ctime(&when));
	if (who)
	   fprintf(tfp, "%%%%For: %s@%s (%s)\n",
			who->pw_name, host, who->pw_gecos);

	if (!center)
	   pages = (urx/pagewidth+1)*(ury/pageheight+1);
	else
	   pages = 1;
        if (landscape)
	   fprintf(tfp, "%%%%BoundingBox: %d %d %d %d\n", 
	      (int)origx+llx, (int)origy+lly, (int)origx+urx, (int)origy+ury);
        else
	   fprintf(tfp, "%%%%BoundingBox: %d %d %d %d\n", 
	      (int)origx+llx, (int)origy-ury, (int)origx+urx, (int)origy-lly);
        fprintf(tfp, "%%%%Pages: %d\n", show_page ? pages : 0 );

	fprintf(tfp, "%%%%EndComments\n");
	fprintf(tfp, "%s", BEGIN_PROLOG);
	if (iso_text_exist(objects))
	{
	   fprintf(tfp, "%s", SPECIAL_CHAR);
	   encode_all_fonts(objects);
           iso_encoding = 1;
	}
	if (ellipse_exist(objects)) fprintf(tfp, "%s\n", ELLIPSE_PS);
	if (normal_spline_exist(objects)) fprintf(tfp, "%s\n", SPLINE_PS);
	fprintf(tfp, "%s\n", END_PROLOG);
	fprintf(tfp, "$F2psBegin\n");
 	fprintf(tfp, "0 setlinecap 0 setlinejoin\n");
  
 	if ( pages > 1 && show_page && !center )
            multi_page = TRUE;
 	else
 	{
	    fprintf (tfp, "%.1f %.1f translate", origx, origy);
	    if (landscape)
	    {
	       fprintf (tfp, " 90 rotate");
	    }
	    fprintf (tfp, " %.3f %.3f scale\n", scalex, scaley );
	}
}

void genps_end()
{
    double dx,dy;
    int i;
  
    if (multi_page)
    {
       for (dy=0;dy<(ury-pageheight*0.1);dy+=pageheight*0.9)
       {
         for (dx=0;dx<(urx-pagewidth*0.1);dx+=pagewidth*0.9)
         {
	    fprintf (tfp, "%.1f %.1f translate", 
		origx+dx, origy+(landscape?-dy:dy));
	    if (landscape)
	    {
	       fprintf(tfp, " 90 rotate");
	    }
	    fprintf (tfp, " %.3f %.3f scale\n", scalex, scaley);
            for (i=0; i<no_obj; i++)
	    {
	       fprintf(tfp, "o%d ", i);
	       if (!(i%10)) fprintf(tfp, "\n", i);
	    }
	    fprintf(tfp, "showpage\n");
         }
       }
    }
    else
       if (show_page) fprintf(tfp, "showpage\n");
    fprintf(tfp, "$F2psEnd\n");
}
 
static set_style(s, v)
int	s;
double	v;
{
	if (s == DASH_LINE) {
	    if (v > 0.0) fprintf(tfp, "\t[%f] 0 setdash\n", v);
	    }
	else if (s == DOTTED_LINE) {
	    if (v > 0.0) fprintf(tfp, "\t1 setlinecap [1 %f] %f setdash\n", v, v);
	    }
	}

static reset_style(s, v)
int	s;
double	v;
{
	if (s == DASH_LINE) {
	    if (v > 0.0) fprintf(tfp, "\t[] 0 setdash\n");
	    }
	else if (s == DOTTED_LINE) {
	    if (v > 0.0) fprintf(tfp, "\t[] 0 setdash 0 setlinecap\n");
	    }
	}

static set_linewidth(w)
int	w;
{
	extern int	cur_thickness;

	if (w != cur_thickness) {
	    cur_thickness = w;
	    fprintf(tfp, "%.3f setlinewidth\n", cur_thickness <= 1 ? 0.5* cur_thickness : cur_thickness -1.0);
	    }
	}

void genps_line(l)
F_line	*l;
{
	F_point		*p, *q;
	/* JNT */
	int		radius, i = 0;
	FILE		*epsf;
	char		buf[512];
	char		*cp;
	int		xmin,xmax,ymin,ymax;
	int		eps_w, eps_h;
	
	if (multi_page)
	   fprintf(tfp, "/o%d {", no_obj++);
	set_linewidth(l->thickness);
	radius = l->radius;                /* radius of rounded-corner boxes */
	p = l->points;
	q = p->next;
	if (q == NULL) { /* A single point line */
            fprintf(tfp, "n %d %d m %d %d l gs col%d s gr\n",
                        p->x, p->y, p->x, p->y, l->color > MAXCOLORS ? -1 : l->color);
	    if (multi_page)
	       fprintf(tfp, "} bind def\n");
	    return;
	    }
	if (l->back_arrow && l->thickness > 0)
	    draw_arrow_head((double)q->x, (double)q->y, (double)p->x,
			(double)p->y, l->back_arrow->ht, l->back_arrow->wid,
			l->color);
	set_style(l->style, l->style_val);
	fprintf(tfp, "%% Polyline\n");

        xmin = xmax = p->x;
        ymin = ymax = p->y;
        while (p->next != NULL) /* find lower left and upper right corne
rs */
        {
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

	if (l->type == T_ARC_BOX)
        {
        fprintf(tfp, "n %d %d m",xmin+radius, ymin);
        fprintf(tfp, " %d %d %d %d %d arcto 4 {pop} repeat",
                                xmin, ymin, xmin, ymax-radius, radius);
        fprintf(tfp, " %d %d %d %d %d arcto 4 {pop} repeat", /* arc thro
ugh bl to br */
                                xmin, ymax, xmax-radius, ymax, radius);
        fprintf(tfp, " %d %d %d %d %d arcto 4 {pop} repeat", /* arc thro
ugh br to tr */
                                xmax, ymax, xmax, ymin+radius, radius);
        fprintf(tfp, " %d %d %d %d %d arcto 4 {pop} repeat", /* arc thro
ugh tr to tl */
                                xmax, ymin, xmin+radius, ymin, radius);
	}
	else if (l->type == T_EPS_BOX)  /* encapsulated postscript (eps) file */
	{
	int             dx, dy, rotation;
	int		llx, lly, urx, ury;

	dx = l->points->next->next->x - l->points->x;
	dy = l->points->next->next->y - l->points->y;
	rotation = 0;
	if (dx < 0 && dy < 0)
                   rotation = 180;
	else if (dx < 0 && dy >= 0)
                   rotation = 270;
	else if (dy < 0 && dx >= 0)
                   rotation = 90;

	fprintf(tfp, "%%\n");
	fprintf(tfp, "%% Begin Imported EPS File: %s\n", l->eps->file);
	fprintf(tfp, "%%\n");
	epsf = fopen(l->eps->file, "r");
	if (epsf == NULL) {
		fprintf (stderr, "Unable to open eps file: %s\n", l->eps->file);
		return;
	}
        while (fgets(buf, 512, epsf) != NULL) {
          lower(buf);
          if (!strncmp(buf, "%%boundingbox", 13)) {
                if (sscanf(buf, "%%%%boundingbox: %d %d %d %d",
                                   &llx, &lly, &urx, &ury) < 4) {
                  fprintf(stderr,"Bad EPS bitmap file: %s", l->eps->file);
                  fclose(epsf);
                  return;
                }
                break;
          }
        }

	fprintf(tfp, "gs\n");
	if (((rotation == 90 || rotation == 270) && !l->eps->flipped) ||
	    (rotation != 90 && rotation != 270 && l->eps->flipped)) {
		eps_h = urx - llx;
		eps_w = ury - lly;
	} else {
		eps_w = urx - llx;
		eps_h = ury - lly;
	}

	/* translate the eps stuff to the right spot on the page */
	fprintf(tfp, "%d %d translate\n", xmin, ymin);

	/* scale the eps stuff to fit into the bounding box */
	/* Note: the origin for fig is in the upper-right corner;
	 *       for postscript its in the lower right hand corner.
	 *       To fix it, we use a "negative"-y scale factor, then
	 *       translate the image up on the page */
	fprintf(tfp, "%f %f scale\n",
		fabs((double)(xmax-xmin)/eps_w), -1.0*(double)(ymax-ymin)/eps_h);
	fprintf(tfp, "0 %d translate\n", -eps_h);

	/* flip the eps stuff */
	/* always translate it back so that the lower-left corner is at the origin */
	if (l->eps->flipped) {
		fprintf(tfp, "0 %d translate\n", eps_h);
		fprintf(tfp, "1 -1 scale\n");
		fprintf(tfp, "%d 0 translate\n", eps_w);
		fprintf(tfp, "-1 1 scale\n");
	}

	/* note: fig measures rotation clockwise; postscript is counter-clockwise */
	/* always translate it back so that the lower-left corner is at the origin */
	switch (rotation) {
	   case 0:
		break;
	   case 90:
		fprintf(tfp, "%d %d translate\n", 0, eps_h);
		fprintf(tfp, "%d rotate\n", 270);
		break;
	   case 180:
		fprintf(tfp, "%d %d translate\n", eps_w, eps_h);
		fprintf(tfp, "%d rotate\n", 180);
		break;
	   case 270:
		fprintf(tfp, "%d %d translate\n", eps_w, 0);
		fprintf(tfp, "%d rotate\n", 90);
		break;
	}

	/* translate the eps stuff so that the lower-left corner is at the origin */
	fprintf(tfp, "%d %d translate\n", -llx, -lly);
	fprintf(tfp, "%%\n");
	epsf = fopen(l->eps->file, "r");
	if (epsf == NULL) {
		fprintf (stderr, "Unable to open eps file: %s\n", l->eps->file);
		fprintf(tfp, "gr\n");
		return;
	}
	while (fgets(buf, sizeof(buf), epsf) != NULL) {
		if (*buf == '%')		/* skip comment lines */
			continue;
		if ((cp=strstr(buf, "showpage")) != NULL)
			strcpy (cp, cp+8);	/* remove showpage */
		fputs(buf, tfp);
	}
	fclose (epsf);
	fprintf(tfp, "gr\n");
	fprintf(tfp, "%%\n");
	fprintf(tfp, "%% End Imported EPS File: %s\n", l->eps->file);
	fprintf(tfp, "%%\n");
	}
        else
        {
		p = l->points;
		q = p->next;
		fprintf(tfp, "n %d %d m", p->x, p->y);
		while (q->next != NULL) {
		    p = q;
		    q = q->next;
		    fprintf(tfp, " %d %d l ", p->x, p->y);
 	    	    if (!((++i)%5)) fprintf(tfp, "\n");
             	     	if (!((++i)%20) && (l->type == T_POLYLINE))
 	    	    	{
               		fprintf(tfp, "gs col%d s gr\n",
				l->color > MAXCOLORS ? -1 : l->color);
 	      		fprintf(tfp, "n %d %d m", p->x, p->y);
	    		}
		}
	}
	if (l->type == T_POLYLINE)
	    fprintf(tfp, " %d %d l ", q->x, q->y);
	else
	    fprintf(tfp, " clp ");
	if (l->area_fill && (int)l->area_fill != DEFAULT)
	    fill_area(l->area_fill, l->color);
	if (l->thickness > 0)
             fprintf(tfp, "gs col%d s gr\n",
			l->color > MAXCOLORS ? -1 : l->color);

	reset_style(l->style, l->style_val);
	if (l->for_arrow && l->thickness > 0)
	    draw_arrow_head((double)p->x, (double)p->y, (double)q->x,
			(double)q->y, l->for_arrow->ht, l->for_arrow->wid,
			l->color);
	if (multi_page)
	   fprintf(tfp, "} bind def\n");
	}

void genps_spline(s)
F_spline	*s;
{
	if (multi_page)
	   fprintf(tfp, "/o%d {", no_obj++);
	if (int_spline(s))
	    genps_itp_spline(s);
	else
	    genps_ctl_spline(s);
	if (multi_page)
	   fprintf(tfp, "} bind def\n");
	}

genps_itp_spline(s)
F_spline	*s;
{
	F_point		*p, *q;
	F_control	*a, *b;

	set_linewidth(s->thickness);
	a = s->controls;
	p = s->points;
        if (s->back_arrow && s->thickness > 0)
	    draw_arrow_head(a->rx, a->ry, (double)p->x,
			(double)p->y, s->back_arrow->ht, s->back_arrow->wid,
			s->color);

	set_style(s->style, s->style_val);
	fprintf(tfp, "%% Interpolated spline\n");
	fprintf(tfp, "n %d %d m\n", p->x, p->y);
	for (q = p->next; q != NULL; p = q, q = q->next) {
	    b = a->next;
	    fprintf(tfp, "\t%.3f %.3f %.3f %.3f %d %d curveto\n",
			a->rx, a->ry, b->lx, b->ly, q->x, q->y);
	    a = b;
	    }
	if (closed_spline(s)) fprintf(tfp, " clp ");
	if (s->area_fill && (int)s->area_fill != DEFAULT)
	    fill_area(s->area_fill, s->color);
	if (s->thickness > 0)
            fprintf(tfp, "gs col%d s gr\n", s->color > MAXCOLORS ? -1 : s->color);
	reset_style(s->style, s->style_val);

	if (s->for_arrow && s->thickness > 0)
	    draw_arrow_head(a->lx, a->ly, (double)p->x,
			(double)p->y, s->for_arrow->ht, s->for_arrow->wid,
			s->color);
	}

genps_ctl_spline(s)
F_spline	*s;
{
	double		a, b, c, d, x1, y1, x2, y2, x3, y3;
	F_point		*p, *q;

	/*
	if (first) {
	    first = FALSE;
	    fprintf(tfp, "%s\n", SPLINE_PS);
	    }
	*/

	p = s->points;
	x1 = p->x; y1 = p->y;
	p = p->next;
	c = p->x; d = p->y;
	set_linewidth(s->thickness);
	x3 = a = (x1 + c) / 2;
	y3 = b = (y1 + d) / 2;
	if (s->back_arrow && s->thickness > 0) {
	    draw_arrow_head(c, d, x1, y1, s->back_arrow->ht, s->back_arrow->wid,
			    s->color);
	    }
	set_style(s->style, s->style_val);
	if (! closed_spline(s)) {
	    fprintf(tfp, "%% Open spline\n");
	    fprintf(tfp, "n %.3f %.3f m %.3f %.3f l\n",
			x1, y1, x3, y3);
	    }
	else {
	    fprintf(tfp, "%% Closed spline\n");
	    fprintf(tfp, "n %.3f %.3f m\n", a, b);
	    }
	for (q = p->next; q != NULL; p = q, q = q->next) {
	    x1 = x3; y1 = y3;
	    x2 = c;  y2 = d;
	    c = q->x; d = q->y;
	    x3 = (x2 + c) / 2;
	    y3 = (y2 + d) / 2;
	    fprintf(tfp, "\t%.3f %.3f %.3f %.3f %.3f %.3f DrawSplineSection\n",
			x1, y1, x2, y2, x3, y3);
	    }
	/*
	* At this point, (x2,y2) and (c,d) are the position of the 
	* next-to-last and last point respectively, in the point list
	*/
	if (closed_spline(s)) {
	    fprintf(tfp, "\t%.3f %.3f %.3f %.3f %.3f %.3f DrawSplineSection closepath ",
			x3, y3, c, d, a, b);
	    }
	else {
	    fprintf(tfp, "\t%.3f %.3f l ", c, d);
	    }
	if (s->area_fill && (int)s->area_fill != DEFAULT)
	    fill_area(s->area_fill, s->color);
	if (s->thickness > 0)
            fprintf(tfp, "gs col%d s gr\n", s->color > MAXCOLORS ? -1 : s->color);
	reset_style(s->style, s->style_val);
	if (s->for_arrow && s->thickness > 0) {
	    draw_arrow_head(x2, y2, c, d, s->for_arrow->ht,
				s->for_arrow->wid, s->color);
	    }
	}

void genps_ellipse(e)
F_ellipse	*e;
{
	if (multi_page)
	   fprintf(tfp, "/o%d {", no_obj++);
	set_linewidth(e->thickness);
	set_style(e->style, e->style_val);
	if (e->angle == 0)
	{
	    fprintf(tfp, "%% Ellipse\n");
	    fprintf(tfp, "n %d %d %d %d 0 360 DrawEllipse ",
	          e->center.x, e->center.y, e->radiuses.x, e->radiuses.y);
	}
	else
	{
	    fprintf(tfp, "%% Rotated Ellipse\n");
	    fprintf(tfp, "gs\n");
	    fprintf(tfp, "%d %d translate\n",e->center.x, e->center.y);
	    fprintf(tfp, "%6.3f rotate\n",-e->angle*180/M_PI);
	    fprintf(tfp, "n 0 0 %d %d 0 360 DrawEllipse ",
		 e->radiuses.x, e->radiuses.y);
	}
	if (e->area_fill && (int)e->area_fill != DEFAULT)
	    fill_area(e->area_fill, e->color);
	if (e->thickness > 0)
            fprintf(tfp, "gs col%d s gr\n", e->color > MAXCOLORS ? -1 : e->color);
	if (e->angle != 0)
	    fprintf(tfp, "gr\n");
	reset_style(e->style, e->style_val);
	if (multi_page)
	   fprintf(tfp, "} bind def\n");
	}

#define	TEXT_PS		"\
/%s%s findfont %.2f scalefont setfont\n\
"
void genps_text(t)
F_text	*t;
{
	unsigned char		*cp;

	if (multi_page)
	   fprintf(tfp, "/o%d {", no_obj++);
	if (iso_encoding)
	   fprintf(tfp, TEXT_PS, PSFONT(t), "-iso", PSFONTMAG(t));
	else
	   fprintf(tfp, TEXT_PS, PSFONT(t), "", PSFONTMAG(t));

	fprintf(tfp, "%d %d m \ngs ", t->base_x,  t->base_y);
	if (coord_system == 2) fprintf(tfp, "1 -1 scale ");

	if (t->angle != 0)
	   fprintf(tfp, " %.1f rotate ", t->angle*180/M_PI);
	/* this loop escapes characters '(', ')', and '\' */
	fputc('(', tfp);
	for(cp = (unsigned char *)t->cstring; *cp; cp++) {
	      if (strchr("()\\", *cp)) fputc('\\', tfp);
              if (*cp>=0x80)
                 fprintf(tfp,"\\%o", *cp);
              else
	         fputc(*cp, tfp);
	      }
	fputc(')', tfp);

	if ((t->type == T_CENTER_JUSTIFIED) || (t->type == T_RIGHT_JUSTIFIED)){

	  	fprintf(tfp, " dup stringwidth pop ");
		if (t->type == T_CENTER_JUSTIFIED) fprintf(tfp, "2 div ");
		fprintf(tfp, "neg 0 rmoveto ");
	      	}

	else if ((t->type != T_LEFT_JUSTIFIED) && (t->type != DEFAULT))
		fprintf(stderr, "Text incorrectly positioned\n");

	fprintf(tfp, " col%d show gr\n", t->color > MAXCOLORS ? -1 : t->color);

	if (multi_page)
	   fprintf(tfp, "} bind def\n");
	}

void genps_arc(a)
F_arc	*a;
{
	double		angle1, angle2, dx, dy, radius, x, y;
	double		cx, cy, sx, sy, ex, ey;
	int		direction;

	if (multi_page)
	   fprintf(tfp, "/o%d {", no_obj++);
	cx = a->center.x; cy = a->center.y;
	sx = a->point[0].x; sy = a->point[0].y;
	ex = a->point[2].x; ey = a->point[2].y;

	if (coord_system == 2)
	    direction = !a->direction;
	else
	    direction = a->direction;
	set_linewidth(a->thickness);
	if (a->for_arrow && a->thickness > 0) {
	    arc_tangent(cx, cy, ex, ey, direction, &x, &y);
	    draw_arrow_head(x, y, ex, ey, a->for_arrow->ht, a->for_arrow->wid, a->color);
	    }
	if (a->back_arrow && a->thickness > 0) {
	    arc_tangent(cx, cy, sx, sy, !direction, &x, &y);
	    draw_arrow_head(x, y, sx, sy, a->back_arrow->ht, a->back_arrow->wid, a->color);
	    }
	dx = cx - sx;
	dy = cy - sy;
	radius = hypot(dx, dy);
	angle1 = atan2(sy-cy, sx-cx) * 180 / M_PI;
	angle2 = atan2(ey-cy, ex-cx) * 180 / M_PI;
	/* direction = 1 -> Counterclockwise */
	set_style(a->style, a->style_val);
	fprintf(tfp, "n %.3f %.3f %.3f %.3f %.3f %s\n",
		cx, cy, radius, angle1, angle2,
		((direction == 1) ? "arc" : "arcn"));
	if (a->area_fill && (int)a->area_fill != DEFAULT)
	    fill_area(a->area_fill, a->color);
	if (a->thickness > 0)
	    fprintf(tfp, "gs col%d s gr\n", a->color > MAXCOLORS ? -1 : a->color);
	reset_style(a->style, a->style_val);
	if (multi_page)
	   fprintf(tfp, "} bind def\n");
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

/*	draw arrow heading from (x1, y1) to (x2, y2)	*/

static draw_arrow_head(x1, y1, x2, y2, arrowht, arrowwid, col)
double	x1, y1, x2, y2, arrowht, arrowwid;
int col;
{
	double	x, y, xb, yb, dx, dy, l, sina, cosa;
	double	xc, yc, xd, yd;

	dx = x2 - x1;  dy = y1 - y2;
	l = hypot(dx, dy);
	if (l == 0) {
	     return;
	}
	else {
	     sina = dy / l;  cosa = dx / l;
	}
	xb = x2*cosa - y2*sina;
	yb = x2*sina + y2*cosa;
	x = xb - arrowht;
	y = yb - arrowwid / 2;
	xc = x*cosa + y*sina;
	yc = -x*sina + y*cosa;
	y = yb + arrowwid / 2;
	xd = x*cosa + y*sina;
	yd = -x*sina + y*cosa;
        fprintf(tfp, "n %.3f %.3f m %.3f %.3f l %.3f %.3f l gs 2 setlinejoin col%d s gr\n", xc, yc, x2, y2, xd, yd, col > MAXCOLORS ? -1 : col);
	}

static fill_area(fill, color)
int fill, color;
{
   if (color < 1)   /* use gray levels for default and black */
	fprintf(tfp, "gs %.2f setgray fill gr\n", 1.0 - GRAYVAL(fill));
   else
	fprintf(tfp, "gs col%d %.2f graycol fill gr ",
		color > MAXCOLORS ? -1 : color, GRAYVAL(fill));
}

static iso_text_exist(ob)
F_compound      *ob;
{
   F_compound	*c;
   F_text          *t;
   unsigned char   *s;

   if (ob->texts != NULL)
   {
      for (t = ob->texts; t != NULL; t = t->next)
      {
         for (s = (unsigned char*)t->cstring; *s != '\0'; s++)
         {
            /* look for characters >= 128 */
            if (*s>127) return(1);
         }
      }
   }

   for (c = ob->compounds; c != NULL; c = c->next) {
       if (iso_text_exist(c)) return(1);
       }
   return(0);
}

static encode_all_fonts(ob)
F_compound	*ob;
{
   F_compound *c;
   F_text     *t;
   static int font_defined[ MAX_PSFONT ];

   if (ob->texts != NULL)
   {
      for (t = ob->texts; t != NULL; t = t->next)
      if (font_defined[t->font] != 1)
      {
         fprintf(tfp, "/%s /%s-iso isovec ReEncode\n", PSFONT(t), PSFONT(t));
         font_defined[t->font] = 1;
      }
   }

   for (c = ob->compounds; c != NULL; c = c->next) {
       encode_all_fonts(c);
       }
}

static ellipse_exist(ob)
F_compound	*ob;
{
	F_compound	*c;

	if (NULL != ob->ellipses) return(1);

	for (c = ob->compounds; c != NULL; c = c->next) {
	    if (ellipse_exist(c)) return(1);
	    }

	return(0);
	}

static normal_spline_exist(ob)
F_compound	*ob;
{
	F_spline	*s;
	F_compound	*c;

	for (s = ob->splines; s != NULL; s = s->next) {
	    if (normal_spline(s)) return(1);
	    }

	for (c = ob->compounds; c != NULL; c = c->next) {
	    if (normal_spline_exist(c)) return(1);
	    }

	return(0);
	}

struct driver dev_ps = {
     	genps_option,
	genps_start,
	genps_arc,
	genps_ellipse,
	genps_line,
	genps_spline,
	genps_text,
	genps_end,
	INCLUDE_TEXT
};
