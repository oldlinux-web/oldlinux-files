/* Copyright (C) 1989, 1992 Aladdin Enterprises.  All rights reserved.
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

/* gdevdjet.c */
/* HP LaserJet/DeskJet driver for Ghostscript */
#include "gdevprn.h"
#include "gdevpcl.h"

/*
 * Thanks to Jim Mayer (mayer@wrc.xerox.com),
 * Jan-Mark Wams (jms@cs.vu.nl), Frans van Hoesel (hoesel@rugr86.rug.nl),
 * and George Cameron (g.cameron@biomed.abdn.ac.uk) for improvements.
 */

/*
 * You may select a resolution of 75, 100, 150, or 300 DPI.
 * Normally you would do this in the makefile or on the gs command line,
 * not here.
 *
 * If the preprocessor symbol A4 is defined, the default paper size is
 * the European A4 size; otherwise it is the U.S. letter size (8.5"x11").
 */
/*#define X_DPI 300*/
/*#define Y_DPI 300*/

#define X_DPI_MAX 300
#define Y_DPI_MAX 300

#ifndef X_DPI
#  define X_DPI X_DPI_MAX
#endif
#ifndef Y_DPI
#  define Y_DPI Y_DPI_MAX
#endif

/*
 * For all DeskJet Printers:
 *
 *  Maximum printing width               = 2400 dots = 8"
 *  Maximum recommended printing height  = 3100 dots = 10 1/3"
 *
 * All Deskjets have 1/2" unprintable bottom margin.
 * The recommendation comes from the HP Software Developer's Guide for
 * the DeskJet 500, DeskJet PLUS, and DeskJet printers, version C.01.00
 * of 12/1/90.
 *
 * Note that the margins defined just below here apply only to the DeskJet;
 * the paper size, width and height apply to the LaserJet as well.
 */

/* Margins are left, bottom, right, top. */
/* DeskJet values from Jim Mayer mayer@wrc.xerox.com. */
#define DESKJET_MARGINS_LETTER  0.25, 0.597, 0.25, 0.07
#define DESKJET_MARGINS_A4      0.15, 0.597, 0.15, 0.07
/* Similar margins for the LaserJet, */
/* from Eddy Andrews eeandrew@pyr.swan.ac.uk. */
#define LASERJET_MARGINS_A4	0.25, 0.20, 0.25, 0.00
#define LASERJET_MARGINS_LETTER	0.35, 0.20, 0.35, 0.00

#ifndef A4
#  define WIDTH_10THS		85
#  define HEIGHT_10THS		110
#  define DESKJET_MARGINS	DESKJET_MARGINS_LETTER
#  define LASERJET_MARGINS	LASERJET_MARGINS_LETTER
#else
#  define WIDTH_10THS		83	/* 210mm */
#  define HEIGHT_10THS		117	/* 297mm */
#  define DESKJET_MARGINS	DESKJET_MARGINS_A4
#  define LASERJET_MARGINS	LASERJET_MARGINS_A4
#endif

/* The number of blank lines that make it worthwhile to reposition */
/* the cursor. */
#define MIN_SKIP_LINES 7

/* We round up the LINE_SIZE to a multiple of a ulong for faster scanning. */
#define W sizeof(word)
#define LINE_SIZE ((X_DPI_MAX * 85 / 10 + W * 8 - 1) / (W * 8) * W)

/* Printer types */
#define LJ	0
#define LJplus	1
#define LJ2p	2
#define LJ3	3
#define DJ	4
#define DJ500	5

/* Printer compression capabilities */
typedef enum {
	mode_0,
	mode_2,
	mode_3		/* includes mode 2 */
} compression_modes;

/* The printer initialization strings. */
private const char *init_strings[] = {
	/* LaserJet PCL 3, no compression */
		"\033*p0x0Y\033*b0M",
	/* LaserJet Plus PCL 3, no compression */
		"\033*p0x0Y\033*b0M",
	/* LaserJet IIP PCL 4, mode 2 compression */
		"\033*r0F\033*p0x75Y\033*b2M",
	/* LaserJet III PCL 5, mode 2&3 compression */
		"\033*r0F\033*p0x75Y",
	/* DeskJet almost PCL 4, mode 2 compression */
		"\033&k1W\033*p0x37Y\033*b2M",
	/* DeskJet 500 almost PCL 4, mode 2&3 compression */
		"\033&k1W\033*p0x37Y",
};

/* The device descriptors */
private dev_proc_open_device(hpjet_open);
private dev_proc_print_page(djet_print_page);
private dev_proc_print_page(djet500_print_page);
private dev_proc_print_page(ljet_print_page);
private dev_proc_print_page(ljetplus_print_page);
private dev_proc_print_page(ljet2p_print_page);
private dev_proc_print_page(ljet3_print_page);

gx_device_procs prn_hp_procs =
  prn_matrix_procs(hpjet_open, gdev_pcl_get_initial_matrix,
    gdev_prn_output_page, gdev_prn_close);

gx_device_printer gs_deskjet_device =
  prn_device(prn_hp_procs, "deskjet",
	WIDTH_10THS, HEIGHT_10THS,
	X_DPI, Y_DPI,
	0, 0, 0, 0,		/* margins filled in by hpjet_open */
	1, djet_print_page);

gx_device_printer gs_djet500_device =
  prn_device(prn_hp_procs, "djet500",
	WIDTH_10THS, HEIGHT_10THS,
	X_DPI, Y_DPI,
	0, 0, 0, 0,		/* margins filled in by hpjet_open */
	1, djet500_print_page);

gx_device_printer gs_laserjet_device =
  prn_device(prn_hp_procs, "laserjet",
	WIDTH_10THS, HEIGHT_10THS,
	X_DPI, Y_DPI,
	0.05, 0.25, 0.55, 0.25,		/* margins */
	1, ljet_print_page);

gx_device_printer gs_ljetplus_device =
  prn_device(prn_hp_procs, "ljetplus",
	WIDTH_10THS, HEIGHT_10THS,
	X_DPI, Y_DPI,
	0.05, 0.25, 0.55, 0.25,		/* margins */
	1, ljetplus_print_page);

gx_device_printer gs_ljet2p_device =
  prn_device(prn_hp_procs, "ljet2p",
	WIDTH_10THS, HEIGHT_10THS,
	X_DPI, Y_DPI,
	0.20, 0.25, 0.25, 0.25,		/* margins */
	1, ljet2p_print_page);

gx_device_printer gs_ljet3_device =
  prn_device(prn_hp_procs, "ljet3",
	WIDTH_10THS, HEIGHT_10THS,
	X_DPI, Y_DPI,
	0.20, 0.25, 0.25, 0.25,		/* margins */
	1, ljet3_print_page);

/* Forward references */
private int hpjet_print_page(P4(gx_device_printer *, FILE *, int, compression_modes));

/* Open the printer, adjusting the margins if necessary. */
private int
hpjet_open(gx_device *pdev)
{	/* Change the margins if necessary. */
	const float _ds *m;
#define ppdev ((gx_device_printer *)pdev)
	if ( ppdev->print_page == djet_print_page ||
	     ppdev->print_page == djet500_print_page
	   )
#undef ppdev
	{	static const float m_a4[4] = { DESKJET_MARGINS_A4 };
		static const float m_letter[4] = { DESKJET_MARGINS_LETTER };
		m = (gdev_pcl_paper_size(pdev) == PAPER_SIZE_A4 ? m_a4 :
			m_letter);
	}
	else	/* LaserJet */
	{	static const float m_a4[4] = { LASERJET_MARGINS_A4 };
		static const float m_letter[4] = { LASERJET_MARGINS_LETTER };
		m = (gdev_pcl_paper_size(pdev) == PAPER_SIZE_A4 ? m_a4 :
			m_letter);
	}
	pdev->l_margin = m[0];
	pdev->b_margin = m[1];
	pdev->r_margin = m[2];
	pdev->t_margin = m[3];
	return gdev_prn_open(pdev);
}

/* ------ Internal routines ------ */

/* The DeskJet can compress (mode 2) */
private int
djet_print_page(gx_device_printer *pdev, FILE *prn_stream)
{	return hpjet_print_page(pdev, prn_stream, DJ, mode_2);
}
/* The DeskJet500 can compress (modes 2&3) */
private int
djet500_print_page(gx_device_printer *pdev, FILE *prn_stream)
{	return hpjet_print_page(pdev, prn_stream, DJ500, mode_3);
}
/* The LaserJet series II can't compress */
private int
ljet_print_page(gx_device_printer *pdev, FILE *prn_stream)
{	return hpjet_print_page(pdev, prn_stream, LJ, mode_0);
}
/* The LaserJet Plus can't compress */
private int
ljetplus_print_page(gx_device_printer *pdev, FILE *prn_stream)
{	return hpjet_print_page(pdev, prn_stream, LJplus, mode_0);
}
/* All LaserJet series IIIs (III,IIId,IIIp,IIIsi) compress (modes 2&3) */
private int
ljet3_print_page(gx_device_printer *pdev, FILE *prn_stream)
{	return hpjet_print_page(pdev, prn_stream, LJ3, mode_3);
}
/* LaserJet series IIp & IId compress (mode 2) */
private int
ljet2p_print_page(gx_device_printer *pdev, FILE *prn_stream)
{	return hpjet_print_page(pdev, prn_stream, LJ2p, mode_2);
}

/* Send the page to the printer.  For speed, compress each scan line, */
/* since computer-to-printer communication time is often a bottleneck. */
private int
hpjet_print_page(gx_device_printer *pdev, FILE *prn_stream, int ptype,
  compression_modes cmodes)
{	int line_size = gdev_mem_bytes_per_scan_line((gx_device *)pdev);
	int line_size_words = (line_size + W - 1) / W;
	uint storage_size_words = line_size_words * 8; /* data, out_row, out_row_alt, prev_row */
	word *storage = (ulong *)gs_malloc(storage_size_words, W,
					   "hpjet_print_page");
	word
	  *data_words,
	  *out_row_words,
	  *out_row_alt_words,
	  *prev_row_words;
#define data ((char *)data_words)
#define out_row ((char *)out_row_words)
#define out_row_alt ((char *)out_row_alt_words)
#define prev_row ((char *)prev_row_words)
	char *out_data;
	int x_dpi = pdev->x_pixels_per_inch;
	int y_dots_per_pixel = Y_DPI_MAX / pdev->y_pixels_per_inch;
	int out_count;
	int compression = -1;
	static const char *from2to3 = "\033*b3M";
	static const char *from3to2 = "\033*b2M";
	int penalty_from2to3 = strlen(from2to3);
	int penalty_from3to2 = strlen(from3to2);
	int paper_size = gdev_pcl_paper_size((gx_device *)pdev);

	if ( storage == 0 )	/* can't allocate working area */
		return_error(gs_error_VMerror);
	data_words = storage;
	out_row_words = data_words + (line_size_words * 2);
	out_row_alt_words = out_row_words + (line_size_words * 2);
	prev_row_words = out_row_alt_words + (line_size_words * 2);
	/* Clear temp storage */
	memset(data, 0, storage_size_words * W);

	/* Initialize printer. */
	fputs("\033E", prn_stream);		/* reset printer */
	fputs("\033*rB", prn_stream);		/* end raster graphics */
	fprintf(prn_stream, "\033*t%dR", x_dpi);	/* set resolution */
	/* If the printer supports it, set the paper size */
	/* based on the actual requested size. */
	if ( !(ptype == LJ || ptype == LJplus) )
	{	fprintf(prn_stream, "\033&l%dA", paper_size);
	}
	fputs("\033&l0o0e0L", prn_stream);
	fputs(init_strings[ptype], prn_stream);

	/* Send each scan line in turn */
	   {	int lnum;
		int num_blank_lines = 0;
		word rmask = ~(word)0 << (-pdev->width & (W * 8 - 1));

		/* Transfer raster graphics. */
		for ( lnum = 0; lnum < pdev->height; lnum++ )
		   {	register word *end_data = data_words + line_size_words;
			gdev_prn_copy_scan_lines(pdev, lnum,
						 (byte *)data, line_size);
		   	/* Mask off 1-bits beyond the line width. */
			end_data[-1] &= rmask;
			/* Remove trailing 0s. */
			while ( end_data > data_words && end_data[-1] == 0 )
			  end_data--;
			if ( end_data == data_words )
			   {	/* Blank line */
				num_blank_lines++;
				continue;
			   }

			/* We've reached a non-blank line. */
			/* Put out a spacing command if necessary. */
			if ( num_blank_lines == lnum )
			{	/* We're at the top of a page. */
				if ( num_blank_lines > 0 )
				{	/* move down from current position */
					fprintf(prn_stream, "\033*p+%dY",
						num_blank_lines * y_dots_per_pixel);
				}
				/* Start raster graphics. */
				fputs("\033*r1A", prn_stream);
			}
			/* Skip blank lines if any */
			else if ( num_blank_lines != 0 )
			{  if ( num_blank_lines < MIN_SKIP_LINES )
			   {	/* Moving down from current position */
				/* causes head motion on the DeskJet, so */
				/* if the number of lines is small, */
				/* we're better off printing blanks. */
				if ( cmodes == mode_3 )
				{	/* Must clear the seed row. */
					fputs("\033*bY", prn_stream);
				}
				for ( ; num_blank_lines; num_blank_lines-- )
					fputs("\033*bW", prn_stream);
			   }
			   else if ( cmodes == mode_0 )	/* PCL 3 */
			   {	fprintf(prn_stream, "\033*p+%dY",
				        num_blank_lines * y_dots_per_pixel);
			   }
			   else
			   {	   fprintf(prn_stream, "\033*b%dY",
					   num_blank_lines);
			   }
			   /* Clear the seed row (only matters for */
			   /* mode 3 compression). */
			   memset(prev_row, 0, line_size);
			}
			num_blank_lines = 0;

			/* Choose the best compression mode */
			/* for this particular line. */
			switch (cmodes)
			  {
			  case mode_3:
			   {	/* Compression modes 2 and 3 are both */
				/* available.  Try both and see which one */
				/* produces the least output data. */
				int count3 = gdev_pcl_mode3compress(line_size, data,
							   prev_row, out_row);
				int count2 = gdev_pcl_mode2compress(data_words, end_data,
							   out_row_alt);
				int penalty3 =
				  (compression == 3 ? 0 : penalty_from2to3);
				int penalty2 =
				  (compression == 2 ? 0 : penalty_from3to2);
				if ( count3 + penalty3 < count2 + penalty2)
				   {	if ( compression != 3 )
					    fputs(from2to3, prn_stream);
					compression = 3;
					out_data = out_row;
					out_count = count3;
				   }
				else
				   {	if ( compression != 2 )
					    fputs(from3to2, prn_stream);
					compression = 2;
					out_data = out_row_alt;
					out_count = count2;
				   }
				break;
			   }
			  case mode_2:
				out_data = out_row;
			   	out_count = gdev_pcl_mode2compress(data_words, end_data,
							  out_row);
				break;
			  default:
				out_data = data;
				out_count = (char *)end_data - data;
			  }

			/* Transfer the data */
			fprintf(prn_stream, "\033*b%dW", out_count);
			fwrite(out_data, sizeof(char), out_count,
			       prn_stream);
		   }
	}

	/* end raster graphics */
	fputs("\033*rB", prn_stream);

	/* eject page */
	fputs("\033&l0H", prn_stream);

	/* free temporary storage */
	gs_free((char *)storage, storage_size_words, W, "hpjet_print_page");

	return 0;
}
