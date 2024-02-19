/* Copyright (C) 1989, 1990, 1991, 1992 Aladdin Enterprises.  All rights reserved.
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

/* gdevnp6.c */
/* NEC p6+ dot-matrix printer driver for Ghostscript */
#include "gdevprn.h"
/************************************************
 * This driver is only for the (360 * 360) dpi resolution.
 * For all other resolutions of the NEC p6+ use the epson-driver.
 * I do not know if this driver works correctly on a NEC p6 or a p60.
 * Report me if it works even on a p6 and a p60, please. Also send me
 * bug reports.
 * Most of the code is from the epson-driver. I just changed it a little bit
 * to print 360 dpi vertical. Also I deleted the 9-pin printer code that is
 * of no use here.
 *                                      Marcus Haebler, 10.08.92
 * email: haebler@dmswwu1a.uni-muenster.de
 ************************************************/

#ifndef X_DPI
#  define X_DPI 360		/* pixels per inch */
#endif
#ifndef Y_DPI
#  define Y_DPI 360		/* pixels per inch */
#endif

/* The device descriptors */
private dev_proc_print_page (necp6_print_page);
gx_device_printer gs_necp6_device =
prn_device (prn_std_procs, "necp6",
	    85,			/* width_10ths, 8.5" */
	    110,		/* height_10ths, 11" */
	    X_DPI, Y_DPI,
	    0, 0, 0.5, 0,	/* margins */
	    1, necp6_print_page);

/* ------ Internal routines ------ */

/* Forward references */
private void necp6_output_run (P4 (byte *, int, FILE *, int));

/* Send the page to the printer. */
private int
necp6_print_page (gx_device_printer * pdev, FILE * prn_stream)
{
  int line_size = gdev_mem_bytes_per_scan_line ((gx_device *) pdev);
  int in_size = line_size * 24;	/* we need 24 lines */
  byte *in = (byte *) gs_malloc (in_size, 1, "necp6_print_page(in)");
  int out_size = ((pdev->width + 7) & -8) * 3;
  byte *out = (byte *) gs_malloc (out_size, 1, "necp6_print_page(out)");
  int skip = 0, lnum = 0, pass;

  /* Check allocations */
  if (in == 0 || out == 0)
    {
      if (in)
	gs_free ((char *) in, in_size, 1, "necp6_print_page(in)");
      if (out)
	gs_free ((char *) out, out_size, 1, "necp6_print_page(out)");
      return -1;
    }

  /* Initialize the printer and reset the margins. */
  fwrite ("\033@\033P\033l\000\r\033Q", 1, 10, prn_stream);
  fputc ((int) (pdev->width / pdev->x_pixels_per_inch * 10) + 2,
	 prn_stream);

  /* Print lines of graphics */
  while (lnum < pdev->height)
    {
      byte *inp = in;
      byte *in_end = in + line_size;
      byte *out_end = out;
      byte *out_blk;
      register byte *outp;
      int lcnt, lcnt2, offset_;

      /* Copy 1 scan line and test for all zero. */
      gdev_prn_copy_scan_lines (pdev, lnum, in, line_size);
      if (in[0] == 0 &&
	  !memcmp ((char *) in, (char *) in + 1, line_size - 1)
	)
	{
	  lnum++;
	  skip++;
	  continue;
	}

      /* Vertical tab to the appropriate position. */
      while (skip > 255)
	{
	  fprintf (prn_stream, "\0343\377\012");
	  skip -= 255;
	}
      if (skip)
	fprintf (prn_stream, "\0343%c\012", skip);

      /* Copy the rest of the scan lines. */
      for (offset_ = 0; offset_ <= 1; offset_++)
	{
	  lcnt = 1;
	  for (lcnt2 = offset_; lcnt2 <= 45; lcnt2 += 2)
	    lcnt += gdev_prn_copy_scan_lines (pdev, lnum + 2 + lcnt2,
					   in + line_size * (lcnt2 / 2 + 1),
					      line_size);
	  if (lcnt < 24)
	    memset (in + lcnt * line_size, 0,
		    in_size - lcnt * line_size);

	  /* We have to 'transpose' blocks of 8 pixels x 8 lines, */
	  /* because that's how the printer wants the data. */
	  /* And because we are in a 24-pin mode, we have to */
	  /* transpose groups of 3 lines at a time. */

	  for (; inp < in_end; inp++, out_end += 24)
	    {
	      gdev_prn_transpose_8x8 (inp, line_size, out_end, 3);
	      gdev_prn_transpose_8x8 (inp + line_size * 8, line_size, out_end + 1, 3);
	      gdev_prn_transpose_8x8 (inp + line_size * 16, line_size, out_end + 2, 3);
	    }
	  /* Remove trailing 0s. */
	  while (out_end > out && out_end[-1] == 0 &&
		 out_end[-2] == 0 && out_end[-3] == 0
	    )
	    out_end -= 3;

	  for (pass = 1; pass <= 2; pass++)
	    {
	      for (out_blk = outp = out; outp < out_end;)
		{		/* Skip a run of leading 0s. */
		  /* At least 10 are needed to make tabbing worth it. */
		  /* We do everything by 3's because we have 24 pins */

		  if (*outp == 0 && outp + 12 <= out_end &&
		      outp[1] == 0 && outp[2] == 0 &&
		      (outp[3] | outp[4] | outp[5]) == 0 &&
		      (outp[6] | outp[7] | outp[8]) == 0 &&
		      (outp[9] | outp[10] | outp[11]) == 0
		    )
		    {
		      byte *zp = outp;
		      int tpos;
		      byte *newp;
		      outp += 12;
		      while (outp + 3 <= out_end && *outp == 0 &&
			     outp[1] == 0 && outp[2] == 0
			)
			outp += 3;
		      tpos = (outp - out) / 108;
		      newp = out + tpos * 108;
		      if (newp > zp + 10)
			{	/* Output preceding bit data. */
			  if (zp > out_blk)	/* only false at */
			    /* beginning of line */
			    necp6_output_run (out_blk, (int) (zp - out_blk),
					    prn_stream, pass);
			  /* Tab over to the appropriate position. */
			  fprintf (prn_stream, "\033D%c%c\t", tpos, 0);
			  out_blk = outp = newp;
			}
		    }
		  else
		    outp += 3;
		}
	      if (outp > out_blk)
		necp6_output_run (out_blk, (int) (outp - out_blk),
				prn_stream, pass);

	      fputc ('\r', prn_stream);
	    }
	  if (offset_ == 0)
	    {
	      fprintf (prn_stream, "\0343\001\012");	/* 1/360 inch */
	      gdev_prn_copy_scan_lines (pdev, (1 + lnum), in, line_size);
	      memset (in + line_size, 0, in_size - line_size);
	      inp = in;
	      in_end = in + line_size;
	      out_end = out;
	    }
	}
      skip = 47;
      lnum += 48;
    }

  /* Eject the page and reinitialize the printer */
  fputs ("\f\033@", prn_stream);
  fflush (prn_stream);

  gs_free ((char *) out, out_size, 1, "necp6_print_page(out)");
  gs_free ((char *) in, in_size, 1, "necp6_print_page(in)");
  return 0;
}

/* Output a single graphics command. */
/* pass=0 for all columns, 1 for even columns, 2 for odd columns. */
private void
necp6_output_run (byte * data, int count, FILE * prn_stream, int pass)
{
  int xcount = count / 3;
  fprintf (prn_stream, "\033*\050%c%c", xcount & 0xff, xcount >> 8);
  if (!pass)
    fwrite (data, 1, count, prn_stream);
  else
    {				/* Only write every other column of 3 bytes. */
      int which = pass;
      byte *dp = data;
      register int i, j;
      for (i = 0; i < xcount; i++, which++)
	for (j = 0; j < 3; j++, dp++)
	  {
	    putc (((which & 1) ? *dp : 0), prn_stream);
	  }
    }
}
