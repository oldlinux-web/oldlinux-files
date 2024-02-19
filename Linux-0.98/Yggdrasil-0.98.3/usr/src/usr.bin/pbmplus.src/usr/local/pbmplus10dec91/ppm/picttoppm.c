/*
 * picttoppm.c -- convert a MacIntosh PICT file to PPM format.
 *
 * Copyright 1989 George Phillips
 *
 * Permission is granted to freely distribute this program in whole or in
 * part provided you don't make money off it, you don't pretend that you
 * wrote it and that this notice accompanies the code.
 *
 * George Phillips <phillips@cs.ubc.ca>
 * Department of Computer Science
 * University of British Columbia
 */

#include "ppm.h"

/*
 * Typical byte, 2 byte and 4 byte integers.
 */
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long longword;

/*
 * Data structures for QuickDraw (and hence PICT) stuff.
 */

struct Rect {
	word top;
	word left;
	word bottom;
	word right;
};

struct pixMap {
	struct Rect Bounds;
	word version;
	word packType;
	longword packSize;
	longword hRes;
	longword vRes;
	word pixelType;
	word pixelSize;
	word cmpCount;
	word cmpSize;
	longword planeBytes;
	longword pmTable;
	longword pmReserved;
};

struct ct_entry {
	word red;
	word green;
	word blue;
};

static char* stage;
static struct Rect picFrame;
static word* red;
static word* green;
static word* blue;
static word rowlen;
static longword planelen;
static int verbose;

struct opdef {
	char* name;
	int len;
	void (*impl)();
	char* description;
};

#define WORD_LEN (-1)

static void interpret_pict ARGS(( void ));
static void compact_plane ARGS(( word* plane, int planelen ));
static void output_ppm ARGS(( int version ));
static void Opcode_9A ARGS(( void ));
static void BitsRect ARGS(( int version ));
static void BitsRegion ARGS(( int version ));
static void do_bitmap ARGS(( int version, int rowBytes, int is_region ));
static void do_pixmap ARGS(( int version, word rowBytes, int is_region ));
static void blit ARGS((
    struct Rect* srcRect, struct Rect* srcBounds, int srcwid, byte* srcplane,
    int pixSize, struct Rect* dstRect, struct Rect* dstBounds, int dstwid,
    struct ct_entry* colour_map, int mode ));
static byte* unpackbits ARGS(( struct Rect* bounds, word rowBytes, int pixelSize ));
static byte* expand_buf ARGS(( byte* buf, int* len, int bits_per_pixel ));
static void Clip ARGS(( int version ));
static void read_pixmap ARGS(( struct pixMap* p, word* rowBytes ));
static struct ct_entry* read_colour_table ARGS(( void ));
static void BkPixPat ARGS(( int version ));
static void PnPixPat ARGS(( int version ));
static void FillPixPat ARGS(( int version ));
static void read_pattern ARGS(( void ));

static void skip_text ARGS(( void ));
static void LongText ARGS(( int version ));
static void DHText ARGS(( int version ));
static void DVText ARGS(( int version ));
static void DHDVText ARGS(( int version ));
static void skip_poly_or_region ARGS(( int version ));
static void LongComment ARGS(( void ));

static int rectsamesize ARGS(( struct Rect* r1, struct Rect* r2 ));
static void rectinter ARGS(( struct Rect* r1, struct Rect* r2, struct Rect* r3 ));

static void read_rect ARGS(( struct Rect* r ));
static void dump_rect ARGS(( char* s, struct Rect* r ));

static word get_op ARGS(( int version ));

static longword read_long ARGS(( void ));
static word read_word ARGS(( void ));
static byte read_byte ARGS(( void ));

static void skip ARGS(( int n ));
static void read_n ARGS(( int n, char* buf ));

/*
 * a table of the first 194(?) opcodes.  The table is too empty.
 *
 * Probably could use an entry specifying if the opcode is valid in version
 * 1, etc.
 */

/* for reserved opcodes of known length */
#define res(length) \
{ "reserved", (length), NULL, "reserved for Apple use" }

/* for reserved opcodes of length determined by a function */
#define resf(skipfunction) \
{ "reserved", NA, (skipfunction), "reserved for Apple use" }

/* seems like RGB colours are 6 bytes, but Apple say's they're variable */
/* I'll use 6 for now as I don't care that much. */
#define RGB_LEN (6)

#define NA (0)

static struct opdef optable[] = {
/* 0x00 */	{ "NOP", 0, NULL, "nop" },
/* 0x01 */	{ "Clip", NA, Clip, "clip" },
/* 0x02 */	{ "BkPat", 8, NULL, "background pattern" },
/* 0x03 */	{ "TxFont", 2, NULL, "text font (word)" },
/* 0x04 */	{ "TxFace", 1, NULL, "text face (byte)" },
/* 0x05 */	{ "TxMode", 2, NULL, "text mode (word)" },
/* 0x06 */	{ "SpExtra", 4, NULL, "space extra (fixed point)" },
/* 0x07 */	{ "PnSize", 4, NULL, "pen size (point)" },
/* 0x08 */	{ "PnMode", 2, NULL, "pen mode (word)" },
/* 0x09 */	{ "PnPat", 8, NULL, "pen pattern" },
/* 0x0a */	{ "FillPat", 8, NULL, "fill pattern" },
/* 0x0b */	{ "OvSize", 4, NULL, "oval size (point)" },
/* 0x0c */	{ "Origin", 4, NULL, "dh, dv (word)" },
/* 0x0d */	{ "TxSize", 2, NULL, "text size (word)" },
/* 0x0e */	{ "FgColor", 4, NULL, "foreground color (longword)" },
/* 0x0f */	{ "BkColor", 4, NULL, "background color (longword)" },
/* 0x10 */	{ "TxRatio", 8, NULL, "numer (point), denom (point)" },
/* 0x11 */	{ "Version", 1, NULL, "version (byte)" },
/* 0x12 */	{ "BkPixPat", NA, BkPixPat, "color background pattern" },
/* 0x13 */	{ "PnPixPat", NA, PnPixPat, "color pen pattern" },
/* 0x14 */	{ "FillPixPat", NA, FillPixPat, "color fill pattern" },
/* 0x15 */	{ "PnLocHFrac", 2, NULL, "fractional pen position" },
/* 0x16 */	{ "ChExtra", 2, NULL, "extra for each character" },
/* 0x17 */	res(0),
/* 0x18 */	res(0),
/* 0x19 */	res(0),
/* 0x1a */	{ "RGBFgCol", RGB_LEN, NULL, "RGB foreColor" },
/* 0x1b */	{ "RGBBkCol", RGB_LEN, NULL, "RGB backColor" },
/* 0x1c */	{ "HiliteMode", 0, NULL, "hilite mode flag" },
/* 0x1d */	{ "HiliteColor", RGB_LEN, NULL, "RGB hilite color" },
/* 0x1e */	{ "DefHilite", 0, NULL, "Use default hilite color" },
/* 0x1f */	{ "OpColor", RGB_LEN, NULL, "RGB OpColor for arithmetic modes" },
/* 0x20 */	{ "Line", 8, NULL, "pnLoc (point), newPt (point)" },
/* 0x21 */	{ "LineFrom", 4, NULL, "newPt (point)" },
/* 0x22 */	{ "ShortLine", 6, NULL, "pnLoc (point, dh, dv (-128 .. 127))" },
/* 0x23 */	{ "ShortLineFrom", 2, NULL, "dh, dv (-128 .. 127)" },
/* 0x24 */	res(WORD_LEN),
/* 0x25 */	res(WORD_LEN),
/* 0x26 */	res(WORD_LEN),
/* 0x27 */	res(WORD_LEN),
/* 0x28 */	{ "LongText", NA, LongText, "txLoc (point), count (0..255), text" },
/* 0x29 */	{ "DHText", NA, DHText, "dh (0..255), count (0..255), text" },
/* 0x2a */	{ "DVText", NA, DVText, "dv (0..255), count (0..255), text" },
/* 0x2b */	{ "DHDVText", NA, DHDVText, "dh, dv (0..255), count (0..255), text" },
/* 0x2c */	res(WORD_LEN),
/* 0x2d */	res(WORD_LEN),
/* 0x2e */	res(WORD_LEN),
/* 0x2f */	res(WORD_LEN),
/* 0x30 */	{ "frameRect", 8, NULL, "rect" },
/* 0x31 */	{ "paintRect", 8, NULL, "rect" },
/* 0x32 */	{ "eraseRect", 8, NULL, "rect" },
/* 0x33 */	{ "invertRect", 8, NULL, "rect" },
/* 0x34 */	{ "fillRect", 8, NULL, "rect" },
/* 0x35 */	res(8),
/* 0x36 */	res(8),
/* 0x37 */	res(8),
/* 0x38 */	{ "frameSameRect", 0, NULL, "rect" },
/* 0x39 */	{ "paintSameRect", 0, NULL, "rect" },
/* 0x3a */	{ "eraseSameRect", 0, NULL, "rect" },
/* 0x3b */	{ "invertSameRect", 0, NULL, "rect" },
/* 0x3c */	{ "fillSameRect", 0, NULL, "rect" },
/* 0x3d */	res(0),
/* 0x3e */	res(0),
/* 0x3f */	res(0),
/* 0x40 */	{ "frameRRect", 8, NULL, "rect" },
/* 0x41 */	{ "paintRRect", 8, NULL, "rect" },
/* 0x42 */	{ "eraseRRect", 8, NULL, "rect" },
/* 0x43 */	{ "invertRRect", 8, NULL, "rect" },
/* 0x44 */	{ "fillRRrect", 8, NULL, "rect" },
/* 0x45 */	res(8),
/* 0x46 */	res(8),
/* 0x47 */	res(8),
/* 0x48 */	{ "frameSameRRect", 0, NULL, "rect" },
/* 0x49 */	{ "paintSameRRect", 0, NULL, "rect" },
/* 0x4a */	{ "eraseSameRRect", 0, NULL, "rect" },
/* 0x4b */	{ "invertSameRRect", 0, NULL, "rect" },
/* 0x4c */	{ "fillSameRRect", 0, NULL, "rect" },
/* 0x4d */	res(0),
/* 0x4e */	res(0),
/* 0x4f */	res(0),
/* 0x50 */	{ "frameOval", 8, NULL, "rect" },
/* 0x51 */	{ "paintOval", 8, NULL, "rect" },
/* 0x52 */	{ "eraseOval", 8, NULL, "rect" },
/* 0x53 */	{ "invertOval", 8, NULL, "rect" },
/* 0x54 */	{ "fillOval", 8, NULL, "rect" },
/* 0x55 */	res(8),
/* 0x56 */	res(8),
/* 0x57 */	res(8),
/* 0x58 */	{ "frameSameOval", 0, NULL, "rect" },
/* 0x59 */	{ "paintSameOval", 0, NULL, "rect" },
/* 0x5a */	{ "eraseSameOval", 0, NULL, "rect" },
/* 0x5b */	{ "invertSameOval", 0, NULL, "rect" },
/* 0x5c */	{ "fillSameOval", 0, NULL, "rect" },
/* 0x5d */	res(0),
/* 0x5e */	res(0),
/* 0x5f */	res(0),
/* 0x60 */	{ "frameArc", 12, NULL, "rect, startAngle, arcAngle" },
/* 0x61 */	{ "paintArc", 12, NULL, "rect, startAngle, arcAngle" },
/* 0x62 */	{ "eraseArc", 12, NULL, "rect, startAngle, arcAngle" },
/* 0x63 */	{ "invertArc", 12, NULL, "rect, startAngle, arcAngle" },
/* 0x64 */	{ "fillArc", 12, NULL, "rect, startAngle, arcAngle" },
/* 0x65 */	res(12),
/* 0x66 */	res(12),
/* 0x67 */	res(12),
/* 0x68 */	{ "frameSameArc", 4, NULL, "rect, startAngle, arcAngle" },
/* 0x69 */	{ "paintSameArc", 4, NULL, "rect, startAngle, arcAngle" },
/* 0x6a */	{ "eraseSameArc", 4, NULL, "rect, startAngle, arcAngle" },
/* 0x6b */	{ "invertSameArc", 4, NULL, "rect, startAngle, arcAngle" },
/* 0x6c */	{ "fillSameArc", 4, NULL, "rect, startAngle, arcAngle" },
/* 0x6d */	res(4),
/* 0x6e */	res(4),
/* 0x6f */	res(4),
/* 0x70 */	{ "framePoly", NA, skip_poly_or_region, "poly" },
/* 0x71 */	{ "paintPoly", NA, skip_poly_or_region, "poly" },
/* 0x72 */	{ "erasePoly", NA, skip_poly_or_region, "poly" },
/* 0x73 */	{ "invertPoly", NA, skip_poly_or_region, "poly" },
/* 0x74 */	{ "fillPoly", NA, skip_poly_or_region, "poly" },
/* 0x75 */	resf(skip_poly_or_region),
/* 0x76 */	resf(skip_poly_or_region),
/* 0x77 */	resf(skip_poly_or_region),
/* 0x78 */	{ "frameSamePoly", 0, NULL, "poly (NYI)" },
/* 0x79 */	{ "paintSamePoly", 0, NULL, "poly (NYI)" },
/* 0x7a */	{ "eraseSamePoly", 0, NULL, "poly (NYI)" },
/* 0x7b */	{ "invertSamePoly", 0, NULL, "poly (NYI)" },
/* 0x7c */	{ "fillSamePoly", 0, NULL, "poly (NYI)" },
/* 0x7d */	res(0),
/* 0x7e */	res(0),
/* 0x7f */	res(0),
/* 0x80 */	{ "frameRgn", NA, skip_poly_or_region, "region" },
/* 0x81 */	{ "paintRgn", NA, skip_poly_or_region, "region" },
/* 0x82 */	{ "eraseRgn", NA, skip_poly_or_region, "region" },
/* 0x83 */	{ "invertRgn", NA, skip_poly_or_region, "region" },
/* 0x84 */	{ "fillRgn", NA, skip_poly_or_region, "region" },
/* 0x85 */	resf(skip_poly_or_region),
/* 0x86 */	resf(skip_poly_or_region),
/* 0x87 */	resf(skip_poly_or_region),
/* 0x88 */	{ "frameSameRgn", 0, NULL, "region (NYI)" },
/* 0x89 */	{ "paintSameRgn", 0, NULL, "region (NYI)" },
/* 0x8a */	{ "eraseSameRgn", 0, NULL, "region (NYI)" },
/* 0x8b */	{ "invertSameRgn", 0, NULL, "region (NYI)" },
/* 0x8c */	{ "fillSameRgn", 0, NULL, "region (NYI)" },
/* 0x8d */	res(0),
/* 0x8e */	res(0),
/* 0x8f */	res(0),
/* 0x90 */	{ "BitsRect", NA, BitsRect, "copybits, rect clipped" },
/* 0x91 */	{ "BitsRgn", NA, BitsRegion, "copybits, rgn clipped" },
/* 0x92 */	res(WORD_LEN),
/* 0x93 */	res(WORD_LEN),
/* 0x94 */	res(WORD_LEN),
/* 0x95 */	res(WORD_LEN),
/* 0x96 */	res(WORD_LEN),
/* 0x97 */	res(WORD_LEN),
/* 0x98 */	{ "PackBitsRect", NA, BitsRect, "packed copybits, rect clipped" },
/* 0x99 */	{ "PackBitsRgn", NA, BitsRegion, "packed copybits, rgn clipped" },
/* 0x9a */	{ "Opcode_9A", NA, Opcode_9A, "the mysterious opcode 9A" },
/* 0x9b */	res(WORD_LEN),
/* 0x9c */	res(WORD_LEN),
/* 0x9d */	res(WORD_LEN),
/* 0x9e */	res(WORD_LEN),
/* 0x9f */	res(WORD_LEN),
/* 0xa0 */	{ "ShortComment", 2, NULL, "kind (word)" },
/* 0xa1 */	{ "LongComment", NA, LongComment, "kind (word), size (word), data" }
};

static int align = 0;
static FILE* ifp;

void
main(argc, argv)
int argc;
char* argv[];
{
	int argn;
	char* usage = "[-verbose] [pictfile]";

	ppm_init( &argc, argv );

	argn = 1;
	verbose = 0;

	if (argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0') {
		if (pm_keymatch(argv[argn], "-verbose", 2))
			verbose = 1;
		else
			pm_usage(usage);
		++argn;
	}

	if (argn < argc) {
		ifp = pm_openr(argv[argn]);
		++argn;
	} else
		ifp = stdin;

	if (argn != argc)
		pm_usage(usage);

	stage = "Reading 512 byte header";
	skip(512);

	interpret_pict();
	exit(0);
}

static void
interpret_pict()
{
	byte ch;
	word picSize;
	word opcode;
	word len;
	int version;

	stage = "Reading picture size";
	picSize = read_word();

	if (verbose)
		pm_message("picture size = %d (0x%x)", picSize, picSize);

	stage = "reading picture frame";
	read_rect(&picFrame);

	if (verbose) {
		dump_rect("Picture frame:", &picFrame);
		pm_message("Picture size is %d x %d",
			picFrame.right - picFrame.left,
			picFrame.bottom - picFrame.top);
	}

	/* allocation is same for version 1 or version 2.  We are super-duper
	 * wasteful of memory for version 1 picts.  Someday, we'll separate
	 * things and only allocate a byte per pixel for version 1 (or heck,
	 * even only a bit, but that would require even more extra work).
	 */

	rowlen = picFrame.right - picFrame.left;
	planelen = rowlen * (picFrame.bottom - picFrame.top);
	if ((red = (word*)malloc(planelen * sizeof(word))) == NULL ||
		(green = (word*)malloc(planelen * sizeof(word))) == NULL ||
		(blue = (word*)malloc(planelen * sizeof(word))) == NULL)

		pm_error("not enough memory to hold picture");

	while ((ch = read_byte()) == 0)
		;
	if (ch != 0x11)
		pm_error("No version number");

	switch (read_byte()) {
	case 1:
		version = 1;
		break;
	case 2:
		if (read_byte() != 0xff)
			pm_error("can only do version 2, subcode 0xff");
		version = 2;
		break;
	default:
		pm_error("Unknown version");
	}

	if (verbose)
		pm_message("PICT version %d", version);

	while((opcode = get_op(version)) != 0xff) {
		if (opcode < 0xa2) {
			if (verbose) {
				stage = optable[opcode].name;
				if (!strcmp(stage, "reserved"))
					pm_message("reserved opcode=0x%x", opcode);
				else
					pm_message("%s", stage = optable[opcode].name);
			}

			if (optable[opcode].impl != NULL)
				(*optable[opcode].impl)(version);
			else if (optable[opcode].len >= 0)
				skip(optable[opcode].len);
			else switch (optable[opcode].len) {
			case WORD_LEN:
				len = read_word();
				skip(len);
				break;
			default:
				pm_error("can't do length of %d",
					optable[opcode].len);
			}
		}
		else if (opcode == 0xc00) {
			if (verbose)
				pm_message("HeaderOp");
			stage = "HeaderOp";
			skip(24);
		}
		else if (opcode >= 0xa2 && opcode <= 0xaf) {
			stage = "skipping reserved";
			if (verbose)
				pm_message("%s 0x%x", stage, opcode);
			skip(read_word());
		}
		else if (opcode >= 0xb0 && opcode <= 0xcf) {
			/* just a reserved opcode, no data */
			if (verbose)
				pm_message("reserved 0x%x", opcode);
		}
		else if (opcode >= 0xd0 && opcode <= 0xfe) {
			stage = "skipping reserved";
			if (verbose)
				pm_message("%s 0x%x", stage, opcode);
			skip(read_long());
		}
		else if (opcode >= 0x100 && opcode <= 0x7fff) {
			stage = "skipping reserved";
			if (verbose)
				pm_message("%s 0x%x", stage, opcode);
			skip((opcode >> 7) & 255);
		}
		else if (opcode >= 0x8000 && opcode <= 0x80ff) {
			/* just a reserved opcode */
			if (verbose)
				pm_message("reserved 0x%x", opcode);
		}
		else if (opcode >= 8100 && opcode <= 0xffff) {
			stage = "skipping reserved";
			if (verbose)
				pm_message("%s 0x%x", stage, opcode);
			skip(read_long());
		}
		else
			pm_error("can't handle opcode of %x", opcode);
	}
	output_ppm(version);
}

static void
compact_plane(plane, planelen)
register word* plane;
register int planelen;
{
	register byte* p;

	for (p = (byte*)plane; planelen-- > 0; )
		*p++ = (*plane++ >> 8) & 255;
}

static void
output_ppm(version)
int version;
{
	int width;
	int height;
	word test;
	int offset1;
	register char* r;
	register char* g;
	register char* b;
	pixel* pixelrow;
	register pixel* pP;
	int row;
	register int col;

	/* determine byte order */
	test = 0x0001;
	offset1 = *((char*)&test);

	stage = "writing PPM";

	width = picFrame.right - picFrame.left;
	height = picFrame.bottom - picFrame.top;
	r = (char*) red;
	compact_plane((word*) r, width * height);
	g = (char*) green;
	compact_plane((word*) g, width * height);
	b = (char*) blue;
	compact_plane((word*) b, width * height);

	ppm_writeppminit(stdout, width, height, (pixval) 255, 0 );
	pixelrow = ppm_allocrow(width);
	for (row = 0; row < height; ++row) {
		for (col = 0, pP = pixelrow; col < width;
		     ++col, ++pP, ++r, ++g, ++b) {
			PPM_ASSIGN(*pP, *r, *g, *b);
		}

		ppm_writeppmrow(stdout, pixelrow, width, (pixval) 255, 0 );
	}
	pm_close(stdout);
}

/*
 * This could use read_pixmap, but I'm too lazy to hack read_pixmap.
 */

static void
Opcode_9A()
{
#ifdef DUMP
	FILE *fp = fopen("data", "w");
	int ch;
	if (fp == NULL) exit(1);
	while ((ch = fgetc(ifp)) != EOF) fputc(ch, fp);
	exit(0);
#else
	struct pixMap	p;
	struct Rect		srcRect;
	struct Rect		dstRect;
	byte*			pm;
	int				pixwidth;
	word			mode;

	/* skip fake len, and fake EOF */
	skip(4);
	read_word();	/* version */
	read_rect(&p.Bounds);
	pixwidth = p.Bounds.right - p.Bounds.left;
	p.packType = read_word();
	p.packSize = read_long();
	p.hRes = read_long();
	p.vRes = read_long();
	p.pixelType = read_word();
	p.pixelSize = read_word();
	p.pixelSize = read_word();
	p.cmpCount = read_word();
	p.cmpSize = read_word();
	p.planeBytes = read_long();
	p.pmTable = read_long();
	p.pmReserved = read_long();

	if (p.pixelSize == 16)
		pixwidth *= 2;
	else if (p.pixelSize == 32)
		pixwidth *= 3;

	read_rect(&srcRect);
	if (verbose)
		dump_rect("source rectangle:", &srcRect);

	read_rect(&dstRect);
	if (verbose)
		dump_rect("destination rectangle:", &dstRect);

	mode = read_word();
	if (verbose)
		pm_message("mode = %x", mode);

	pm = unpackbits(&p.Bounds, 0, p.pixelSize);

	blit(&srcRect, &(p.Bounds), pixwidth, pm, p.pixelSize,
		 &dstRect, &picFrame, rowlen,
		 NULL,
		 mode);

	free(pm);
#endif
}

static void
BitsRect(version)
int version;
{
	word rowBytes;

	stage = "Reading rowBytes for bitsrect";
	rowBytes = read_word();

	if (verbose)
		pm_message("rowbytes = 0x%x (%d)", rowBytes, rowBytes & 0x7fff);

	if (rowBytes & 0x8000)
		do_pixmap(version, rowBytes, 0);
	else
		do_bitmap(version, rowBytes, 0);
}

static void
BitsRegion(version)
int version;
{
	word rowBytes;

	stage = "Reading rowBytes for bitsregion";
	rowBytes = read_word();

	if (rowBytes & 0x8000)
		do_pixmap(version, rowBytes, 1);
	else
		do_bitmap(version, rowBytes, 1);
}

static void
do_bitmap(version, rowBytes, is_region)
int version;
int rowBytes;
int is_region;
{
	struct Rect Bounds;
	struct Rect srcRect;
	struct Rect dstRect;
	word mode;
	byte* pm;
	static struct ct_entry colour_table[] = { {65535L, 65535L, 65535L}, {0, 0, 0} };

	read_rect(&Bounds);
	read_rect(&srcRect);
	read_rect(&dstRect);
	mode = read_word();

	if (is_region)
		skip_poly_or_region(version);

	stage = "unpacking rectangle";

	pm = unpackbits(&Bounds, rowBytes, 1);

	blit(&srcRect, &Bounds, Bounds.right - Bounds.left, pm, 8,
		 &dstRect, &picFrame, rowlen,
		 colour_table,
		 mode);

	free(pm);
}

#if __STDC__
static void
do_pixmap( int version, word rowBytes, int is_region )
#else /*__STDC__*/
static void
do_pixmap(version, rowBytes, is_region)
int version;
word rowBytes;
int is_region;
#endif /*__STDC__*/
{
	word mode;
	struct pixMap p;
	word pixwidth;
	byte* pm;
	struct ct_entry* colour_table;
	struct Rect srcRect;
	struct Rect dstRect;

	read_pixmap(&p, NULL);

	pixwidth = p.Bounds.right - p.Bounds.left;

	if (verbose)
		pm_message("%d x %d rectangle", pixwidth,
			p.Bounds.bottom - p.Bounds.top);

	colour_table = read_colour_table();

	read_rect(&srcRect);

	if (verbose)
		dump_rect("source rectangle:", &srcRect);

	read_rect(&dstRect);

	if (verbose)
		dump_rect("destination rectangle:", &dstRect);

	mode = read_word();

	if (verbose)
		pm_message("mode = %x", mode);

	if (is_region)
		skip_poly_or_region(version);

	stage = "unpacking rectangle";

	pm = unpackbits(&p.Bounds, rowBytes, p.pixelSize);

	blit(&srcRect, &(p.Bounds), pixwidth, pm, 8,
		 &dstRect, &picFrame, rowlen,
		 colour_table,
		 mode);

	free(colour_table);
	free(pm);
}

static void
blit(srcRect, srcBounds, srcwid, srcplane, pixSize, dstRect, dstBounds, dstwid, colour_map, mode)
struct Rect* srcRect;
struct Rect* srcBounds;
int srcwid;
byte* srcplane;
int pixSize;
struct Rect* dstRect;
struct Rect* dstBounds;
int dstwid;
struct ct_entry* colour_map;
int mode;
{
	struct Rect clipsrc;
	struct Rect clipdst;
	register byte* src;
	register word* reddst;
	register word* greendst;
	register word* bluedst;
	register int i;
	register int j;
	int dstoff;
	int xsize;
	int ysize;
	int srcadd;
	int dstadd;
	struct ct_entry* ct;
	int pkpixsize;

	/* almost got it.  clip source rect with source bounds.
	 * clip dest rect with dest bounds.
	 * If they're not the same size - die!
	 * (it would require zeroing some area!)
	 * co-ordinate translations are actually done!
	 */
	rectinter(srcBounds, srcRect, &clipsrc);
	rectinter(dstBounds, dstRect, &clipdst);

	if (verbose) {
		dump_rect("copying from:", &clipsrc);
		dump_rect("to:          ", &clipdst);
	}

	if (!rectsamesize(&clipsrc, &clipdst))
		pm_message("warning, rectangles of different sizes after clipping!");


	/* lots of assumptions about 8 bits per component, chunky bits, etc! */
	/* :-( :-( */

	pkpixsize = 1;
	if (pixSize == 16)
		pkpixsize = 2;

	src = srcplane + (clipsrc.top - srcBounds->top) * srcwid +
		(clipsrc.left - srcBounds->left) * pkpixsize;
	dstoff = (clipdst.top - dstBounds->top) * dstwid +
		(clipdst.left - dstBounds->left);

	reddst = red + dstoff;
	greendst = green + dstoff;
	bluedst = blue + dstoff;

	xsize = clipsrc.right - clipsrc.left;
	ysize = clipsrc.bottom - clipsrc.top;
	srcadd = srcwid - xsize * pkpixsize;
	dstadd = dstwid - xsize;

	switch (pixSize) {
	case 8:
		for (i = 0; i < ysize; ++i) {
			for (j = 0; j < xsize; ++j) {
				ct = colour_map + *src++;
				*reddst++ = ct->red;
				*greendst++ = ct->green;
				*bluedst++ = ct->blue;
			}
			src += srcadd;
			reddst += dstadd;
			greendst += dstadd;
			bluedst += dstadd;
		}
		break;
	case 16:
		for (i = 0; i < ysize; ++i) {
			for (j = 0; j < xsize; ++j) {
				*reddst++ = (*src & 0x7c) << 9;
				*greendst = (*src++ & 3) << 14;
				*greendst++ |= (*src & 0xe0) << 6;
				*bluedst++ = (*src++ & 0x1f) << 11;
			}
			src += srcadd;
			reddst += dstadd;
			greendst += dstadd;
			bluedst += dstadd;
		}
		break;
	case 32:
		srcadd = (srcwid / 3) - xsize;
		for (i = 0; i < ysize; ++i) {
			for (j = 0; j < xsize; ++j)
				*reddst++ = *src++ << 8;

			reddst += dstadd;
			src += srcadd;

			for (j = 0; j < xsize; ++j)
				*greendst++ = *src++ << 8;

			greendst += dstadd;
			src += srcadd;

			for (j = 0; j < xsize; ++j)
				*bluedst++ = *src++ << 8;

			bluedst += dstadd;
			src += srcadd;
		}
	}
}

#if __STDC__
static byte*
unpackbits( struct Rect* bounds, word rowBytes, int pixelSize )
#else /*__STDC__*/
static byte*
unpackbits(bounds, rowBytes, pixelSize)
struct Rect* bounds;
word rowBytes;
int pixelSize;
#endif /*__STDC__*/
{
	byte* linebuf;
	byte* pm;
	byte* pm_ptr;
	register int i,j,k,l;
	word pixwidth;
	int linelen;
	int len;
	byte* bytepixels;
	int buflen;
	int pkpixsize;

	if (pixelSize <= 8)
		rowBytes &= 0x7fff;

	stage = "unpacking packbits";

	pixwidth = bounds->right - bounds->left;

	pkpixsize = 1;
	if (pixelSize == 16) {
		pkpixsize = 2;
		pixwidth *= 2;
	}
	else if (pixelSize == 32)
		pixwidth *= 3;
	
	if (rowBytes == 0)
		rowBytes = pixwidth;

	/* we're sloppy and allocate some extra space because we can overshoot
	 * by as many as 8 bytes when we unpack the raster lines.  Really, I
	 * should be checking to see if we go over the scan line (it is
	 * possbile) and complain of a corrupt file.  That fix is more complex
	 * (and probably costly in CPU cycles) and will have to come later.
	 */
	if ((pm = (byte*)malloc((pixwidth * (bounds->bottom - bounds->top) + 8) * sizeof(byte))) == NULL)
		pm_error("no mem for packbits rectangle");

	/* Sometimes we get rows with length > rowBytes.  I'll allocate some
	 * extra for slop and only die if the size is _way_ out of wack.
	 */
	if ((linebuf = (byte*)malloc(rowBytes + 100)) == NULL)
		pm_error("can't allocate memory for line buffer");

	if (rowBytes < 8) {
		/* ah-ha!  The bits aren't actually packed.  This will be easy */
		for (i = 0; i < bounds->bottom - bounds->top; i++) {
			pm_ptr = pm + i * pixwidth;
			read_n(buflen = rowBytes, (char*) linebuf);
			bytepixels = expand_buf(linebuf, &buflen, pixelSize);
			for (j = 0; j < buflen; j++)
				*pm_ptr++ = *bytepixels++;
		}
	}
	else {
		for (i = 0; i < bounds->bottom - bounds->top; i++) {
			pm_ptr = pm + i * pixwidth;
			if (rowBytes > 250 || pixelSize > 8)
				linelen = read_word();
			else
				linelen = read_byte();

			if (verbose)
				pm_message("linelen: %d", linelen);

			if (linelen > rowBytes) {
				pm_message("linelen > rowbytes! (%d > %d) at line %d",
					linelen, rowBytes, i);
			}

			read_n(linelen, (char*) linebuf);

			for (j = 0; j < linelen; ) {
				if (linebuf[j] & 0x80) {
					len = ((linebuf[j] ^ 255) & 255) + 2;
					buflen = pkpixsize;
					bytepixels = expand_buf(linebuf + j+1, &buflen, pixelSize);
					for (k = 0; k < len; k++) {
						for (l = 0; l < buflen; l++)
							*pm_ptr++ = *bytepixels++;
						bytepixels -= buflen;
					}
					j += 1 + pkpixsize;
				}
				else {
					len = (linebuf[j] & 255) + 1;
					buflen = len * pkpixsize;
					bytepixels = expand_buf(linebuf + j+1, &buflen, pixelSize);
					for (k = 0; k < buflen; k++)
						*pm_ptr++ = *bytepixels++;
					j += len * pkpixsize + 1;
				}
			}
		}
	}

	free(linebuf);

	return(pm);
}

static byte*
expand_buf(buf, len, bits_per_pixel)
byte* buf;
int* len;
int bits_per_pixel;
{
	static byte pixbuf[256 * 8];
	register byte* src;
	register byte* dst;
	register int i;

	src = buf;
	dst = pixbuf;

	switch (bits_per_pixel) {
	case 8:
	case 16:
	case 32:
		return(buf);
	case 4:
		for (i = 0; i < *len; i++) {
			*dst++ = (*src >> 4) & 15;
			*dst++ = *src++ & 15;
		}
		*len *= 2;
		break;
	case 2:
		for (i = 0; i < *len; i++) {
			*dst++ = (*src >> 6) & 3;
			*dst++ = (*src >> 4) & 3;
			*dst++ = (*src >> 2) & 3;
			*dst++ = *src++ & 3;
		}
		*len *= 4;
		break;
	case 1:
		for (i = 0; i < *len; i++) {
			*dst++ = (*src >> 7) & 1;
			*dst++ = (*src >> 6) & 1;
			*dst++ = (*src >> 5) & 1;
			*dst++ = (*src >> 4) & 1;
			*dst++ = (*src >> 3) & 1;
			*dst++ = (*src >> 2) & 1;
			*dst++ = (*src >> 1) & 1;
			*dst++ = *src++ & 1;
		}
		*len *= 8;
		break;
	default:
		pm_error("bad bits per pixel in expand_buf");
	}
	return(pixbuf);
}

static void
Clip(version)
int version;
{
	skip(read_word() - 2);
}

static void
read_pixmap(p, rowBytes)
struct pixMap* p;
word* rowBytes;
{
	stage = "getting pixMap header";

	if (rowBytes != NULL)
		*rowBytes = read_word();

	read_rect(&p->Bounds);
	p->version = read_word();
	p->packType = read_word();
	p->packSize = read_long();
	p->hRes = read_long();
	p->vRes = read_long();
	p->pixelType = read_word();
	p->pixelSize = read_word();
	p->cmpCount = read_word();
	p->cmpSize = read_word();
	p->planeBytes = read_long();
	p->pmTable = read_long();
	p->pmReserved = read_long();

	if (verbose) {
		pm_message("pixelType: %d", p->pixelType);
		pm_message("pixelSize: %d", p->pixelSize);
		pm_message("cmpCount:  %d", p->cmpCount);
		pm_message("cmpSize:   %d", p->cmpSize);
	}

	if (p->pixelType != 0)
		pm_error("sorry, I only do chunky format");
	if (p->cmpCount != 1)
		pm_error("sorry, cmpCount != 1");
	if (p->pixelSize != p->cmpSize)
		pm_error("oops, pixelSize != cmpSize");
}

static struct ct_entry*
read_colour_table()
{
	longword ctSeed;
	word ctFlags;
	word ctSize;
	word val;
	int i;
	struct ct_entry* colour_table;

	stage = "getting color table info";

	ctSeed = read_long();
	ctFlags = read_word();
	ctSize = read_word();

	if (verbose) {
		pm_message("ctSeed:  %d", ctSeed);
		pm_message("ctFlags: %d", ctFlags);
		pm_message("ctSize:  %d", ctSize);
	}

	stage = "reading colour table";

	if ((colour_table = (struct ct_entry*) malloc(sizeof(struct ct_entry) * (ctSize + 1))) == NULL)
		pm_error("no memory for colour table");

	for (i = 0; i <= ctSize; i++) {
		if ((val = read_word()) > ctSize)
			pm_error("pixel value greater than colour table size");
		/* seems that if we have a device colour table, the val is
		 * always zero, so I assume we allocate up the list of colours.
		 */
		if (ctFlags & 0x8000)
			val = i;
		colour_table[val].red = read_word();
		colour_table[val].green = read_word();
		colour_table[val].blue = read_word();

		if (verbose)
			pm_message("%d: [%d,%d,%d]", val,
				colour_table[val].red,
				colour_table[val].green,
				colour_table[val].blue, 0);
	}

	return(colour_table);
}

/* these 3 do nothing but skip over their data! */
static void
BkPixPat(version)
int version;
{
	read_pattern();
}

static void
PnPixPat(version)
int version;
{
	read_pattern();
}

static void
FillPixPat(version)
int version;
{
	read_pattern();
}

/* this just skips over a version 2 pattern.  Probabaly will return
 * a pattern in the fabled complete version.
 */
static void
read_pattern()
{
	word PatType;
	word rowBytes;
	struct pixMap p;
	byte* pm;
	struct ct_entry* ct;

	stage = "Reading a pattern";

	PatType = read_word();

	switch (PatType) {
	case 2:
		skip(8); /* old pattern data */
		skip(5); /* RGB for pattern */
		break;
	case 1:
		skip(8); /* old pattern data */
		read_pixmap(&p, &rowBytes);
		ct = read_colour_table();
		pm = unpackbits(&p.Bounds, rowBytes, p.pixelSize);
		free(pm);
		free(ct);
		break;
	default:
		pm_error("unknown pattern type in read_pattern");
	}
}

/* more stubs for text output */

static void
skip_text()
{
	skip(read_byte());
}

static void
LongText(version)
int version;
{
	skip(4);
	skip_text();
}

static void
DHText(version)
int version;
{
	skip(1);
	skip_text();
}

static void
DVText(version)
int version;
{
	skip(1);
	skip_text();
}

static void
DHDVText(version)
int version;
{
	skip(2);
	skip_text();
}

static void
skip_poly_or_region(version)
int version;
{
	stage = "skipping polygon or region";
	skip(read_word() - 2);
}

static void
LongComment()
{
	stage = "skipping longword comment";

	skip(2);
	skip(read_word());
}

static int
rectequal(r1, r2)
struct Rect* r1;
struct Rect* r2;
{
	return(r1->top == r2->top &&
		   r1->bottom == r2->bottom &&
		   r1->left == r2->left &&
		   r1->right == r2->right);
}

static int
rectsamesize(r1, r2)
struct Rect* r1;
struct Rect* r2;
{
	return(r1->right - r1->left == r2->right - r2->left &&
		   r1->bottom - r1->top == r2->bottom - r2->top);
}

static void
rectinter(r1, r2, r3)
struct Rect* r1;
struct Rect* r2;
struct Rect* r3;
{
	r3->left = max(r1->left, r2->left);
	r3->top = max(r1->top, r2->top);
	r3->right = min(r1->right, r2->right);
	r3->bottom = min(r1->bottom, r2->bottom);
}

static void
read_rect(r)
struct Rect* r;
{
	r->top = read_word();
	r->left = read_word();
	r->bottom = read_word();
	r->right = read_word();
}

static void
dump_rect(s, r)
char* s;
struct Rect* r;
{
	pm_message("%s (%d,%d) (%d,%d)",
		s, r->left, r->top, r->right, r->bottom);
}

/*
 * All data in version 2 is 2-byte word aligned.  Odd size data
 * is padded with a null.
 */
static word
get_op(version)
int version;
{
	if ((align & 1) && version == 2) {
		stage = "aligning for opcode";
		read_byte();
	}

	stage = "reading opcode";

	if (version == 1)
		return(read_byte());
	else
		return(read_word());
}

static longword
read_long()
{
	word i;

	i = read_word();
	return((i << 16) | read_word());
}

static word
read_word()
{
	byte b;

	b = read_byte();

	return((b << 8) | read_byte());
}

static byte
read_byte()
{
	int c;

	if ((c = fgetc(ifp)) == EOF)
		pm_error("EOF / read error while %s", stage);

	++align;
	return(c & 255);
}

static void
skip(n)
int n;
{
	static byte buf[1024];

	align += n;

	for (; n > 0; n -= 1024)
		if (fread(buf, n > 1024 ? 1024 : n, 1, ifp) != 1)
			pm_error("EOF / read error while %s", stage);
}

static void
read_n(n, buf)
int n;
char* buf;
{
	align += n;

	if (fread(buf, n, 1, ifp) != 1)
		pm_error("EOF / read error while %s", stage);
}
