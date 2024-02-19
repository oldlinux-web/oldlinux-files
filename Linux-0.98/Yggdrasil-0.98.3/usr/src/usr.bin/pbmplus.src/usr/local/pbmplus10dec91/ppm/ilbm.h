/* ilbm.h - header file for IFF ILBM files
*/

#define RowBytes(cols)		( ( ( (cols) + 15 ) / 16 ) * 2 )

/* definitions for BMHD */

typedef struct
    {
    unsigned short w, h;
    short x, y;
    unsigned char nPlanes, masking, compression, pad1;
    unsigned short transparentColor;
    unsigned char xAspect, yAspect;
    short pageWidth, pageHeight;
    } BitMapHeader;

#define mskNone			0
#define mskHasMask		1
#define mskHasTransparentColor	2
#define mskLasso		3

#define cmpNone			0
#define cmpByteRun1		1

/* definitions for CMAP */

typedef struct
    {
    unsigned char r, g, b;
    } ColorRegister;

/* definitions for CAMG */

#define	vmEXTRA_HALFBRITE	0x80
#define	vmHAM			0x800
