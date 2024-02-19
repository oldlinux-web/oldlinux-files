/*$Source: /usr/home/dhesi/zoo/RCS/huf.c,v $*/
/*$Id: huf.c,v 1.9 91/07/09 01:39:55 dhesi Exp $*/
/***********************************************************
	huf.c -- static Huffman

Adapted from "ar" archiver written by Haruhiko Okumura.
***********************************************************/
#include "options.h"
#include "zoo.h"
#include "ar.h"
#include "lzh.h"
#include "errors.i"

extern void prterror();

#ifdef ANSI_HDRS
# include <stdlib.h>
#endif

#define NP (DICBIT + 1)
#define NT (CODE_BIT + 3)
#define PBIT 4  /* smallest integer such that (1U << PBIT) > NP */
#define TBIT 5  /* smallest integer such that (1U << TBIT) > NT */
#if NT > NP
# define NPT NT
#else
# define NPT NP
#endif

int decoded;		/* for use in decode.c */

ushort left[2 * NC - 1], right[2 * NC - 1];
static uchar *buf, c_len[NC], pt_len[NPT];
static uint   bufsiz = 0, blocksize;
static ushort c_freq[2 * NC - 1], c_table[4096], c_code[NC],
			  p_freq[2 * NP - 1], pt_table[256], pt_code[NPT],
			  t_freq[2 * NT - 1];

/***** encoding *****/

static void count_t_freq()
{
	int i, k, n, count;

	for (i = 0; i < NT; i++) t_freq[i] = 0;
	n = NC;
	while (n > 0 && c_len[n - 1] == 0) n--;
	i = 0;
	while (i < n) {
		k = c_len[i++];
		if (k == 0) {
			count = 1;
			while (i < n && c_len[i] == 0) {  i++;  count++;  }
			if (count <= 2) t_freq[0] += count;
			else if (count <= 18) t_freq[1]++;
			else if (count == 19) {  t_freq[0]++;  t_freq[1]++;  }
			else t_freq[2]++;
		} else t_freq[k + 2]++;
	}
}

static void write_pt_len(n, nbit, i_special)
int n;
int nbit;
int i_special;
{
	int i, k;

	while (n > 0 && pt_len[n - 1] == 0) n--;
	putbits(nbit, (uint) n);
	i = 0;
	while (i < n) {
		k = pt_len[i++];
		if (k <= 6) putbits(3, (uint) k);
		else putbits(k - 3, (uint) (((unsigned) 1 << (k - 3)) - 2));
		if (i == i_special) {
			while (i < 6 && pt_len[i] == 0) i++;
			putbits(2, (uint) ((i - 3) & 3));
		}
	}
}

static void write_c_len()
{
	int i, k, n, count;

	n = NC;
	while (n > 0 && c_len[n - 1] == 0) n--;
	putbits(CBIT, (uint) n);
	i = 0;
	while (i < n) {
		k = c_len[i++];
		if (k == 0) {
			count = 1;
			while (i < n && c_len[i] == 0) {  i++;  count++;  }
			if (count <= 2) {
				for (k = 0; k < count; k++)
					putbits((int) pt_len[0], (uint) pt_code[0]);
			} else if (count <= 18) {
				putbits((int) pt_len[1], (uint) pt_code[1]);
				putbits(4, (uint) (count - 3));
			} else if (count == 19) {
				putbits((int) pt_len[0], (uint) pt_code[0]);
				putbits((int) pt_len[1], (uint) pt_code[1]);
				putbits(4, 15);
			} else {
				putbits((int) pt_len[2], (uint) pt_code[2]);
				putbits(CBIT, (uint) (count - 20));
			}
		} else putbits((int) pt_len[k + 2], (uint) pt_code[k + 2]);
	}
}

static void encode_c(c)
int c;
{
	putbits((int) c_len[c], (uint) c_code[c]);
}

static void encode_p(p)
uint p;
{
	uint c, q;

	c = 0;  q = p;  while (q) {  q >>= 1;  c++;  }
	putbits((int) pt_len[c], (uint) pt_code[c]);
	if (c > 1) putbits((int) (c - 1), (uint) (p & ((unsigned) 0xFFFF >> (17 - c))));
}

static void send_block()
{
	uint i, k, flags, root, pos, size;

	root = make_tree(NC, c_freq, c_len, c_code);
	size = c_freq[root];
#if 0
	/*debug*/ (void) fprintf(stderr, "\nsize = %u\n", size);
#endif
	putbits(16, size);
	if (root >= NC) {
		count_t_freq();
		root = make_tree(NT, t_freq, pt_len, pt_code);
		if (root >= NT) {
			write_pt_len(NT, TBIT, 3);
		} else {
			putbits(TBIT, 0);  putbits(TBIT, root);
		}
		write_c_len();
	} else {
        putbits(TBIT, 0);  putbits(TBIT, 0);
		putbits(CBIT, 0);  putbits(CBIT, root);
	}
	root = make_tree(NP, p_freq, pt_len, pt_code);
	if (root >= NP) {
		write_pt_len(NP, PBIT, -1);
	} else {
		putbits(PBIT, 0);  putbits(PBIT, root);
	}
	pos = 0;
	for (i = 0; i < size; i++) {
		if (i % CHAR_BIT == 0) flags = buf[pos++];  else flags <<= 1;
		if (flags & ((unsigned) 1 << (CHAR_BIT - 1))) {
			encode_c((int) (buf[pos++] + ((unsigned) 1 << CHAR_BIT)));
			k = buf[pos++] << CHAR_BIT;  k += buf[pos++];
			encode_p(k);
		} else encode_c((int) buf[pos++]);
		if (unpackable) return;
	}
	for (i = 0; i < NC; i++) c_freq[i] = 0;
	for (i = 0; i < NP; i++) p_freq[i] = 0;
}

static uint output_pos, output_mask;

void output(c, p)
uint c;
uint p;
{
	static uint cpos;

	if ((output_mask >>= 1) == 0) {
		output_mask = (unsigned) 1 << (CHAR_BIT - 1);
		if (output_pos >= bufsiz - 3 * CHAR_BIT) {
			send_block();
			if (unpackable) return;
			output_pos = 0;
		}
		cpos = output_pos++;  buf[cpos] = 0;
	}
	buf[output_pos++] = (uchar) c;  c_freq[c]++;
	if (c >= ((unsigned) 1 << CHAR_BIT)) {
		buf[cpos] |= output_mask;
		buf[output_pos++] = (uchar)(p >> CHAR_BIT);
		buf[output_pos++] = (uchar) p;
		c = 0;  while (p) {  p >>= 1;  c++;  }
		p_freq[c]++;
	}
}

void huf_encode_start()
{
	int i;

	if (bufsiz == 0) {
		bufsiz = 16 * (unsigned) 1024;
		while ((buf = (uchar *) malloc(bufsiz)) == NULL) {
			bufsiz = (bufsiz / (unsigned) 10) * (unsigned) 9;
			if (bufsiz < 4 * (unsigned) 1024) 
				prterror('f', no_memory);
		}
	}
	buf[0] = 0;
	for (i = 0; i < NC; i++) c_freq[i] = 0;
	for (i = 0; i < NP; i++) p_freq[i] = 0;
	output_pos = output_mask = 0;
	init_putbits();
}

void huf_encode_end()
{
	if (! unpackable) {
		send_block();
		putbits(CHAR_BIT - 1, 0);  /* flush remaining bits */
		putbits(16, 0);				/* EOF marker */
	}
}

/***** decoding *****/

static void read_pt_len(nn, nbit, i_special)
int nn;
int nbit;
int i_special;
{
	int i, c, n;
	uint mask;

	n = getbits(nbit);
	if (n == 0) {
		c = getbits(nbit);
		for (i = 0; i < nn; i++) pt_len[i] = 0;
		for (i = 0; i < 256; i++) pt_table[i] = c;
	} else {
		i = 0;
		while (i < n) {
			c = bitbuf >> (BITBUFSIZ - 3);
			if (c == 7) {
				mask = (unsigned) 1 << (BITBUFSIZ - 1 - 3);
				while (mask & bitbuf) {  mask >>= 1;  c++;  }
			}
			fillbuf((c < 7) ? 3 : c - 3);
			pt_len[i++] = c;
			if (i == i_special) {
				c = getbits(2);
				while (--c >= 0) pt_len[i++] = 0;
			}
		}
		while (i < nn) pt_len[i++] = 0;
		make_table(nn, pt_len, 8, pt_table);
	}
}

static void read_c_len()
{
	int i, c, n;
	uint mask;

	n = getbits(CBIT);
	if (n == 0) {
		c = getbits(CBIT);
		for (i = 0; i < NC; i++) c_len[i] = 0;
		for (i = 0; i < 4096; i++) c_table[i] = c;
	} else {
		i = 0;
		while (i < n) {
			c = pt_table[bitbuf >> (BITBUFSIZ - 8)];
			if (c >= NT) {
				mask = (unsigned) 1 << (BITBUFSIZ - 1 - 8);
				do {
					if (bitbuf & mask) c = right[c];
					else               c = left [c];
					mask >>= 1;
				} while (c >= NT);
			}
			fillbuf((int) pt_len[c]);
			if (c <= 2) {
				if      (c == 0) c = 1;
				else if (c == 1) c = getbits(4) + 3;
				else             c = getbits(CBIT) + 20;
				while (--c >= 0) c_len[i++] = 0;
			} else c_len[i++] = c - 2;
		}
		while (i < NC) c_len[i++] = 0;
		make_table(NC, c_len, 12, c_table);
	}
}

uint decode_c()
{
	uint j, mask;

	if (blocksize == 0) {
		blocksize = getbits(16);
		if (blocksize == 0) {
#if 0
			(void) fprintf(stderr, "block size = 0, decoded\n");  /* debug */
#endif
			decoded = 1;
			return 0;
		}
		read_pt_len(NT, TBIT, 3);
		read_c_len();
		read_pt_len(NP, PBIT, -1);
	}
	blocksize--;
	j = c_table[bitbuf >> (BITBUFSIZ - 12)];
	if (j >= NC) {
		mask = (unsigned) 1 << (BITBUFSIZ - 1 - 12);
		do {
			if (bitbuf & mask) j = right[j];
			else               j = left [j];
			mask >>= 1;
		} while (j >= NC);
	}
	fillbuf((int) c_len[j]);
	return j;
}

uint decode_p()
{
	uint j, mask;

	j = pt_table[bitbuf >> (BITBUFSIZ - 8)];
	if (j >= NP) {
		mask = (unsigned) 1 << (BITBUFSIZ - 1 - 8);
		do {
			if (bitbuf & mask) j = right[j];
			else               j = left [j];
			mask >>= 1;
		} while (j >= NP);
	}
	fillbuf((int) pt_len[j]);
	if (j != 0) j = ((unsigned) 1 << (j - 1)) + getbits((int) (j - 1));
	return j;
}

void huf_decode_start()
{
	init_getbits();  blocksize = 0;
}
