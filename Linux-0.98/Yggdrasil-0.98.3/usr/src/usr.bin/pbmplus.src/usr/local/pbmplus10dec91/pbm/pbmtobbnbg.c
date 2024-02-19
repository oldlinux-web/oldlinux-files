/* pbmtobg.c - read a portable bitmap and produce BitGraph graphics
**
** Copyright 1989 by Mike Parker.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/
  
/*
** Changed to take advantage of negative Packed Pixed Data values and
** supply ANSI-standard string terminator.  Paul Milazzo, 28 May 1990.
*/

#include "pbm.h"

static void write16 ARGS(( unsigned int	));

static int nco;

void
main(argc,argv)
int argc;
char **argv;
{
 int rows;
 int cols;
 int format;
 bit *bitrow;
 int row;
 unsigned int sixteen;
 int i;
 unsigned int mask;
 int op;
 int x;
 int y;

 pbm_init( &argc, argv );

 op = 3;
 switch (argc)
  { case 1:
       break;
    case 2:
       op = atoi(argv[1]);
       break;
    case 3:
       x = atoi(argv[1]);
       y = atoi(argv[2]);
       printf("\33:%d;%dm",x,y);
       break;
    case 4:
       op = atoi(argv[1]);
       x = atoi(argv[2]);
       y = atoi(argv[3]);
       printf("\33:%d;%dm",x,y);
       break;
  }
 nco = 0;
 pbm_readpbminit(stdin,&cols,&rows,&format);
 printf("\33P:%d;%d;%ds\n",op,cols,rows);
 bitrow = pbm_allocrow(cols);
 for (row=0;row<rows;row++)
  { pbm_readpbmrow(stdin,bitrow,cols,format);
    sixteen = 0;
    mask = 0x8000;
    for (i=0;i<cols;i++)
     { if (bitrow[i]==PBM_BLACK) sixteen |= mask;
       mask >>= 1;
       if (mask == 0)
	{ mask = 0x8000;
	  write16(sixteen);
	  sixteen = 0;
	}
     }
    if (mask != 0x8000)
     { write16(sixteen);
     }
  }
 puts("\033\\");
 exit(0);
}

#ifdef POSITIVE_VALUES_ONLY
static void
write16(sixteen)
unsigned int sixteen;
{
 if (nco > 75)
  { putchar('\n');
    nco = 0;
  }
 if (sixteen & 0xfc00)
  { putchar(0100+(sixteen>>10));
    nco ++;
  }
 if (sixteen & 0xfff0)
  { putchar(0100+((sixteen>>4)&0x3f));
    nco ++;
  }
 putchar(060+(sixteen&0xf));
 nco ++;
}
#else
/*
 *  This version of "write16" uses negative Packed Pixel Data values to
 *  represent numbers in the range 0x7fff--0xffff; negative values will
 *  require fewer characters as they approach the upper end of that range.
 */
static void
write16 (word)
unsigned int	word;
{
    int		high;
    int		mid;
    int		low;
    int		signChar;

    if (nco > 75) {
	putchar ('\n');
	nco = 0;
    }

    if (word > 0x7fff) {
	word = (unsigned int) (0x10000L - (long) word);
	signChar = ' ';
    }
    else
	signChar = '0';

    high = (word >> 10) + '@';
    mid	= ((word & 0x3f0) >> 4) + '@';
    low	= (word & 0xf) + signChar;

    if (high != '@') {
	printf ("%c%c%c", high, mid, low);
	nco += 3;
    }
    else if (mid != '@') {
	printf ("%c%c", mid, low);
	nco += 2;
    }
    else {
	putchar (low);
	nco++;
    }
}
#endif
