/* $Header: /usr/people/sam/tiff/libtiff/RCS/tiffioP.h,v 1.13 91/08/19 14:40:23 sam Exp $ */

/*
 * Copyright (c) 1988, 1989, 1990, 1991 Sam Leffler
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

#ifndef _TIFFIOP_
#define	_TIFFIOP_
/*
 * ``Library-private'' definitions.
 */
#include "tiffcompat.h"
#include "tiff.h"

/*
 * Internal format of a TIFF directory entry.
 */
typedef	struct {
	u_long	td_imagewidth, td_imagelength, td_imagedepth;
	u_long	td_tilewidth, td_tilelength, td_tiledepth;
	u_short	td_subfiletype;
	u_short	td_bitspersample;
	u_short	td_datatype;
	u_short	td_compression;
	u_short	td_photometric;
	u_short	td_threshholding;
	u_short	td_fillorder;
	u_short	td_orientation;
	u_short	td_samplesperpixel;
	u_short	td_predictor;
	u_long	td_rowsperstrip;
	u_long	td_minsamplevalue, td_maxsamplevalue;	/* maybe float? */
	float	td_xresolution, td_yresolution;
	u_short	td_resolutionunit;
	u_short	td_planarconfig;
	float	td_xposition, td_yposition;
	u_long	td_group3options;
	u_long	td_group4options;
	u_short	td_pagenumber[2];
	u_short	td_grayresponseunit;
	u_short	td_colorresponseunit;
	u_short	td_matteing;
	u_short	td_inkset;
	u_short	td_cleanfaxdata;
	u_short	td_badfaxrun;
	u_long	td_badfaxlines;
	u_short	*td_grayresponsecurve;
	u_short	*td_redresponsecurve;
	u_short	*td_greenresponsecurve;
	u_short	*td_blueresponsecurve;
	u_short	*td_redcolormap;
	u_short	*td_greencolormap;
	u_short	*td_bluecolormap;
	char	*td_documentname;
	char	*td_artist;
	char	*td_datetime;
	char	*td_hostcomputer;
	char	*td_imagedescription;
	char	*td_make;
	char	*td_model;
	char	*td_software;
	char	*td_pagename;
	u_long	td_fieldsset[2];	/* bit vector of fields that are set */
	u_long	td_stripsperimage;
	u_long	td_nstrips;		/* size of offset & bytecount arrays */
	u_long	*td_stripoffset;
	u_long	*td_stripbytecount;
#ifdef JPEG_SUPPORT
	u_short	td_jpegproc;
	u_short	td_jpegprec;
	u_short	**td_qtab;
	u_char	**td_dctab;
	u_char	**td_actab;
#endif
} TIFFDirectory;

/*
 * Field flags used to indicate fields that have
 * been set in a directory, and to reference fields
 * when manipulating a directory.
 */
/* multi-entry fields */
#define	FIELD_IMAGEDIMENSIONS		0
#define FIELD_TILEDIMENSIONS		1
#define	FIELD_CELLDIMENSIONS		2		/* XXX */
#define	FIELD_RESOLUTION		3
#define	FIELD_POSITION			4
/* single-entry fields */
#define	FIELD_SUBFILETYPE		5
#define	FIELD_BITSPERSAMPLE		6
#define	FIELD_COMPRESSION		7
#define	FIELD_PHOTOMETRIC		8
#define	FIELD_THRESHHOLDING		9
#define	FIELD_FILLORDER			10
#define	FIELD_DOCUMENTNAME		11
#define	FIELD_IMAGEDESCRIPTION		12
#define	FIELD_MAKE			13
#define	FIELD_MODEL			14
#define	FIELD_ORIENTATION		15
#define	FIELD_SAMPLESPERPIXEL		16
#define	FIELD_ROWSPERSTRIP		17
#define	FIELD_MINSAMPLEVALUE		18
#define	FIELD_MAXSAMPLEVALUE		19
#define	FIELD_PLANARCONFIG		20
#define	FIELD_PAGENAME			21
#define	FIELD_GRAYRESPONSEUNIT		22
#define	FIELD_GRAYRESPONSECURVE		23
#define	FIELD_GROUP3OPTIONS		24
#define	FIELD_GROUP4OPTIONS		25
#define	FIELD_RESOLUTIONUNIT		26
#define	FIELD_PAGENUMBER		27
#define	FIELD_COLORRESPONSEUNIT		28
#define	FIELD_COLORRESPONSECURVE	29
#define	FIELD_STRIPBYTECOUNTS		30
#define	FIELD_STRIPOFFSETS		31
#define	FIELD_COLORMAP			32
#define FIELD_PREDICTOR			33
#define FIELD_ARTIST			34
#define FIELD_DATETIME			35
#define FIELD_HOSTCOMPUTER		36
#define FIELD_SOFTWARE			37
#define	FIELD_MATTEING			38
#define	FIELD_BADFAXLINES		39
#define	FIELD_CLEANFAXDATA		40
#define	FIELD_BADFAXRUN			41
#define FIELD_DATATYPE			42
#define FIELD_IMAGEDEPTH		43
#define FIELD_TILEDEPTH			44
#define FIELD_INKSET			45
#define	FIELD_LUMACOEFS			46
#define FIELD_YCBCRSAMPLING		47
#define FIELD_JPEGPROC			48
#define FIELD_JPEGQTABLEPREC		49
#define FIELD_JPEGQTABLES		50
#define FIELD_JPEGDCTABLES		51
#define FIELD_JPEGACTABLES		52
#define	FIELD_LAST			52

#define BITFIELDn(tif, n)		((tif)->tif_dir.td_fieldsset[(n)/32]) 
#define BITn(n)				(((unsigned)1L)<<((n)&0x1f)) 
#define TIFFFieldSet(tif, field)	(BITFIELDn(tif, field) & BITn(field)) 
#define TIFFSetFieldBit(tif, field)	(BITFIELDn(tif, field) |= BITn(field))
#define TIFFClrFieldBit(tif, field)	(BITFIELDn(tif, field) &= ~BITn(field))

struct tiff {
	char	*tif_name;		/* name of open file */
	short	tif_fd;			/* open file descriptor */
	short	tif_mode;		/* open mode (O_*) */
	char	tif_fillorder;		/* natural bit fill order for machine */
	char	tif_options;		/* compression-specific options */
	short	tif_flags;
#define	TIFF_DIRTYHEADER	0x1	/* header must be written on close */
#define	TIFF_DIRTYDIRECT	0x2	/* current directory must be written */
#define	TIFF_BUFFERSETUP	0x4	/* data buffers setup */
#define	TIFF_BEENWRITING	0x8	/* written 1+ scanlines to file */
#define	TIFF_SWAB		0x10	/* byte swap file information */
#define	TIFF_NOBITREV		0x20	/* inhibit bit reversal logic */
#define	TIFF_MYBUFFER		0x40	/* my raw data buffer; free on close */
#define	TIFF_ISTILED		0x80	/* file is tile, not strip- based */
#define	TIFF_MAPPED		0x100	/* file is mapped into memory */
	long	tif_diroff;		/* file offset of current directory */
	long	tif_nextdiroff;		/* file offset of following directory */
	TIFFDirectory tif_dir;		/* internal rep of current directory */
	TIFFHeader tif_header;		/* file's header block */
	int	tif_typeshift[6];	/* data type shift counts */
	long	tif_typemask[6];	/* data type masks */
	long	tif_row;		/* current scanline */
	int	tif_curstrip;		/* current strip for read/write */
	long	tif_curoff;		/* current offset for read/write */
/* tiling support */
	long 	tif_col;		/* current column (offset by row too) */
	int 	tif_curtile;		/* current tile for read/write */
	long 	tif_tilesize;		/* # of bytes in a tile */
/* compression scheme hooks */
	int	(*tif_predecode)();	/* pre row/strip/tile decoding */
	int	(*tif_preencode)();	/* pre row/strip/tile encoding */
	int	(*tif_postencode)();	/* post row/strip/tile encoding */
	int	(*tif_decoderow)();	/* scanline decoding routine */
	int	(*tif_encoderow)();	/* scanline encoding routine */
	int	(*tif_decodestrip)();	/* strip decoding routine */
	int	(*tif_encodestrip)();	/* strip encoding routine */
	int	(*tif_decodetile)();	/* tile decoding routine */
	int	(*tif_encodetile)();	/* tile encoding routine */
	int	(*tif_close)();		/* cleanup-on-close routine */
	int	(*tif_seek)();		/* position within a strip routine */
	int	(*tif_cleanup)();	/* routine called to cleanup state */
	char	*tif_data;		/* compression scheme private data */
/* input/output buffering */
	int	tif_scanlinesize;	/* # of bytes in a scanline */
	int	tif_scanlineskew;	/* scanline skew for reading strips */
	char	*tif_rawdata;		/* raw data buffer */
	long	tif_rawdatasize;	/* # of bytes in raw data buffer */
	char	*tif_rawcp;		/* current spot in raw buffer */
	long	tif_rawcc;		/* bytes unread from raw buffer */
#ifdef MMAP_SUPPORT
	char*	tif_base;		/* base of mapped file */
	long	tif_size;		/* size of mapped file region (bytes) */
#endif
};

#define	isTiled(tif)	(((tif)->tif_flags & TIFF_ISTILED) != 0)
#ifdef MMAP_SUPPORT
#define	isMapped(tif)	(((tif)->tif_flags & TIFF_MAPPED) != 0)
#else
#define	isMapped(tif)	0		/* force dead code */
#endif

/* generic option bit names */
#define	TIFF_OPT0	0x1
#define	TIFF_OPT1	0x2
#define	TIFF_OPT2	0x4
#define	TIFF_OPT3	0x8
#define	TIFF_OPT4	0x10
#define	TIFF_OPT5	0x20
#define	TIFF_OPT6	0x40
#define	TIFF_OPT7	0x80

#include "tiffio.h"

/* NB: the u_int casts are to silence certain ANSI-C compilers */
#ifdef howmany
#undef howmany
#endif
#define	howmany(x, y)	((((u_int)(x))+(((u_int)(y))-1))/((u_int)(y)))
#ifdef roundup
#undef roundup
#endif
#define	roundup(x, y)	(howmany(x,y)*((u_int)(y)))

#if defined(c_plusplus) || defined(__cplusplus) || defined(__STDC__) || USE_PROTOTYPES
#if defined(__cplusplus)
extern "C" {
#endif
extern	int TIFFNoEncode(TIFF*, u_char*, int, u_int);
extern	int TIFFNoDecode(TIFF*, u_char*, int, u_int);
#if defined(__cplusplus)
}
#endif
#else
extern	int TIFFNoEncode();
extern	int TIFFNoDecode();
#endif
#endif /* _TIFFIOP_ */
