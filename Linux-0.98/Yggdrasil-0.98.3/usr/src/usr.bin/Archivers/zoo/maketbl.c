/*$Source: /usr/home/dhesi/zoo/RCS/maketbl.c,v $*/
/*$Id: maketbl.c,v 1.8 91/07/09 01:39:52 dhesi Exp $*/
/***********************************************************
	maketbl.c -- make table for decoding

Adapted from "ar" archiver written by Haruhiko Okumura.
***********************************************************/
#include "options.h"
#include "zoo.h"
#include "ar.h"
#include "lzh.h"

extern void prterror();

void make_table(nchar, bitlen, tablebits, table)
int nchar;
uchar bitlen[];
int tablebits;
ushort table[];
{
	ushort count[17], weight[17], start[18], *p;
	uint i, k, len, ch, jutbits, avail, nextcode, mask;

	for (i = 1; i <= 16; i++) count[i] = 0;
	for (i = 0; i < nchar; i++) count[bitlen[i]]++;

	start[1] = 0;
	for (i = 1; i <= 16; i++)
		start[i + 1] = start[i] + (count[i] << (16 - i));
	if (start[17] != (ushort)((unsigned) 1 << 16))
		prterror('f', "Bad decode table\n");

	jutbits = 16 - tablebits;
	for (i = 1; i <= tablebits; i++) {
		start[i] >>= jutbits;
		weight[i] = (unsigned) 1 << (tablebits - i);
	}
	while (i <= 16) {
	   weight[i] = (unsigned) 1 << (16 - i);
	   i++;
        }

	i = start[tablebits + 1] >> jutbits;
	if (i != (ushort)((unsigned) 1 << 16)) {
		k = 1 << tablebits;
		while (i != k) table[i++] = 0;
	}

	avail = nchar;
	mask = (unsigned) 1 << (15 - tablebits);
	for (ch = 0; ch < nchar; ch++) {
		if ((len = bitlen[ch]) == 0) continue;
		nextcode = start[len] + weight[len];
		if (len <= tablebits) {
			for (i = start[len]; i < nextcode; i++) table[i] = ch;
		} else {
			k = start[len];
			p = &table[k >> jutbits];
			i = len - tablebits;
			while (i != 0) {
				if (*p == 0) {
					right[avail] = left[avail] = 0;
					*p = avail++;
				}
				if (k & mask) p = &right[*p];
				else          p = &left[*p];
				k <<= 1;  i--;
			}
			*p = ch;
		}
		start[len] = nextcode;
	}
}
