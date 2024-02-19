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

#define		DEFAULT				(-1)

typedef		struct f_point {
			int			x, y;
			struct f_point		*next;
			}
		F_point;

typedef		struct f_pos {
			int			x, y;
			}
		F_pos;

typedef		struct f_arrow {
			int			type;
			int			style;
			double			thickness;
			double			wid;
			double			ht;
			}
		F_arrow;

typedef		struct f_ellipse {
			int			type;
#define					T_ELLIPSE_BY_RAD	1
#define					T_ELLIPSE_BY_DIA	2
#define					T_CIRCLE_BY_RAD		3
#define					T_CIRCLE_BY_DIA		4
			int			style;
			int			thickness;
			int			color;
#define 	 			BLACK_COLOR		0
#define					BLUE_COLOR		1
#define					GREEN_COLOR		2
#define					CYAN_COLOR		3
#define					RED_COLOR		4
#define					MAGENTA_COLOR		5
#define					YELLOW_COLOR		6
#define					WHITE_COLOR		7
			int			depth;
			int			direction;
			double			style_val;
			double			angle;
			int			pen;
			int			area_fill;
#define		       			UNFILLED	0
#define		       			WHITE_FILL	1
#define		       			BLACK_FILL	21
			struct f_pos		center;
			struct f_pos		radiuses;
			struct f_pos		start;
			struct f_pos		end;
			struct f_ellipse	*next;
			}
		F_ellipse;

typedef		struct f_arc {
			int			type;
#define					T_3_POINTS_ARC		1
			int			style;
			int			thickness;
			int			color;
			int			depth;
			int			pen;
			int			area_fill;
			double			style_val;
			int			direction;
			struct f_arrow		*for_arrow;
			struct f_arrow		*back_arrow;
			struct {double x, y;}	center;
			struct f_pos		point[3];
			struct f_arc		*next;
			}
		F_arc;

typedef		struct f_line {
			int			type;
#define					T_POLYLINE	1
#define					T_BOX		2
#define					T_POLYGON	3
#define	                                T_ARC_BOX       4
#define	                                T_EPS_BOX       5 

			int			style;
			int			thickness;
			int			color;
			int			depth;
			double			style_val;
			int			pen;
 			int			area_fill;
			int			radius;	/* for T_ARC_BOX */
			struct f_arrow		*for_arrow;
			struct f_arrow		*back_arrow;
			struct f_point		*points;
		    	struct f_eps   		*eps;
			struct f_line		*next;
			}
		F_line;

typedef struct f_eps {
    char            file[256];
    int             flipped;
    unsigned char  *bitmap;
    float	    hw_ratio;
    struct f_pos    bit_size;
    int             pix_rotation, pix_width, pix_height, pix_flipped;
}
		F_eps;

extern char EMPTY_EPS[];

typedef		struct f_text {
			int			type;
#define					T_LEFT_JUSTIFIED	0
#define					T_CENTER_JUSTIFIED	1
#define					T_RIGHT_JUSTIFIED	2
			int			font;
#define					DEFAULT_FONT		0
#define					ROMAN_FONT		1
#define					BOLD_FONT		2
#define					ITALIC_FONT		3
#define					MODERN_FONT		4
#define					TYPEWRITER_FONT		5
#define					MAX_FONT		5
			double			size;	/* point size */
			int			color;
			int			depth;
			double			angle;	/* in radian */
			int			flags;
#define					RIGID_TEXT	1	
#define					SPECIAL_TEXT	2
#define					PSFONT_TEXT	4
			double			height;	/* pixels */
			double			length;	/* pixels */
			int			base_x;
			int			base_y;
			int			pen;
			char			*cstring;
			struct f_text		*next;
			}
		F_text;

#define MAX_PSFONT	35
#define MAXFONT(T) (psfont_text(T) ? MAX_PSFONT : MAX_FONT)

#define		rigid_text(t) \
			(t->flags == DEFAULT \
				|| (t->flags & RIGID_TEXT))

#define		special_text(t) \
			((t->flags != DEFAULT \
				&& (t->flags & SPECIAL_TEXT)))

#define		psfont_text(t) \
			(t->flags != DEFAULT \
				&& (t->flags & PSFONT_TEXT))

typedef		struct f_control {
			double			lx, ly, rx, ry;
			struct f_control	*next;
			}
		F_control;

#define		int_spline(s)		(s->type & 0x2)
#define		normal_spline(s)	(!(s->type & 0x2))
#define		closed_spline(s)	(s->type & 0x1)
#define		open_spline(s)		(!(s->type & 0x1))

typedef		struct f_spline {
			int			type;
#define					T_OPEN_NORMAL		0
#define					T_CLOSED_NORMAL		1
#define					T_OPEN_INTERPOLATED	2
#define					T_CLOSED_INTERPOLATED	3
			int			style;
			int			thickness;
			int			color;
			int			depth;
			double			style_val;
			int			pen;
			int			area_fill;
			struct f_arrow		*for_arrow;
			struct f_arrow		*back_arrow;
			/*
			For T_OPEN_NORMAL and T_CLOSED_NORMAL points
			are control points while they are knots for
			T_OPEN_INTERPOLATED and T_CLOSED_INTERPOLTED
			whose control points are stored in controls.
			*/
			struct f_point		*points;
			struct f_control	*controls;
			struct f_spline		*next;
			}
		F_spline;

typedef		struct f_compound {
			struct f_pos		nwcorner;
			struct f_pos		secorner;
			struct f_line		*lines;
			struct f_ellipse	*ellipses;
			struct f_spline		*splines;
			struct f_text		*texts;
			struct f_arc		*arcs;
			struct f_compound	*compounds;
			struct f_compound	*next;
			}
		F_compound;

#define		ARROW_SIZE		sizeof(struct f_arrow)
#define		POINT_SIZE		sizeof(struct f_point)
#define		CONTROL_SIZE		sizeof(struct f_control)
#define		ELLOBJ_SIZE		sizeof(struct f_ellipse)
#define		ARCOBJ_SIZE		sizeof(struct f_arc)
#define		LINOBJ_SIZE		sizeof(struct f_line)
#define		EPS_SIZE		sizeof(struct f_eps)
#define		TEXOBJ_SIZE		sizeof(struct f_text)
#define		SPLOBJ_SIZE		sizeof(struct f_spline)
#define		COMOBJ_SIZE		sizeof(struct f_compound)

/**********************  object codes  **********************/

#define		O_ELLIPSE		1
#define		O_POLYLINE		2
#define		O_SPLINE		3
#define		O_TEXT			4
#define		O_ARC			5
#define		O_COMPOUND		6
#define		O_END_COMPOUND		(-O_COMPOUND)
#define		O_ALL_OBJECT		99

/************  object styles (except for f_text)  ************/

#define		SOLID_LINE		0
#define		DASH_LINE		1
#define		DOTTED_LINE		2

#define		CLOSED_PATH		0
#define		OPEN_PATH		1
