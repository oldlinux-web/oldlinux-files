/* libpbm5.c - pbm utility library part 5
**
** Font routines.
**
** Copyright (C) 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pbm.h"
#include "pbmfont.h"

/* The default font, packed in hex so this source file doesn't get huge.
** You can replace this with your own font using pbm_dumpfont().
*/
#define DEFAULTFONT_ROWS 155
#define DEFAULTFONT_COLS 112
static unsigned long defaultfont_bits[DEFAULTFONT_ROWS][(DEFAULTFONT_COLS+31)/32] = {
    {0x00000000L,0x20000c00L,0x10000000L,0x00000000L},
    {0xc600a000L,0x42000810L,0x00000002L,0x00000063L},
    {0x6c00a000L,0x45000810L,0x00000002L,0x00000036L},
    {0x6c00a000L,0x88800808L,0xf2e1dee2L,0x00000036L},
    {0x54000000L,0x80000800L,0x11122442L,0x0000002aL},
    {0x54000001L,0x00000800L,0x11122442L,0x0000002aL},
    {0x54000001L,0x00000800L,0x11122282L,0x0000002aL},
    {0x44000102L,0x00000800L,0x11122382L,0x00000022L},
    {0xee000102L,0x00000800L,0x11e1e102L,0x00000077L},
    {0x00000204L,0x00000800L,0x11002102L,0x00000000L},
    {0x00000000L,0x00000c00L,0x11002102L,0x00000000L},
    {0x00000000L,0x003f8000L,0xe3807600L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x02000080L,0x00040000L,0x00120000L,0x00000001L},
    {0x04000082L,0x828e1838L,0x20210100L,0x00000002L},
    {0x04000082L,0x82912448L,0x20210100L,0x00000002L},
    {0x08000082L,0x8fd01940L,0x404087c2L,0x00000004L},
    {0x08000080L,0x050c0622L,0x00408102L,0x00000004L},
    {0x10000080L,0x05061874L,0x0040828fL,0x00008008L},
    {0x10000080L,0x1f912688L,0x00408002L,0x00000008L},
    {0x20000000L,0x0a11098cL,0x00408002L,0x00000010L},
    {0x20000080L,0x0a0e0672L,0x00210000L,0x00000010L},
    {0x40000000L,0x00040000L,0x00210000L,0x00000020L},
    {0x00000000L,0x00000000L,0x00120000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x004e0838L,0x7023e1cfL,0x00008000L},
    {0x00000000L,0x00913844L,0x88620208L,0x00008000L},
    {0x08000000L,0x00910844L,0x08a20401L,0x00000004L},
    {0x10000000L,0x01110844L,0x08a20401L,0x00000008L},
    {0x20000000L,0x01110808L,0x3123c781L,0x00000010L},
    {0x400003e0L,0x02110810L,0x0a202441L,0x00000020L},
    {0x20000000L,0x02110820L,0x0bf02442L,0x00000010L},
    {0x10008000L,0x04110844L,0x88242442L,0x00000008L},
    {0x08008002L,0x040e3e7cL,0x7073c382L,0x00000004L},
    {0x00010000L,0x08000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x0000e1c0L,0x00000000L,0x00000000L,0x00000000L},
    {0x00011220L,0x00000000L,0x70e38f87L,0x00000000L},
    {0x20011220L,0x00020020L,0x89108448L,0x00008010L},
    {0x10011220L,0x00040010L,0x09314448L,0x00008008L},
    {0x0800e221L,0x02083e08L,0x11514788L,0x00000004L},
    {0x040111e0L,0x00100004L,0x2153e448L,0x00000002L},
    {0x08011020L,0x00083e08L,0x213a2448L,0x00008004L},
    {0x10011040L,0x02040010L,0x01022448L,0x00008008L},
    {0x2000e381L,0x02020020L,0x20e77f87L,0x00000010L},
    {0x00000000L,0x04000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x3803e7efL,0xc73bbe3dL,0xdb863ce7L,0x0000001cL},
    {0x44011224L,0x48910808L,0x91036648L,0x00008022L},
    {0x4c011285L,0x48910808L,0xa1036648L,0x00008026L},
    {0x54011387L,0x081f0808L,0xc102a548L,0x0000802aL},
    {0x54011285L,0x09910808L,0xe102a548L,0x0000802aL},
    {0x4e011204L,0x08910848L,0x9112a4c8L,0x00008027L},
    {0x40011224L,0x08910848L,0x891224c8L,0x00008020L},
    {0x3803e7efL,0x073bbe31L,0xcff77e47L,0x0000001cL},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000003L,0x00000000L},
    {0x0003e1cfL,0x87bff7efL,0xdfbf77c2L,0x00000000L},
    {0x00013224L,0x48a4a244L,0x89122442L,0x00000000L},
    {0x00011224L,0x4824a244L,0xa8a14482L,0x00000000L},
    {0x00013227L,0x8e04226cL,0xa8414102L,0x00000000L},
    {0x0001e224L,0x83842228L,0xa8a08102L,0x00000000L},
    {0x00010224L,0x40842228L,0xd8a08242L,0x00000000L},
    {0x00010224L,0x48843638L,0x51108442L,0x00000000L},
    {0x0003c1ceL,0x6f1f1c10L,0x53b9c7c2L,0x00000000L},
    {0x00000060L,0x00000000L,0x00000002L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000003L,0x00000000L},
    {0xfe000000L,0x00000000L,0x00000000L,0x0000007fL},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00010180L,0x000000c0L,0x003001c0L,0x00000000L},
    {0x08008081L,0x00040040L,0x00100200L,0x00000004L},
    {0x10008082L,0x80040040L,0x00100200L,0x00000008L},
    {0x10004084L,0x40023c78L,0x70f1c7c7L,0x00004008L},
    {0x10004080L,0x00000244L,0x89122208L,0x00008008L},
    {0x20002080L,0x00001e44L,0x8113e208L,0x00008010L},
    {0x10002080L,0x00002244L,0x81120208L,0x00008008L},
    {0x10001080L,0x00002244L,0x89122208L,0x00008008L},
    {0x10001080L,0x00001db8L,0x70e9c787L,0x00008008L},
    {0x10000880L,0x00000000L,0x00000000L,0x00008008L},
    {0x08000180L,0x00000000L,0x00000000L,0x00008004L},
    {0x00000000L,0x1fc00000L,0x00000007L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00030080L,0x981c0000L,0x00000000L,0x00000000L},
    {0x20010000L,0x08040000L,0x00000000L,0x00000010L},
    {0x10010000L,0x08040000L,0x00000000L,0x00000008L},
    {0x10016387L,0x898474b8L,0x72e1d5c7L,0x00000008L},
    {0x10019080L,0x8a042a64L,0x89122208L,0x00008008L},
    {0x08011080L,0x8c042a44L,0x89122207L,0x00000004L},
    {0x10011080L,0x8a042a44L,0x89122200L,0x00008008L},
    {0x10011080L,0x89042a44L,0x89122208L,0x00008008L},
    {0x1003bbe0L,0x98dfebe6L,0x71e1e787L,0x00000008L},
    {0x10000000L,0x80000000L,0x01002000L,0x00000008L},
    {0x20000000L,0x80000000L,0x01002000L,0x00000010L},
    {0x00000007L,0x00000000L,0x03807000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00008000L,0x00000000L,0x10410000L,0x00000000L},
    {0x00008000L,0x00000000L,0x20408000L,0x00000000L},
    {0x0001f66eL,0xfdfbf77cL,0x20408000L,0x00000000L},
    {0x24008224L,0x488a2248L,0x20408240L,0x00000012L},
    {0x54008224L,0x4a842210L,0x40404540L,0x0000002aL},
    {0x48008222L,0x8a8a1420L,0x20408480L,0x00000024L},
    {0x00008a23L,0x85111c44L,0x20408000L,0x00000000L},
    {0x000071d1L,0x0531887cL,0x20408000L,0x00000000L},
    {0x00000000L,0x00000800L,0x20408000L,0x00000000L},
    {0x00000000L,0x00000800L,0x10410000L,0x00000000L},
    {0x00000000L,0x00003000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x00000000L,0x00000000L,0x00000000L},
    {0x00000000L,0x20000c00L,0x10000000L,0x00000000L},
    {0xc600a000L,0x42000810L,0x00000002L,0x00000063L},
    {0x6c00a000L,0x45000810L,0x00000002L,0x00000036L},
    {0x6c00a000L,0x88800808L,0xf2e1dee2L,0x00000036L},
    {0x54000000L,0x80000800L,0x11122442L,0x0000002aL},
    {0x54000001L,0x00000800L,0x11122442L,0x0000002aL},
    {0x54000001L,0x00000800L,0x11122282L,0x0000002aL},
    {0x44000102L,0x00000800L,0x11122382L,0x00000022L},
    {0xee000102L,0x00000800L,0x11e1e102L,0x00000077L},
    {0x00000204L,0x00000800L,0x11002102L,0x00000000L},
    {0x00000000L,0x00000c00L,0x11002102L,0x00000000L},
    {0x00000000L,0x003f8000L,0xe3807600L,0x00000000L}
    };

bit**
pbm_defaultfont( fcolsP, frowsP )
    int* fcolsP;
    int* frowsP;
    {
    bit** defaultfont;
    int row, col, scol;
    unsigned long l;

    defaultfont = pbm_allocarray( DEFAULTFONT_COLS, DEFAULTFONT_ROWS );
    for ( row = 0; row < DEFAULTFONT_ROWS; ++row )
	{
	for ( col = 0; col < DEFAULTFONT_COLS; col += 32 )
	    {
	    l = defaultfont_bits[row][col / 32];
	    for ( scol = min( col + 32, DEFAULTFONT_COLS ) - 1;
		  scol >= col; --scol )
		{
		if ( l & 1 )
		    defaultfont[row][scol] = 1;
		else
		    defaultfont[row][scol] = 0;
		l >>= 1;
		}
	    }
	}

    *fcolsP = DEFAULTFONT_COLS;
    *frowsP = DEFAULTFONT_ROWS;
    return defaultfont;
    }

void
pbm_dissectfont( font, frows, fcols, char_heightP, char_widthP, char_aheightP, char_awidthP, char_row0, char_col0 )
    bit** font;
    int frows;
    int fcols;
    int* char_heightP;
    int* char_widthP;
    int* char_aheightP;
    int* char_awidthP;
    int char_row0[95];
    int char_col0[95];
    {
    /*
    ** This routine expects a font bitmap representing the following text:
    **
    ** (0,0)
    **    M ",/^_[`jpqy| M
    **
    **    /  !"#$%&'()*+ /
    **    < ,-./01234567 <
    **    > 89:;<=>?@ABC >
    **    @ DEFGHIJKLMNO @
    **    _ PQRSTUVWXYZ[ _
    **    { \]^_`abcdefg {
    **    } hijklmnopqrs }
    **    ~ tuvwxyz{|}~  ~
    **
    **    M ",/^_[`jpqy| M
    **
    ** The bitmap must be cropped exactly to the edges.
    **
    ** The dissection works by finding the first blank row and column; that
    ** gives the height and width of the maximum-sized character, which is
    ** not too useful.  But the distance from there to the opposite side is
    ** an integral multiple of the cell size, and that's what we need.  Then
    ** it's just a matter of filling in all the coordinates.
    **
    ** The difference between char_height, char_width and char_aheight,
    ** char_awidth is that the first is the size of the cell including
    ** spacing, while the second is just the actual maximum-size character.
    */
    int brow, bcol, row, col, d, ch;
    bit b;

    /* Find first blank row. */
    for ( brow = 0; brow < frows / 6; ++brow )
	{
	b = font[brow][0];
	for ( col = 1; col < fcols; ++col )
	    if ( font[brow][col] != b )
		goto nextrow;
	goto gotblankrow;
    nextrow: ;
	}
    pm_error( "couldn't find blank row in font" );

gotblankrow:
    /* Find first blank col. */
    for ( bcol = 0; bcol < fcols / 8; ++bcol )
	{
	b = font[0][bcol];
	for ( row = 1; row < frows; ++row )
	    if ( font[row][bcol] != b )
		goto nextcol;
	goto gotblankcol;
    nextcol: ;
	}
    pm_error( "couldn't find blank col in font" );

gotblankcol:
    /* Now compute character cell size. */
    d = frows - brow;
    *char_heightP = d / 11;
    if ( *char_heightP * 11 != d )
	pm_error( "problem computing character cell height" );
    d = fcols - bcol;
    *char_widthP = d / 15;
    if ( *char_widthP * 15 != d )
	pm_error( "problem computing character cell width" );
    *char_aheightP = brow;
    *char_awidthP = bcol;

    /* Now fill in the 0,0 coords. */
    row = *char_heightP * 2;
    col = *char_widthP * 2;
    for ( ch = 0; ch < 95; ++ch )
	{
	char_row0[ch] = row;
	char_col0[ch] = col;
	col += *char_widthP;
	if ( col >= *char_widthP * 14 )
	    {
	    col = *char_widthP * 2;
	    row += *char_heightP;
	    }
	}
    }

void
pbm_dumpfont( font, fcols, frows )
    bit** font;
    int fcols;
    int frows;
    {
    /* Dump out font as C source code. */
    int row, col, scol, lperrow;
    unsigned long l;

    printf( "#define DEFAULTFONT_ROWS %d\n", frows );
    printf( "#define DEFAULTFONT_COLS %d\n", fcols );
    printf( "static unsigned long defaultfont_bits[DEFAULTFONT_ROWS][(DEFAULTFONT_COLS+31)/32] = {\n" );
    for ( row = 0; row < frows; ++row )
	{
	lperrow = 0;
	for ( col = 0; col < fcols; col += 32 )
	    {
	    if ( lperrow == 0 )
		printf( "    {" );
	    else if ( lperrow % 6 == 0 )
		{
		printf( ",\n     " );
		lperrow = 0;
		}
	    else
		printf( "," );
	    l = 0;
	    for ( scol = col; scol < min( col + 32, fcols ); ++scol )
		{
		l <<= 1;
		if ( font[row][scol] )
		    l |= 1;
		}
	    printf( "0x%08lxL", l );
	    ++lperrow;
	    }
	printf( "}%s\n", row == frows - 1 ? "" : "," );
	}
    printf( "    };\n" );
    }
