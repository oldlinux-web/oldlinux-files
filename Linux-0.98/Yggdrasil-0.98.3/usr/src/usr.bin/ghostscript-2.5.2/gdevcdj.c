/* Copyright (C) 1991, 1992 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */

/* gdevcdj.c */
/* H-P DeskJet 500C driver (colour) for Ghostscript */
#include <stdlib.h>		/* for rand() */
#include "gdevprn.h"
#include "gdevpcl.h"
#include "gsprops.h"

/***
 *** Note: this driver was contributed by a user, George Cameron:
 ***       please contact g.cameron@biomed.abdn.ac.uk if you have questions.
 ***/

/*
 * Note that there are three drivers contained in this code:
 *
 *     1 - cdeskjet:    A slightly updated version of the original
 *                      cdeskjet driver.
 *     2 - cdjcolor:    This is the significant addition - a 24-bit
 *                      Floyd-Steinberg dithering driver, which gives
 *                      excellent quality, but necessarily results in
 *                      slow printing.
 *     3 - cdjmono:     Included purely to give dj500c owners the benefit
 *                      of Mode 9 fast printing with their black cartridge.
 */

/*
 * You may select a resolution of 75, 100, 150, or 300 DPI. Normally you
 * would do this in the makefile or on the gs command line, not here.
 * 
 * If the preprocessor symbol A4 is defined, the default paper size is the
 * European A4 size; otherwise it is the U.S. letter size (8.5"x11").
 */

#define X_DPI_MAX 300
#define Y_DPI_MAX 300

#ifndef X_DPI
#  define X_DPI X_DPI_MAX
#endif
#ifndef Y_DPI
#  define Y_DPI Y_DPI_MAX
#endif

/*
 * Maximum printing width = 2400 dots = 8"
 *
 * All Deskjets have 1/2" unprintable bottom margin
 */

#define PRINT_LIMIT 0.0625	/* 'real' top margin? */

/* Margins are left, bottom, right, top. */
#define DESKJET_MARGINS_LETTER  0.25, 0.50, 0.25, 0.167
#define DESKJET_MARGINS_A4      0.15, 0.50, 0.15, 0.167

#ifndef A4
#  define WIDTH_10THS           85
#  define HEIGHT_10THS          110
#  define DESKJET_MARGINS       DESKJET_MARGINS_LETTER
#else
#  define WIDTH_10THS           83      /* 210mm */
#  define HEIGHT_10THS          117     /* 297mm */
#  define DESKJET_MARGINS       DESKJET_MARGINS_A4
#endif

/* The number of blank lines that make it worthwhile to reposition */
/* the cursor. */
#define MIN_SKIP_LINES 7

#define W sizeof(word)
#define I sizeof(int)
#define WSIZE(a) (((a) + W - 1) / W)

/* Printer types */
#define DJ500C_COLOUR    0	/* Standard colour, GS internal dithering */
#define DJ500C_COLOUR_FS 1	/* High quality dithering, but can be slow */
#define DJ500C_MONO      2	/* Black ink + mode 9 compression */

/* Procedures */
private dev_proc_map_rgb_color (gdev_pcl_true_map_rgb_color);
private dev_proc_map_color_rgb (gdev_pcl_true_map_color_rgb);

/* The device descriptors */
private dev_proc_open_device(hp_dj500c_open);
private dev_proc_print_page(cdeskjet_print_page);
private dev_proc_print_page(cdjcolor_print_page);
private dev_proc_print_page(cdjmono_print_page);
private dev_proc_get_props(hp_dj500c_get_props);
private dev_proc_put_props(hp_dj500c_put_props);

/* The device descriptor */
typedef struct gx_device_hp_dj500c_s gx_device_hp_dj500c;
struct gx_device_hp_dj500c_s {
        gx_device_common;
        gx_prn_device_common;
	int shingling;		/* Interlaced, multi-pass printing */
	                        /* 0 = none, 1 = 50%, 2 = 25%, 2 is best &
				 * slowest */
	int depletion;		/* 'Intelligent' dot-removal */
		                /* 0 = none, 1 = 25%, 2 = 50%, 1 best for
				/* graphics? */
				/* Use 0 for transparencies */
};

#define ppdev ((gx_device_hp_dj500c *)pdev)

#define hp_dj500c_device(procs, dev_name, width_10ths, height_10ths, x_dpi, y_dpi, l_margin, b_margin, r_margin, t_margin, color_bits, print_page, shingling, depletion)\
{ prn_device_body(gx_device_printer, procs, dev_name,\
    width_10ths, height_10ths, x_dpi, y_dpi,\
    l_margin, b_margin, r_margin, t_margin,\
    (color_bits > 1 ? 3 : 1),\
    ((color_bits > 1) & (color_bits < 8) ? 8 : color_bits),\
    (color_bits >= 8 ? 255 : 1),\
    (color_bits >= 8 ? 255 : color_bits > 1 ? 1 : 0),\
    (color_bits >= 8 ? 5 : 2),\
    (color_bits >= 8 ? 5 : color_bits > 1 ? 2 : 0),\
    print_page),\
    shingling,\
    depletion\
}

#define hp_dj500c_procs(proc_map_rgb_color, proc_map_color_rgb, proc_get_props, proc_put_props) {\
	hp_dj500c_open,\
	gdev_pcl_get_initial_matrix,\
	gx_default_sync_output,\
	gdev_prn_output_page,\
	gdev_prn_close,\
	proc_map_rgb_color,\
	proc_map_color_rgb,\
	NULL,	/* fill_rectangle */\
	NULL,	/* tile_rectangle */\
	NULL,	/* copy_mono */\
	NULL,	/* copy_color */\
	NULL,	/* draw_line */\
	gx_default_get_bits,\
	proc_get_props,\
	proc_put_props\
}

private gx_device_procs cdeskjet_procs =
hp_dj500c_procs(gdev_pcl_3bit_map_rgb_color, gdev_pcl_3bit_map_color_rgb,
		hp_dj500c_get_props, hp_dj500c_put_props);

private gx_device_procs cdjcolor_procs =
hp_dj500c_procs(gdev_pcl_true_map_rgb_color, gdev_pcl_true_map_color_rgb,
		hp_dj500c_get_props, hp_dj500c_put_props);

private gx_device_procs cdjmono_procs =
hp_dj500c_procs(gdev_prn_map_rgb_color, gdev_prn_map_color_rgb,
		hp_dj500c_get_props, hp_dj500c_put_props);

gx_device_hp_dj500c gs_cdeskjet_device =
hp_dj500c_device(cdeskjet_procs, "cdeskjet",
	   WIDTH_10THS, HEIGHT_10THS,
	   X_DPI, Y_DPI,
	   0, 0, 0, 0,
	   3, cdeskjet_print_page, 1, 1);

gx_device_hp_dj500c gs_cdjcolor_device =
hp_dj500c_device(cdjcolor_procs, "cdjcolor",
	   WIDTH_10THS, HEIGHT_10THS,
	   X_DPI, Y_DPI,
	   0, 0, 0, 0,
	   24, cdjcolor_print_page, 2, 1);

gx_device_hp_dj500c gs_cdjmono_device =
hp_dj500c_device(cdjmono_procs, "cdjmono",
	   WIDTH_10THS, HEIGHT_10THS,
	   X_DPI, Y_DPI,
	   0, 0, 0, 0,
	   1, cdjmono_print_page, 1, 1);

/* Forward references */
private int gdev_pcl_mode9compress(P4(int, const byte *, byte *, byte *));
private int hp_dj500c_print_page(P3(gx_device_printer *, FILE *, int));

/* Open the printer and set up the margins. */
private int
hp_dj500c_open(gx_device *pdev)
{       /* Change the margins if necessary. */
  static const float m_a4[4] = { DESKJET_MARGINS_A4 };
  static const float m_letter[4] = { DESKJET_MARGINS_LETTER };
  const float _ds *m =
    (gdev_pcl_paper_size(pdev) == PAPER_SIZE_A4 ? m_a4 :
     m_letter);
  pdev->l_margin = m[0];
  pdev->b_margin = m[1];
  pdev->r_margin = m[2];
  pdev->t_margin = m[3];
  return gdev_prn_open(pdev);
}

/* Added properties for DeskJet 500C */

private const gs_prop_item props_dj500c[] = {
  /* Read-write properties. */
  prop_def("Shingling", prt_int),
  prop_def("Depletion", prt_int),
};

/* Get properties.  In addition to the standard and printer */
/* properties, we supply shingling and depletion parameters */
private int
hp_dj500c_get_props(gx_device *pdev, gs_prop_item *plist)
{	int start = gdev_prn_get_props(pdev, plist);
	if ( plist != 0 )
	   {	register gs_prop_item *pi = plist + start;
		memcpy(pi, props_dj500c, sizeof(props_dj500c));
		pi[0].value.i = ppdev->shingling;
		pi[1].value.i = ppdev->depletion;
	   }
	return start + sizeof(props_dj500c) / sizeof(gs_prop_item);
}

/* Put properties. */
private int
hp_dj500c_put_props(gx_device *pdev, gs_prop_item *plist, int count)
{	gs_prop_item *known[2];
	int code = 0;
	int j;
	props_extract(plist, count, props_dj500c, 2, known, 0);
	code = gdev_prn_put_props(pdev, plist, count);
	if ( code < 0 ) return code;
	for (j = 0; j < 2; j++) {
	  if ( known[j] != 0 ) {
	    gs_prop_item *pi = known[j];
	    if ( pi->value.i < 0 || pi->value.i > 2 )
	      pi->status = pv_rangecheck,
	      code = gs_error_rangecheck;
	    else {
	      switch (j) {
	      case 0:
		ppdev->shingling = known[j]->value.i;
		break;
	      case 1:
		ppdev->depletion = known[j]->value.i;
		break;
	      }
	      if ( code == 0 ) code = 1;
	    }
	  }
	}
	if ( code < 0 )
		return_error(code);
	return code;
}

/* ------ Internal routines ------ */

/* The DeskJet 500C can compress (mode 9, for all versions) */
private int
cdeskjet_print_page(gx_device_printer * pdev, FILE * prn_stream)
{
  return hp_dj500c_print_page(pdev, prn_stream, DJ500C_COLOUR);
}

private int
cdjcolor_print_page(gx_device_printer * pdev, FILE * prn_stream)
{
  return hp_dj500c_print_page(pdev, prn_stream, DJ500C_COLOUR_FS);
}

private int
cdjmono_print_page(gx_device_printer * pdev, FILE * prn_stream)
{
  return hp_dj500c_print_page(pdev, prn_stream, DJ500C_MONO);
}

/* Some convenient shorthand .. */
#define x_dpi    (pdev->x_pixels_per_inch)
#define y_dpi    (pdev->y_pixels_per_inch)
#define height   (pdev->height)
#define t_margin (pdev->t_margin)
#define b_margin (pdev->b_margin)
#define XTRA 12		        /* 2 x 6 XTRA values for end-of-line */
                                /* in FSdither error buffers */

/* Floyd-Steinberg dithering. Often results in a dramatic improvement in
 * subjective image quality, but can also produce dramatic increases in
 * amount of printer data generated and actual printing time!! Mode 9 2D
 * compression is still useful for fairly flat colour or blank areas but its
 * compression is much less effective in areas where the dithering has
 * effectively randomised the dot distribution. This is a first attempt, but
 * it seems to work reasonably well for the images I've tried. */
#define MAXVALUE  0xff
#define THRESHOLD 0x80
#define C 4                /* ought to be 8, but this seems to be too much */
#define FSdither(inP, out, errP, Err, Bit, DD, II, Offset)\
	oldErr = Err;\
	Err = (* DD errP + ((Err * 7 + C) >> 4) + *(DD inP II));\
	if (Err > THRESHOLD) {\
	  out |= Bit;\
	  Err -= MAXVALUE;\
	}\
	errP[Offset 6] += ((oldErr * 3 + C) >> 4);\
	errP[Offset 3] += ((oldErr * 5 + C) >> 4);\
	* errP II = ((oldErr + C) >> 4);

/* Send the page to the printer.  Compress each scan line. */
private int
hp_dj500c_print_page(gx_device_printer * pdev, FILE * prn_stream, int ptype)
{
  int line_size = gdev_prn_raster(pdev);
  int line_size_words = WSIZE(line_size);
  int paper_size = gdev_pcl_paper_size((gx_device *)pdev);
  int num_comps = pdev->color_info.num_components;
  int plane_size;
  int buffer_size;
  int *errors;
  byte *data, *plane_data[3], *prev_plane_data[3], *out_data;
  word *storage, *data_words;
  uint storage_size_words;

  switch (ptype) {
  case DJ500C_COLOUR:
    plane_size = (line_size + 7) / 8;
    buffer_size = plane_size * 8;
    storage_size_words = WSIZE(plane_size * (8 + 3 + 3));
    /* data, plane, prev_plane */
    break;
  case DJ500C_COLOUR_FS:
    plane_size = (line_size + 23) / 24;
    buffer_size = plane_size * 24;
    storage_size_words = WSIZE(plane_size * (24 * I + 24 + 3 + 3) + XTRA * I);
    /* errors, data, plane, prev_plane, XTRA */
    break;
  case DJ500C_MONO:
    plane_size = WSIZE(line_size) * W;
    buffer_size = plane_size;
    storage_size_words = WSIZE(plane_size * (1 + 1 + 2));
    /* plane, prev_plane, out_data */
    break;
  }
  storage = (ulong *) gs_malloc(storage_size_words, W, "hp_dj500c_print_page");
  data_words = storage;

  if (storage == 0)		/* can't allocate working area */
    return_error(gs_error_VMerror);
  else {
    int i;
    byte *p = data = out_data = (byte *) storage;
    if ((ptype == DJ500C_COLOUR) || (ptype == DJ500C_COLOUR_FS)) {
      p += buffer_size;
    }
    if (ptype == DJ500C_COLOUR_FS) {
      errors = (int *)p;
      p += (buffer_size + XTRA) * I;
    }
    for (i = 0; i < num_comps; i++) {
      plane_data[i] = p;
      p += plane_size;
    }
    for (i = 0; i < num_comps; i++) {
      prev_plane_data[i] = p;
      p += plane_size;
    }
    if (ptype == DJ500C_MONO) {
      out_data = p;		/* size is plane_size x 2 */
    }
  }

  /* Clear temp storage */
  memset(storage, 0, storage_size_words * W);

  /* Initialize printer. */
  fputs("\033E", prn_stream);	                      /* reset printer */
  fputs("\033*rbC", prn_stream);                      /* end raster graphics */
  fprintf(prn_stream, "\033*t%dR", (int)x_dpi);	      /* set resolution */
  fprintf(prn_stream, "\033&l%da0o0e0L", paper_size); /* paper size etc. */

  /* set the number of color planes (1 or 3). -3 means CMY in colour mode */
  fprintf(prn_stream, "\033*r-%dU", num_comps);

  /* set depletion and shingling levels */
  fprintf(prn_stream, "\033*o%dd%dQ", ppdev->depletion, ppdev->shingling);

  /* move to top left of printed area */
#define OFFSET (t_margin - PRINT_LIMIT)	/* Offset to print position */
  fprintf(prn_stream, "\033*p0x%dY", (int)(Y_DPI_MAX * OFFSET));

  /* select data compression */
  fputs("\033*b9M", prn_stream);/* mode 9 */

  /* Start raster graphics.  From now on */
  /* all escape commands start with \033*b, */
  /* so we combine them. */
  fputs("\033*r1A\033*b", prn_stream);

  /* Send each scan line in turn */
  {
    int lnum, i;
    int lend = height - (t_margin + b_margin) * y_dpi;
    int num_blank_lines = 0;
    word rmask = ~(word) 0 << (-pdev->width & (W * 8 - 1));
    int c, m, y, cErr, mErr, yErr;
    int going_up = 1;
    byte *cP = plane_data[0], *mP = plane_data[1], *yP = plane_data[2];
    register int *ep = errors + (XTRA / 2);
    register byte *dp = data;

    c = m = y = cErr = mErr = yErr = 0;

    if (ptype == DJ500C_COLOUR_FS) { /* Randomly seed initial error buffer */
      for (i = 0; i < buffer_size; i++) {
	*ep++ = (rand() % MAXVALUE) >> 4;
      }
      ep = errors + (XTRA / 2);
    }

    for (lnum = 0; lnum < lend; lnum++) {
      register word *end_data = data_words + line_size_words;
      gdev_prn_copy_scan_lines(pdev, lnum, data, line_size);

      /* Mask off 1-bits beyond the line width. */
      end_data[-1] &= rmask;

      /* Remove trailing 0s. */
      while (end_data > data_words && end_data[-1] == 0)
	end_data--;
      if (end_data == data_words) {	/* Blank line */
	num_blank_lines++;
	continue;
      }
      /* Skip blank lines if any */
      if (num_blank_lines > 0) {
	if (num_blank_lines < MIN_SKIP_LINES) {
	  /* Moving down from current position */
	  /* causes head motion on the DeskJet, so */
	  /* if the number of lines is small, */
	  /* we're better off printing blanks. */
	  fputs("y", prn_stream); /* Clear current and seed rows */
	  for (; num_blank_lines; num_blank_lines--)
	    fputs("w", prn_stream);
	} else {
	  fprintf(prn_stream, "%dy", num_blank_lines);
	}
	memset(prev_plane_data[0], 0, plane_size * num_comps);
	num_blank_lines = 0;
      } {			/* Printing non-blank lines */
	int i, j;
	byte *odp;

	/* In colour modes, we have some bit-shuffling to do before */
	/* we can print the data; in FS mode we also have the */
	/* dithering to take care of. */
	switch (ptype) {
	case DJ500C_COLOUR:

	  /* Transpose the data to get pixel planes. */
	  for (i = 0, odp = plane_data[0]; i < buffer_size;
	       i += 8, odp++) {	/* The following is for 16-bit
				 * machines */
#define spread3(c)\
{ 0, c, c*0x100, c*0x101, c*0x10000L, c*0x10001L, c*0x10100L, c*0x10101L }
	    static ulong spr40[8] = spread3(0x40);
	    static ulong spr08[8] = spread3(8);
	    static ulong spr02[8] = spread3(2);
	    register byte *dp = data + i;
	    register ulong pword =
	    (spr40[dp[0]] << 1) +
	    (spr40[dp[1]]) +
	    (spr40[dp[2]] >> 1) +
	    (spr08[dp[3]] << 1) +
	    (spr08[dp[4]]) +
	    (spr08[dp[5]] >> 1) +
	    (spr02[dp[6]]) +
	    (spr02[dp[7]] >> 1);
	    odp[0] = (byte) (pword >> 16);
	    odp[plane_size] = (byte) (pword >> 8);
	    odp[plane_size * 2] = (byte) (pword);
	  }
	  break;

	case DJ500C_COLOUR_FS:

	  if (going_up) {
	    for (i = 0; i < plane_size; i++) {
	      byte c, y, m, bitmask;
	      int oldErr;

	      bitmask = 0x80;
	      for (c = m = y = j = 0; j < 8; j++) {
		FSdither(dp, c, ep, cErr, bitmask,, ++, -);
		FSdither(dp, m, ep, mErr, bitmask,, ++, -);
		FSdither(dp, y, ep, yErr, bitmask,, ++, -);
		bitmask >>= 1;
	      }
	      *cP++ = c;
	      *mP++ = m;
	      *yP++ = y;
	    }
	  } else {		/* going_down */
	    for (i = 0; i < plane_size; i++) {
	      byte c, y, m, bitmask;
	      int oldErr;

	      bitmask = 0x01;
	      for (c = m = y = j = 0; j < 8; j++) {
		FSdither(dp, y, ep, yErr, bitmask, --,, +);
		FSdither(dp, m, ep, mErr, bitmask, --,, +);
		FSdither(dp, c, ep, cErr, bitmask, --,, +);
		bitmask <<= 1;
	      }
	      *--cP = c;
	      *--mP = m;
	      *--yP = y;
	    }
	  }
	  going_up = !going_up;       /* toggle scan direction */
	  break;
	} /* switch() DJET500C_COLOUR and DJET500C_COLOUR_FS */

	/* Transfer raster graphics */
	/* in the order C, M, Y. */
	for (i = num_comps - 1; i >= 0; i--) {

	  int out_count = gdev_pcl_mode9compress(plane_size,
						 plane_data[i],
						 prev_plane_data[i],
						 out_data);

	  fprintf(prn_stream, "%d%c", out_count, "wvv"[i]);
	  fwrite(out_data, sizeof(byte), out_count, prn_stream);
	}
      }	  /* Printing non-blank lines */
    }     /* for lnum ... */
  }       /* send each scan line in turn */

  /* end raster graphics */
  fputs("\033*rbC", prn_stream);

  /* reset to monochrome */
  fputs("\033*r1U", prn_stream);

  /* eject page */
  fputs("\033&l0H", prn_stream);

  /* free temporary storage */
  gs_free((char *) storage, storage_size_words, W, "hp_dj500c_print_page");

  return 0;
}

/*
 * Mode 9 2D compression for the HP DeskJet 500C. This mode can give
 * very good compression ratios, especially if there are areas of flat
 * colour (or blank areas), and so is 'highly recommended' for colour
 * printing in particular because of the very large amounts of data which
 * can be generated
 */
private int
gdev_pcl_mode9compress(int bytecount, const byte * current, byte * previous, byte * compressed)
{
  register const byte *cur = current;
  register byte *prev = previous;
  register byte *out = compressed;
  const byte *end = current + bytecount;

  while (cur < end) {		/* Detect a run of unchanged bytes. */
    const byte *run = cur;
    register const byte *diff;
    int offset;
    while (cur < end && *cur == *prev) {
      cur++, prev++;
    }
    if (cur == end)
      break;			/* rest of row is unchanged */
    /* Detect a run of changed bytes. */
    /* We know that *cur != *prev. */
    diff = cur;
    do {
      *prev++ = *cur++;
    }
    while (cur < end && *cur != *prev);
    /* Now [run..diff) are unchanged, and */
    /* [diff..cur) are changed. */
    offset = diff - run;
    {
      const byte *stop_test = cur - 4;
      int dissimilar, similar;

      while (diff < cur) {
	const byte *compr = diff;
	const byte *next;	/* end of run */
	byte value;
	while (diff <= stop_test &&
	       ((value = *diff) != diff[1] ||
		value != diff[2] ||
		value != diff[3]))
	  diff++;

	/* Find out how long the run is */
	if (diff > stop_test)	/* no run */
	  next = diff = cur;
	else {
	  next = diff + 4;
	  while (next < cur && *next == value)
	    next++;
	}

#define MAXOFFSETU 15
#define MAXCOUNTU 7
	/* output 'dissimilar' bytes, uncompressed */
	if ((dissimilar = diff - compr)) {
	  int temp, i;

	  if ((temp = --dissimilar) > MAXCOUNTU)
	    temp = MAXCOUNTU;
	  if (offset < MAXOFFSETU)
	    *out++ = (offset << 3) | (byte) temp;
	  else {
	    *out++ = (MAXOFFSETU << 3) | (byte) temp;
	    offset -= MAXOFFSETU;
	    while (offset >= 255) {
	      *out++ = 255;
	      offset -= 255;
	    }
	    *out++ = offset;
	  }
	  if (temp == MAXCOUNTU) {
	    temp = dissimilar - MAXCOUNTU;
	    while (temp >= 255) {
	      *out++ = 255;
	      temp -= 255;
	    }
	    *out++ = (byte) temp;
	  }
	  for (i = 0; i <= dissimilar; i++)
	    *out++ = *compr++;
	  offset = 0;
	}			/* end uncompressed */
#define MAXOFFSETC 3
#define MAXCOUNTC 31
	/* output 'similar' bytes, run-length encoded */
	if ((similar = next - diff)) {
	  int temp;

	  if ((temp = (similar -= 2)) > MAXCOUNTC)
	    temp = MAXCOUNTC;
	  if (offset < MAXOFFSETC)
	    *out++ = 0x80 | (offset << 5) | (byte) temp;
	  else {
	    *out++ = 0x80 | (MAXOFFSETC << 5) | (byte) temp;
	    offset -= MAXOFFSETC;
	    while (offset >= 255) {
	      *out++ = 255;
	      offset -= 255;
	    }
	    *out++ = offset;
	  }
	  if (temp == MAXCOUNTC) {
	    temp = similar - MAXCOUNTC;
	    while (temp >= 255) {
	      *out++ = 255;
	      temp -= 255;
	    }
	    *out++ = (byte) temp;
	  }
	  *out++ = value;
	  offset = 0;
	}			/* end compressed */
	diff = next;
      }
    }
  }
  return out - compressed;
}

/* Map a r-g-b color to a color index. */
/* We complement the colours, since we're using cmy anyway, */
/* and because the buffering routines expect white to be zero. */
private gx_color_index
gdev_pcl_true_map_rgb_color(gx_device * dev, gx_color_value r,
			    gx_color_value g, gx_color_value b)
{
  if ((r & g & b) == 0xff)
    return (gx_color_index)0;   /* white */
  else
    return (gx_color_value_to_byte(r) +
	    ((uint) gx_color_value_to_byte(g) << 8) +
	    ((ulong) gx_color_value_to_byte(b) << 16)) ^ 0xffffff;
}

/* Map a color index to a r-g-b color. */
/* Includes colour balancing, following HP recommendations, to try */
/* and correct the greenish cast resulting from an equal mix of the */
/* c, m, y, inks to give a truer black. */
/* Someday we can add gamma-lookup tables as well */
private int
gdev_pcl_true_map_color_rgb(gx_device * dev, gx_color_index color,
			    gx_color_value prgb[3])
{
  /* NB. We actually have cmy colours, from the way we set up the */
  /* mapping rgb_color */
  int c = (int)(color >> 16);
  int m = (int)((color >> 8) & 0xff);
  int y = (int)(color & 0xff);
  if ((c & m & y) == 0)    /* white */
    return 0;
  else {
    int maxval, minval, range;

    maxval = c >= m ? c : m;
    maxval = maxval >= y ? maxval : y;
    if (maxval > 0) {
      minval = c <= m ? c : m;
      minval = minval <= y ? minval : y;
      range = maxval - minval;

#define CORRECTION 4  /* ie. 4/5 reduction in cyan to get true black */
      c = ((long)(c << 1) + (range + (long)(maxval * CORRECTION)) + 1) /
	(long)(maxval * ((CORRECTION + 1) << 1));
      
      prgb[0] = (gx_color_value)c;
      prgb[1] = (gx_color_value)m;
      prgb[2] = (gx_color_value)y;
    }
  }
  return 0;
}

#undef t_margin
#undef b_margin
#undef height
