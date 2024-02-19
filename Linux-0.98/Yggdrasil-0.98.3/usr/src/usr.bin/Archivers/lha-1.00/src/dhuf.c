/**************************************************************
	title   dhuf.c
***************************************************************
    Dynamic Huffman routine
                                                H.Yoshizaki
**************************************************************/
#include <stdio.h>
#include "slidehuf.h"

#define N_CHAR      (256 + 60 - THRESHOLD + 1)
#define TREESIZE_C  (N_CHAR * 2)
#define TREESIZE_P  (128 * 2)
#define TREESIZE    (TREESIZE_C + TREESIZE_P)
#define ROOT_C      0
#define ROOT_P      TREESIZE_C

unsigned int n_max;

static short child [TREESIZE],
			 parent[TREESIZE],
			 block [TREESIZE],
			 edge  [TREESIZE],
			 stock [TREESIZE],
			 node  [TREESIZE / 2];

static unsigned short freq[TREESIZE];

static unsigned short total_p;
static int avail, n1;
static int most_p, nn;
static unsigned long nextcount;

void start_c_dyn(void)
{
	int i, j, f;

	n1 = (n_max >= 256 + maxmatch - THRESHOLD + 1) ? 512 : n_max - 1;
	for (i = 0; i < TREESIZE_C; i++) {
		stock[i] = i;
		block[i] = 0;
	}
	for (i = 0, j = n_max * 2 - 2; i < n_max; i++, j--) {
		freq[j] = 1;
		child[j] = ~i;
		node[i] = j;
		block[j] = 1;
	}
	avail = 2;
	edge[1] = n_max - 1;
	i = n_max * 2 - 2;
	while (j >= 0) {
		f = freq[j] = freq[i] + freq[i - 1];
		child[j] = i;
		parent[i] = parent[i - 1] = j;
		if (f == freq[j + 1]) {
			edge[block[j] = block[j + 1]] = j;
		} else {
			edge[block[j] = stock[avail++]] = j;
		}
		i -= 2;
		j--;
	}
}

static void start_p_dyn(void)
{
	freq[ROOT_P] = 1;
	child[ROOT_P] = ~(N_CHAR);
	node[N_CHAR] = ROOT_P;
	edge[block[ROOT_P] = stock[avail++]] = ROOT_P;
	most_p = ROOT_P;
	total_p = 0;
	nn = 1 << dicbit;
	nextcount = 64;
}

void decode_start_dyn(void)
{
	n_max = 286;
	maxmatch = MAXMATCH;
	init_getbits();
	start_c_dyn();
	start_p_dyn();
}

static void reconst( start , end )
int start;
int end;
{
	int i, j, k, l, b;
	unsigned int f, g;

	for (i = j = start; i < end; i++) {
		if ((k = child[i]) < 0) {
			freq[j] = (freq[i] + 1) / 2;
			child[j] = k;
			j++;
		}
		if (edge[b = block[i]] == i) {
			stock[--avail] = b;
		}
	}
	j--;
	i = end - 1;
	l = end - 2;
	while (i >= start) {
		while (i >= l) {
			freq[i] = freq[j]; child[i] = child[j];
			i--, j--;
		}
		f = freq[l] + freq[l + 1];
		for (k = start; f < freq[k]; k++);
		while(j >= k) {
			freq[i] = freq[j]; child[i] = child[j];
			i--, j--;
		}
		freq[i] = f; child[i] = l + 1;
		i--;
		l -= 2;
	}
	f = 0;
	for (i = start; i < end; i++) {
		if ((j = child[i]) < 0) node[~j] = i;
		else parent[j] = parent[j - 1] = i;
		if ((g = freq[i]) == f) {
			block[i] = b;
		} else {
			edge[b = block[i] = stock[avail++]] = i;
			f = g;
		}
	}
}

static int swap_inc(p)
int p;
{
	int b, q, r, s;

	b = block[p];
	if ((q = edge[b]) != p) {	/* swap for leader */
		r = child[p]; s = child[q];
		child[p] = s; child[q] = r;
		if (r >= 0) parent[r] = parent[r - 1] = q;
		else		node[~r] = q;
		if (s >= 0)	parent[s] = parent[s - 1] = p;
		else		node[~s] = p;
		p = q;
		goto Adjust;
	} else if (b == block[p + 1]) {
Adjust:
		edge[b]++;
		if (++freq[p] == freq[p - 1]) {
			block[p] = block[p - 1];
		} else {
			edge[block[p] = stock[avail++]] = p;	/* create block */
		}
	} else if (++freq[p] == freq[p - 1]) {
		stock[--avail] = b;		/* delete block */
		block[p] = block[p - 1];
	}
	return parent[p];
}

static void update_c(p)
int p;
{
	int q;

	if (freq[ROOT_C] == 0x8000) {
		reconst(0, n_max * 2 - 1);
	}
	freq[ROOT_C]++;
	q = node[p];
	do {
		q = swap_inc(q);
	} while (q != ROOT_C);
}

static void update_p(p)
int p;
{
	int q;

	if (total_p == 0x8000) {
		reconst(ROOT_P, most_p + 1);
		total_p = freq[ROOT_P];
		freq[ROOT_P] = 0xffff;
	}
	q = node[p + N_CHAR];
	while (q != ROOT_P) {
		q = swap_inc(q);
	}
	total_p++;
}

static void make_new_node(p)
int p;
{
	int q, r;

	r = most_p + 1; q = r + 1;
	node[~(child[r] = child[most_p])] = r;
	child[q] = ~(p + N_CHAR);
	child[most_p] = q;
	freq[r] = freq[most_p];
	freq[q] = 0;
	block[r] = block[most_p];
	if (most_p == ROOT_P) {
		freq[ROOT_P] = 0xffff;
		edge[block[ROOT_P]]++;
	}
	parent[r] = parent[q] = most_p;
	edge[block[q] = stock[avail++]] = node[p + N_CHAR] = most_p = q;
	update_p(p);
}

static void encode_c_dyn(c)
int c;
{
	unsigned int bits;
	int p, d, cnt;

	d = c - n1;
	if (d >= 0) {
		c = n1;
	}
	cnt = bits = 0;  
	p = node[c];
	do {
		bits >>= 1;
		if (p & 1) {
			bits |= 0x8000;
		}
		if (++cnt == 16) {
			putcode(16, bits);
			cnt = bits = 0;
		}
	} while ((p = parent[p]) != ROOT_C);
	putcode(cnt, bits);
	if (d >= 0) putbits(8, d);
	update_c(c);
}

unsigned short decode_c_dyn(void)
{
	int c;
	short buf, cnt;

	c = child[ROOT_C];
	buf = bitbuf;
	cnt = 0;
	do {
		c = child[c - (buf < 0)];
		buf <<= 1;
		if (++cnt == 16) {
			fillbuf(16);
			buf = bitbuf; cnt = 0;
		}
	} while (c > 0);
	fillbuf(cnt);
	c = ~c;
	update_c(c);
	if (c == n1) c += getbits(8);
	return c;
}

unsigned short decode_p_dyn(void)
{
	int c;
	short buf, cnt;

	while (count > nextcount) {
		make_new_node(nextcount / 64);
		if ((nextcount += 64) >= nn)
			nextcount = 0xffffffff;
	}
	c = child[ROOT_P];
	buf = bitbuf; cnt = 0;
	while (c > 0) {
		c = child[c - (buf < 0)];
		buf <<= 1;
		if (++cnt == 16) {
			fillbuf(16);
			buf = bitbuf; cnt = 0;
		}
	}
	fillbuf(cnt);
	c = (~c) - N_CHAR;
	update_p(c);

	return (c << 6) + getbits(6);
}

void output_dyn( code , pos )
int code;
unsigned int pos;
{
	encode_c_dyn(code);
	if (code >= 0x100) {
		encode_p_st0(pos);
	}
}

void encode_end_dyn(void)
{
	putcode(7, 0);
}
