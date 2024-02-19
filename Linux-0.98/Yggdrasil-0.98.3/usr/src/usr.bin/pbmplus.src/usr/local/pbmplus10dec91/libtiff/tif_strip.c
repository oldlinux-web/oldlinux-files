#ifndef lint
static char rcsid[] = "$Header: /usr/people/sam/tiff/libtiff/RCS/tif_strip.c,v 1.3 91/08/19 14:40:47 sam Exp $";
#endif

/*
 * Copyright (c) 1991 Sam Leffler
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * TIFF Library.
 *
 * Strip-organized Image Support Routines.
 */
#include "tiffioP.h"

/*
 * Compute which strip a (row,sample) value is in.
 */
u_int
TIFFComputeStrip(tif, row, sample)
	TIFF *tif;
	u_long row;
	u_int sample;
{
	TIFFDirectory *td = &tif->tif_dir;
	u_int strip;

	strip = row / td->td_rowsperstrip;
	if (td->td_planarconfig == PLANARCONFIG_SEPARATE) {
		if (sample >= td->td_samplesperpixel) {
			TIFFError(tif->tif_name,
			    "%d: Sample out of range, max %d",
			    sample, td->td_samplesperpixel);
			return (0);
		}
		strip += sample*td->td_stripsperimage;
	}
	return (strip);
}

/*
 * Compute how many strips are in an image.
 */
u_int
TIFFNumberOfStrips(tif)
	TIFF *tif;
{
	TIFFDirectory *td = &tif->tif_dir;

	return (td->td_rowsperstrip == 0xffffffff ?
	     (td->td_imagelength != 0 ? 1 : 0) :
	     howmany(td->td_imagelength, td->td_rowsperstrip));
}

/*
 * Compute the # bytes in a (row-aligned) strip.
 */
u_long
TIFFStripSize(tif)
	TIFF *tif;
{
	TIFFDirectory *td = &tif->tif_dir;
	u_long rowsperstrip = td->td_rowsperstrip;
	
	if (rowsperstrip == (u_long)-1)
		rowsperstrip = td->td_imagelength;
	return (rowsperstrip * TIFFScanlineSize(tif));
}
