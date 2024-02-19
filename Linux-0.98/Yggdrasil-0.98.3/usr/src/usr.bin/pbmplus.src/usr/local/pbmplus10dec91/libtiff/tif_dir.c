#ifndef lint
static char rcsid[] = "$Header: /usr/people/sam/tiff/libtiff/RCS/tif_dir.c,v 1.86 91/08/19 14:40:43 sam Exp $";
#endif

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

/*
 * TIFF Library.
 *
 * Directory Support Routines.
 *
 * NB: Beware of the varargs declarations for routines in
 *     this file.  The names and types of variables has been
 *     carefully chosen to make things work with compilers that
 *     are busted in one way or another (e.g. SGI/MIPS).
 */
#include "tiffioP.h"

#ifndef TRUE
#define	TRUE	1
#define	FALSE	0
#endif
#define	TIFF_ANY	TIFF_NOTYPE

#define	NITEMS(a)	(sizeof (a) / sizeof  (a[0]))

#define	FieldSet(fields, f)		(fields[f/32] & (1L<<(f&0x1f)))
#define	ResetFieldBit(fields, f)	(fields[f/32] &= ~(1L<<(f&0x1f)))

#define	TIFFExtractData(tif, type, v) \
    ((tif)->tif_header.tiff_magic == TIFF_BIGENDIAN ? \
        ((v) >> (tif)->tif_typeshift[type]) & (tif)->tif_typemask[type] : \
	(v) & (tif)->tif_typemask[type])
#define	TIFFInsertData(tif, type, v) \
    ((tif)->tif_header.tiff_magic == TIFF_BIGENDIAN ? \
        ((v) & (tif)->tif_typemask[type]) << (tif)->tif_typeshift[type] : \
	(v) & (tif)->tif_typemask[type])

typedef	struct {
	u_short	field_tag;		/* field's tag */
	short	field_count;		/* write count (-1 for unknown) */
	TIFFDataType field_type;	/* type of associated data */
	u_short	field_bit;		/* bit in fieldsset bit vector */
	u_short	field_oktochange;	/* if true, can change while writing */
	char	*field_name;		/* ASCII name */
} TIFFFieldInfo;

/*
 * NB: THIS ARRAY IS ASSUMED TO BE SORTED BY TAG.
 *     Also, if a tag can have both LONG and SHORT types
 *     then the LONG must be placed before the SHORT.
 */
static	TIFFFieldInfo FieldInfo[] = {
    { TIFFTAG_SUBFILETYPE,	 1, TIFF_LONG,	FIELD_SUBFILETYPE,
      TRUE,	"SubfileType" },
#ifdef SUBFILE_COMPAT
    { TIFFTAG_SUBFILETYPE,	 1, TIFF_SHORT,	FIELD_SUBFILETYPE,
      FALSE,	"SubfileType" },
#endif
    { TIFFTAG_OSUBFILETYPE,	 1, TIFF_SHORT,	FIELD_SUBFILETYPE,
      FALSE,	"OldSubfileType" },
    { TIFFTAG_IMAGEWIDTH,	 1, TIFF_LONG,	FIELD_IMAGEDIMENSIONS,
      FALSE,	"ImageWidth" },
    { TIFFTAG_IMAGEWIDTH,	 1, TIFF_SHORT,	FIELD_IMAGEDIMENSIONS,
      FALSE,	"ImageWidth" },
    { TIFFTAG_IMAGELENGTH,	 1, TIFF_LONG,	FIELD_IMAGEDIMENSIONS,
      TRUE,	"ImageLength" },
    { TIFFTAG_IMAGELENGTH,	 1, TIFF_SHORT,	FIELD_IMAGEDIMENSIONS,
      TRUE,	"ImageLength" },
    { TIFFTAG_BITSPERSAMPLE,	-1, TIFF_SHORT,	FIELD_BITSPERSAMPLE,
      FALSE,	"BitsPerSample" },
    { TIFFTAG_COMPRESSION,	 1, TIFF_SHORT,	FIELD_COMPRESSION,
      FALSE,	"Compression" },
    { TIFFTAG_PHOTOMETRIC,	 1, TIFF_SHORT,	FIELD_PHOTOMETRIC,
      TRUE,	"PhotometricInterpretation" },
    { TIFFTAG_THRESHHOLDING,	 1, TIFF_SHORT,	FIELD_THRESHHOLDING,
      TRUE,	"Threshholding" },
    { TIFFTAG_CELLWIDTH,	 1, TIFF_SHORT,	(u_short)-1,
      TRUE,	"CellWidth" },
    { TIFFTAG_CELLLENGTH,	 1, TIFF_SHORT,	(u_short)-1,
      TRUE,	"CellLength" },
    { TIFFTAG_FILLORDER,	 1, TIFF_SHORT,	FIELD_FILLORDER,
      FALSE,	"FillOrder" },
    { TIFFTAG_DOCUMENTNAME,	-1, TIFF_ASCII,	FIELD_DOCUMENTNAME,
      TRUE,	"DocumentName" },
    { TIFFTAG_IMAGEDESCRIPTION,	-1, TIFF_ASCII,	FIELD_IMAGEDESCRIPTION,
      TRUE,	"ImageDescription" },
    { TIFFTAG_MAKE,		-1, TIFF_ASCII,	FIELD_MAKE,
      TRUE,	"Make" },
    { TIFFTAG_MODEL,		-1, TIFF_ASCII,	FIELD_MODEL,
      TRUE,	"Model" },
    { TIFFTAG_STRIPOFFSETS,	-1, TIFF_LONG,	FIELD_STRIPOFFSETS,
      FALSE,	"StripOffsets" },
    { TIFFTAG_STRIPOFFSETS,	-1, TIFF_SHORT,	FIELD_STRIPOFFSETS,
      FALSE,	"StripOffsets" },
    { TIFFTAG_ORIENTATION,	 1, TIFF_SHORT,	FIELD_ORIENTATION,
      TRUE,	"Orientation" },
    { TIFFTAG_SAMPLESPERPIXEL,	 1, TIFF_SHORT,	FIELD_SAMPLESPERPIXEL,
      FALSE,	"SamplesPerPixel" },
    { TIFFTAG_ROWSPERSTRIP,	 1, TIFF_LONG,	FIELD_ROWSPERSTRIP,
      FALSE,	"RowsPerStrip" },
    { TIFFTAG_ROWSPERSTRIP,	 1, TIFF_SHORT,	FIELD_ROWSPERSTRIP,
      FALSE,	"RowsPerStrip" },
    { TIFFTAG_STRIPBYTECOUNTS,	-1, TIFF_LONG,	FIELD_STRIPBYTECOUNTS,
      FALSE,	"StripByteCounts" },
    { TIFFTAG_STRIPBYTECOUNTS,	-1, TIFF_SHORT,	FIELD_STRIPBYTECOUNTS,
      FALSE,	"StripByteCounts" },
    { TIFFTAG_MINSAMPLEVALUE,	-1, TIFF_SHORT,	FIELD_MINSAMPLEVALUE,
      TRUE,	"MinSampleValue" },
    { TIFFTAG_MAXSAMPLEVALUE,	-1, TIFF_SHORT,	FIELD_MAXSAMPLEVALUE,
      TRUE,	"MaxSampleValue" },
    { TIFFTAG_XRESOLUTION,	 1, TIFF_RATIONAL,FIELD_RESOLUTION,
      TRUE,	"XResolution" },
    { TIFFTAG_YRESOLUTION,	 1, TIFF_RATIONAL,FIELD_RESOLUTION,
      TRUE,	"YResolution" },
    { TIFFTAG_PLANARCONFIG,	 1, TIFF_SHORT,	FIELD_PLANARCONFIG,
      FALSE,	"PlanarConfiguration" },
    { TIFFTAG_PAGENAME,		-1, TIFF_ASCII,	FIELD_PAGENAME,
      TRUE,	"PageName" },
    { TIFFTAG_XPOSITION,	 1, TIFF_RATIONAL,FIELD_POSITION,
      TRUE,	"XPosition" },
    { TIFFTAG_YPOSITION,	 1, TIFF_RATIONAL,FIELD_POSITION,
      TRUE,	"YPosition" },
    { TIFFTAG_FREEOFFSETS,	-1, TIFF_LONG,	(u_short)-1,
      FALSE,	"FreeOffsets" },
    { TIFFTAG_FREEBYTECOUNTS,	-1, TIFF_LONG,	(u_short)-1,
      FALSE,	"FreeByteCounts" },
    { TIFFTAG_GRAYRESPONSEUNIT,	 1, TIFF_SHORT,	FIELD_GRAYRESPONSEUNIT,
      TRUE,	"GrayResponseUnit" },
    { TIFFTAG_GRAYRESPONSECURVE,-1, TIFF_SHORT,	FIELD_GRAYRESPONSECURVE,
      TRUE,	"GrayResponseCurve" },
    { TIFFTAG_GROUP3OPTIONS,	 1, TIFF_LONG,	FIELD_GROUP3OPTIONS,
      FALSE,	"Group3Options" },
    { TIFFTAG_GROUP4OPTIONS,	 1, TIFF_LONG,	FIELD_GROUP4OPTIONS,
      FALSE,	"Group4Options" },
    { TIFFTAG_RESOLUTIONUNIT,	 1, TIFF_SHORT,	FIELD_RESOLUTIONUNIT,
      TRUE,	"ResolutionUnit" },
    { TIFFTAG_PAGENUMBER,	 2, TIFF_SHORT,	FIELD_PAGENUMBER,
      TRUE,	"PageNumber" },
    { TIFFTAG_COLORRESPONSEUNIT, 1, TIFF_SHORT,	FIELD_COLORRESPONSEUNIT,
      TRUE,	"ColorResponseUnit" },
    { TIFFTAG_COLORRESPONSECURVE,-1,TIFF_SHORT,	FIELD_COLORRESPONSECURVE,
      TRUE,	"ColorResponseCurve" },
    { TIFFTAG_SOFTWARE,		-1, TIFF_ASCII,	FIELD_SOFTWARE,
      TRUE,	"Software" },
    { TIFFTAG_DATETIME,		-1, TIFF_ASCII,	FIELD_DATETIME,
      TRUE,	"DateTime" },
    { TIFFTAG_ARTIST,		-1, TIFF_ASCII,	FIELD_ARTIST,
      TRUE,	"Artist" },
    { TIFFTAG_HOSTCOMPUTER,	-1, TIFF_ASCII,	FIELD_HOSTCOMPUTER,
      TRUE,	"HostComputer" },
    { TIFFTAG_PREDICTOR,	 1, TIFF_SHORT,	FIELD_PREDICTOR,
      FALSE,	"Predictor" },
    { TIFFTAG_COLORMAP,		 1, TIFF_SHORT,	FIELD_COLORMAP,
      TRUE,	"ColorMap" },
    { TIFFTAG_TILEWIDTH,	 1, TIFF_LONG,	FIELD_TILEDIMENSIONS,
      FALSE,	"TileWidth" },
    { TIFFTAG_TILEWIDTH,	 1, TIFF_SHORT,	FIELD_TILEDIMENSIONS,
      FALSE,	"TileWidth" },
    { TIFFTAG_TILELENGTH,	 1, TIFF_LONG,	FIELD_TILEDIMENSIONS,
      FALSE,	"TileLength" },
    { TIFFTAG_TILELENGTH,	 1, TIFF_SHORT,	FIELD_TILEDIMENSIONS,
      FALSE,	"TileLength" },
    { TIFFTAG_TILEOFFSETS,	1, TIFF_LONG,	FIELD_STRIPOFFSETS,
      FALSE,	"TileOffsets" },
    { TIFFTAG_TILEBYTECOUNTS,	1, TIFF_LONG,	FIELD_STRIPBYTECOUNTS,
      FALSE,	"TileByteCounts" },
    { TIFFTAG_TILEBYTECOUNTS,	1, TIFF_SHORT,	FIELD_STRIPBYTECOUNTS,
      FALSE,	"TileByteCounts" },
    { TIFFTAG_BADFAXLINES,	 1, TIFF_LONG,	FIELD_BADFAXLINES,
      TRUE,	"BadFaxLines" },
    { TIFFTAG_BADFAXLINES,	 1, TIFF_SHORT,	FIELD_BADFAXLINES,
      TRUE,	"BadFaxLines" },
    { TIFFTAG_CLEANFAXDATA,	 1, TIFF_SHORT,	FIELD_CLEANFAXDATA,
      TRUE,	"CleanFaxData" },
    { TIFFTAG_CONSECUTIVEBADFAXLINES,1, TIFF_LONG,FIELD_BADFAXRUN,
      TRUE,	"ConsecutiveBadFaxLines" },
    { TIFFTAG_CONSECUTIVEBADFAXLINES,1, TIFF_SHORT,FIELD_BADFAXRUN,
      TRUE,	"ConsecutiveBadFaxLines" },
    { TIFFTAG_INKSET,		1, TIFF_SHORT,	FIELD_INKSET,
      FALSE,	"InkSet" },
/* begin SGI tags */
    { TIFFTAG_MATTEING,		1, TIFF_SHORT,	FIELD_MATTEING,
      FALSE,	"Matteing" },
    { TIFFTAG_DATATYPE,		1, TIFF_SHORT,	FIELD_DATATYPE,
      FALSE,	"DataType" },
    { TIFFTAG_IMAGEDEPTH,	1, TIFF_LONG,	FIELD_IMAGEDEPTH,
      FALSE,	"ImageDepth" },
    { TIFFTAG_IMAGEDEPTH,	1, TIFF_SHORT,	FIELD_IMAGEDEPTH,
      FALSE,	"ImageDepth" },
    { TIFFTAG_TILEDEPTH,	1, TIFF_LONG,	FIELD_TILEDEPTH,
      FALSE,	"TileDepth" },
    { TIFFTAG_TILEDEPTH,	1, TIFF_SHORT,	FIELD_TILEDEPTH,
      FALSE,	"TileDepth" },
/* end SGI tags */
#ifdef JPEG_SUPPORT		/* C-Cube JPEG tags */
    { TIFFTAG_JPEGPROC,		1, TIFF_SHORT,	FIELD_JPEGPROC,
      FALSE,	"JPEGProc" },
    { TIFFTAG_JPEGQTABLEPREC,	1, TIFF_SHORT,	FIELD_JPEGQTABLEPREC,
      FALSE,	"JPEGQTablePrecision" },
    { TIFFTAG_JPEGQTABLES,	-1, TIFF_LONG,	FIELD_JPEGQTABLES,
      FALSE,	"JPEGQTables" },
    { TIFFTAG_JPEGDCTABLES,	-1, TIFF_LONG,	FIELD_JPEGDCTABLES,
      FALSE,	"JPEGDCTables" },
    { TIFFTAG_JPEGACTABLES,	-1, TIFF_LONG,	FIELD_JPEGACTABLES,
      FALSE,	"JPEGACTables" },
#endif
};
#define	NFIELDINFO	NITEMS(FieldInfo)

#define	IGNORE	0		/* tag placeholder used below */

static int datawidth[] = {
    1,	/* nothing */
    1,	/* TIFF_BYTE */
    1,	/* TIFF_ASCII */
    2,	/* TIFF_SHORT */
    4,	/* TIFF_LONG */
    8,	/* TIFF_RATIONAL */
};

#include "prototypes.h"
#if USE_PROTOTYPES
static	EstimateStripByteCounts(TIFF *, TIFFDirEntry *, u_int);
static	MissingRequired(TIFF *, char *);
static	TIFFFetchData(TIFF *, TIFFDirEntry *, char *);
static	TIFFFetchString(TIFF *, TIFFDirEntry *, char *);
static	double TIFFFetchRational(TIFF *, TIFFDirEntry *);
static	TIFFUnpackShorts(TIFF *, u_short [], TIFFDirEntry *);
static	TIFFFetchPerSampleShorts(TIFF *, TIFFDirEntry *, long *);
static	TIFFFetchStripThing(TIFF *, TIFFDirEntry *, long, u_long **);
static	TIFFFetchJPEGQTables(TIFF *, TIFFDirEntry *);
static	TIFFFetchJPEGCTables(TIFF *, TIFFDirEntry *, u_char ***);
static	TIFFFieldInfo *FindFieldInfo(u_short, TIFFDataType);
static	TIFFFieldInfo *FieldWithTag(u_short);
static	TIFFGetField1(TIFFDirectory *, int, va_list);
extern	int TIFFSetCompressionScheme(TIFF *, int);
extern	int TIFFDefaultDirectory(TIFF*);
extern	int TIFFFreeDirectory(TIFF*);
#else
static	EstimateStripByteCounts();
static	MissingRequired();
static	TIFFFetchData();
static	TIFFFetchString();
static	double TIFFFetchRational();
static	TIFFUnpackShorts();
static	TIFFFetchPerSampleShorts();
static	TIFFFetchStripThing();
static	TIFFFetchJPEGQTables();
static	TIFFFetchJPEGCTables();
static	TIFFFieldInfo *FindFieldInfo();
static	TIFFFieldInfo *FieldWithTag();
static	TIFFGetField1();
extern	int TIFFSetCompressionScheme();
extern	int TIFFDefaultDirectory();
extern	int TIFFFreeDirectory();
#endif

/*
 * Read the next TIFF directory from a file
 * and convert it to the internal format.
 * We read directories sequentially.
 */
TIFFReadDirectory(tif)
	TIFF *tif;
{
	register TIFFDirEntry *dp;
	register int n;
	register TIFFDirectory *td = NULL;
	TIFFDirEntry *dir = NULL;
	long v;
	TIFFFieldInfo *fip;
	u_short dircount;
	char *cp;
	int diroutoforderwarning = 0, ok;

	tif->tif_diroff = tif->tif_nextdiroff;
	if (tif->tif_diroff == 0)		/* no more directories */
		return (0);
	if (!isMapped(tif)) {
		if (!SeekOK(tif->tif_fd, tif->tif_diroff)) {
			TIFFError(tif->tif_name,
			    "Seek error accessing TIFF directory");
			return (0);
		}
		if (!ReadOK(tif->tif_fd, &dircount, sizeof (short))) {
			TIFFError(tif->tif_name,
			    "Can not read TIFF directory count");
			return (0);
		}
		if (tif->tif_flags & TIFF_SWAB)
			TIFFSwabShort(&dircount);
		dir = (TIFFDirEntry *)malloc(dircount * sizeof (TIFFDirEntry));
		if (dir == NULL) {
			TIFFError(tif->tif_name,
			    "No space to read TIFF directory");
			return (0);
		}
		if (!ReadOK(tif->tif_fd, dir, dircount*sizeof (TIFFDirEntry))) {
			TIFFError(tif->tif_name, "Can not read TIFF directory");
			goto bad;
		}
		/*
		 * Read offset to next directory for sequential scans.
		 */
		if (!ReadOK(tif->tif_fd, &tif->tif_nextdiroff, sizeof (long)))
			tif->tif_nextdiroff = 0;
#ifdef MMAP_SUPPORT
	} else {
		off_t off = tif->tif_diroff;

		if (off + sizeof (short) > tif->tif_size) {
			TIFFError(tif->tif_name,
			    "Can not read TIFF directory count");
			return (0);
		} else
			bcopy(tif->tif_base + off, &dircount, sizeof (short));
		off += sizeof (short);
		if (tif->tif_flags & TIFF_SWAB)
			TIFFSwabShort(&dircount);
		dir = (TIFFDirEntry *)malloc(dircount * sizeof (TIFFDirEntry));
		if (dir == NULL) {
			TIFFError(tif->tif_name,
			    "No space to read TIFF directory");
			return (0);
		}
		if (off + dircount*sizeof (TIFFDirEntry) > tif->tif_size) {
			TIFFError(tif->tif_name, "Can not read TIFF directory");
			goto bad;
		} else
			bcopy(tif->tif_base + off, dir,
			    dircount*sizeof (TIFFDirEntry));
		off += dircount* sizeof (TIFFDirEntry);
		if (off + sizeof (long) < tif->tif_size)
			bcopy(tif->tif_base + off, &tif->tif_nextdiroff,
			    sizeof (long));
		else
			tif->tif_nextdiroff = 0;
#endif
	}
	if (tif->tif_flags & TIFF_SWAB)
		TIFFSwabLong((u_long *)&tif->tif_nextdiroff);

	tif->tif_flags &= ~TIFF_BEENWRITING;	/* reset before new dir */
	/*
	 * Setup default value and then make a pass over
	 * the fields to check type and tag information,
	 * and to extract info required to size data
	 * structures.  A second pass is made afterwards
	 * to read in everthing not taken in the first pass.
	 */
	td = &tif->tif_dir;
	if (tif->tif_diroff != tif->tif_header.tiff_diroff) {
		/* free any old stuff and reinit */
		TIFFFreeDirectory(tif);
	}
	TIFFDefaultDirectory(tif);
	/*
	 * Electronic Arts writes gray-scale TIFF files
	 * without a PlanarConfiguration directory entry.
	 * Thus we setup a default value here, even though
	 * the TIFF spec says there is no default value.
	 */
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	for (fip = FieldInfo, dp = dir, n = dircount; n > 0; n--, dp++) {
		if (tif->tif_flags & TIFF_SWAB) {
			TIFFSwabArrayOfShort(&dp->tdir_tag, 2);
			TIFFSwabArrayOfLong(&dp->tdir_count, 2);
		}
		/*
		 * Find the field information entry for this tag.
		 */
		/*
		 * Silicon Beach (at least) writes unordered
		 * directory tags (violating the spec).  Handle
		 * it here, but be obnoxious (maybe they'll
		 * fix it?).
		 */
		if (dp->tdir_tag < fip->field_tag) {
			if (!diroutoforderwarning) {
				TIFFWarning(tif->tif_name,
	"Invalid TIFF directory; tags are not sorted in ascending order");
				diroutoforderwarning = 1;
			}
			fip = FieldInfo;	/* O(n^2) */
		}
		while (fip < &FieldInfo[NFIELDINFO] &&
		    fip->field_tag < dp->tdir_tag)
			fip++;
		if (fip >= &FieldInfo[NFIELDINFO] ||
		    fip->field_tag != dp->tdir_tag) {
			TIFFWarning(tif->tif_name,
			    "ignoring unknown field with tag %d (0x%x)",
			    dp->tdir_tag,  dp->tdir_tag);
			dp->tdir_tag = IGNORE;
			fip = FieldInfo;	/* restart search */
			continue;
		}
		/*
		 * Null out old tags that we ignore.
		 */
		if (fip->field_bit == (u_short)-1) {
			dp->tdir_tag = IGNORE;
			continue;
		}
		/*
		 * Check data type.
		 */
		while (dp->tdir_type != (u_short)fip->field_type) {
			fip++;
			if (fip >= &FieldInfo[NFIELDINFO] ||
			    fip->field_tag != dp->tdir_tag) {
				TIFFError(tif->tif_name,
				    "Wrong data type %d for field \"%s\"",
				    dp->tdir_type, fip[-1].field_name);
				goto bad;
			}
		}
		switch (dp->tdir_tag) {
		case TIFFTAG_STRIPOFFSETS:
		case TIFFTAG_STRIPBYTECOUNTS:
		case TIFFTAG_TILEOFFSETS:
		case TIFFTAG_TILEBYTECOUNTS:
			TIFFSetFieldBit(tif, fip->field_bit);
			break;
		case TIFFTAG_IMAGEWIDTH:
		case TIFFTAG_IMAGELENGTH:
		case TIFFTAG_IMAGEDEPTH:
		case TIFFTAG_TILELENGTH:
		case TIFFTAG_TILEWIDTH:
		case TIFFTAG_TILEDEPTH:
		case TIFFTAG_PLANARCONFIG:
		case TIFFTAG_SAMPLESPERPIXEL:
			if (!TIFFSetField(tif, dp->tdir_tag, (int)
			  TIFFExtractData(tif, dp->tdir_type, dp->tdir_offset)))
				goto bad;
			break;
		case TIFFTAG_ROWSPERSTRIP:
			if (!TIFFSetField(tif, dp->tdir_tag, (long)
			  TIFFExtractData(tif, dp->tdir_type, dp->tdir_offset)))
				goto bad;
			break;
		}
	}

	/*
	 * Allocate directory structure and setup defaults.
	 */
	if (!TIFFFieldSet(tif, FIELD_IMAGEDIMENSIONS)) {
		MissingRequired(tif, "ImageLength");
		goto bad;
	}
	if (!TIFFFieldSet(tif, FIELD_PLANARCONFIG)) {
		MissingRequired(tif, "PlanarConfiguration");
		goto bad;
	}
	/* 
 	 * Setup appropriate structures (by strip or by tile)
	 */
	if (!TIFFFieldSet(tif, FIELD_TILEDIMENSIONS)) {
		td->td_stripsperimage = (td->td_rowsperstrip == 0xffffffff ?
		     (td->td_imagelength != 0 ? 1 : 0) :
		     howmany(td->td_imagelength, td->td_rowsperstrip));
		td->td_tilewidth = td->td_imagewidth;
		td->td_tilelength = td->td_rowsperstrip;
		td->td_tiledepth = td->td_imagedepth;
		tif->tif_flags &= ~TIFF_ISTILED;
	} else {
		td->td_stripsperimage = TIFFNumberOfTiles(tif);
		tif->tif_flags |= TIFF_ISTILED;
	}
	td->td_nstrips = td->td_stripsperimage;
	if (td->td_planarconfig == PLANARCONFIG_SEPARATE)
		td->td_nstrips *= td->td_samplesperpixel;
	if (td->td_nstrips > 0 && !TIFFFieldSet(tif, FIELD_STRIPOFFSETS)) {
		MissingRequired(tif,
		    isTiled(tif) ? "TileOffsets" : "StripOffsets");
		goto bad;
	}

	/*
	 * Second pass: extract other information.
	 *
	 * Should do a better job of verifying values.
	 */
	for (dp = dir, n = dircount; n > 0; n--, dp++) {
		if (dp->tdir_tag == IGNORE)
			continue;
		switch (dp->tdir_tag) {
		case TIFFTAG_COMPRESSION:
			/*
			 * The 5.0 spec says the compression tag has
			 * one value, while earlier specs say it has
			 * one value per sample.  Because of this, we
			 * accept the tag if one value is supplied.
			 */
			if (dp->tdir_count == 1) {
				v = TIFFExtractData(tif,
				    dp->tdir_type, dp->tdir_offset);
				if (!TIFFSetField(tif,
				     TIFFTAG_COMPRESSION, (int)v))
					goto bad;
				break;
			}
			/* fall thru... */
		case TIFFTAG_MINSAMPLEVALUE:
		case TIFFTAG_MAXSAMPLEVALUE:
		case TIFFTAG_BITSPERSAMPLE:
		case TIFFTAG_DATATYPE:
			if (!TIFFFetchPerSampleShorts(tif, dp, &v) ||
			    !TIFFSetField(tif, dp->tdir_tag, (int)v))
				goto bad;
			break;
		case TIFFTAG_STRIPOFFSETS:
		case TIFFTAG_TILEOFFSETS:
			if (!TIFFFetchStripThing(tif, dp,
			    td->td_nstrips, &td->td_stripoffset))
				goto bad;
			TIFFSetFieldBit(tif, FIELD_STRIPOFFSETS);
			break;
		case TIFFTAG_STRIPBYTECOUNTS:
		case TIFFTAG_TILEBYTECOUNTS:
			if (!TIFFFetchStripThing(tif, dp,
			    td->td_nstrips, &td->td_stripbytecount))
				goto bad;
			TIFFSetFieldBit(tif, FIELD_STRIPBYTECOUNTS);
			break;
		case TIFFTAG_IMAGELENGTH:
		case TIFFTAG_ROWSPERSTRIP:
		case TIFFTAG_TILELENGTH:
		case TIFFTAG_TILEWIDTH:
		case TIFFTAG_TILEDEPTH:
		case TIFFTAG_SAMPLESPERPIXEL:
		case TIFFTAG_PLANARCONFIG:
			/* handled in first pass above */
			break;
		case TIFFTAG_GRAYRESPONSECURVE:
		case TIFFTAG_COLORRESPONSECURVE:
		case TIFFTAG_COLORMAP:
			v = (1L<<td->td_bitspersample) * sizeof (u_short);
			cp = malloc(dp->tdir_tag == TIFFTAG_GRAYRESPONSECURVE ? 
				v : 3*v);
			if (cp == NULL)
				goto bad;
			ok = (TIFFFetchData(tif, dp, cp) &&
			    TIFFSetField(tif, dp->tdir_tag, cp,cp+v,cp+2*v));
			free(cp);
			if (!ok)
				goto bad;
			break;
		case TIFFTAG_PAGENUMBER:
			TIFFUnpackShorts(tif, td->td_pagenumber, dp);
			TIFFSetFieldBit(tif, FIELD_PAGENUMBER);
			break;
#ifdef JPEG_SUPPORT
		case TIFFTAG_JPEGQTABLES:
			if (!TIFFFetchJPEGQTables(tif, dp))
				goto bad;
			TIFFSetFieldBit(tif, FIELD_JPEGQTABLES);
			break;
		case TIFFTAG_JPEGDCTABLES:
			if (!TIFFFetchJPEGCTables(tif, dp, &td->td_dctab))
				goto bad;
			TIFFSetFieldBit(tif, FIELD_JPEGDCTABLES);
			break;
		case TIFFTAG_JPEGACTABLES:
			if (!TIFFFetchJPEGCTables(tif, dp, &td->td_actab))
				goto bad;
			TIFFSetFieldBit(tif, FIELD_JPEGACTABLES);
			break;
#endif
/* BEGIN REV 4.0 COMPATIBILITY */
		case TIFFTAG_OSUBFILETYPE:
			v = 0;
			switch (TIFFExtractData(tif, dp->tdir_type,
			    dp->tdir_offset)) {
			case OFILETYPE_REDUCEDIMAGE:
				v = FILETYPE_REDUCEDIMAGE;
				break;
			case OFILETYPE_PAGE:
				v = FILETYPE_PAGE;
				break;
			}
			if (!TIFFSetField(tif, dp->tdir_tag, (int)v))
				goto bad;
			break;
/* END REV 4.0 COMPATIBILITY */
		default:
			switch (dp->tdir_type) {
			case TIFF_BYTE:
			case TIFF_SHORT:
			case TIFF_LONG:
				if (!TIFFSetField(tif, dp->tdir_tag, (int)
			  TIFFExtractData(tif, dp->tdir_type, dp->tdir_offset)))
					goto bad;
				break;
			case TIFF_RATIONAL:
				if (!TIFFSetField(tif, dp->tdir_tag,
				    TIFFFetchRational(tif, dp)))
					goto bad;
				break;
			case TIFF_ASCII:
				if (dp->tdir_count > 0) {
					cp = malloc(dp->tdir_count);
					if (cp == NULL)
						goto bad;
					ok = (TIFFFetchString(tif, dp, cp) &&
					   TIFFSetField(tif, dp->tdir_tag, cp));
					free(cp);
					if (!ok)
						goto bad;
				}
				break;
			}
			break;
		}
	}
	if (!TIFFFieldSet(tif, FIELD_STRIPBYTECOUNTS)) {
		/*
		 * Some manufacturers violate the spec by not giving
		 * the size of the strips.  In this case, assume there
		 * is one uncompressed strip of data.
		 */
		if (td->td_nstrips > 1) {
		    MissingRequired(tif, "StripByteCounts");
		    goto bad;
		}
		TIFFWarning(tif->tif_name,
"TIFF directory is missing required \"%s\" field, calculating from imagelength",
		    FieldWithTag(TIFFTAG_STRIPBYTECOUNTS)->field_name);
		EstimateStripByteCounts(tif, dir, dircount);
	} else if (td->td_nstrips == 1 && td->td_stripbytecount[0] == 0) {
		/*
		 * Plexus (and others) sometimes give a value
		 * of zero for a tag when they don't know what
		 * the correct value is!  Try and handle the
		 * simple case of estimating the size of a one
		 * strip image.
		 */
		TIFFWarning(tif->tif_name,
"Bogus \"%s\" field, ignoring and calculating from imagelength",
		    FieldWithTag(TIFFTAG_STRIPBYTECOUNTS)->field_name);
		EstimateStripByteCounts(tif, dir, dircount);
	}
	if (dir)
		free((char *)dir);
	if (!TIFFFieldSet(tif, FIELD_MAXSAMPLEVALUE))
		td->td_maxsamplevalue = (1L<<td->td_bitspersample)-1;
	/*
	 * Setup default compression scheme.
	 */
	if (!TIFFFieldSet(tif, FIELD_COMPRESSION))
		TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	/*
	 * Reinitialize i/o since we are starting on a new directory.
	 */
	tif->tif_row = -1;
	tif->tif_curstrip = -1;
	tif->tif_col = -1;
	tif->tif_curtile = -1;
	tif->tif_tilesize = TIFFTileSize(tif);
	tif->tif_scanlinesize = TIFFScanlineSize(tif);
	return (1);
bad:
	if (dir)
		free((char *)dir);
	return (0);
}

static
EstimateStripByteCounts(tif, dir, dircount)
	TIFF *tif;
	TIFFDirEntry *dir;
	u_int dircount;
{
	register TIFFDirEntry *dp;
	register TIFFDirectory *td = &tif->tif_dir;
	register int n;

	td->td_stripbytecount = (u_long *)malloc(sizeof (u_long));
	if (td->td_compression != COMPRESSION_NONE) {
		u_long space = sizeof (TIFFHeader)
		    + sizeof (short)
		    + (dircount * sizeof (TIFFDirEntry))
		    + sizeof (long);
		/* calculate amount of space used by indirect values */
		for (dp = dir, n = dircount; n > 0; n--, dp++) {
			int cc = dp->tdir_count * datawidth[dp->tdir_type];
			if (cc > sizeof (long))
				space += cc;
		}
		td->td_stripbytecount[0] = TIFFGetFileSize(tif->tif_fd) - space;
	} else {
		u_long rowbytes = howmany(td->td_bitspersample *
		    td->td_samplesperpixel * td->td_imagewidth, 8);
		td->td_stripbytecount[0] = td->td_imagelength * rowbytes;
	}
	TIFFSetFieldBit(tif, FIELD_STRIPBYTECOUNTS);
	if (!TIFFFieldSet(tif, FIELD_ROWSPERSTRIP))
		td->td_rowsperstrip = td->td_imagelength;
}

static
MissingRequired(tif, tagname)
	TIFF *tif;
	char *tagname;
{
	TIFFError(tif->tif_name,
	    "TIFF directory is missing required \"%s\" field", tagname);
}

/*
 * Fetch a contiguous directory item.
 */
static
TIFFFetchData(tif, dir, cp)
	TIFF *tif;
	TIFFDirEntry *dir;
	char *cp;
{
	int cc, w;

	w = datawidth[dir->tdir_type];
	cc = dir->tdir_count * w;
	if (!isMapped(tif)) {
		if (!SeekOK(tif->tif_fd, dir->tdir_offset))
			goto bad;
		if (!ReadOK(tif->tif_fd, cp, cc))
			goto bad;
#ifdef MMAP_SUPPORT
	} else {
		if (dir->tdir_offset + cc > tif->tif_size)
			goto bad;
		bcopy(tif->tif_base + dir->tdir_offset, cp, cc);
#endif
	}
	if (tif->tif_flags & TIFF_SWAB) {
		switch (dir->tdir_type) {
		case TIFF_SHORT:
			TIFFSwabArrayOfShort((u_short *)cp, dir->tdir_count);
			break;
		case TIFF_LONG:
			TIFFSwabArrayOfLong((u_long *)cp, dir->tdir_count);
			break;
		case TIFF_RATIONAL:
			TIFFSwabArrayOfLong((u_long *)cp, 2*dir->tdir_count);
			break;
		}
	}
	return (cc);
bad:
	TIFFError(tif->tif_name, "Error fetching data for field \"%s\"",
	    FieldWithTag(dir->tdir_tag)->field_name);
	return (0);
}

/*
 * Fetch an ASCII item from the file.
 */
static
TIFFFetchString(tif, dir, cp)
	TIFF *tif;
	TIFFDirEntry *dir;
	char *cp;
{
	if (dir->tdir_count <= 4) {
		u_long l = dir->tdir_offset;
		if (tif->tif_flags & TIFF_SWAB)
			TIFFSwabLong(&l);
		bcopy(&l, cp, dir->tdir_count);
		return (1);
	}
	return (TIFFFetchData(tif, dir, cp));
}

/*
 * Fetch a rational item from the file
 * at offset off.  We return the value
 * as floating point number.
 */
static double
TIFFFetchRational(tif, dir)
	TIFF *tif;
	TIFFDirEntry *dir;
{
	long l[2];

	if (!TIFFFetchData(tif, dir, (char *)l))
		return (1.);
	if (l[1] == 0) {
		TIFFError(tif->tif_name, "%s: Rational with zero denominator",
		    FieldWithTag(dir->tdir_tag)->field_name);
		return (1.);
	}
	return ((double)l[0] / (double)l[1]);
}

/*
 * Unpack a pair of short values from the
 * offset field of a directory entry.
 */
static
TIFFUnpackShorts(tif, v, dir)
	TIFF *tif;
	u_short v[];
	TIFFDirEntry *dir;
{
	if (tif->tif_header.tiff_magic == TIFF_BIGENDIAN) {
		v[0] = dir->tdir_offset >> 16;
		v[1] = dir->tdir_offset & 0xffff;
	} else {
		v[0] = dir->tdir_offset & 0xffff;
		v[1] = dir->tdir_offset >> 16;
	}
}

/*
 * Check the count field of a directory
 * entry against the samples/pixel.
 */
static
CheckSampleCount(tif, dir)
	TIFF *tif;
	TIFFDirEntry *dir;
{
	if (tif->tif_dir.td_samplesperpixel != dir->tdir_count) {
		TIFFError(tif->tif_name, "Incorrect count %d for field \"%s\"",
		    dir->tdir_count, FieldWithTag(dir->tdir_tag)->field_name);
		return (0);
	}
	return (1);
}

/*
 * Fetch samples/pixel short values for 
 * the specified tag and verify that
 * all values are the same.
 */
static
TIFFFetchPerSampleShorts(tif, dir, pl)
	TIFF *tif;
	TIFFDirEntry *dir;
	long *pl;
{
	u_short v[4];
	int i;

	if (!CheckSampleCount(tif, dir))
		return (0);
	switch (dir->tdir_count) {
	case 1:
		*pl = TIFFExtractData(tif, dir->tdir_type, dir->tdir_offset);
		return (1);
	case 2:
		TIFFUnpackShorts(tif, v, dir);
		break;
	default:
		if (!TIFFFetchData(tif, dir, (char *)v))
			return (0);
		break;
	}
	for (i = 1; i < dir->tdir_count; i++)
		if (v[i] != v[0]) {
			TIFFError(tif->tif_name,
	"Cannot handle different per-sample values for field \"%s\"",
			     FieldWithTag(dir->tdir_tag)->field_name);
			return (0);
		}
	*pl = v[0];
	return (1);
}

/*
 * Fetch a set of offsets or lengths.
 * While this routine says "strips",
 * in fact it's also used for tiles.
 */
static
TIFFFetchStripThing(tif, dir, nstrips, lpp)
	TIFF *tif;
	TIFFDirEntry *dir;
	long nstrips;
	u_long **lpp;
{
	register u_long *lp;
	int status;

	if (nstrips != dir->tdir_count) {
		TIFFError(tif->tif_name,
		    "Count mismatch for field \"%s\"; expecting %d, got %d",
		    FieldWithTag(dir->tdir_tag)->field_name, nstrips,
		    dir->tdir_count);
		return (0);
	}
	/*
	 * Allocate space for strip information.
	 */
	if (*lpp == NULL &&
	    (*lpp = (u_long *)malloc(nstrips * sizeof (u_long))) == NULL) {
		TIFFError(tif->tif_name, "No space for \"%s\" array",
		    FieldWithTag(dir->tdir_tag)->field_name);
		return (0);
	}
	lp = *lpp;
	status = 1;
	if (dir->tdir_type == (int)TIFF_SHORT) {
		/*
		 * Handle short->long expansion.
		 */
		if (dir->tdir_count > 2) {
			char *dp =
			    malloc(dir->tdir_count*datawidth[(int)TIFF_SHORT]);
			if (dp == NULL) {
				TIFFError(tif->tif_name,
				    "No memory to fetch field \"%s\"",
				    FieldWithTag(dir->tdir_tag)->field_name);
				return (0);
			}
			if (status = TIFFFetchData(tif, dir, dp)) {
				register u_short *wp = (u_short *)dp;
				while (nstrips-- > 0)
					*lp++ = *wp++;
			}
			free(dp);
		} else {
			/*
			 * Extract data from offset field.
			 */
			if (tif->tif_header.tiff_magic == TIFF_BIGENDIAN) {
				*lp++ = dir->tdir_offset >> 16;
				if (dir->tdir_count > 1)
					*lp = dir->tdir_offset & 0xffff;
			} else {
				*lp++ = dir->tdir_offset & 0xffff;
				if (dir->tdir_count > 1)
					*lp = dir->tdir_offset >> 16;
			}
		}
	} else {
		if (dir->tdir_count > 1)
			status = TIFFFetchData(tif, dir, (char *)lp);
		else
			*lp = dir->tdir_offset;
	}
	return (status);
}

#ifdef JPEG_SUPPORT
/*
 * Fetch the JPEG Quantization tables
 * for the specified directory entry.
 * Storage for the td_qtab array is
 * allocated as a side effect.
 */
static
TIFFFetchJPEGQTables(tif, dir)
	TIFF *tif;
	TIFFDirEntry *dir;
{
	TIFFDirectory *td = &tif->tif_dir;
	long off[4];
	int i, j;
	TIFFDirEntry tdir;
	char *qmat;

	if (!CheckSampleCount(tif, dir))
		return (0);
	if (!TIFFFieldSet(tif, FIELD_JPEGQTABLEPREC)) {
		TIFFError(tif->tif_name, "Missing JPEG Q table precision");
		return (0);
	}
	if (dir->tdir_count > 1) {
		/* XXX verify count <= 4 */
		if (!TIFFFetchData(tif, dir, (char *)off))
			return (0);
	} else
		off[0] = dir->tdir_offset;
	/*
	 * We don't share per-component q matrices because
	 * (besides complicating this logic even more), it
	 * would make it very painful if the user does a ``set''.
	 */
	td->td_qtab = (u_short **)
	    malloc(dir->tdir_count*(sizeof (u_short *) + 64*sizeof (u_short)));
	tdir.tdir_type =
	   (td->td_jpegprec == JPEGQTABLEPREC_8BIT ? TIFF_BYTE : TIFF_SHORT);
	tdir.tdir_count = 64;
	qmat = (((char *)td->td_qtab) + dir->tdir_count*sizeof (u_short *));
	for (i = 0; i < dir->tdir_count; i++) {
		td->td_qtab[i] = (u_short *)qmat;
		tdir.tdir_offset = off[i];
		if (tdir.tdir_type == TIFF_BYTE) {
			/*
			 * Read Q matrix into the upper half of the
			 * short-sized array and then expand from
			 * 8-bit values to 16-bit values.
			 */
			if (!TIFFFetchData(tif, &tdir, qmat+64))
				return (0);
			for (j = 0; j < 64; j++)
				td->td_qtab[i][j] = qmat[64+j];
		} else {
			if (!TIFFFetchData(tif, &tdir, qmat))
				return (0);
		}
		qmat += 64*sizeof (u_short);
	}
	return (1);
}

/*
 * Fetch JPEG Huffman code tables for the
 * specified directory entry.  Storage for
 * the tables are allocated as a side effect.
 */
static
TIFFFetchJPEGCTables(tif, dir, ptab)
	TIFF *tif;
	TIFFDirEntry *dir;
	u_char ***ptab;
{
	TIFFDirectory *td = &tif->tif_dir;
	long off[4];
	int i, j, ncodes;
	TIFFDirEntry tdir;
	char *tab;

	if (!CheckSampleCount(tif, dir))
		return (0);
	if (dir->tdir_count > 1) {
		/* XXX verify count <= 4 */
		if (!TIFFFetchData(tif, dir, (char *)off))
			return (0);
	} else
		off[0] = dir->tdir_offset;
	/*
	 * We don't share per-component tables because
	 * (besides complicating this logic even more), it
	 * would make it very painful if the user does a
	 * ``set''.  Note also that we don't try to optimize
	 * storage of the tables -- we just allocate enough
	 * space to hold the largest possible.  All this
	 * stuff is so complicated 'cuz the tag is defined
	 * to be compatible with the JPEG table format,
	 * rather than something that fits well into the
	 * structure of TIFF -- argh!
	 */
	*ptab = (u_char **)malloc(dir->tdir_count*
	    (sizeof (u_char *) + (16+256)*sizeof (u_char)));
	tdir.tdir_type = TIFF_BYTE;
	tab = (((char *)*ptab) + dir->tdir_count*sizeof (u_char *));
	for (i = 0; i < dir->tdir_count; i++) {
		(*ptab)[i] = (u_char *)tab;
		tdir.tdir_offset = off[i];
		tdir.tdir_count = 16;
		/*
		 * We must fetch the array that holds the
		 * count of codes for each bit length first
		 * and the count up the number of codes that
		 * are in the variable length table.  This
		 * information is implicit in the JPEG format
		 * 'cuz it's preceded by a length field.
		 */
		if (!TIFFFetchData(tif, &tdir, tab))	/* count array */
			return (0);
		for (ncodes = 0, j = 0; j < 16; j++)
			ncodes += tab[j];
		/*
		 * Adjust offsets and fetch codes separately.
		 */
		tdir.tdir_offset += 16;
		tdir.tdir_count = ncodes;
		tab += 16;
		if (!TIFFFetchData(tif, &tdir, tab))
			return (0);
		tab += ncodes;
	}
	return (1);
}
#endif

static TIFFFieldInfo *
DECLARE2(FindFieldInfo, u_short, tag, TIFFDataType, dt)
{
	register TIFFFieldInfo *fip;

	/* NB: if table gets big, use sorted search (e.g. binary search) */
	for (fip = FieldInfo; fip < &FieldInfo[NFIELDINFO]; fip++)
		if (fip->field_tag == tag &&
		    (dt == TIFF_ANY || fip->field_type == dt))
			return (fip);
	return ((TIFFFieldInfo *)0);
}

static TIFFFieldInfo *
DECLARE1(FieldWithTag, u_short, tag)
{
	TIFFFieldInfo *fip = FindFieldInfo(tag, TIFF_ANY);
	if (fip)
		return (fip);
	TIFFError("FieldWithTag", "Internal error, unknown tag 0x%x", tag);
	exit(-1);
	/*NOTREACHED*/
}

#define	CleanupField(field, member) {		\
    if (TIFFFieldSet(tif, field)) {		\
	free((char *)td->member);		\
	td->member = 0;				\
    }						\
}
TIFFFreeDirectory(tif)
	TIFF *tif;
{
	register TIFFDirectory *td = &tif->tif_dir;

	CleanupField(FIELD_GRAYRESPONSECURVE, td_grayresponsecurve);
	if (TIFFFieldSet(tif, FIELD_COLORRESPONSECURVE)) {
		free(td->td_redresponsecurve), td->td_redresponsecurve = 0;
		free(td->td_greenresponsecurve), td->td_greenresponsecurve = 0;
		free(td->td_blueresponsecurve), td->td_blueresponsecurve = 0;
	}
	if (TIFFFieldSet(tif, FIELD_COLORMAP)) {
		free(td->td_redcolormap), td->td_redcolormap = 0;
		free(td->td_greencolormap), td->td_greencolormap = 0;
		free(td->td_bluecolormap), td->td_bluecolormap = 0;
	}
	CleanupField(FIELD_DOCUMENTNAME, td_documentname);
	CleanupField(FIELD_ARTIST, td_artist);
	CleanupField(FIELD_DATETIME, td_datetime);
	CleanupField(FIELD_HOSTCOMPUTER, td_hostcomputer);
	CleanupField(FIELD_IMAGEDESCRIPTION, td_imagedescription);
	CleanupField(FIELD_MAKE, td_make);
	CleanupField(FIELD_MODEL, td_model);
	CleanupField(FIELD_SOFTWARE, td_software);
	CleanupField(FIELD_PAGENAME, td_pagename);
#ifdef JPEG_SUPPORT
	CleanupField(FIELD_JPEGQTABLES, td_qtab);
	CleanupField(FIELD_JPEGDCTABLES, td_dctab);
	CleanupField(FIELD_JPEGACTABLES, td_actab);
#endif
	if  (td->td_stripoffset) {
		free((char *)td->td_stripoffset);
		td->td_stripoffset = NULL;
	}
	if  (td->td_stripbytecount) {
		free((char *)td->td_stripbytecount);
		td->td_stripbytecount = NULL;
	}
}
#undef CleanupField

/*
 * Setup a default directory structure.
 */
TIFFDefaultDirectory(tif)
	TIFF *tif;
{
	register TIFFDirectory *td = &tif->tif_dir;

	bzero((char *)td, sizeof (*td));
	td->td_fillorder = FILLORDER_MSB2LSB;
	td->td_bitspersample = 1;
	td->td_threshholding = THRESHHOLD_BILEVEL;
	td->td_orientation = ORIENTATION_TOPLEFT;
	td->td_samplesperpixel = 1;
	td->td_predictor = 1;
	td->td_rowsperstrip = 0xffffffff;
	td->td_tilewidth = 0xffffffff;
	td->td_tilelength = 0xffffffff;
	td->td_tiledepth = 1;
	td->td_grayresponseunit = GRAYRESPONSEUNIT_100S;
	td->td_resolutionunit = RESUNIT_INCH;
	td->td_colorresponseunit = COLORRESPONSEUNIT_100S;
	td->td_datatype = DATATYPE_VOID;
	td->td_imagedepth = 1;
	td->td_inkset = INKSET_CMYK;
	(void) TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	/*
	 * NB: The directory is marked dirty as a result of setting
	 * up the default compression scheme.  However, this really
	 * isn't correct -- we want TIFF_DIRTYDIRECT to be set only
	 * if the user does something.  We could just do the setup
	 * by hand, but it seems better to use the normal mechanism
	 * (i.e. TIFFSetField).
	 */
	tif->tif_flags &= ~TIFF_DIRTYDIRECT;
	return (1);
}

static
DECLARE2(setString, char**, cpp, char*, cp)
{
	if (*cpp)
		free(*cpp), *cpp = 0;
	if (cp) {
		int len = strlen(cp)+1;
		if (*cpp = malloc(len))
			bcopy(cp, *cpp, len);
	}
}

static
DECLARE3(setCurve, u_short**, wpp, u_short*, wp, long, nw)
{
	if (*wpp)
		free((char *)*wpp), *wpp = 0;
	nw *= sizeof (u_short);
	if (wp && (*wpp = (u_short *)malloc(nw)))
		bcopy(wp, *wpp, nw);
}

#ifdef notdef
static
DECLARE3(setFloatArray, float**, wpp, float*, wp, long, nw)
{
	if (*wpp)
		free((char *)*wpp), *wpp = 0;
	nw *= sizeof (float);
	if (wp && (*wpp = (float *)malloc(nw)))
		bcopy(wp, *wpp, nw);
}
#endif

#ifdef JPEG_SUPPORT
/*
 * Install a JPEG Quantization table.
 * Note that we reorder the elements
 * of the array in the zig-zag order
 * that is expected by the compression code
 * and that is to be stored in the file.
 */
static
DECLARE3(setJPEGQTable, u_short***, wpp, u_short**, wp, int, nc)
{
	static u_char zigzag[64] = {
	    0,  1,  5,  6, 14, 15, 27, 28,
	    2,  4,  7, 13, 16, 26, 29, 42,
	    3,  8, 12, 17, 25, 30, 41, 43,
	    9, 11, 18, 24, 31, 40, 44, 53,
	   10, 19, 23, 32, 39, 45, 52, 54,
	   20, 22, 33, 38, 46, 51, 55, 60,
	   21, 34, 37, 47, 50, 56, 59, 61,
	   35, 36, 48, 49, 57, 58, 62, 63
	};
	char *tab;
	int i, j;

	if (*wpp)
		free((char *)*wpp), *wpp = 0;
	*wpp = (u_short **)
	    malloc(nc * (sizeof (u_short *) + 64*sizeof (u_short)));
	tab = (((char *)*wpp) + nc*sizeof (u_short *));
	for (i = 0; i < nc; i++) {
		(*wpp)[i] = (u_short *)tab;
		for (j = 0; j < 64; j++)
			(*wpp)[zigzag[j]] = wp[j];
		tab += 64*sizeof (u_short);
	}
}

/*
 * Install a JPEG Coefficient table.
 */
static
DECLARE3(setJPEGCTable, u_char***, cpp, u_char**, cp, int, nc)
{
	u_char *tab;
	int i, j, ncodes, nw;

	if (*cpp)
		free(*cpp), *cpp = 0;
	/*
	 * Calculate the size of the table by counting
	 * the number of codes specified in the bits array.
	 */
	nw = 0;
	for (i = 0; i < nc; i++) {
		nw += 16;		/* 16 bytes for bits array */
		for (j = 0; j < 16; j++)/* sum up count of codes */
			nw += cp[i][j];
	}
	*cpp = (u_char **)malloc(nc*sizeof (u_char *) + nw);
	tab = ((u_char *)*cpp) + nc*sizeof (u_char *);
	/*
	 * Setup internal array and copy user data.
	 */
	for (i = 0; i < nc; i++) {
		(*cpp)[i] = tab;
		for (ncodes = 0, j = 0; j < 16; j++)
			ncodes += tab[j];
		bcopy(cp[i], (*cpp)[i], ncodes);
		tab += 16+ncodes;
	}
}
#endif

static
TIFFSetField1(tif, tag, ap)
	TIFF *tif;
	int tag;
	va_list ap;
{
	static char module[] = "TIFFSetField";
	TIFFDirectory *td = &tif->tif_dir;
	long v;
	int field = -1, status = 1;

	switch (tag) {
	case TIFFTAG_SUBFILETYPE:
		td->td_subfiletype = va_arg(ap, int);
		field = FIELD_SUBFILETYPE;
		break;
	case TIFFTAG_IMAGEWIDTH:
		td->td_imagewidth = va_arg(ap, u_long);
		field = FIELD_IMAGEDIMENSIONS;
		break;
	case TIFFTAG_IMAGELENGTH:
		td->td_imagelength = va_arg(ap, u_long);
		field = FIELD_IMAGEDIMENSIONS;
		break;
	case TIFFTAG_BITSPERSAMPLE:
		td->td_bitspersample = va_arg(ap, int);
		field = FIELD_BITSPERSAMPLE;
		break;
	case TIFFTAG_COMPRESSION:
		v = va_arg(ap, int) & 0xffff;
		/*
		 * If we're changing the compression scheme,
		 * the notify the previous module so that it
		 * can cleanup any state it's setup.
		 */
		if (TIFFFieldSet(tif, FIELD_COMPRESSION)) {
			if (td->td_compression == v)
				break;
			if (tif->tif_cleanup)
				(*tif->tif_cleanup)(tif);
		}
		/*
		 * Setup new compression routine state.
		 */
		if (status = TIFFSetCompressionScheme(tif, v)) {
			td->td_compression = v;
			field = FIELD_COMPRESSION;
		}
		break;
	case TIFFTAG_PHOTOMETRIC:
		td->td_photometric = va_arg(ap, int);
		field = FIELD_PHOTOMETRIC;
		break;
	case TIFFTAG_THRESHHOLDING:
		td->td_threshholding = va_arg(ap, int);
		field = FIELD_THRESHHOLDING;
		break;
	case TIFFTAG_FILLORDER:
		td->td_fillorder = va_arg(ap, int);
		field = FIELD_FILLORDER;
		break;
	case TIFFTAG_DOCUMENTNAME:
		setString(&td->td_documentname, va_arg(ap, char *));
		field = FIELD_DOCUMENTNAME;
		break;
	case TIFFTAG_ARTIST:
		setString(&td->td_artist, va_arg(ap, char *));
		field = FIELD_ARTIST;
		break;
	case TIFFTAG_DATETIME:
		setString(&td->td_datetime, va_arg(ap, char *));
		field = FIELD_DATETIME;
		break;
	case TIFFTAG_HOSTCOMPUTER:
		setString(&td->td_hostcomputer, va_arg(ap, char *));
		field = FIELD_HOSTCOMPUTER;
		break;
	case TIFFTAG_IMAGEDESCRIPTION:
		setString(&td->td_imagedescription, va_arg(ap, char *));
		field = FIELD_IMAGEDESCRIPTION;
		break;
	case TIFFTAG_MAKE:
		setString(&td->td_make, va_arg(ap, char *));
		field = FIELD_MAKE;
		break;
	case TIFFTAG_MODEL:
		setString(&td->td_model, va_arg(ap, char *));
		field = FIELD_MODEL;
		break;
	case TIFFTAG_SOFTWARE:
		setString(&td->td_software, va_arg(ap, char *));
		field = FIELD_SOFTWARE;
		break;
	case TIFFTAG_ORIENTATION:
		td->td_orientation = va_arg(ap, int);
		field = FIELD_ORIENTATION;
		break;
	case TIFFTAG_SAMPLESPERPIXEL:
		v = va_arg(ap, int);
		if (v == 0)
			goto badvalue;
		if (v > 4) {
			TIFFError(tif->tif_name,
			    "Cannot handle %d-channel data", v); 
			goto bad;
		}
		td->td_samplesperpixel = v;
		field = FIELD_SAMPLESPERPIXEL;
		break;
	case TIFFTAG_ROWSPERSTRIP:
		v = va_arg(ap, u_long);
		if (v == 0)
			goto badvalue;
		td->td_rowsperstrip = v;
		td->td_tilelength = v;
		td->td_tilewidth = td->td_imagewidth;
		field = FIELD_ROWSPERSTRIP;
		break;
	case TIFFTAG_MINSAMPLEVALUE:
		td->td_minsamplevalue = va_arg(ap, int) & 0xffff;
		field = FIELD_MINSAMPLEVALUE;
		break;
	case TIFFTAG_MAXSAMPLEVALUE:
		td->td_maxsamplevalue = va_arg(ap, int) & 0xffff;
		field = FIELD_MAXSAMPLEVALUE;
		break;
	case TIFFTAG_XRESOLUTION:
		td->td_xresolution = va_arg(ap, dblparam_t);
		field = FIELD_RESOLUTION;
		break;
	case TIFFTAG_YRESOLUTION:
		td->td_yresolution = va_arg(ap, dblparam_t);
		field = FIELD_RESOLUTION;
		break;
	case TIFFTAG_PLANARCONFIG:
		v = va_arg(ap, int);
		td->td_planarconfig = v;
		field = FIELD_PLANARCONFIG;
		break;
	case TIFFTAG_PAGENAME:
		setString(&td->td_pagename, va_arg(ap, char *));
		field = FIELD_PAGENAME;
		break;
	case TIFFTAG_XPOSITION:
		td->td_xposition = va_arg(ap, dblparam_t);
		field = FIELD_POSITION;
		break;
	case TIFFTAG_YPOSITION:
		td->td_yposition = va_arg(ap, dblparam_t);
		field = FIELD_POSITION;
		break;
	case TIFFTAG_GRAYRESPONSEUNIT:
		td->td_grayresponseunit = va_arg(ap, int);
		field = FIELD_GRAYRESPONSEUNIT;
		break;
	case TIFFTAG_GRAYRESPONSECURVE:
		setCurve(&td->td_grayresponsecurve,
		    va_arg(ap, u_short *), 1L<<td->td_bitspersample);
		field = FIELD_GRAYRESPONSECURVE;
		break;
	case TIFFTAG_GROUP3OPTIONS:
		td->td_group3options = va_arg(ap, u_long);
		field = FIELD_GROUP3OPTIONS;
		break;
	case TIFFTAG_GROUP4OPTIONS:
		td->td_group4options = va_arg(ap, u_long);
		field = FIELD_GROUP4OPTIONS;
		break;
	case TIFFTAG_RESOLUTIONUNIT:
		td->td_resolutionunit = va_arg(ap, int);
		field = FIELD_RESOLUTIONUNIT;
		break;
	case TIFFTAG_PAGENUMBER:
		td->td_pagenumber[0] = va_arg(ap, int);
		td->td_pagenumber[1] = va_arg(ap, int);
		field = FIELD_PAGENUMBER;
		break;
	case TIFFTAG_COLORRESPONSEUNIT:
		td->td_colorresponseunit = va_arg(ap, int);
		field = FIELD_COLORRESPONSEUNIT;
		break;
	case TIFFTAG_COLORRESPONSECURVE:
		v = 1L<<td->td_bitspersample;
		setCurve(&td->td_redresponsecurve, va_arg(ap, u_short *), v);
		setCurve(&td->td_greenresponsecurve, va_arg(ap, u_short *), v);
		setCurve(&td->td_blueresponsecurve, va_arg(ap, u_short *), v);
		field = FIELD_COLORRESPONSECURVE;
		break;
	case TIFFTAG_COLORMAP:
		v = 1L<<td->td_bitspersample;
		setCurve(&td->td_redcolormap, va_arg(ap, u_short *), v);
		setCurve(&td->td_greencolormap, va_arg(ap, u_short *), v);
		setCurve(&td->td_bluecolormap, va_arg(ap, u_short *), v);
		field = FIELD_COLORMAP;
		break;
	case TIFFTAG_PREDICTOR:
		td->td_predictor = va_arg(ap, int);
		field = FIELD_PREDICTOR;
		break;
	case TIFFTAG_MATTEING:
		td->td_matteing = va_arg(ap, int);
		field = FIELD_MATTEING;
		break;
	case TIFFTAG_INKSET:
		td->td_inkset = va_arg(ap, int);
		field = FIELD_INKSET;
		break;
	case TIFFTAG_BADFAXLINES:
		td->td_badfaxlines = va_arg(ap, u_long);
		field = FIELD_BADFAXLINES;
		break;
	case TIFFTAG_CLEANFAXDATA:
		td->td_cleanfaxdata = va_arg(ap, int);
		field = FIELD_CLEANFAXDATA;
		break;
	case TIFFTAG_CONSECUTIVEBADFAXLINES:
		td->td_badfaxrun = va_arg(ap, int);
		field = FIELD_BADFAXRUN;
		break;
	case TIFFTAG_TILEWIDTH:
		td->td_tilewidth = va_arg(ap, u_long);
		field = FIELD_TILEDIMENSIONS;
		tif->tif_flags |= TIFF_ISTILED;
		break;
	case TIFFTAG_TILELENGTH:
		td->td_tilelength = va_arg(ap, u_long);
		field = FIELD_TILEDIMENSIONS;
		tif->tif_flags |= TIFF_ISTILED;
		break;
	case TIFFTAG_TILEDEPTH:
		v = va_arg(ap, u_long);
		if (v == 0)
			goto badvalue;
		td->td_tiledepth = v;
		field = FIELD_TILEDEPTH;
		break;
	case TIFFTAG_DATATYPE:
		td->td_datatype = va_arg(ap, int);
		field = FIELD_DATATYPE;
		break;
	case TIFFTAG_IMAGEDEPTH:
		td->td_imagedepth = va_arg(ap, u_long);
		field = FIELD_IMAGEDEPTH;
		break;
#ifdef JPEG_SUPPORT
	case TIFFTAG_JPEGPROC:
		td->td_jpegproc = va_arg(ap, int);
		field = FIELD_JPEGPROC;
		break;
	case TIFFTAG_JPEGQTABLEPREC:
		td->td_jpegprec = va_arg(ap, int);
		field = FIELD_JPEGQTABLEPREC;
		break;
	case TIFFTAG_JPEGQTABLES:
		setJPEGQTable(&td->td_qtab, va_arg(ap, u_short **),
		    td->td_samplesperpixel);
		field = FIELD_JPEGQTABLES;
		break;
	case TIFFTAG_JPEGDCTABLES:
		setJPEGCTable(&td->td_dctab, va_arg(ap, u_char **),
		    td->td_samplesperpixel);
		field = FIELD_JPEGDCTABLES;
		break;
	case TIFFTAG_JPEGACTABLES:
		setJPEGCTable(&td->td_actab, va_arg(ap, u_char **),
		    td->td_samplesperpixel);
		field = FIELD_JPEGACTABLES;
		break;
#endif
	}
	if (field >= 0) {
		TIFFSetFieldBit(tif, field);
		tif->tif_flags |= TIFF_DIRTYDIRECT;
	}
	va_end(ap);
	return (status);
badvalue:
	TIFFError(tif->tif_name, "%d: Bad value for \"%s\"", v,
	    FieldWithTag(tag)->field_name);
bad:
	va_end(ap);
	return (0);
}

/*
 * Return 1/0 according to whether or not
 * it is permissible to set the tag's value.
 * Note that we allow ImageLength to be changed
 * so that we can append and extend to images.
 * Any other tag may not be altered once writing
 * has commenced, unless its value has no effect
 * on the format of the data that is written.
 */
static
OkToChangeTag(tif, tag)
	TIFF *tif;
	int tag;
{
	if (tag != TIFFTAG_IMAGELENGTH &&
	    (tif->tif_flags & TIFF_BEENWRITING)) {
		TIFFFieldInfo *fip = FindFieldInfo(tag, TIFF_ANY);
		/*
		 * Consult info table to see if tag can be changed
		 * after we've started writing.  We only allow changes
		 * to those tags that don't/shouldn't affect the
		 * compression and/or format of the data.
		 */
		if (fip && !fip->field_oktochange)
			return (0);
	}
	return (1);
}

/*
 * Record the value of a field in the
 * internal directory structure.  The
 * field will be written to the file
 * when/if the directory structure is
 * updated.
 */
/*VARARGS2*/
DECLARE2V(TIFFSetField, TIFF*, tif, int, tag)
{
	int status = 0;

	if (OkToChangeTag(tif, tag)) {
		va_list ap;

		VA_START(ap, tag);
		status = TIFFSetField1(tif, tag, ap);
		va_end(ap);
	} else {
		TIFFFieldInfo *fip = FindFieldInfo(tag, TIFF_ANY);
		if (fip)
			TIFFError("TIFFSetField",
			    "%s: Cannot modify tag \"%s\" while writing",
			    tif->tif_name, fip->field_name);
	}
	return (status);
}

/*
 * Like TIFFSetField, but taking a varargs
 * parameter list.  This routine is useful
 * for building higher-level interfaces on
 * top of the library.
 */
TIFFVSetField(tif, tag, ap)
	TIFF *tif;
	int tag;
	va_list ap;
{
	int status = 0;

	if (!OkToChangeTag(tif, tag)) {
		TIFFFieldInfo *fip = FindFieldInfo(tag, TIFF_ANY);
		if (fip)
			TIFFError("TIFFVSetField",
			    "%s: Cannot modify tag \"%s\" while writing",
			    tif->tif_name, fip->field_name);
	} else
		status = TIFFSetField1(tif, tag, ap);
	return (status);
}

/*
 * Return the value of a field in the
 * internal directory structure.
 */
/*VARARGS2*/
DECLARE2V(TIFFGetField, TIFF*, tif, int, tag)
{
	TIFFFieldInfo *fip = FindFieldInfo(tag, TIFF_ANY);

	if (!fip)
		TIFFError("TIFFGetField", "Unknown field, tag 0x%x", tag);
	else if (TIFFFieldSet(tif, fip->field_bit)) {
		va_list ap;

		VA_START(ap, tag);
		(void) TIFFGetField1(&tif->tif_dir, tag, ap);
		va_end(ap);
		return (1);
	}
	return (0);
}

/*
 * Like TIFFGetField, but taking a varargs
 * parameter list.  This routine is useful
 * for building higher-level interfaces on
 * top of the library.
 */
TIFFVGetField(tif, tag, ap)
	TIFF *tif;
	int tag;
	va_list ap;
{
	TIFFFieldInfo *fip = FindFieldInfo(tag, TIFF_ANY);

	if (fip) {
		if (TIFFFieldSet(tif, fip->field_bit)) {
			(void) TIFFGetField1(&tif->tif_dir, tag, ap);
			return (1);
		}
	} else
		TIFFError("TIFFGetField", "Unknown field, tag 0x%x", tag);
	return (0);
}

static
TIFFGetField1(td, tag, ap)
	TIFFDirectory *td;
	int tag;
	va_list ap;
{

	switch (tag) {
	case TIFFTAG_SUBFILETYPE:
		*va_arg(ap, u_long *) = td->td_subfiletype;
		break;
	case TIFFTAG_IMAGEWIDTH:
		*va_arg(ap, u_long *) = td->td_imagewidth;
		break;
	case TIFFTAG_IMAGELENGTH:
		*va_arg(ap, u_long *) = td->td_imagelength;
		break;
	case TIFFTAG_BITSPERSAMPLE:
		*va_arg(ap, u_short *) = td->td_bitspersample;
		break;
	case TIFFTAG_COMPRESSION:
		*va_arg(ap, u_short *) = td->td_compression;
		break;
	case TIFFTAG_PHOTOMETRIC:
		*va_arg(ap, u_short *) = td->td_photometric;
		break;
	case TIFFTAG_THRESHHOLDING:
		*va_arg(ap, u_short *) = td->td_threshholding;
		break;
	case TIFFTAG_FILLORDER:
		*va_arg(ap, u_short *) = td->td_fillorder;
		break;
	case TIFFTAG_DOCUMENTNAME:
		*va_arg(ap, char **) = td->td_documentname;
		break;
	case TIFFTAG_ARTIST:
		*va_arg(ap, char **) = td->td_artist;
		break;
	case TIFFTAG_DATETIME:
		*va_arg(ap, char **) = td->td_datetime;
		break;
	case TIFFTAG_HOSTCOMPUTER:
		*va_arg(ap, char **) = td->td_hostcomputer;
		break;
	case TIFFTAG_IMAGEDESCRIPTION:
		*va_arg(ap, char **) = td->td_imagedescription;
		break;
	case TIFFTAG_MAKE:
		*va_arg(ap, char **) = td->td_make;
		break;
	case TIFFTAG_MODEL:
		*va_arg(ap, char **) = td->td_model;
		break;
	case TIFFTAG_SOFTWARE:
		*va_arg(ap, char **) = td->td_software;
		break;
	case TIFFTAG_ORIENTATION:
		*va_arg(ap, u_short *) = td->td_orientation;
		break;
	case TIFFTAG_SAMPLESPERPIXEL:
		*va_arg(ap, u_short *) = td->td_samplesperpixel;
		break;
	case TIFFTAG_ROWSPERSTRIP:
		*va_arg(ap, u_long *) = td->td_rowsperstrip;
		break;
	case TIFFTAG_MINSAMPLEVALUE:
		*va_arg(ap, u_short *) = td->td_minsamplevalue;
		break;
	case TIFFTAG_MAXSAMPLEVALUE:
		*va_arg(ap, u_short *) = td->td_maxsamplevalue;
		break;
	case TIFFTAG_XRESOLUTION:
		*va_arg(ap, float *) = td->td_xresolution;
		break;
	case TIFFTAG_YRESOLUTION:
		*va_arg(ap, float *) = td->td_yresolution;
		break;
	case TIFFTAG_PLANARCONFIG:
		*va_arg(ap, u_short *) = td->td_planarconfig;
		break;
	case TIFFTAG_XPOSITION:
		*va_arg(ap, float *) = td->td_xposition;
		break;
	case TIFFTAG_YPOSITION:
		*va_arg(ap, float *) = td->td_yposition;
		break;
	case TIFFTAG_PAGENAME:
		*va_arg(ap, char **) = td->td_pagename;
		break;
	case TIFFTAG_GRAYRESPONSEUNIT:
		*va_arg(ap, u_short *) = td->td_grayresponseunit;
		break;
	case TIFFTAG_GRAYRESPONSECURVE:
		*va_arg(ap, u_short **) = td->td_grayresponsecurve;
		break;
	case TIFFTAG_GROUP3OPTIONS:
		*va_arg(ap, u_long *) = td->td_group3options;
		break;
	case TIFFTAG_GROUP4OPTIONS:
		*va_arg(ap, u_long *) = td->td_group4options;
		break;
	case TIFFTAG_RESOLUTIONUNIT:
		*va_arg(ap, u_short *) = td->td_resolutionunit;
		break;
	case TIFFTAG_PAGENUMBER:
		*va_arg(ap, u_short *) = td->td_pagenumber[0];
		*va_arg(ap, u_short *) = td->td_pagenumber[1];
		break;
	case TIFFTAG_COLORRESPONSEUNIT:
		*va_arg(ap, u_short *) = td->td_colorresponseunit;
		break;
	case TIFFTAG_COLORRESPONSECURVE:
		*va_arg(ap, u_short **) = td->td_redresponsecurve;
		*va_arg(ap, u_short **) = td->td_greenresponsecurve;
		*va_arg(ap, u_short **) = td->td_blueresponsecurve;
		break;
	case TIFFTAG_COLORMAP:
		*va_arg(ap, u_short **) = td->td_redcolormap;
		*va_arg(ap, u_short **) = td->td_greencolormap;
		*va_arg(ap, u_short **) = td->td_bluecolormap;
		break;
	case TIFFTAG_PREDICTOR:
		*va_arg(ap, u_short *) = td->td_predictor;
		break;
	case TIFFTAG_STRIPOFFSETS:
		*va_arg(ap, u_long **) = td->td_stripoffset;
		break;
	case TIFFTAG_STRIPBYTECOUNTS:
		*va_arg(ap, u_long **) = td->td_stripbytecount;
		break;
	case TIFFTAG_MATTEING:
		*va_arg(ap, u_short *) = td->td_matteing;
		break;
	case TIFFTAG_INKSET:
		*va_arg(ap, u_short *) = td->td_inkset;
		break;
	case TIFFTAG_BADFAXLINES:
		*va_arg(ap, u_long *) = td->td_badfaxlines;
		break;
	case TIFFTAG_CLEANFAXDATA:
		*va_arg(ap, u_short *) = td->td_cleanfaxdata;
		break;
	case TIFFTAG_CONSECUTIVEBADFAXLINES:
		*va_arg(ap, u_long *) = td->td_badfaxrun;
		break;
	case TIFFTAG_TILEWIDTH:
		*va_arg(ap, u_long *) = td->td_tilewidth;
		break;
	case TIFFTAG_TILELENGTH:
		*va_arg(ap, u_long *) = td->td_tilelength;
		break;
	case TIFFTAG_TILEDEPTH:
		*va_arg(ap, u_long *) = td->td_tiledepth;
		break;
	case TIFFTAG_DATATYPE:
		*va_arg(ap, u_short *) = td->td_datatype;
		break;
	case TIFFTAG_IMAGEDEPTH:
		*va_arg(ap, u_short *) = td->td_imagedepth;
		break;
#ifdef JPEG_SUPPORT
	case TIFFTAG_JPEGPROC:
		*va_arg(ap, u_short *) = td->td_jpegproc;
		break;
	case TIFFTAG_JPEGQTABLEPREC:
		*va_arg(ap, u_short *) = td->td_jpegprec;
		break;
	case TIFFTAG_JPEGQTABLES:
		*va_arg(ap, u_short ***) = td->td_qtab;
		break;
	case TIFFTAG_JPEGDCTABLES:
		*va_arg(ap, u_char ***) = td->td_dctab;
		break;
	case TIFFTAG_JPEGACTABLES:
		*va_arg(ap, u_char ***) = td->td_actab;
		break;
#endif
	}
	va_end(ap);
}

/*
 * Internal interface to TIFFGetField...
 */
static
/*VARARGS2*/
DECLARE2V(TIFFgetfield, TIFFDirectory*, td, int, tag)
{
	va_list ap;

	VA_START(ap, tag);
	(void) TIFFGetField1(td, tag, ap);
	va_end(ap);
}

#if USE_PROTOTYPES
static	TIFFSetupShortLong(TIFF *, TIFFDirEntry *, u_short, u_long);
static	TIFFWriteRational(TIFF *, u_short, TIFFDirEntry *, float);
static	TIFFWritePerSampleShorts(TIFF *, u_short, TIFFDirEntry *, short);
static	TIFFWriteStripThing(TIFF *, u_short, TIFFDirEntry *, u_long *);
#ifdef JPEG_SUPPORT
static	TIFFWriteJPEGQTables(TIFF *, TIFFDirEntry *);
static	TIFFWriteJPEGCTables(TIFF *, u_short, TIFFDirEntry *, u_char **);
#endif
static	TIFFWriteData(TIFF *, TIFFDirEntry *, char *);
static	TIFFLinkDirectory(TIFF *);
#else
static	TIFFSetupShortLong();
static	TIFFWriteRational();
static	TIFFWritePerSampleShorts();
static	TIFFWriteStripThing();
#ifdef JPEG_SUPPORT
static	TIFFWriteJPEGQTables();
static	TIFFWriteJPEGCTables();
#endif
static	TIFFWriteData();
static	TIFFLinkDirectory();
#endif

#define	WriteRationalPair(tag1, v1, tag2, v2) \
	(TIFFWriteRational(tif, tag1, dir++, v1) && \
	 TIFFWriteRational(tif, tag2, dir++, v2))

static	long dataoff;
/*
 * Write the contents of the current directory
 * to the specified file.  This routine doesn't
 * handle overwriting a directory with auxiliary
 * storage that's been changed.
 */
TIFFWriteDirectory(tif)
	TIFF *tif;
{
	short dircount, v;
	int nfields, dirsize;
	char *data, *cp;
	TIFFFieldInfo *fip;
	TIFFDirEntry *dir;
	TIFFDirectory *td;
	u_long b, off, fields[sizeof (td->td_fieldsset) / sizeof (u_long)];

	if (tif->tif_mode == O_RDONLY)
		return (1);
	/*
	 * Clear write state so that subsequent images with
	 * different characteristics get the right buffers
	 * setup for them.
	 */
	if (tif->tif_close)
		(*tif->tif_close)(tif);
	if (tif->tif_cleanup)
		(*tif->tif_cleanup)(tif);
	/*
	 * Flush any data that might have been written
	 * by the compression close+cleanup routines.
	 */
	if (tif->tif_rawcc > 0 && !TIFFFlushData1(tif)) {
		TIFFError(tif->tif_name,
		    "Error flushing data before directory write");
		return (0);
	}
	if ((tif->tif_flags & TIFF_MYBUFFER) && tif->tif_rawdata) {
		free(tif->tif_rawdata);
		tif->tif_rawdata = NULL;
		tif->tif_rawcc = 0;
	}
	tif->tif_flags &= ~(TIFF_BEENWRITING|TIFF_BUFFERSETUP);

	td = &tif->tif_dir;
	/*
	 * Size the directory so that we can calculate
	 * offsets for the data items that aren't kept
	 * in-place in each field.
	 */
	nfields = 0;
	for (b = 0; b <= FIELD_LAST; b++)
		if (TIFFFieldSet(tif, b))
			nfields += (b < FIELD_SUBFILETYPE ? 2 : 1);
	dirsize = nfields * sizeof (TIFFDirEntry);
	data = malloc(dirsize);
	if (data == NULL) {
		TIFFError(tif->tif_name,
		    "Cannot write directory, out of space");
		return (0);
	}
	/*
	 * Directory hasn't been placed yet, put
	 * it at the end of the file and link it
	 * into the existing directory structure.
	 */
	if (tif->tif_diroff == 0 && !TIFFLinkDirectory(tif))
		return (0);
	dataoff = tif->tif_diroff + sizeof (short) + dirsize + sizeof (long);
	if (dataoff & 1)
		dataoff++;
	(void) lseek(tif->tif_fd, dataoff, L_SET);
	dir = (TIFFDirEntry *)data;
	/*
	 * Setup external form of directory
	 * entries and write data items.
	 */
	bcopy(td->td_fieldsset, fields, sizeof (fields));
	for (fip = FieldInfo; fip < &FieldInfo[NFIELDINFO]; fip++) {
		if (fip->field_bit == (u_short)-1 ||
		    !FieldSet(fields, fip->field_bit))
			continue;
		if (fip->field_type == TIFF_ASCII) {
			TIFFgetfield(td, fip->field_tag, &cp);
			dir->tdir_tag = fip->field_tag;
			dir->tdir_type = (short)TIFF_ASCII;
			dir->tdir_count = strlen(cp) + 1;
			if (!TIFFWriteData(tif, dir++, cp))
				goto bad;
			ResetFieldBit(fields, fip->field_bit);
			continue;
		}
		switch (fip->field_bit) {
		case FIELD_STRIPOFFSETS:
		case FIELD_STRIPBYTECOUNTS:
			if (!TIFFWriteStripThing(tif,
			    fip->field_tag, dir++,
			    fip->field_bit == FIELD_STRIPOFFSETS ?
			      td->td_stripoffset : td->td_stripbytecount))
				goto bad;
			break;
		case FIELD_GRAYRESPONSECURVE:
			dir->tdir_tag = fip->field_tag;
			dir->tdir_type = (short)TIFF_SHORT;
			dir->tdir_count = 1L<<td->td_bitspersample;
#define	WRITE(x)	TIFFWriteData(tif, dir, (char*) x)
			if (!WRITE(td->td_grayresponsecurve))
				goto bad;
			dir++;
			break;
		case FIELD_COLORRESPONSECURVE:
		case FIELD_COLORMAP:
			dir->tdir_tag = fip->field_tag;
			dir->tdir_type = (short)TIFF_SHORT;
			/* XXX -- yech, fool TIFFWriteData */
			dir->tdir_count = 1L<<td->td_bitspersample;
			off = dataoff;
			if (fip->field_tag == TIFFTAG_COLORMAP) {
				if (!WRITE(td->td_redcolormap) ||
				    !WRITE(td->td_greencolormap) ||
				    !WRITE(td->td_bluecolormap))
					goto bad;
			} else {
				if (!WRITE(td->td_redresponsecurve) ||
				    !WRITE(td->td_greenresponsecurve) ||
				    !WRITE(td->td_blueresponsecurve))
					goto bad;
			}
#undef WRITE
			dir->tdir_count *= 3;
			dir->tdir_offset = off;
			dir++;
			break;
		case FIELD_IMAGEDIMENSIONS:
			TIFFSetupShortLong(tif, dir++,
			    TIFFTAG_IMAGEWIDTH, td->td_imagewidth);
			TIFFSetupShortLong(tif, dir++,
			    TIFFTAG_IMAGELENGTH, td->td_imagelength);
			break;
		case FIELD_TILEDIMENSIONS:
			TIFFSetupShortLong(tif, dir++,
			    TIFFTAG_TILEWIDTH, td->td_tilewidth);
			TIFFSetupShortLong(tif, dir++,
			    TIFFTAG_TILELENGTH, td->td_tilelength);
			break;
		case FIELD_POSITION:
			if (!WriteRationalPair(
			    TIFFTAG_XPOSITION, td->td_xposition,
			    TIFFTAG_YPOSITION, td->td_yposition))
				goto bad;
			break;
		case FIELD_RESOLUTION:
			if (!WriteRationalPair(
			    TIFFTAG_XRESOLUTION, td->td_xresolution,
			    TIFFTAG_YRESOLUTION, td->td_yresolution))
				goto bad;
			break;
		case FIELD_BITSPERSAMPLE:
		case FIELD_MINSAMPLEVALUE:
		case FIELD_MAXSAMPLEVALUE:
		case FIELD_DATATYPE:
			TIFFgetfield(td, fip->field_tag, &v);
			if (!TIFFWritePerSampleShorts(tif, fip->field_tag,
			    dir++, v))
				goto bad;
			break;
		case FIELD_PAGENUMBER: {
			short v1, v2;
			dir->tdir_tag = TIFFTAG_PAGENUMBER;
			dir->tdir_type = (short)TIFF_SHORT;
			dir->tdir_count = 2;
			TIFFgetfield(td, TIFFTAG_PAGENUMBER, &v1, &v2);
			if (tif->tif_header.tiff_magic == TIFF_BIGENDIAN)
				dir->tdir_offset = (v1 << 16) | (v2 & 0xffff);
			else
				dir->tdir_offset = (v2 << 16) | (v1 & 0xffff);
			dir++;
			break;
#ifdef JPEG_SUPPORT
		case FIELD_JPEGQTABLES:
			if (!TIFFWriteJPEGQTables(tif, dir))
				goto bad;
			break;
		case FIELD_JPEGDCTABLES:
			if (!TIFFWriteJPEGCTables(tif,
			    TIFFTAG_JPEGDCTABLES, dir, td->td_dctab))
				goto bad;
			break;
		case FIELD_JPEGACTABLES:
			if (!TIFFWriteJPEGCTables(tif,
			    TIFFTAG_JPEGACTABLES, dir, td->td_actab))
				goto bad;
			break;
#endif
		}
		default:
			dir->tdir_tag = fip->field_tag;
			dir->tdir_type = (u_short)fip->field_type;
			dir->tdir_count = fip->field_count;
			if (fip->field_type == TIFF_SHORT) {
				TIFFgetfield(td, fip->field_tag, &v);
				dir->tdir_offset =
				    TIFFInsertData(tif, dir->tdir_type, v);
			} else
				TIFFgetfield(td, fip->field_tag,
				    &dir->tdir_offset);
			dir++;
			break;
		}
		ResetFieldBit(fields, fip->field_bit);
	}
	/*
	 * Write directory.
	 */
	(void) lseek(tif->tif_fd, tif->tif_diroff, L_SET);
	dircount = nfields;
	if (!WriteOK(tif->tif_fd, &dircount, sizeof (short))) {
		TIFFError(tif->tif_name, "Error writing directory count");
		goto bad;
	}
	if (!WriteOK(tif->tif_fd, data, dirsize)) {
		TIFFError(tif->tif_name, "Error writing directory contents");
		goto bad;
	}
	if (!WriteOK(tif->tif_fd, &tif->tif_nextdiroff, sizeof (long))) {
		TIFFError(tif->tif_name, "Error writing directory link");
		goto bad;
	}
	TIFFFreeDirectory(tif);
	free(data);
	tif->tif_flags &= ~TIFF_DIRTYDIRECT;

	/*
	 * Reset directory-related state for subsequent
	 * directories.
	 */
	TIFFDefaultDirectory(tif);
	tif->tif_diroff = 0;
	tif->tif_curoff = 0;
	tif->tif_row = -1;
	tif->tif_curstrip = -1;
	return (1);
bad:
	free(data);
	return (0);
}
#undef WriteRationalPair

/*
 * Setup a directory entry with either a SHORT
 * or LONG type according to the value.
 */
static
DECLARE4(TIFFSetupShortLong,
    TIFF*, tif, TIFFDirEntry*, dir, u_short, tag, u_long, v)
{
	dir->tdir_tag = tag;
	dir->tdir_count = 1;
	if (v > 0xffffL) {
		dir->tdir_type = (short)TIFF_LONG;
		dir->tdir_offset = v;
	} else {
		dir->tdir_type = (short)TIFF_SHORT;
		dir->tdir_offset = TIFFInsertData(tif, (int)TIFF_SHORT, v);
	}
}
#undef MakeShortDirent

/*
 * Setup a RATIONAL directory entry and
 * write the associated indirect value.
 */
static
DECLARE4(TIFFWriteRational,
    TIFF*, tif, u_short, tag, TIFFDirEntry*, dir, float, v)
{
	long t[2];

	dir->tdir_tag = tag;
	dir->tdir_type = (short)TIFF_RATIONAL;
	dir->tdir_count = 1;
	/* need algorithm to convert ... XXX */
	t[0] = v * 10000.0;
	t[1] = 10000;
	return (TIFFWriteData(tif, dir, (char *)t));
}

/*
 * Setup a directory entry that references a
 * samples/pixel array of SHORT values and
 * (potentially) write the associated indirect
 * values.
 */
static
DECLARE4(TIFFWritePerSampleShorts,
    TIFF*, tif, u_short, tag, TIFFDirEntry*, dir, short, v)
{
	short w[4];
	int i, samplesperpixel = tif->tif_dir.td_samplesperpixel;

	dir->tdir_tag = tag;
	dir->tdir_type = (short)TIFF_SHORT;
	dir->tdir_count = samplesperpixel;
	if (samplesperpixel <= 2) {
		if (tif->tif_header.tiff_magic == TIFF_BIGENDIAN) {
			dir->tdir_offset = (long)v << 16;
			if (samplesperpixel == 2)
				dir->tdir_offset |= v & 0xffff;
		} else {
			dir->tdir_offset = v & 0xffff;
			if (samplesperpixel == 2)
				dir->tdir_offset |= (long)v << 16;
		}
		return (1);
	}
	for (i = 0; i < samplesperpixel; i++)
		w[i] = v;
	return (TIFFWriteData(tif, dir, (char *)w));
}

/*
 * Setup a directory entry of strip or
 * tile offsets or lengths and write the
 * associated indirect values.
 */
static
DECLARE4(TIFFWriteStripThing,
    TIFF*, tif, u_short, tag, TIFFDirEntry*, dir, u_long*, lp)
{

	dir->tdir_tag = tag;
	dir->tdir_type = (short)TIFF_LONG;		/* XXX */
	dir->tdir_count = tif->tif_dir.td_nstrips;
	if (dir->tdir_count > 1)
		return (TIFFWriteData(tif, dir, (char *)lp));
	dir->tdir_offset = *lp;
	return (1);
}

#ifdef JPEG_SUPPORT
/*
 * Setup a directory entry for JPEG Quantization
 * tables and write the associated indirect values.
 */
static
DECLARE2(TIFFWriteJPEGQTables, TIFF*, tif, TIFFDirEntry*, dir)
{
	TIFFDirectory *td = &tif->tif_dir;
	TIFFDirEntry tdir;
	u_short **qtab;
	u_long off[4];
	int i;

	dir->tdir_tag = TIFFTAG_JPEGQTABLES;
	dir->tdir_type = (short)TIFF_LONG;
	dir->tdir_count = td->td_samplesperpixel;
	qtab = td->td_qtab;
	tdir.tdir_tag = TIFFTAG_JPEGQTABLES;	/* for diagnostics */
	tdir.tdir_type = (short)
	    (td->td_jpegprec == JPEGQTABLEPREC_8BIT ? TIFF_BYTE : TIFF_SHORT);
	tdir.tdir_count = 64;
	for (i = 0; i < dir->tdir_count; i++) {
		if (tdir.tdir_type == TIFF_BYTE) {
			char v[64];
			int j;

			for (j = 0; j < 64; j++)
				v[j] = qtab[i][j];
			if (!TIFFWriteData(tif, &tdir, v))
				return (0);
		} else if (!TIFFWriteData(tif, &tdir, (char *)qtab[i]))
			return (0);
		off[i] = tdir.tdir_offset;
	}
	return (TIFFWriteData(tif, dir, (char *)off));
}

/*
 * Setup a directory entry for JPEG Coefficient
 * tables and write the associated indirect values.
 */
static
DECLARE4(TIFFWriteJPEGCTables,
    TIFF*, tif, u_short, tag, TIFFDirEntry*, dir, u_char **, tab)
{
	TIFFDirectory *td = &tif->tif_dir;
	TIFFDirEntry tdir;
	u_long off[4];
	int i, j, ncodes;

	dir->tdir_tag = tag;
	dir->tdir_type = (short)TIFF_LONG;
	dir->tdir_count = td->td_samplesperpixel;
	tdir.tdir_tag = tag;		/* for diagnostics */
	tdir.tdir_type = (short)TIFF_BYTE;
	for (i = 0; i < dir->tdir_count; i++) {
		for (ncodes = 0, j = 0; j < 16; j++)
			ncodes += tab[i][j];
		tdir.tdir_count = 16+ncodes;
		if (!TIFFWriteData(tif, &tdir, (char *)tab[i]))
			return (0);
		off[i] = tdir.tdir_offset;
	}
	return (TIFFWriteData(tif, dir, (char *)off));
}
#endif

/*
 * Write a contiguous directory item.
 */
static
TIFFWriteData(tif, dir, cp)
	TIFF *tif;
	TIFFDirEntry *dir;
	char *cp;
{
	int cc;

	dir->tdir_offset = dataoff;
	cc = dir->tdir_count * datawidth[dir->tdir_type];
	if (SeekOK(tif->tif_fd, dir->tdir_offset) &&
	    WriteOK(tif->tif_fd, cp, cc)) {
		dataoff += (cc + 1) & ~1;
		return (1);
	}
	TIFFError(tif->tif_name, "Error writing data for field \"%s\"",
	    FieldWithTag(dir->tdir_tag)->field_name);
	return (0);
}

/*
 * Link the current directory into the
 * directory chain for the file.
 */
static
TIFFLinkDirectory(tif)
	register TIFF *tif;
{
	static char module[] = "TIFFLinkDirectory";
	u_short dircount;
	long nextdir;

	tif->tif_diroff = (lseek(tif->tif_fd, 0L, L_XTND)+1) &~ 1L;
	if (tif->tif_header.tiff_diroff == 0) {
		/*
		 * First directory, overwrite header.
		 */
		tif->tif_header.tiff_diroff = tif->tif_diroff;
		(void) lseek(tif->tif_fd, 0L, L_SET);
		if (!WriteOK(tif->tif_fd, &tif->tif_header,
		    sizeof (tif->tif_header))) {
			TIFFError(tif->tif_name, "Error writing TIFF header");
			return (0);
		}
		return (1);
	}
	/*
	 * Not the first directory, search to the last and append.
	 */
	nextdir = tif->tif_header.tiff_diroff;
	do {
		if (!SeekOK(tif->tif_fd, nextdir) ||
		    !ReadOK(tif->tif_fd, &dircount, sizeof (dircount))) {
			TIFFError(module, "Error fetching directory count");
			return (0);
		}
		if (tif->tif_flags & TIFF_SWAB)
			TIFFSwabShort(&dircount);
		lseek(tif->tif_fd, dircount * sizeof (TIFFDirEntry), L_INCR);
		if (!ReadOK(tif->tif_fd, &nextdir, sizeof (nextdir))) {
			TIFFError(module, "Error fetching directory link");
			return (0);
		}
		if (tif->tif_flags & TIFF_SWAB)
			TIFFSwabLong((u_long *)&nextdir);
	} while (nextdir != 0);
	(void) lseek(tif->tif_fd, -sizeof (nextdir), L_INCR);
	if (!WriteOK(tif->tif_fd, &tif->tif_diroff, sizeof (tif->tif_diroff))) {
		TIFFError(module, "Error writing directory link");
		return (0);
	}
	return (1);
}

/*
 * Set the n-th directory as the current directory.
 * NB: Directories are numbered starting at 0.
 */
TIFFSetDirectory(tif, n)
	register TIFF *tif;
	int n;
{
	static char module[] = "TIFFSetDirectory";
	u_short dircount;
	long nextdir;

	nextdir = tif->tif_header.tiff_diroff;
	while (n-- > 0 && nextdir != 0) {
		if (!SeekOK(tif->tif_fd, nextdir) ||
		    !ReadOK(tif->tif_fd, &dircount, sizeof (dircount))) {
			TIFFError(module, "%s: Error fetching directory count",
			    tif->tif_name);
			return (0);
		}
		if (tif->tif_flags & TIFF_SWAB)
			TIFFSwabShort(&dircount);
		lseek(tif->tif_fd, dircount*sizeof (TIFFDirEntry), L_INCR);
		if (!ReadOK(tif->tif_fd, &nextdir, sizeof (nextdir))) {
			TIFFError(module, "%s: Error fetching directory link",
			    tif->tif_name);
			return (0);
		}
		if (tif->tif_flags & TIFF_SWAB)
			TIFFSwabLong((u_long *)&nextdir);
	}
	tif->tif_nextdiroff = nextdir;
	return (TIFFReadDirectory(tif));
}
