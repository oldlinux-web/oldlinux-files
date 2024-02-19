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

/* gdevpcfb.h */
/* IBM PC frame buffer definitions for Ghostscript */
/* (requires dos_.h) */

/* For testing, the 16-color display may be defined as a monochrome, */
/* 8-color, or 16-color device. */
#define ega_bits_of_color 2		/* 0, 1, or 2 */

/* Range of r-g-b values */
#define rgb_max ega_bits_of_color

#ifndef USE_ASM
#  define USE_ASM 0			/* don't use assembly language */
#endif

/* Define the short (integer) version of "transparent" color. */
/* ****** Depends on gx_no_color_index being all 1's. ******/
#define no_color ((int)gx_no_color_index)

/* Procedures */

	/* See gxdevice.h for the definitions of the procedures. */

extern dev_proc_open_device(ega_open);
extern dev_proc_close_device(ega_close);
extern dev_proc_map_rgb_color(ega_map_rgb_color);
extern dev_proc_map_color_rgb(ega_map_color_rgb);
extern dev_proc_fill_rectangle(ega_fill_rectangle);
extern dev_proc_tile_rectangle(ega_tile_rectangle);
extern dev_proc_copy_mono(ega_copy_mono);
extern dev_proc_copy_color(ega_copy_color);
extern dev_proc_get_bits(ega_get_bits);

/* Types for frame buffer pointers. */
/* Note that 'far' gets defined as null on 32-bit systems. */
typedef byte far *fb_ptr;
typedef volatile byte far *volatile_fb_ptr;

/* The device descriptor */
typedef struct gx_device_ega_s gx_device_ega;
struct gx_device_ega_s {
	gx_device_common;
	int raster;			/* frame buffer bytes per line */
	int fb_seg_mult;		/* multiplier for segment part */
					/* of frame buffer pointer */
	int fb_byte_mult;		/* multiplier for word part ditto */
#define mk_fb_ptr(x, y)\
  (fb_dev->fb_byte_mult == 0 ?\
   (fb_ptr)MK_PTR(regen + (y) * (fb_dev->fb_seg_mult), (x) >> 3) :\
   (fb_ptr)MK_PTR(regen + ((y) >> 4) * (fb_dev->fb_seg_mult),\
		 (((y) & 15) * fb_dev->fb_byte_mult) + ((x) >> 3)))
/* Define the largest height that can be processed */
/* within a single 64K segment.  If fb_dev->height > max_rop_height, */
/* we may have to break up operations into pieces. */
	unsigned max_rop_height;
	int video_mode;
};

/* Macro for creating instances */
/* The initial parameters map an appropriate fraction of */
/* the screen to an 8.5" x 11" coordinate space. */
/* This may or may not be what is desired! */
#define ega_device(dev_name, procs, fb_raster, screen_height, aspect_ratio, video_mode)\
   {	sizeof(gx_device_ega),\
	&procs,\
	dev_name,\
	fb_raster * 8, screen_height,\
	  (screen_height * (aspect_ratio)) / 11.0,	/* x density */\
	  screen_height / 11.0,		/* y density */\
	no_margins,\
	   {	(rgb_max ? 3 : 1),	/* num_components */\
		4,			/* depth */\
		(rgb_max ? rgb_max : 1),	/* gray_max */\
		rgb_max,\
		3,			/* dither_gray */\
		(rgb_max ? rgb_max + 1 : 0)	/* dither_rgb */\
	   },\
	0,			/* not opened yet */\
	fb_raster,\
	(fb_raster & 15 ? fb_raster : fb_raster >> 4),\
	(fb_raster & 15 ? fb_raster : 0),\
	((unsigned)(0xffff - fb_raster) / fb_raster),\
	video_mode\
   }

/* Define the device port and register numbers, and the regen map base */
#define out2(port, index, data)\
  (outportb(port, index), outportb((port)+1, data))
#define outin2(port, index)\
  (outportb(port, index), inportb((port)+1))
#define seq_addr 0x3c4
#define s_map 2
#define set_s_map(mask) out2(seq_addr, s_map, mask)
#define graph_addr 0x3ce
#define g_const 0			/* set/reset */
#define set_g_const(color) out2(graph_addr, g_const, color)
#define g_const_map 1			/* enable set/reset */
#define set_g_const_map(map) out2(graph_addr, g_const_map, map)
#define g_function 3
#define set_g_function(func) out2(graph_addr, g_function, func)
#define g_read_plane 4
#define set_g_read_plane(plane) out2(graph_addr, g_read_plane, plane)
#  define gf_WRITE 0
#  define gf_AND 8
#  define gf_OR 0x10
#  define gf_XOR 0x18
#define g_mode 5
#define set_g_mode(mode) out2(graph_addr, g_mode, mode)
#  define gm_DATA 0
#  define gm_FILL 2
#define g_mask 8
#define set_g_mask(mask) out2(graph_addr, g_mask, mask)
#define select_g_mask() outportb(graph_addr, g_mask)
#define out_g_mask(mask) outportb(graph_addr+1, mask)
#define regen 0xa000
