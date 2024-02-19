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

/* gdevmswn.c */
/*
 * Microsoft Windows 3.n driver for Ghostscript.
 * Original version by Russell Lang and Maurice Castro with help from
 * Programming Windows, 2nd Ed., Charles Petzold, Microsoft Press;
 * created from gdevbgi.c and gnuplot/term/win.trm 5th June 1992.
 * Extensively modified by L. Peter Deutsch, Aladdin Enterprises.
 */
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include "gx.h"
#include "gserrors.h"
#include "gxdevice.h"
#include "gp.h"
#include "gpcheck.h"

/* system menu constants for image window */
#define M_COPY_CLIP 1

/* externals from gp_win.c */
extern HWND FAR hwndeasy;
extern HANDLE FAR phInstance;
extern const char FAR szAppName[];
extern gx_device *gdev_win_open_list;

typedef struct gx_device_win_s gx_device_win;

/* Forward references */
private LPLOGPALETTE near win_makepalette(P1(gx_device_win *));
private void near win_makeimg(P1(gx_device_win *));
private void near win_addtool(P2(gx_device_win *, int));
private void near win_maketools(P2(gx_device_win *, HDC));
private void near win_destroytools(P1(gx_device_win *));
private void near win_nomemory();
private void near win_update(P2(gx_device_win *, int));
private const char FAR imgname[] = "Ghostscript Image";

/* See gxdevice.h for the definitions of the procedures. */

private dev_proc_open_device(win_open);
private dev_proc_sync_output(win_sync_output);
private dev_proc_output_page(win_output_page);
private dev_proc_close_device(win_close);
private dev_proc_map_rgb_color(win_map_rgb_color);
private dev_proc_map_color_rgb(win_map_color_rgb);
private dev_proc_fill_rectangle(win_fill_rectangle);
private dev_proc_tile_rectangle(win_tile_rectangle);
private dev_proc_copy_mono(win_copy_mono);
private dev_proc_copy_color(win_copy_color);
private dev_proc_draw_line(win_draw_line);

/* The device descriptor */
struct gx_device_win_s {
	gx_device_common;
	int nColors;

	/* Handles */

	HWND FAR hwndimg;
	HBITMAP FAR hbitmap;
	HDC FAR hdcbit;
	HPEN hpen, *hpens;
	HBRUSH hbrush, *hbrushs;
	HPALETTE hpalette, himgpalette;
	LPLOGPALETTE lpalette, limgpalette;

	/* A staging bitmap for copy_mono. */
	/* We want one big enough to handle the standard 16x16 halftone; */
	/* this is also big enough for ordinary-size characters. */

#define bmWidthBytes 4		/* must be even */
#define bmWidthBits (bmWidthBytes * 8)
#define bmHeight 16
	HBITMAP FAR hbmmono;
	HDC FAR hdcmono;
#define rop_write_at_1s 0xE20746L	/* write brush at 1's */
#define rop_write_at_0s 0xB8074AL	/* write brush at 0's */
	gx_bitmap_id bm_id;

	/* Window scrolling stuff */

	int cxClient, cyClient;
	int cxAdjust, cyAdjust;
	int nVscrollPos, nVscrollMax;
	int nHscrollPos, nHscrollMax;

	/* Link for the list of open Windows devices */

	gx_device_win *next_wdev;
};
#define wdev ((gx_device_win *)dev)
private gx_device_procs win_procs = {
	win_open,
	gx_default_get_initial_matrix,
	win_sync_output,
	win_output_page,
	win_close,
	win_map_rgb_color,
	win_map_color_rgb,
	win_fill_rectangle,
	win_tile_rectangle,
	win_copy_mono,
	win_copy_color,
	win_draw_line,
	gx_default_get_bits,
	gx_default_get_props,
	gx_default_put_props
};
gx_device_win gs_mswin_device = {
	sizeof(gx_device_win),
	&win_procs,
	"win",
#define INITIAL_RESOLUTION 96
#define INITIAL_WIDTH (INITIAL_RESOLUTION * 85 / 10 + 1)
#define INITIAL_HEIGHT (INITIAL_RESOLUTION * 11 + 1)
	INITIAL_WIDTH, INITIAL_HEIGHT, 	/* win_open() fills these in later */
	INITIAL_RESOLUTION, INITIAL_RESOLUTION,	/* win_open() fills these in later */
	no_margins,
	dci_black_and_white,
	0,		/* not open yet */
	2		/* nColors = 2 */
};

/* Open the win driver */
private int
win_open(gx_device *dev)
{	HDC hdc;
	int i;
	int depth;

	win_makeimg(wdev);
	ShowWindow(wdev->hwndimg, SW_SHOWMINIMIZED);
	BringWindowToTop(hwndeasy);

	hdc = GetDC(wdev->hwndimg);

	/* Set parameters that were unknown before opening device */
	if (dev->width == INITIAL_WIDTH)
	  dev->width  = (int)(8.5  * dev->x_pixels_per_inch);
	if (dev->height == INITIAL_HEIGHT)
	  dev->height = (int)(11.0 * dev->y_pixels_per_inch);

	/* Find out if the device supports color */
	/* We recognize 2, 16 or 256 color devices */
	depth = GetDeviceCaps(hdc,PLANES) * GetDeviceCaps(hdc,BITSPIXEL);
	if ( depth >= 8 ) { /* use 64 static colors and 166 dynamic colors from 8 planes */
		static const gx_device_color_info win_256color = dci_color(8,31,4);
		dev->color_info = win_256color;
		wdev->nColors = 64;
	}
	else if ( depth >= 4 ) {
		static const gx_device_color_info win_16color = dci_color(4, 2, 3);
		dev->color_info = win_16color;
		wdev->nColors = 16;
	} 
	else {   /* default is black_and_white */
		wdev->nColors = 2;
	}
	
	wdev->hbitmap = CreateCompatibleBitmap(hdc,dev->width,dev->height);
	for (i=0; (wdev->hbitmap==(HBITMAP)NULL) && (i<4); i++) {
		fprintf(stderr,"\nNot enough memory for bitmap.  Halving resolution... ");
		dev->x_pixels_per_inch = dev->x_pixels_per_inch/2;
		dev->y_pixels_per_inch = dev->y_pixels_per_inch/2;
		dev->width  = dev->width/2;
		dev->height = dev->height/2;
		wdev->hbitmap = CreateCompatibleBitmap(hdc,dev->width,dev->height);
	}
	if (wdev->hbitmap==(HBITMAP)NULL) {
		ReleaseDC(wdev->hwndimg, hdc);
		win_nomemory();
	}

	/* Create the bitmap and DC for copy_mono. */
	wdev->hbmmono = CreateBitmap(bmWidthBits, bmHeight, 1, 1, NULL);
	wdev->hdcmono = CreateCompatibleDC(hdc);
	if ( wdev->hbmmono == NULL || wdev->hdcmono == NULL )
	{	DeleteObject(wdev->hbitmap);
		ReleaseDC(wdev->hwndimg, hdc);
		win_nomemory();
	}
	SetMapMode(wdev->hdcmono, GetMapMode(hdc));
	SelectObject(wdev->hdcmono, wdev->hbmmono);
	wdev->bm_id = gx_no_bitmap_id;
	   
	wdev->hdcbit = CreateCompatibleDC(hdc);  /* create Device Context for drawing */
	SelectObject(wdev->hdcbit,wdev->hbitmap);
	ReleaseDC(wdev->hwndimg, hdc);

	/* create palette and tools for bitmap */
	if ((wdev->lpalette = win_makepalette(wdev))
		== (LPLOGPALETTE)NULL)
		win_nomemory();
	wdev->hpalette = CreatePalette(wdev->lpalette);
	(void) SelectPalette(wdev->hdcbit,wdev->hpalette,NULL);
	RealizePalette(wdev->hdcbit);
	win_maketools(wdev,wdev->hdcbit);

	/* create palette for display */
	if ((wdev->limgpalette = win_makepalette(wdev))
		== (LPLOGPALETTE)NULL)
		win_nomemory();
	wdev->himgpalette = CreatePalette(wdev->limgpalette);

	/* Initialize the scrolling information. */
	
	wdev->cxClient = wdev->cyClient = 0;
	wdev->nVscrollPos = wdev->nVscrollMax = 0;
	wdev->nHscrollPos = wdev->nHscrollMax = 0;

	/* Link this device onto the list */
	wdev->next_wdev = (gx_device_win *)gdev_win_open_list;
	gdev_win_open_list = dev;	

	return 0;
}

/* Make the output appear on the screen. */
private int
win_sync_output(gx_device *dev)
{
    RECT rect;
	if ( !IsWindow(wdev->hwndimg) ) {  /* some clod closed the window */
		win_makeimg(wdev);
		ShowWindow(wdev->hwndimg, SW_SHOWMINIMIZED);
	}
	if ( !IsIconic(wdev->hwndimg) ) {  /* redraw window */
		GetClientRect(wdev->hwndimg, &rect);
		InvalidateRect(wdev->hwndimg, (LPRECT) &rect, 1);
		UpdateWindow(wdev->hwndimg);
	}
	return(0);
}

/* Make the window visible, and display the output. */
private int
win_output_page(gx_device *dev, int copies, int flush)
{
	if (IsIconic(wdev->hwndimg))    /* useless as an Icon so fix it */
		ShowWindow(wdev->hwndimg, SW_SHOWNORMAL);

	BringWindowToTop(wdev->hwndimg);
	return( win_sync_output(dev) );
}

/* Close the win driver */
private int
win_close(gx_device *dev)
{
	/* Free resources */

	win_destroytools(wdev);
	DeleteDC(wdev->hdcbit);
	DeleteDC(wdev->hdcmono);
	DeleteObject(wdev->hbitmap);
	DeleteObject(wdev->hpalette);
	DeleteObject(wdev->himgpalette);
	DeleteObject(wdev->hbmmono);
	free(wdev->lpalette);
	free(wdev->limgpalette);
	DestroyWindow(wdev->hwndimg);
	gp_check_interrupts();	/* process WIN_DESTROY message */

	/* Unlink from list. We do this last so that the WndProc */
	/* can process the destroy message. */

	{	gx_device_win **pwd = (gx_device_win **)&gdev_win_open_list;
		while ( *pwd != 0 && *pwd != wdev )
			pwd = &(*pwd)->next_wdev;
		if ( pwd != 0 )		/* == 0 should never happen! */
			*pwd = (*pwd)->next_wdev;
	}

	return(0);
}

/* Map a r-g-b color to the colors available under Windows */
#define win_map32(z)\
  ((((z) >> (gx_color_value_bits - 5)) << 3) +\
   ((z) >> (gx_color_value_bits - 3)))
private gx_color_index
win_map_rgb_color(gx_device *dev, gx_color_value r, gx_color_value g,
  gx_color_value b)
{
	switch(dev->color_info.depth) {
	  case 8: {
		int i;
		LPLOGPALETTE lpal = wdev->lpalette;
		PALETTEENTRY *pep;
		byte cr, cg, cb;

		/* map colors to 0->255 in 32 steps */
		cr = win_map32(r);
		cg = win_map32(g);
		cb = win_map32(b);

		/* search in palette */
		for ( i = 0, pep = &lpal->palPalEntry[i];
		      i < wdev->nColors; i++, pep++
		    )
		{	if ((cr == pep->peRed) &&
			    (cg == pep->peGreen) &&
			    (cb == pep->peBlue))
				return((gx_color_index)i);	/* found it */
		}
		
		/* next try adding it to palette */
		if (i < 220) { /* allow 36 for windows and other apps */
			LPLOGPALETTE lipal = wdev->limgpalette;
			wdev->nColors = i+1;
			DeleteObject(wdev->hpalette);
	  		lpal->palPalEntry[i].peFlags = NULL;
			lpal->palPalEntry[i].peRed   =  cr;
			lpal->palPalEntry[i].peGreen =  cg;
			lpal->palPalEntry[i].peBlue  =  cb;
			lpal->palNumEntries = wdev->nColors;
			wdev->hpalette = CreatePalette(lpal);
			(void) SelectPalette(wdev->hdcbit,wdev->hpalette,NULL);
			RealizePalette(wdev->hdcbit);
			win_addtool(wdev, i);

			DeleteObject(wdev->himgpalette);
	 		lipal->palPalEntry[i].peFlags = NULL;
			lipal->palPalEntry[i].peRed   =  cr;
			lipal->palPalEntry[i].peGreen =  cg;
			lipal->palPalEntry[i].peBlue  =  cb;
			lipal->palNumEntries = wdev->nColors;
			wdev->himgpalette = CreatePalette(lipal);

			return((gx_color_index)i);	/* return new palette index */
		}

		return(-1);	/* not found - dither instead */
		}
	  case 4: {
		int color =  ((r > gx_max_color_value / 4 ? 4 : 0) +
			 (g > gx_max_color_value / 4 ? 2 : 0) +
			 (b > gx_max_color_value / 4 ? 1 : 0) +
	 		 (r > gx_max_color_value / 4 * 3 ||
			  g > gx_max_color_value / 4 * 3 ? 8 : 0));
		return((gx_color_index)color);
		}
	}
	return (gx_default_map_rgb_color(dev,r,g,b));
}

/* Map a color code to r-g-b. */
private int
win_map_color_rgb(gx_device *dev, gx_color_index color,
  gx_color_value prgb[3])
{
gx_color_value one;
	switch(dev->color_info.depth) {
	  case 8:
		one = (gx_color_value) (gx_max_color_value / 255);
		prgb[0] = wdev->lpalette->palPalEntry[(int)color].peRed * one;
		prgb[1] = wdev->lpalette->palPalEntry[(int)color].peGreen * one;
		prgb[2] = wdev->lpalette->palPalEntry[(int)color].peBlue * one;
		break;
	  case 4:
		one = ((int)color & 8 ? gx_max_color_value : gx_max_color_value / 2);
		prgb[0] = ((int)color & 4 ? one : 0);
		prgb[1] = ((int)color & 2 ? one : 0);
		prgb[2] = ((int)color & 1 ? one : 0);
		break;
	  default:
		prgb[0] = prgb[1] = prgb[2] = 
			(int)color ? gx_max_color_value : 0;
	}
	return 0;
}


/* Macro for filling a rectangle with a color. */
/* Note that it starts with a declaration. */
#define fill_rect(x, y, w, h, color)\
RECT rect;\
rect.left = x, rect.top = y;\
rect.right = x + w, rect.bottom = y + h;\
FillRect(wdev->hdcbit, &rect, wdev->hbrushs[(int)color])


/* Fill a rectangle. */
private int
win_fill_rectangle(gx_device *dev, int x, int y, int w, int h,
  gx_color_index color)
{
	fit_fill(dev, x, y, w, h);
	{	fill_rect(x, y, w, h, color);
	}
	win_update(wdev, h);
	
	return 0;
}

/* Tile a rectangle.  If neither color is transparent, */
/* pre-clear the rectangle to color0 and just tile with color1. */
/* This is faster because of how win_copy_mono is implemented. */
/* Note that this also does the right thing for colored tiles. */
private int
win_tile_rectangle(gx_device *dev, const gx_bitmap *tile,
  int x, int y, int w, int h, gx_color_index czero, gx_color_index cone,
  int px, int py)
{	fit_fill(dev, x, y, w, h);
	if ( czero != gx_no_color_index && cone != gx_no_color_index )
	   {	fill_rect(x, y, w, h, czero);
		czero = gx_no_color_index;
	   }
	if ( tile->raster == bmWidthBytes && tile->size.y <= bmHeight &&
	     (px | py) == 0 && cone != gx_no_color_index
	   )
	{	/* We can do this much more efficiently */
		/* by using the internal algorithms of copy_mono */
		/* and gx_default_tile_rectangle. */
		int width = tile->size.x;
		int height = tile->size.y;
		int rwidth = tile->rep_width;
		int irx = ((rwidth & (rwidth - 1)) == 0 ? /* power of 2 */
			x & (rwidth - 1) :
			x % rwidth);
		int ry = y % tile->rep_height;
		int icw = width - irx;
		int ch = height - ry;
		int ex = x + w, ey = y + h;
		int fex = ex - width, fey = ey - height;
		int cx, cy;

		if (wdev->hbrush != wdev->hbrushs[(int)cone])
		{	wdev->hbrush = wdev->hbrushs[(int)cone];
			SelectObject(wdev->hdcbit,wdev->hbrush);
		}

		if ( tile->id != wdev->bm_id || tile->id == gx_no_bitmap_id )
		{	wdev->bm_id = tile->id;
			SetBitmapBits(wdev->hbmmono,
				      (DWORD)(bmWidthBytes * tile->size.y),
				      (BYTE *)tile->data);
		}

#define copy_tile(srcx, srcy, tx, ty, tw, th)\
  BitBlt(wdev->hdcbit, tx, ty, tw, th, wdev->hdcmono, srcx, srcy, rop_write_at_1s)

		if ( ch > h ) ch = h;
		for ( cy = y; ; )
		   {	if ( w <= icw )
				copy_tile(irx, ry, x, cy, w, ch);
			else
			{	copy_tile(irx, ry, x, cy, icw, ch);
				cx = x + icw;
				while ( cx <= fex )
				{	copy_tile(0, ry, cx, cy, width, ch);
					cx += width;
				}
				if ( cx < ex )
				{	copy_tile(0, ry, cx, cy, ex - cx, ch);
				}
			}
			if ( (cy += ch) >= ey ) break;
			ch = (cy > fey ? ey - cy : height);
			ry = 0;
		   }

		win_update(wdev, h);
		return 0;
	}
	return gx_default_tile_rectangle(dev, tile, x, y, w, h, czero, cone, px, py);
}


/* Draw a line */
private int
win_draw_line(gx_device *dev, int x0, int y0, int x1, int y1,
  gx_color_index color)
{
	if (wdev->hpen != wdev->hpens[(int)color]) {
		wdev->hpen = wdev->hpens[(int)color];
		SelectObject(wdev->hdcbit,wdev->hpen);
	}
	MoveTo(wdev->hdcbit, x0, y0);
	LineTo(wdev->hdcbit, x1, y1);
	return 0;
}

/* Copy a monochrome bitmap.  The colors are given explicitly. */
/* Color = gx_no_color_index means transparent (no effect on the image). */
private int
win_copy_mono(gx_device *dev,
  const byte *base, int sourcex, int raster, gx_bitmap_id id,
  int x, int y, int w, int h,
  gx_color_index zero, gx_color_index one)
{	int endx;
	const byte *ptr_line;
	int width_bytes, height;
	DWORD rop = rop_write_at_1s;
	int color;
	BYTE aBit[bmWidthBytes * bmHeight];
	BYTE *aptr = aBit;

	fit_copy(dev, base, sourcex, raster, id, x, y, w, h);

	if ( sourcex & ~7 )
	{	base += sourcex >> 3;
		sourcex &= 7;
	}

	/* Break up large transfers into smaller ones. */
	while ( (endx = sourcex + w) > bmWidthBits )
	{	int lastx = (endx - 1) & -bmWidthBits;
		int subw = endx - lastx;
		int code = win_copy_mono(dev, base, lastx,
					 raster, gx_no_bitmap_id,
					 x + lastx - sourcex, y,
					 subw, h, zero, one);
		if ( code < 0 ) return code;
		w -= subw;
	}
	while ( h > bmHeight )
	{	int code;
		h -= bmHeight;
		code = win_copy_mono(dev, base + h * raster, sourcex, raster,
				     gx_no_bitmap_id, x, y + h, w, bmHeight,
				     zero, one);
		if ( code < 0 ) return code;
	}

	if ( w <= 0 || h <= 0 ) return 0;
	width_bytes = (sourcex + w + 7) >> 3;
	ptr_line = base;

	if ( zero == gx_no_color_index )
	   {	if ( one == gx_no_color_index ) return 0;
		color = (int)one;
	   }
	else
	   {	if ( one == gx_no_color_index )
		   {	color = (int)zero;
			rop = rop_write_at_0s;
		   }
		else
		   {	/* Pre-clear the rectangle to zero */
			fill_rect(x, y, w, h, zero);
			color = (int)one;
		   }
	   }

	if (wdev->hbrush != wdev->hbrushs[(int)color])
	{	wdev->hbrush = wdev->hbrushs[(int)color];
		SelectObject(wdev->hdcbit,wdev->hbrush);
	}

	if ( id != wdev->bm_id || id == gx_no_bitmap_id )
	{	wdev->bm_id = id;
		if ( raster == bmWidthBytes )
		{	/* We can do the whole thing in a single transfer! */
			SetBitmapBits(wdev->hbmmono,
				      (DWORD)(bmWidthBytes * h),
				      (BYTE *)base);
		}
		else
		{	for ( height = h; height--;
			      ptr_line += raster, aptr += bmWidthBytes
			    )
			{	/* Pack the bits into the bitmap. */
				switch ( width_bytes )
				{
					default: memcpy(aptr, ptr_line, width_bytes); break;
					case 4: aptr[3] = ptr_line[3];
					case 3: aptr[2] = ptr_line[2];
					case 2: aptr[1] = ptr_line[1];
					case 1: aptr[0] = ptr_line[0];
				}
			}
			SetBitmapBits(wdev->hbmmono,
				      (DWORD)(bmWidthBytes * h),
				      &aBit[0]);
		}
	}

	BitBlt(wdev->hdcbit, x, y, w, h, wdev->hdcmono, sourcex, 0, rop);
	win_update(wdev, h);	/* message handler */
	return 0;
}


/* Copy a color pixel map.  This is just like a bitmap, except that */
/* each pixel takes 8 or 4 bits instead of 1 when device driver has color. */
private int
win_copy_color(gx_device *dev,
  const byte *base, int sourcex, int raster, gx_bitmap_id id,
  int x, int y, int w, int h)
{
	fit_copy(dev, base, sourcex, raster, id, x, y, w, h);

	if ( gx_device_has_color(dev) )
	{
	switch(dev->color_info.depth) {
	  case 8:
	    {	int xi, yi;
		int skip = raster - w;
		const byte *sptr = base + sourcex;
  		if ( w <= 0 ) return 0;
  		if ( x < 0 || x + w > dev->width )
			return_error(gs_error_rangecheck);
		for ( yi = y; yi - y < h; yi++ )
		   {
			for ( xi = x; xi - x < w; xi++ )
			   {	int color =  *sptr++;
				SetPixel(wdev->hdcbit,xi,yi,PALETTEINDEX(color));
			   }
			sptr += skip;
		   }
		}
		break;
	  case 4:
	   {	/* color device, four bits per pixel */
		const byte *line = base + (sourcex >> 1);
		int dest_y = y, end_x = x + w;

		if ( w <= 0 ) return 0;
		while ( h-- )              /* for each line */
		   {	const byte *source = line;
			register int dest_x = x;
			if ( sourcex & 1 )    /* odd nibble first */
			   {	int color =  *source++ & 0xf;
				SetPixel(wdev->hdcbit,dest_x,dest_y,PALETTEINDEX(color));
				dest_x++;
			   }
			/* Now do full bytes */
			while ( dest_x < end_x )
			   {	int color = *source >> 4;
				SetPixel(wdev->hdcbit,dest_x,dest_y,PALETTEINDEX(color));
				dest_x++;
				if ( dest_x < end_x )
				   {	color =  *source++ & 0xf;
					SetPixel(wdev->hdcbit,dest_x,dest_y,PALETTEINDEX(color));
					dest_x++;
				   }
			   }
			dest_y++;
			line += raster;
		   }
	   }
	   break;
	default:
		return(-1); /* panic */
	}
	}
	else 
	/* monochrome device: one bit per pixel */
	   {	/* bitmap is the same as win_copy_mono: one bit per pixel */
		win_copy_mono(dev, base, sourcex, raster, id, x, y, w, h,
			(gx_color_index)0, 
			(gx_color_index)(dev->color_info.depth==8 ? 63 : dev->color_info.max_gray));
	   }
	win_update(wdev, h);	/* message handler */
	return 0;
}

/* ------ Internal routines ------ */

#undef wdev

/* make image window */
private void near 
win_makeimg(gx_device_win *wdev)
{	HMENU sysmenu;
	wdev->hwndimg = CreateWindow(szAppName, (LPSTR)imgname,
		  WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
		  CW_USEDEFAULT, CW_USEDEFAULT, 
		  CW_USEDEFAULT, CW_USEDEFAULT, 
		  NULL, NULL, phInstance, (LPSTR)NULL);
	/* modify the menu to have the new items we want */
	sysmenu = GetSystemMenu(wdev->hwndimg,0);	/* get the sysmenu */
	AppendMenu(sysmenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(sysmenu, MF_STRING, M_COPY_CLIP, "Copy to Clip&board");
}


/* out of memory error exit with message box */
private void near
win_nomemory()
{
       	MessageBox(hwndeasy,(LPSTR)"Not enough memory",(LPSTR) szAppName, MB_ICONSTOP);
	gs_exit(1);
}


private LPLOGPALETTE near
win_makepalette(gx_device_win *wdev)
{	int i, val;
	LPLOGPALETTE logpalette;
	logpalette = (LPLOGPALETTE) malloc( sizeof(LOGPALETTE) + 
		(1<<(wdev->color_info.depth)) * sizeof(PALETTEENTRY) );
	if (logpalette == (LPLOGPALETTE)NULL)
		return(0);
	logpalette->palVersion = 0x300;
	logpalette->palNumEntries = wdev->nColors;
	for (i=0; i<wdev->nColors; i++) {
	  logpalette->palPalEntry[i].peFlags = NULL;
	  switch (wdev->nColors) {
		case 64:
		  /* colors are rrggbb */
		  logpalette->palPalEntry[i].peRed   =
			  win_map32(((i & 0x30)>>4)*(gx_max_color_value/3));
		  logpalette->palPalEntry[i].peGreen =
			  win_map32(((i & 0xC)>>2)*(gx_max_color_value/3));
		  logpalette->palPalEntry[i].peBlue  =
			  win_map32((i & 3)*(gx_max_color_value/3));
		  break;
		case 16:
		  /* colors are irgb */
		  val = (i & 8 ? 255 : 255 / 2);
		  logpalette->palPalEntry[i].peRed   = i & 4 ? val : 0;
		  logpalette->palPalEntry[i].peGreen = i & 2 ? val : 0;
		  logpalette->palPalEntry[i].peBlue  = i & 1 ? val : 0;
		  break;
		case 2:
		  logpalette->palPalEntry[i].peRed =
		    logpalette->palPalEntry[i].peGreen =
		    logpalette->palPalEntry[i].peBlue = (i ? 255 : 0);
		  break;
	  }
	}
	return(logpalette);
}


private void near
win_addtool(gx_device_win *wdev, int i)
{
	wdev->hpens[i] = CreatePen(PS_SOLID, 1, PALETTEINDEX(i));
	wdev->hbrushs[i] = CreateSolidBrush(PALETTEINDEX(i));
}


private void near
win_maketools(gx_device_win *wdev, HDC hdc)
{	int i;
	wdev->hpens = malloc( (1<<(wdev->color_info.depth)) * sizeof(HPEN) );
	wdev->hbrushs = malloc( (1<<(wdev->color_info.depth)) * sizeof(HBRUSH) );
	if (wdev->hpens && wdev->hbrushs) {
		for (i=0; i<wdev->nColors; i++)
			win_addtool(wdev, i);

		wdev->hpen = wdev->hpens[0];
		SelectObject(hdc,wdev->hpen);

		wdev->hbrush = wdev->hbrushs[0];
		SelectObject(hdc,wdev->hbrush);
	}
}


private void near
win_destroytools(gx_device_win *wdev)
{	int i;
	for (i=0; i<wdev->nColors; i++) {
		DeleteObject(wdev->hpens[i]);
		DeleteObject(wdev->hbrushs[i]);
	}
	free(wdev->hpens);
	free(wdev->hbrushs);
}


private void near
win_update(gx_device_win *wdev, int amount)
{	/* Update the image screen every 5 seconds, */
	/* checking the time every N scan lines. */
	static int fcount = 0;
	static long time;
	long date_time[2];
	
	if (fcount == 0)
	{	gp_get_clock(date_time);
		time = date_time[1];
	}
	if ((fcount += amount) > 200 || fcount < 0)
	{	long deltatime;
		gp_get_clock(date_time);
		deltatime = date_time[1] - time;
		if ( (deltatime > 5000L) || (deltatime < 0L) )
		{	win_sync_output((gx_device *)wdev);	/* time to redraw */
			time = date_time[1];
		}
		fcount = 1;
	}
}


long far PASCAL 
WndProc(HWND hwnd, WORD message, WORD wParam, LONG lParam)
{	gx_device_win *wdev;
	HDC hdc, mem;
	PAINTSTRUCT ps;
	RECT rect;
	HBITMAP bitmap;
	HPALETTE oldpalette;
	int nVscrollInc, nHscrollInc;

	/* Search the list of active Windows devices */
	/* to find the one owning this window. */
	for ( wdev = (gx_device_win *)gdev_win_open_list;
	      wdev != 0; wdev = wdev->next_wdev
	    )
	  if ( wdev->hwndimg == hwnd ) break;
	if ( wdev == 0 ) goto not_ours;		/* can this happen??? */

	switch(message) {
		case WM_SYSCOMMAND:
			switch(wParam) {
				case M_COPY_CLIP:
					/* make some where to put it and copy */
					bitmap = CreateCompatibleBitmap(wdev->hdcbit, wdev->width,
						wdev->height);
					if (bitmap) {
						/* there is enough memory and the bitmaps OK */
						mem = CreateCompatibleDC(wdev->hdcbit);
						SelectObject(mem, bitmap);
						BitBlt(mem,0,0,wdev->width,
							wdev->height, wdev->hdcbit, 0, 0, SRCCOPY);
						DeleteDC(mem);
					}
					else {
						puts("\nInsufficient Memory to Copy Clipboard");
						MessageBeep(0);
					}

					OpenClipboard(hwnd);
					EmptyClipboard();
					SetClipboardData(CF_BITMAP, bitmap);
					CloseClipboard();

					return 0;
			}
			break;
		case WM_SIZE:
			wdev->cyClient = HIWORD(lParam);
			wdev->cxClient = LOWORD(lParam);

			wdev->cyAdjust = max(32,min(wdev->height, wdev->cyClient)) - wdev->cyClient;
			wdev->cyClient += wdev->cyAdjust;

			wdev->nVscrollMax = max(0, wdev->height - wdev->cyClient);
			wdev->nVscrollPos = min(wdev->nVscrollPos, wdev->nVscrollMax);

			SetScrollRange(hwnd, SB_VERT, 0, wdev->nVscrollMax, FALSE);
			SetScrollPos(hwnd, SB_VERT, wdev->nVscrollPos, TRUE);

			wdev->cxAdjust = max(32,min(wdev->width,  wdev->cxClient)) - wdev->cxClient;
			wdev->cxClient += wdev->cxAdjust;

			wdev->nHscrollMax = max(0, wdev->width - wdev->cxClient);
			wdev->nHscrollPos = min(wdev->nHscrollPos, wdev->nHscrollMax);

			SetScrollRange(hwnd, SB_HORZ, 0, wdev->nHscrollMax, FALSE);
			SetScrollPos(hwnd, SB_HORZ, wdev->nHscrollPos, TRUE);

			if ((wParam==SIZENORMAL) && (wdev->cxAdjust!=0 || wdev->cyAdjust!=0)) {
			    GetWindowRect(hwnd,&rect);
			    MoveWindow(hwnd,rect.left,rect.top,
				rect.right-rect.left+wdev->cxAdjust,
				rect.bottom-rect.top+wdev->cyAdjust, TRUE);
			    wdev->cxAdjust = wdev->cyAdjust = 0;
			}
			return(0);
		case WM_VSCROLL:
			switch(wParam) {
				case SB_TOP:
					nVscrollInc = -wdev->nVscrollPos;
					break;
				case SB_BOTTOM:
					nVscrollInc = wdev->nVscrollMax - wdev->nVscrollPos;
					break;
				case SB_LINEUP:
					nVscrollInc = -wdev->cyClient/16;
					break;
				case SB_LINEDOWN:
					nVscrollInc = wdev->cyClient/16;
					break;
				case SB_PAGEUP:
					nVscrollInc = min(-1,-wdev->cyClient);
					break;
				case SB_PAGEDOWN:
					nVscrollInc = max(1,wdev->cyClient);
					break;
				case SB_THUMBPOSITION:
					nVscrollInc = LOWORD(lParam) - wdev->nVscrollPos;
					break;
				default:
					nVscrollInc = 0;
				}
			if (nVscrollInc = max(-wdev->nVscrollPos, 
				min(nVscrollInc, wdev->nVscrollMax - wdev->nVscrollPos))) {
				wdev->nVscrollPos += nVscrollInc;
				ScrollWindow(hwnd,0,-nVscrollInc,NULL,NULL);
				SetScrollPos(hwnd,SB_VERT,wdev->nVscrollPos,TRUE);
				UpdateWindow(hwnd);
			}
			return(0);
		case WM_HSCROLL:
			switch(wParam) {
				case SB_LINEUP:
					nHscrollInc = -wdev->cxClient/16;
					break;
				case SB_LINEDOWN:
					nHscrollInc = wdev->cyClient/16;
					break;
				case SB_PAGEUP:
					nHscrollInc = min(-1,-wdev->cxClient);
					break;
				case SB_PAGEDOWN:
					nHscrollInc = max(1,wdev->cxClient);
					break;
				case SB_THUMBPOSITION:
					nHscrollInc = LOWORD(lParam) - wdev->nHscrollPos;
					break;
				default:
					nHscrollInc = 0;
				}
			if (nHscrollInc = max(-wdev->nHscrollPos, 
				min(nHscrollInc, wdev->nHscrollMax - wdev->nHscrollPos))) {
				wdev->nHscrollPos += nHscrollInc;
				ScrollWindow(hwnd,-nHscrollInc,0,NULL,NULL);
				SetScrollPos(hwnd,SB_HORZ,wdev->nHscrollPos,TRUE);
				UpdateWindow(hwnd);
			}
			return(0);
		case WM_KEYDOWN:
			switch(wParam) {
				case VK_HOME:
					SendMessage(hwnd,WM_VSCROLL,SB_TOP,0L);
					break;
				case VK_END:
					SendMessage(hwnd,WM_VSCROLL,SB_BOTTOM,0L);
					break;
				case VK_PRIOR:
					SendMessage(hwnd,WM_VSCROLL,SB_PAGEUP,0L);
					break;
				case VK_NEXT:
					SendMessage(hwnd,WM_VSCROLL,SB_PAGEDOWN,0L);
					break;
				case VK_UP:
					SendMessage(hwnd,WM_VSCROLL,SB_LINEUP,0L);
					break;
				case VK_DOWN:
					SendMessage(hwnd,WM_VSCROLL,SB_LINEDOWN,0L);
					break;
				case VK_LEFT:
					SendMessage(hwnd,WM_HSCROLL,SB_PAGEUP,0L);
					break;
				case VK_RIGHT:
					SendMessage(hwnd,WM_HSCROLL,SB_PAGEDOWN,0L);
					break;
			}
			return(0);
		case WM_PAINT:
			{
			int sx,sy,wx,wy,dx,dy;
			hdc = BeginPaint(hwnd, &ps);
			oldpalette = SelectPalette(hdc,wdev->himgpalette,NULL);
			RealizePalette(hdc);
			SetMapMode(hdc, MM_TEXT);
			SetBkMode(hdc,OPAQUE);
			GetClientRect(hwnd, &rect);
			SetViewportExt(hdc, rect.right, rect.bottom);
			dx = rect.left;	/* destination */
			dy = rect.top;
			wx = rect.right-rect.left; /* width */
			wy = rect.bottom-rect.top;
			sx = rect.left;	/* source */
			sy = rect.top;
			sx += wdev->nHscrollPos; /* scrollbars */
			sy += wdev->nVscrollPos;	
			if (sx+wx > wdev->width)
				wx = wdev->width - sx;
			if (sy+wy > wdev->height)
				wy = wdev->height - sy;
			BitBlt(hdc,dx,dy,wx,wy,wdev->hdcbit,sx,sy,SRCCOPY);
			SelectPalette(hdc,oldpalette,NULL);
			EndPaint(hwnd, &ps);
			return 0;
			}
	}

not_ours:
	return DefWindowProc((HWND)hwnd, (WORD)message, (WORD)wParam, (DWORD)lParam);
}
