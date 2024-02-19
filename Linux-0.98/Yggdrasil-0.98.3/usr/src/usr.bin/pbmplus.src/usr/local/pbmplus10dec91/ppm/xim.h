/* xim.h - header file for Xim files
**
** Taken from the X.V11R4 version of XimHeader.h:
**
** Author: Philip R. Thompson
** Address:  phils@athena.mit.edu, 9-526 
** Note:  size of header should be 1024 (1K) bytes.
** $Header: /mit/phils/X/RCS/XimHeader.h,v 1.7 89/11/09 17:26:54 phils Exp Locker: phils $
** $Date: 89/11/09 17:26:54 $
** $Source: /mit/phils/X/RCS/XimHeader.h,v $
*/

#define IMAGE_VERSION    3
#ifndef _BYTE
typedef unsigned char  byte;
#define _BYTE  1
#endif

/* External ascii file format. */
typedef struct ImageHeader {
    char file_version[8];   /* header version */
    char header_size[8];    /* Size of file header in bytes  */
    char image_width[8];    /* Width of the raster image */
    char image_height[8];   /* Height of the raster imgage */
    char num_colors[8];     /* Actual number of entries in c_map */
    char num_channels[3];   /* 0 or 1 = pixmap, 3 = RG&B buffers */
    char bytes_per_line[5]; /* bytes per scanline */
    char num_pictures[4];   /* Number of pictures in file */
    char bits_per_channel[4]; /* usually 1 or 8 */
    char alpha_channel[4];  /* Alpha channel flag */
    char runlength[4];      /* Runlength encoded flag */
    char author[48];        /* Name of who made it */
    char date[32];          /* Date and time image was made */
    char program[16];       /* Program that created this file */
    char comment[96];       /* other viewing info. for this image */
    unsigned char c_map[256][3]; /* RGB values of the pixmap indices */
} ImageHeader, XimAsciiHeader;


/* Internal binary format. */
typedef struct Color {
    byte pixel, red, grn, blu;
} Color;

typedef struct XimImage {
    int width;             /* width of the image in pixels */
    int height;            /* height of the image in pixels */
    unsigned datasize;     /* size of one channel of data */
    short nchannels;     /* number data channels in image */
    short bits_channel;  /* usually 1 or 8 */
    short bytes_per_line; /* bytes to hold one scanline */
    byte* data;            /* pixmap or red channel data */
    byte* grn_data;        /* green channel data */
    byte* blu_data;        /* blue  channel data */
    byte* other;           /* other (alpha) data */
    unsigned alpha_flag :1; /* alpha channel flag */
    unsigned packed_flag:1; /* data packed in one chunk of memory */
    unsigned runlen_flag:1; /* runlength encoded data flag */
    unsigned : 0;           /* future flags, word alignment */
    short tpics, npics;    /* number of images, total & left in file */
    short ncolors;         /*   "    "  colors in the color table */
    Color* colors;         /* colortable, one byte per r/g/b & pixel */
    char* author;         /* author credit, copyright, etc */
    char* date;           /* date image was made, grabbed, etc. */
    char* program;        /* program used to make this */
    short ncomments;       /* number of comments strings */
    char** comments;      /* pointers to null terminated strings */
    char* offset;         /* original offset in machine memory */
    float chroma_red[2];   /* x, y image chromacity coords */
    float chroma_grn[2];
    float chroma_blu[2];
    float chroma_wht[2];
    float gamma;           /* image storage gamma */
} XimImage;

/* Future external ascii variable length header - under review. */
#if (VERSION == 4)
typedef struct XimAsciiHeader {
    char file_version[4];   /* header version */
    char header_size[8];    /* Size of file header (fixed part only) */
    char image_height[8];   /* Height of the raster imgage in pixels */
    char image_width[8];    /* Width of the raster image in pixels */
    char bytes_line[8];     /* Actual # of bytes separating scanlines */
    char bits_channel[4];   /* Bits per channel (usually 1 or 8) */
    char num_channels[4];   /* 1 = pixmap, 3 = RG&B buffers */
    char alpha_channel[2];  /* Alpha channel flag */
    char num_colors[4];     /* Number of entries in c_map (if any) */
    char num_pictures[4];   /* Number of images in file */
    char runlength_flag[2]; /* Runlength encoded flag */
    char future_flags[8];
    char author[48];        /* Name of who made it, from passwd entry */
    char date[32];          /* Unix format date */
    char program[32];       /* Program that created this */
    char gamma[12];         /* image storage gamma */
    char chroma_red[24];    /* image red primary chromaticity coords. */
    char chroma_grn[24];    /*   "   green "          "         "     */
    char chroma_blu[24];    /*   "   blue  "          "         "     */
    char chroma_wht[24];    /*   "   white point      "         "     */
    char comment_length[8]  /* Total length of comments */
    /* char* comment;           Null separated comments  */
    /* unsigned char c_map[];   RGB Colortable, (ncolors * 3 bytes) */
}  XimAsciiHeader;
#endif /*VERSION 4*/

#ifndef rnd
#define rnd(x)  ((int)((float)(x) + 0.5)) /* round a float to an int */
#endif

/* Note:
* - All data is in char's in order to maintain easily portability
*   across machines, and some human readibility.
* - Images may be stored as pixmaps (8 bits/pixel) or as seperate
*   red, green, blue channel data (24+ bits/pixel).
* - An alpha channel is optional and is found after every num_channels
*   of data.
* - Pixmaps or RGB (and alpha) channel data are stored respectively
*   after the header.
* - If num_channels = 1, a pixmap is assumed and the colormap in the
*   header is used.
* - Data size = image_width * image_height.
*/
