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

/* gsimage.c */
/* Image procedures for Ghostscript library */
#include "gx.h"
#include "memory_.h"
#include "gpcheck.h"
#include "gserrors.h"
#include "gxfixed.h"
#include "gxarith.h"
#include "gxmatrix.h"
#include "gspaint.h"
#include "gzstate.h"
#include "gzdevice.h"			/* requires gsstate.h */
#include "gzcolor.h"			/* requires gxdevice.h */
#include "gzpath.h"
#include "gxcpath.h"
#include "gxdevmem.h"
#include "gximage.h"

/* Exported size of enumerator */
const uint gs_image_enum_sizeof = sizeof(gs_image_enum);

/* Forward declarations */
private int image_init_map(P3(byte *, int, const float *));
private int image_init(P9(gs_image_enum *, int, int, int, int, int,
  gs_matrix *, gs_state *, fixed));
/* Procedures for unpacking the input data into 8 bits/sample. */
/* Some of these have been split off to gsimage2. */
private void image_unpack_1(iunpack_proc_args);
extern void image_unpack_1_spread(iunpack_proc_args);
private void image_unpack_2(iunpack_proc_args);
extern void image_unpack_2_spread(iunpack_proc_args);
private void image_unpack_4(iunpack_proc_args);
private void image_unpack_8(iunpack_proc_args);
extern void image_unpack_8_spread(iunpack_proc_args);
extern void image_unpack_12(iunpack_proc_args);
/* The image_render procedures work on fully expanded, complete rows. */
/* These take a height argument, which is an integer > 0; */
/* they return a negative code, or the number of */
/* rows actually processed (which may be less than the height). */
private int image_render_skip(irender_proc_args);
private int image_render_direct(irender_proc_args);
private int image_render_simple(irender_proc_args);
private int image_render_mono(irender_proc_args);
extern int image_render_color(irender_proc_args);

/* Set up a gs_color with a transfer-mapped gray sample. */
#define image_set_rgb(rcolor,sample_value)\
  (rcolor.luminance = rcolor.red = rcolor.green = rcolor.blue =\
    gx_map_color_param_byte(pgs, sample_value, gray),\
   gx_color_from_rgb(&rcolor))

/* Standard mask tables for spreading input data. */
/* Note that the mask tables depend on the end-orientation of the CPU. */
/* We can't simply define them as byte arrays, because */
/* they might not wind up properly long- or short-aligned. */
#define map4tox(z,a,b,c,d)\
	z, z^a, z^b, z^(a+b),\
	z^c, z^(a+c), z^(b+c), z^(a+b+c),\
	z^d, z^(a+d), z^(b+d), z^(a+b+d),\
	z^(c+d), z^(a+c+d), z^(b+c+d), z^(a+b+c+d)
#if arch_is_big_endian
private const unsigned long map_4x1_to_32[16] =
   {	map4tox(0L, 0xffL, 0xff00L, 0xff0000L, 0xff000000L)	};
private const unsigned long map_4x1_to_32_invert[16] =
   {	map4tox(0xffffffffL, 0xffL, 0xff00L, 0xff0000L, 0xff000000L)	};
#else					/* !arch_is_big_endian */
private const unsigned long map_4x1_to_32[16] =
   {	map4tox(0L, 0xff000000L, 0xff0000L, 0xff00L, 0xffL)	};
private const unsigned long map_4x1_to_32_invert[16] =
   {	map4tox(0xffffffffL, 0xff000000L, 0xff0000L, 0xff00L, 0xffL)	};
#endif

/* Start processing an image */
int
gs_image_init(gs_image_enum *penum, gs_state *pgs,
  int width, int height, int bps, int spp, const float *decode,
  gs_matrix *pmat)
{	int spread;
	int code;
	static const float decode_01[2] = { 0.0, 1.0 };
	if ( pgs->in_cachedevice )
		return_error(gs_error_undefined);
	switch ( spp )
	   {
	case 1: case 3: case 4:
		spread = 1; break;
	case -3: case -4:
		spp = -spp; spread = spp; break;
	default:
		return_error(gs_error_rangecheck);
	   }
	if ( spp == 1 )
	   {	/* Initialize the color table */
#define chtl(i)\
  penum->dev_colors[i].halftone_level
		switch ( bps )
		   {
		default:
		   {	/* Yes, clearing the entire table is slow, */
			/* but for 8 bit-per-sample images, it's worth it. */
			register gx_device_color *pcht = &penum->dev_colors[0];
			register int n = 64;
			do
			   {	pcht[0].halftone_level =
				  pcht[1].halftone_level =
				  pcht[2].halftone_level =
				  pcht[3].halftone_level = -1;
				pcht += 4;
			   }
			while ( --n > 0 );
			break;
		   }
		case 4:
			chtl(17) = chtl(2*17) = chtl(3*17) =
			  chtl(4*17) = chtl(6*17) = chtl(7*17) =
			  chtl(8*17) = chtl(9*17) = chtl(11*17) =
			  chtl(12*17) = chtl(13*17) = chtl(14*17) = -1;
			/* falls through */
		case 2:
			chtl(5*17) = chtl(10*17) = -1;
		case 1:
			;
		   }
		/* Pre-map entries 0 and 255. */
		   {	gs_color rcolor;
			image_set_rgb(rcolor, 0);
			gx_color_render(&rcolor, &penum->icolor0, pgs);
			image_set_rgb(rcolor, 255);
			gx_color_render(&rcolor, &penum->icolor1, pgs);
		   }
#undef chtl
	   }
	penum->masked = 0;
	/* Initialize the map from samples to intensities. */
	if ( decode == 0 )
		decode = decode_01;
	if ( bps > 2 || spread != 1 )
		code = image_init_map(&penum->map.to8[0], 1 << bps, decode);
	else
	{	/* The map index encompasses more than one pixel. */
		byte map[4];
		register int i;
		code = image_init_map(&map[0], 1 << bps, decode);
		switch ( bps )
		{
		case 1:
		{	register ulong *p = &penum->map.from4x1to32[0];
			if ( map[0] == 0 && map[1] == 0xff )
				memcpy((byte *)p, map_4x1_to_32, 16 * 4);
			else if ( map[0] == 0xff && map[1] == 0 )
				memcpy((byte *)p, map_4x1_to_32_invert, 16 * 4);
			else
			  for ( i = 0; i < 16; i++, p++ )
				((byte *)p)[0] = map[i >> 3],
				((byte *)p)[1] = map[(i >> 2) & 1],
				((byte *)p)[2] = map[(i >> 1) & 1],
				((byte *)p)[3] = map[i & 1];
		}	break;
		case 2:
		{	register ushort *p = &penum->map.from2x2to16[0];
			for ( i = 0; i < 16; i++, p++ )
				((byte *)p)[0] = map[i >> 2],
				((byte *)p)[1] = map[i & 3];
		}	break;
		}
	}
	if ( code < 0 ) return code;
	return image_init(penum, width, height, bps, spp, spread,
			  pmat, pgs, (fixed)0);
}
/* Construct a mapping table for sample values. */
/* map_size is 2, 4, 16, or 256.  Note that 255 % (map_size - 1) == 0. */
private int
image_init_map(byte *map, int map_size, const float *decode)
{	float min_v = decode[0], max_v = decode[1];
	uint value;
	uint diff;
	byte *limit = map + map_size;
	if ( min_v < 0 || min_v > 1 || max_v < 0 || max_v > 1 )
		return_error(gs_error_rangecheck);
	value = min_v * 0xffffL;
	/* The division in the next statement is exact, */
	/* see the comment above. */
	diff = (max_v - min_v) * (0xffffL / (map_size - 1));
	for ( ; map != limit; map++, value += diff )
		*map = value >> 8;
	return 0;
}

/* Start processing a masked image */
int
gs_imagemask_init(gs_image_enum *penum, gs_state *pgs,
  int width, int height, int invert, gs_matrix *pmat, int adjust)
{	/* Initialize color entries 0 and 255. */
	penum->icolor0.halftone_level = 0;
	penum->icolor0.color1 = penum->icolor0.color2 = gx_no_color_index;
	penum->icolor1 = *pgs->dev_color;
	penum->masked = 1;
	memcpy(&penum->map.from4x1to32[0],
	       (invert ? map_4x1_to_32_invert : map_4x1_to_32),
	       16 * 4);
	return image_init(penum, width, height, 1, 1, 1, pmat, pgs,
			  (adjust && pgs->in_cachedevice ?
			   float2fixed(0.25) : (fixed)0));
}

/* Common setup for image and imagemask. */
/* Caller has set penum->masked, map, dev_colors[]. */
private int
image_init(register gs_image_enum *penum, int width, int height,
  int bps, int spp, int spread, gs_matrix *pmat, gs_state *pgs,
  fixed adjust)
{	int code;
	int index_bps;
	gs_matrix mat;
	gs_fixed_rect clip_box;
	uint bsize = (width + 8) * spp;	/* round up, +1 for end-of-run byte */
	byte *buffer;
	fixed mtx, mty;
	if ( width <= 0 || height < 0 )
		return_error(gs_error_rangecheck);
	switch ( bps )
	   {
	case 1: index_bps = 0; break;
	case 2: index_bps = 1; break;
	case 4: index_bps = 2; break;
	case 8: index_bps = 3; break;
	case 12: index_bps = 4; break;
	default: return_error(gs_error_rangecheck);
	   }
	if ( height == 0 ) return 0;	/* empty image */
	if (	(code = gs_matrix_invert(pmat, &mat)) < 0 ||
		(code = gs_matrix_multiply(&mat, &ctm_only(pgs), &mat)) < 0
	   )	return code;
	buffer = (byte *)gs_malloc(1, bsize, "image buffer");
	if ( buffer == 0 ) return_error(gs_error_VMerror);
	penum->width = width;
	penum->height = height;
	penum->bps = bps;
	penum->spp = spp;
	penum->spread = spread;
	penum->fxx = float2fixed(mat.xx);
	penum->fxy = float2fixed(mat.xy);
	penum->fyx = float2fixed(mat.yx);
	penum->fyy = float2fixed(mat.yy);
	penum->skewed = (penum->fxy | penum->fyx) != 0;
	penum->xcur = mtx = float2fixed(mat.tx);
	penum->ycur = mty = float2fixed(mat.ty);
	penum->pgs = pgs;
	clip_box = pgs->clip_path->path.bbox;	/* box is known to be up to date */
	penum->clip_box = clip_box;
	penum->buffer = buffer;
	penum->buffer_size = bsize;
	penum->bytes_per_row =
		(uint)(((ulong)width * (bps * spp) / spread + 7) >> 3);
	penum->slow_loop = penum->skewed;
	/* If all four extrema of the image fall within the clipping */
	/* rectangle, clipping is never required. */
	   {	gs_fixed_rect cbox;
		fixed edx = float2fixed(mat.xx * width);
		fixed edy = float2fixed(mat.yy * height);
		fixed epx, epy, eqx, eqy;
		if ( edx < 0 ) epx = edx, eqx = 0;
		else epx = 0, eqx = edx;
		if ( edy < 0 ) epy = edy, eqy = 0;
		else epy = 0, eqy = edy;
		if ( penum->skewed )
		   {	edx = float2fixed(mat.yx * height);
			edy = float2fixed(mat.xy * width);
			if ( edx < 0 ) epx += edx; else eqx += edx;
			if ( edy < 0 ) epy += edy; else eqy += edy;
		   }
		gx_cpath_box_for_check(pgs->clip_path, &cbox);
		penum->never_clip =
			mtx + epx >= cbox.p.x && mtx + eqx <= cbox.q.x &&
			mty + epy >= cbox.p.y && mty + eqy <= cbox.q.y;
		if_debug7('b',
			  "[b]Image: cbox=(%g,%g),(%g,%g)\n     mt=(%g,%g) never_clip=%d\n",
			  fixed2float(cbox.p.x), fixed2float(cbox.p.y),
			  fixed2float(cbox.q.x), fixed2float(cbox.q.y),
			  fixed2float(mtx), fixed2float(mty),
			  penum->never_clip);
	   }
	   {	static void (*procs[5])(iunpack_proc_args) = {
			image_unpack_1, image_unpack_2,
			image_unpack_4, image_unpack_8, image_unpack_12
		   };
		static void (*spread_procs[5])(iunpack_proc_args) = {
			image_unpack_1_spread, image_unpack_2_spread,
			image_unpack_4, image_unpack_8_spread,
			image_unpack_12
		   };
		penum->slow_loop |=
			/* Use slow loop for imagemask with a halftone */
			(penum->masked &&
			 !color_is_pure(pgs->dev_color));
		if ( pgs->in_charpath )
			penum->render = image_render_skip;
		else if ( spp == 1 && bps == 1 && !penum->slow_loop &&
			  (penum->masked ||
			   (color_is_pure(&penum->icolor0) &&
			    color_is_pure(&penum->icolor1)))
		   )
			penum->render =
			  (fixed2long_rounded(mtx + width * penum->fxx) -
			   fixed2long_rounded(mtx) == width ?
			    image_render_direct :
			   penum->masked ? image_render_mono :
			   image_render_simple);
		else
			penum->render =
			  (spp == 1 ? image_render_mono : image_render_color);
		/* The following should just be an assignment of */
		/* a conditional expression, but the Ultrix C compiler */
		/* can't handle it. */
		if ( penum->render == image_render_direct ||
		     penum->render == image_render_simple
		   )
		  { penum->unpack = image_unpack_8;
		    /* If the image is 1-for-1 with the device, */
		    /* we don't want to spread the samples, */
		    /* but we have to reset bps to prevent the buffer */
		    /* pointer from being incremented by 8 bytes */
		    /* per input byte. */
		    penum->bps = 8;
		  }
		else if ( spread != 1 )
		  penum->unpack = spread_procs[index_bps];
		else
		  penum->unpack = procs[index_bps];
	   }
	if ( !penum->never_clip )
	   {	/* Set up the clipping device. */
		gx_device *dev = (gx_device *)&penum->clip_dev;
		penum->clip_dev = gs_clip_device;
		penum->clip_dev.target = gs_currentdevice(pgs);
		penum->clip_dev.list = pgs->clip_path->list;
		(*dev->procs->open_device)(dev);
	   }
	penum->adjust = adjust;
	penum->plane_index = 0;
	penum->byte_in_row = 0;
	penum->y = 0;
	if_debug9('b', "[b]Image: w=%d h=%d %s\n   [%f %f %f %f %f %f]\n",
		 width, height,
		 (penum->never_clip ? "no clip" : "must clip"),
		 mat.xx, mat.xy, mat.yx, mat.yy, mat.tx, mat.ty);
	return 0;
}

/* Process the next piece of an image */
int
gs_image_next(register gs_image_enum *penum, byte *dbytes, uint dsize)
{	uint rsize = penum->bytes_per_row;
	uint pos = penum->byte_in_row;
	int width = penum->width;
	uint dleft = dsize;
	uint dpos = 0;
	gs_state *pgs = penum->pgs;
	gx_device *save_dev = 0;
	int code;
	/* Accumulate separated colors, if needed */
	if ( penum->plane_index == 0 )
		penum->plane_size = dsize;
	else if ( dsize != penum->plane_size )
		return_error(gs_error_undefinedresult);
	penum->planes[penum->plane_index] = dbytes;
	if ( ++(penum->plane_index) != penum->spread )
		return 0;
	penum->plane_index = 0;
	/* We've accumulated an entire set of planes. */
	if ( !penum->never_clip )
	   {	/* Install the clipping device if needed. */
		gx_device *dev = (gx_device *)&penum->clip_dev;
		save_dev = gs_currentdevice(pgs);
		penum->clip_dev.target = save_dev;
		gx_set_device_only(pgs, dev);
	   }
	while ( dleft )
	   {	/* Fill up a row, then display it. */
		uint bcount = min(dleft, rsize - pos);
		byte *bptr =
		  penum->buffer + (pos << 3) / penum->bps * penum->spread;
		int px;
		for ( px = 0; px < penum->spread; px++ )
		  (*penum->unpack)(penum, bptr + px, penum->planes[px] + dpos, bcount, pos);
		pos += bcount;
		dpos += bcount;
		dleft -= bcount;
		if ( pos == rsize )	/* filled an entire row */
		   {
#ifdef DEBUG
if ( gs_debug['B'] )
   {			int i, n = width * penum->spp;
			dputs("[B]row:");
			for ( i = 0; i < n; i++ )
				dprintf1(" %02x", penum->buffer[i]);
			dputs("\n");
   }
#endif
			if ( !penum->skewed )
			  { /* Precompute integer y and height, */
			    /* and check for clipping. */
			    fixed yc = penum->ycur, yn;
			    fixed dyy = penum->fyy;
			    fixed adjust = penum->adjust;
			    if ( dyy > 0 )
			      dyy += adjust << 1,
			      yc -= adjust;
			    else
			      dyy = (adjust << 1) - dyy,
			      yc -= dyy - adjust;
			    if ( yc >= penum->clip_box.q.y ) goto mt;
			    yn = yc + dyy;
			    if ( yn <= penum->clip_box.p.y ) goto mt;
			    penum->yci = fixed2int_var_rounded(yc);
			    penum->hci =
			      fixed2int_var_rounded(yn) - penum->yci;
			    if ( penum->hci == 0 ) goto mt;
			  }
			code = (*penum->render)(penum, penum->buffer, width * penum->spp, 1);
			if ( code < 0 ) goto err;
mt:			if ( ++(penum->y) == penum->height ) goto end;
			pos = 0;
			penum->xcur += penum->fyx;
			penum->ycur += penum->fyy;
		   }
	   }
	penum->byte_in_row = pos;
	code = 0;
	goto out;
end:	/* End of data */
	code = 1;
	/* falls through */
err:	/* Error, abort */
	gs_free((char *)penum->buffer, penum->buffer_size, 1, "image buffer");
out:	if ( save_dev != 0 ) gx_set_device_only(pgs, save_dev);
	return code;
}

/* ------ Unpacking procedures ------ */

private void
image_unpack_1(const gs_image_enum *penum, byte *bptr,
  register const byte *data, uint dsize, uint inpos)
{	register ulong *bufp = (unsigned long *)bptr;
	int left = dsize;
	register const ulong *map = &penum->map.from4x1to32[0];
	register uint b;
	if ( left & 1 )
	   {	b = data[0];
		bufp[0] = map[b >> 4];
		bufp[1] = map[b & 0xf];
		data++, bufp += 2;
	   }
	left >>= 1;
	while ( left-- )
	   {	b = data[0];
		bufp[0] = map[b >> 4];
		bufp[1] = map[b & 0xf];
		b = data[1];
		bufp[2] = map[b >> 4];
		bufp[3] = map[b & 0xf];
		data += 2, bufp += 4;
	   }
}

private void
image_unpack_2(const gs_image_enum *penum, byte *bptr,
  register const byte *data, uint dsize, uint inpos)
{	register ushort *bufp = (unsigned short *)bptr;
	int left = dsize;
	register const ushort *map = &penum->map.from2x2to16[0];
	while ( left-- )
	   {	register unsigned b = *data++;
		*bufp++ = map[b >> 4];
		*bufp++ = map[b & 0xf];
	   }
}

private void
image_unpack_4(const gs_image_enum *penum, register byte *bufp,
  register const byte *data, uint dsize, uint inpos)
{	register int spread = penum->spread;
	int left = dsize;
	register const byte *map = &penum->map.to8[0];
	while ( left-- )
	   {	register unsigned b = *data++;
		*bufp = map[b >> 4]; bufp += spread;
		*bufp = map[b & 0xf]; bufp += spread;
	   }
}

private void
image_unpack_8(const gs_image_enum *penum, byte *bufp,
  const byte *data, uint dsize, uint inpos)
{	if ( data != bufp ) memcpy(bufp, data, dsize);
}

/* ------ Rendering procedures ------ */

/* Rendering procedure for ignoring an image.  We still need to iterate */
/* over the samples, because the procedure might have side effects. */
private int
image_render_skip(gs_image_enum *penum, byte *data, uint w, int h)
{	return h;
}

/* Rendering procedure for a 1-bit-per-pixel sampled image */
/* with no skewing/rotation or X scaling. */
/* In this case a direct BitBlt is possible. */
private int
image_render_direct(register gs_image_enum *penum, byte *data, uint w, int h)
{	int ix = fixed2int_var_rounded(penum->xcur), iw = w;
	int iy = penum->yci, ih = penum->hci;
	uint raster = (w + 7) >> 3;
	gx_device *dev = penum->pgs->device->info;
	dev_proc_copy_mono((*copy_mono)) = dev->procs->copy_mono;
	gx_color_index
		zero = penum->icolor0.color1,
		one = penum->icolor1.color1;
	if_debug4('b', "[b]direct (%d,%d),(%d,%d)\n", ix, iy, iw, ih);
	/* Check for inverted imagemask */
	if ( penum->map.from4x1to32[0] != 0 )
		zero = penum->icolor1.color1,
		one = penum->icolor0.color1;
	   {	/* Do just one row, clipping if necessary. */
		int dy;
		for ( dy = 0; dy < ih; dy++ )
			(*copy_mono)(dev, data, 0, raster, gx_no_bitmap_id,
				ix, iy + dy, iw, 1, zero, one);
		gp_check_interrupts();
		return 1;
	   }
}

/* Rendering procedure for a non-mask monobit image with no */
/* skew or rotation and pure colors. */
private int
image_render_simple(gs_image_enum *penum, byte *buffer, uint w, int h)
{	fixed xrun = penum->xcur + fixed_half;
	fixed xl;
	const fixed dxx = penum->fxx;
	const fixed dxx_8 = dxx << 3;
	register const byte *psrc = buffer;
	gx_device *dev = penum->pgs->device->info;
	dev_proc_fill_rectangle((*fill_proc)) = dev->procs->fill_rectangle;
	const int iy = penum->yci, ih = penum->hci;
	gx_color_index
		zero = penum->icolor0.color1,
		one = penum->icolor1.color1;
	int sbit = 7;
	byte *endp = buffer + (w >> 3);
	const int endbit = ~w & 7;
	const byte endmask = 1 << endbit;
	int xi = fixed2int_var(xrun);
	int xni, code;
	byte data;
	if ( penum->map.from4x1to32[0] != 0 )
		zero = penum->icolor1.color1,
		one = penum->icolor0.color1;
	/* Invert the bit following the last valid data bit. */
	if ( endmask == 0x80 ) *endp = ~endp[-1] << 7;
	else if ( *endp & (endmask << 1) ) *endp &= ~endmask;
	else *endp |= endmask;
	data = *psrc;
#define fill_run(color)\
  xni = fixed2int_var(xl);\
  if ( xni > xi )\
   { code = (*fill_proc)(dev, xi, iy, xni-xi, ih, color);\
     if ( code < 0 ) return code;\
   }\
  else if ( xni < xi )\
   { code = (*fill_proc)(dev, xni, iy, xi-xni, ih, color);\
     if ( code < 0 ) return code;\
   }
	/* Pre-fill the scan line with the zero color. */
	/* This halves the number of fill_proc calls. */
	xl = xrun + (dxx * w);
	fill_run(zero);
	xl = xrun;
	/* Loop invariants: data = *psrc; 0<=sbit<=7. */
	do
	{	/* Scan a run of zeros. */
		switch ( sbit )
		{
		case 7:
w7:			if ( data & 0x80 ) { sbit = 7; break; } xl += dxx;
		case 6:
			if ( data & 0x40 ) { sbit = 6; break; } xl += dxx;
		case 5:
			if ( data & 0x20 ) { sbit = 5; break; } xl += dxx;
		case 4:
			if ( data & 0x10 ) { sbit = 4; break; } xl += dxx;
		case 3:
			if ( data & 0x08 ) { sbit = 3; break; } xl += dxx;
		case 2:
			if ( data & 0x04 ) { sbit = 2; break; } xl += dxx;
		case 1:
			if ( data & 0x02 ) { sbit = 1; break; } xl += dxx;
		case 0:
			if ( data & 0x01 ) { sbit = 0; break; } xl += dxx;
			while ( (data = *++psrc) == 0 ) xl += dxx_8;
			goto w7;
		}
		if ( !(psrc < endp || sbit > endbit) )
			break;
		xrun = xl, xi = fixed2int_var(xrun);
		/* Scan a run of ones. */
		while ( (data >> sbit) & 1 )
		{	xl += dxx;
			if ( --sbit < 0 )
			{	sbit = 7;
				while ( (data = *++psrc) == 0xff )
					xl += dxx_8;
			}
		}
		fill_run(one);
	} while ( psrc < endp || sbit > endbit );
#undef fill_run
	return 1;
}

/* Rendering procedure for the general case of displaying a */
/* monochrome image, dealing with multiple bit-per-sample images, */
/* bits not 1-for-1 with the device, and general transformations. */
/* This procedure handles a single scan line. */
private int
image_render_mono(gs_image_enum *penum, byte *buffer, uint w, int h)
{	gs_state *pgs = penum->pgs;
	const int masked = penum->masked;
	const fixed dxx = penum->fxx;
	fixed xt = penum->xcur;
	gs_color rcolor;
	gx_device_color *pdevc = pgs->dev_color;
	/* Make sure the cache setup matches the graphics state. */
	/* Also determine whether all tiles fit in the cache. */
	int tiles_fit = gx_check_tile_cache(pgs);
#define image_set_gray(sample_value)\
   { pdevc = &penum->dev_colors[sample_value];\
     switch ( pdevc->halftone_level )\
      { default:		/* halftone */\
	  if ( !tiles_fit ) gx_color_load(pdevc, pgs); break;\
        case -1:		/* not computed yet */\
	  image_set_rgb(rcolor, sample_value);\
	  gx_color_render(&rcolor, pdevc, pgs);\
	case 0: ;		/* pure color */\
      }\
   }
	fixed xl = xt;
	register const byte *psrc = buffer;
	byte *endp = buffer + w;
	fixed xrun = xt;		/* x at start of run */
	register byte run = *psrc;	/* run value */
	int htrun =			/* halftone run value */
	  (masked ? 255 : -2);
	*endp = ~endp[-1];	/* force end of run */
	gx_set_gray_only(&rcolor, (color_param)0);
	if ( penum->slow_loop )
	  { /* Skewed, or imagemask with a halftone. */
	    const fixed
	      dxy = penum->fxy, dyx = penum->fyx,
	      dyy = penum->fyy;
	    fixed ytf = penum->ycur;
	    fixed yrun = ytf;
	    for ( ; ; )
	      { if ( *psrc++ != run )
		  { /* Fill the region between xrun and xl. */
		    if ( run != htrun )
		      { if ( run == 0 )
			  { if ( masked ) goto trans;
			  }
			htrun = run;
			image_set_gray(run);
		      }
		    gz_fill_pgram_fixed(xrun, yrun, xl - xrun,
					ytf - yrun, dyx, dyy,
					pdevc, pgs);
trans:		    if ( psrc > endp ) break;
		    yrun = ytf;
		    xrun = xl;
		    run = psrc[-1];
		  }
		xl += dxx;
		ytf += dxy;		/* harmless if no skew */
	      }
	  }
	else			/* fast loop */
	  { /* No skew, and not imagemask with a halftone. */
	    const fixed adjust = penum->adjust;
	    fixed xa = (dxx >= 0 ? adjust : -adjust);
	    const int yt = penum->yci, iht = penum->hci;
	    gx_device *dev = pgs->device->info;
	    dev_proc_fill_rectangle((*fill_proc)) = dev->procs->fill_rectangle;
	    dev_proc_tile_rectangle((*tile_proc)) = dev->procs->tile_rectangle;
	    dev_proc_copy_mono((*copy_mono_proc)) = dev->procs->copy_mono;
	    dev_proc_copy_color((*copy_color_proc)) = dev->procs->copy_color;
	    /* If each pixel is likely to fit in a single halftone tile, */
	    /* determine that now (tile_offset = offset of row within tile). */
	    int tile_offset =
	      gx_check_tile_size(pgs,
				 fixed2int_rounded(any_abs(dxx) + (xa << 1)),
				 yt, iht);
	    /* Fold the adjustment into xrun and xl, */
	    /* including the +0.5 for rounding. */
	    xrun = xrun - xa + fixed_half;
	    xl = xl + xa + fixed_half;
	    xa <<= 1;
	    for ( ; ; )
	      { /* Skip large constant regions quickly, */
	        /* but don't slow down transitions too much. */
	        while ( psrc[0] == run )
		{ if ( psrc[1] == run )
		  { if ( psrc[2] == run )
		    { if ( psrc[3] == run )
		      { psrc += 4, xl += dxx << 2;
			continue;
		      }
		      else
		        psrc += 3, xl += (dxx << 1) + dxx;
		    }
		    else
		      psrc += 2, xl += dxx << 1;
		  }
		  else
		    psrc++, xl += dxx;
		  break;
		}
		psrc++;
		  { /* Fill the region between xrun and xl. */
		    int xi = fixed2int_var(xrun);
		    int wi = fixed2int_var(xl) - xi;
		    int tsx, code;
		    gx_bitmap *tile;
		    if ( wi <= 0 )
		      { if ( wi == 0 ) goto mt;
			xi += wi, wi = -wi;
		      }
		    switch ( run )
		      {
		      case 0:
			if ( masked ) goto mt;
			if ( !color_is_pure(&penum->icolor0) ) goto ht;
			code = (*fill_proc)(dev, xi, yt, wi, iht, penum->icolor0.color1);
			break;
		      case 255:		/* just for speed */
			if ( !color_is_pure(&penum->icolor1) ) goto ht;
			code = (*fill_proc)(dev, xi, yt, wi, iht, penum->icolor1.color1);
			break;
		      default:
ht:			/* Use halftone if needed */
			if ( run != htrun )
			  { image_set_gray(run);
			    htrun = run;
			  }
			/* We open-code gz_fill_rectangle_open, */
			/* because we've done some of the work for */
			/* halftone tiles in advance. */
			if ( color_is_pure(pdevc) )
			  { code = (*fill_proc)(dev, xi, yt, wi, iht, pdevc->color1);
			  }
			else if ( tile_offset >= 0 &&
				  (tile = pdevc->tile,
				   (tsx = (xi + pgs->phase_mod.x) % tile->rep_width) + wi <= tile->size.x)
				)
			  { /* The pixel(s) fit(s) in a single tile. */
			    byte *row = tile->data + tile_offset;
			    code = (color_is_color_halftone(pdevc) ?
				    (*copy_color_proc)
				      (dev, row, tsx, tile->raster, gx_no_bitmap_id,
				       xi, yt, wi, iht) :
				    (*copy_mono_proc)
				      (dev, row, tsx, tile->raster, gx_no_bitmap_id,
				       xi, yt, wi, iht,
				       pdevc->color1, pdevc->color2)
				    );
			    gp_check_interrupts();
			  }
			else
			  { code = (*tile_proc)(dev, pdevc->tile, xi, yt, wi, iht,
					     pdevc->color1, pdevc->color2,
					     pgs->phase_mod.x, pgs->phase_mod.y);
			  }
		      }
		    if ( code < 0 ) return code;
mt:		    if ( psrc > endp ) break;
		    xrun = xl - xa;	/* original xa << 1 */
		    run = psrc[-1];
		  }
		xl += dxx;
	      }
	  }
	return 1;
}
