/* pbmto10x.c - read a portable bitmap and produce a Gemini 10X printer file
**
** Copyright (C) 1990 by Ken Yap
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pbm.h"

#define	LOW_RES_ROWS	8		/* printed per pass */
#define	HIGH_RES_ROWS	16		/* printed per pass */

static void res_60x72 ARGS(( void ));
static void res_120x144 ARGS(( void ));

static int	highres = 0;
static FILE	*ifp;
static int	rows, cols, format;

void
main(argc, argv)
	int	argc;
	char	*argv[];
{
	pbm_init( &argc, argv );
	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'h')
	{
		highres = 1;
		--argc;
		++argv;
	}
	if (argc > 2)
		pm_usage("[pbmfile]");
	if (argc == 2)
		ifp = pm_openr(argv[1]);
	else
		ifp = stdin;

	pbm_readpbminit(ifp, &cols, &rows, &format);
	if (highres)
		res_120x144();
	else
		res_60x72();

	pm_close(ifp);
	exit(0);
}

static void
res_60x72()
{
	register int	i, item, npins, row, col;
	bit		*bitrows[LOW_RES_ROWS], *bP[LOW_RES_ROWS];

	for (i = 0; i < LOW_RES_ROWS; ++i)
		bitrows[i] = pbm_allocrow(cols);
	printf("\033A\010");		/* '\n' = 8/72 */
	for (row = 0; row < rows; row += LOW_RES_ROWS)
	{
		if (row + LOW_RES_ROWS <= rows)
			npins = LOW_RES_ROWS;
		else
			npins = rows - row;
		for (i = 0; i < npins; ++i)
			pbm_readpbmrow(ifp, bP[i] = bitrows[i], cols, format);
		printf("\033K%c%c", cols % 256, cols / 256);
		for (col = 0; col < cols; ++col)
		{
			item = 0;
			for (i = 0; i < npins; ++i)
				if (*(bP[i]++) == PBM_BLACK)
					item |= 1 << (7 - i);
			putchar(item);
		}
		putchar('\n');
	}
}

static void
res_120x144()
{
	register int	i, pin, item, npins, row, col;
	bit		*bitrows[HIGH_RES_ROWS], *bP[HIGH_RES_ROWS];

	for (i = 0; i < HIGH_RES_ROWS; ++i)
		bitrows[i] = pbm_allocrow(cols);
	putchar('\033'); putchar('3'); putchar('\0');
	for (row = 0; row < rows; row += HIGH_RES_ROWS)
	{
		if (row + HIGH_RES_ROWS <= rows)
			npins = HIGH_RES_ROWS;
		else
			npins = rows - row;
		for (i = 0; i < npins; ++i)
			pbm_readpbmrow(ifp, bP[i] = bitrows[i], cols, format);
		printf("\033L%c%c", cols % 256, cols / 256);
		for (col = 0; col < cols; ++col)
		{
			item = 0;
			/* even rows */
			for (pin = i = 0; i < npins; i += 2, ++pin)
				if (*(bP[i]++) == PBM_BLACK)
					item |= 1 << (7 - pin);
			putchar(item);
		}
		putchar('\n');			/* flush buffer */
		printf("\033J\001");		/* 1/144 down */
		printf("\033L%c%c", cols % 256, cols / 256);
		for (col = 0; col < cols; ++col)
		{
			item = 0;
			/* odd rows */
			for (i = 1, pin = 0; i < npins; i += 2, ++pin)
				if (*(bP[i]++) == PBM_BLACK)
					item |= 1 << (7 - pin);
			putchar(item);
		}
		putchar('\n');			/* flush buffer */
		printf("\033J\017");		/* 15/144 down */
	}
}
