/* x11wd.h - the following defs are taken from various X.V11R2 header files
*/

#ifndef _X11WD_H_
#define _X11WD_H_

#define LSBFirst	0
#define MSBFirst	1

#define XYBitmap	0
#define XYPixmap	1
#define ZPixmap		2

#define StaticGray	0
#define GrayScale	1
#define StaticColor	2
#define PseudoColor	3
#define TrueColor	4
#define DirectColor	5

typedef unsigned long xwdval;
#define X11WD_FILE_VERSION 7
typedef struct {
    xwdval header_size;		/* Size of the entire file header (bytes). */
    xwdval file_version;	/* X11WD_FILE_VERSION */
    xwdval pixmap_format;	/* Pixmap format */
    xwdval pixmap_depth;	/* Pixmap depth */
    xwdval pixmap_width;	/* Pixmap width */
    xwdval pixmap_height;	/* Pixmap height */
    xwdval xoffset;		/* Bitmap x offset */
    xwdval byte_order;		/* MSBFirst, LSBFirst */
    xwdval bitmap_unit;		/* Bitmap unit */
    xwdval bitmap_bit_order;	/* MSBFirst, LSBFirst */
    xwdval bitmap_pad;		/* Bitmap scanline pad */
    xwdval bits_per_pixel;	/* Bits per pixel */
    xwdval bytes_per_line;	/* Bytes per scanline */
    xwdval visual_class;	/* Class of colormap */
    xwdval red_mask;		/* Z red mask */
    xwdval green_mask;		/* Z green mask */
    xwdval blue_mask;		/* Z blue mask */
    xwdval bits_per_rgb;	/* Log base 2 of distinct color values */
    xwdval colormap_entries;	/* Number of entries in colormap */
    xwdval ncolors;		/* Number of Color structures */
    xwdval window_width;	/* Window width */
    xwdval window_height;	/* Window height */
    long window_x;		/* Window upper left X coordinate */
    long window_y;		/* Window upper left Y coordinate */
    xwdval window_bdrwidth;	/* Window border width */
    } X11WDFileHeader;

typedef struct {
    unsigned long num;
    unsigned short red, green, blue;
    char flags;			/* do_red, do_green, do_blue */
    char pad;
    } X11XColor;

#endif /*_X11WD_H_*/
