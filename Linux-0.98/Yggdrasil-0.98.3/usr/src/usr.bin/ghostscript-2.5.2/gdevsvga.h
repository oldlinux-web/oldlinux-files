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

/* gdevsvga.h */
/* SuperVGA display definitions for Ghostscript */
/* (requires gdevpcfb.h) */

/* Common procedures */

	/* See gxdevice.h for the definitions of the procedures. */

dev_proc_close_device(svga_close);
dev_proc_map_rgb_color(svga_map_rgb_color);
dev_proc_map_color_rgb(svga_map_color_rgb);
dev_proc_fill_rectangle(svga_fill_rectangle);
dev_proc_copy_mono(svga_copy_mono);
dev_proc_copy_color(svga_copy_color);
dev_proc_get_bits(svga_get_bits);

/* Table structure for looking up graphics modes. */
typedef struct {
	int width, height;		/* "key" */
	int mode;			/* "value" */
} mode_info;

/* The device descriptor structure */
typedef struct gx_device_svga_s gx_device_svga;
struct gx_device_svga_s {
	gx_device_common;
	int (*get_mode)(P0());
	void (*set_mode)(P1(int));
	void (*set_page)(P3(gx_device_svga *fbdev, int pnum, int wnum));
	const mode_info _ds *mode;	/* BIOS display mode info */
	uint raster;			/* frame buffer bytes per line */
	int page;			/* current page */
	int wnum_read, wnum_write;	/* window #s for read vs. write */
	/* Following are device-specific. */
	union {
	  struct {
		void (*bios_set_page)(P2(int, int));	/* set-page function */
	  } vesa;
	  struct {
		int select_reg;			/* page-select register */
	  } atiw;
	  struct {
		int et_model;			/* 4 for ET4000, */
						/* 3 for ET3000 */
	  } tseng;
	} info;
};

/* The initial parameters map an appropriate fraction of */
/* the screen to an 8.5" x 11" coordinate space. */
/* This may or may not be what is desired! */
#define svga_device(procs, name, get_mode, set_mode, set_page) {\
	sizeof(gx_device_svga),\
	&procs,\
	name,\
	640, 480,		/* screen size */\
	480 / 11.0, 480 / 11.0,	/* resolution */\
	no_margins,\
	dci_color(8, 31, 4),\
	0,			/* not opened yet */\
	get_mode, set_mode, set_page\
   }

/* Utility procedures */
extern int svga_find_mode(P2(gx_device *, const mode_info _ds *));
extern int svga_open(P1(gx_device *));
