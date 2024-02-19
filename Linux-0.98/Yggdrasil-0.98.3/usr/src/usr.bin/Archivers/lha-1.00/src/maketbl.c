/***********************************************************
	maketbl.c -- makes decoding table
***********************************************************/
#include "slidehuf.h"

#if 0
static short c, n, tblsiz, len, depth, maxdepth, avail;
static unsigned short codeword, bit, *tbl;
static unsigned char *blen;

static short mktbl(void)
{
  short i;

  if (len == depth) {
    while (++c < n)
      if (blen[c] == len) {
	i = codeword;  codeword += bit;
	if (codeword > tblsiz) error(BROKENARC, "Bad table (1)");
	while (i < codeword) tbl[i++] = c;
	return c;
      }
    c = -1;  len++;  bit >>= 1;
  }
  depth++;
  if (depth < maxdepth) {
    (void) mktbl();  (void) mktbl();
  } else if (depth > USHRT_BIT) {
    error(BROKENARC, "Bad table (2)");
  } else {
    if ((i = avail++) >= 2 * n - 1) error(BROKENARC, "Bad table (3)");
    left[i] = mktbl();  right[i] = mktbl();
    if (codeword >= tblsiz) error(BROKENARC, "Bad table (4)");
    if (depth == maxdepth) tbl[codeword++] = i;
  }
  depth--;
  return i;
}

void make_table(short nchar, unsigned char bitlen[],
		short tablebits, unsigned short table[])
{
  n = avail = nchar;  blen = bitlen;  tbl = table;
  tblsiz = 1U << tablebits;  bit = tblsiz / 2;
  maxdepth = tablebits + 1;
  depth = len = 1;  c = -1;  codeword = 0;
  (void) mktbl();  /* left subtree */
  (void) mktbl();  /* right subtree */
  if (codeword != tblsiz) error(BROKENARC, "Bad table (5)");
}
#else
void make_table(nchar, bitlen, tablebits, table)
short nchar;
unsigned char bitlen[];
short tablebits;
unsigned short table[];
{
	unsigned short count[17];  /* count of bitlen */
	unsigned short weight[17]; /* 0x10000ul >> bitlen */
	unsigned short start[17];  /* first code of bitlen */
	unsigned short total;
	unsigned int i;
	int j, k, l, m, n, avail;
	unsigned short *p;

	avail = nchar;

/* initialize */
	for (i = 1; i <= 16; i++) {
		count[i] = 0;
		weight[i] = 1 << (16 - i);
	}

/* count */
	for (i = 0; i < nchar; i++) count[bitlen[i]]++;

/* calculate first code */
	total = 0;
	for (i = 1; i <= 16; i++) {
		start[i] = total;
		total += weight[i] * count[i];
	}
	if ((total & 0xffff) != 0)
	  error("Bad table (5)\n");

/* shift data for make table. */
	m = 16 - tablebits;
	for (i = 1; i <= tablebits; i++) {
		start[i] >>= m;
		weight[i] >>= m;
	}

/* initialize */
	j = start[tablebits + 1] >> m;
	k = 1 << tablebits;
	if (j != 0)
		for (i = j; i < k; i++) table[i] = 0;

/* create table and tree */
	for (j = 0; j < nchar; j++) {
		k = bitlen[j];
		if (k == 0) continue;
		l = start[k] + weight[k];
		if (k <= tablebits) {
		/* code in table */
			for (i = start[k]; i < l; i++) table[i] = j;
		} else {
		/* code not in table */
			p = &table[(i = start[k]) >> m];
			i <<= tablebits;
			n = k - tablebits;
		/* make tree (n length) */
			while (--n >= 0) {
				if (*p == 0) {
					right[avail] = left[avail] = 0;
					*p = avail++;
				}
				if (i & 0x8000) p = &right[*p];
				else            p = &left[*p];
				i <<= 1;
			}
			*p = j;
		}
		start[k] = l;
	}
}
#endif



