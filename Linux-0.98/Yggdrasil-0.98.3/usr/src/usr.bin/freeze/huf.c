#include "freeze.h"
#include "huf.h"
#include "bitio.h"

/*----------------------------------------------------------------------*/
/*									*/
/*		HUFFMAN ENCODING					*/
/*									*/
/*----------------------------------------------------------------------*/

/* TABLES OF ENCODE/DECODE for upper 6 bits position information */

/* The contents of `Table' are used for freezing only, so we use
 * it freely when melting.
 */

uc_t Table2[9] = { 0, 0, 1, 1, 1, 4, 10, 27, 18 };

uc_t p_len[64];        /* These arrays are built accordingly to values */
uc_t d_len[256];       /* of `Table' above which are default, from the */
		      /* command line or from the header of frozen file */

uc_t code[256];

us_t freq[T2 + 1];           /* frequency table */
short   son[T2];                /* points to son node (son[i],son[i+1]) */
short   prnt[T2 + N_CHAR2];     /* points to parent node */

static  short t, r, chars;

/* notes :
   prnt[Tx .. Tx + N_CHARx - 1] used by
   indicates leaf position that corresponding to code.
*/

/* Initializes Huffman tree, bit I/O variables, etc.
   Static array is initialized with `table', dynamic Huffman tree
   has `n_char' leaves.
*/

void StartHuff (n_char)
	int n_char;
{
	register short i, j;
	t = n_char * 2 - 1;
	r = t - 1;
	chars = n_char;

/* A priori frequences are 1 */

	for (i = 0; i < n_char; i++) {
		freq[i] = 1;
		son[i] = i + t;
		prnt[i + t] = i;
	}
	i = 0; j = n_char;

/* Building the balanced tree */

	while (j <= r) {
		freq[j] = freq[i] + freq[i + 1];
		son[j] = i;
		prnt[i] = prnt[i + 1] = j;
		i += 2; j++;
	}
	freq[t] = 0xffff;
	prnt[r] = 0;
	in_count = 1;
	bytes_out = 5;
#ifdef DEBUG
	symbols_out = refers_out = 0;
#endif
}

/* Reconstructs tree with `chars' leaves */

void reconst ()
{
	register us_t i, j, k;
	register us_t f;

#ifdef DEBUG
	if (quiet < 0)
	  fprintf(stderr,
	    "Reconstructing Huffman tree: symbols: %ld, references: %ld\n",
	    symbols_out, refers_out);
#endif

/* correct leaf node into of first half,
   and set these freqency to (freq+1)/2
*/
	j = 0;
	for (i = 0; i < t; i++) {
		if (son[i] >= t) {
			freq[j] = (freq[i] + 1) / 2;
			son[j] = son[i];
			j++;
		}
	}
/* Build tree.  Link sons first */

	for (i = 0, j = chars; j < t; i += 2, j++) {
		k = i + 1;
		f = freq[j] = freq[i] + freq[k];
		for (k = j - 1; f < freq[k]; k--);
		k++;
		{       register us_t *p, *e;
			for (p = &freq[j], e = &freq[k]; p > e; p--)
				p[0] = p[-1];
			freq[k] = f;
		}
		{       register short *p, *e;
			for (p = &son[j], e = &son[k]; p > e; p--)
				p[0] = p[-1];
			son[k] = i;
		}
	}

/* Link parents */
	for (i = 0; i < t; i++) {
		if ((k = son[i]) >= t) {
			prnt[k] = i;
		} else {
			prnt[k] = prnt[k + 1] = i;
		}
	}
}


/* Updates given code's frequency, and updates tree */

void update (c)
	us_t c;
{
	register us_t *p;
	register us_t i, j, k, l;

	if (freq[r] == MAX_FREQ) {
		reconst();
	}
	c = prnt[c + t];
	do {
		k = ++freq[c];

		/* swap nodes when become wrong frequency order. */
		if (k > freq[l = c + 1]) {
			for (p = freq+l+1; k > *p++; ) ;
			l = p - freq - 2;
			freq[c] = p[-2];
			p[-2] = k;

			i = son[c];
			prnt[i] = l;
			if (i < t) prnt[i + 1] = l;

			j = son[l];
			son[l] = i;

			prnt[j] = c;
			if (j < t) prnt[j + 1] = c;
			son[c] = j;

			c = l;
		}
	} while ((c = prnt[c]) != 0);	/* loop until reach to root */
}

/* Encodes the literal or the length information */

void EncodeChar (c)
	us_t c;
{
	ul_t i;
	register us_t j, k;

	i = 0;
	j = 0;
	k = prnt[c + t];

/* trace links from leaf node to root */

	do {
		i >>= 1;

/* if node index is odd, trace larger of sons */
		if (k & 1) i += 0x80000000;

		j++;
	} while ((k = prnt[k]) != r) ;

/* `j' never reaches the value of 32 ! */

	if (j > 16) {
		Putcode(16, (us_t)(i >> 16));
		Putcode(j - 16, (us_t)i);
	} else {
		Putcode(j, (us_t)(i >> 16));
	}
	update(c);
}

/* Encodes the position information */

void EncodePosition (c)
	register us_t c;
{
	register us_t i;

	/* output upper 6 bit from table */
	i = c >> 7;
	Putcode((us_t)(p_len[i]), (us_t)(code[i]) << 8);

	/* output lower 7 bit */
	Putcode(7, (us_t)(c & 0x7f) << 9);
}


/* Decodes the literal or length info and returns its value.
	Returns ENDOF, if the file is corrupt.
*/

short DecodeChar ()
{
	register us_t c;
	c = son[r];

	/* trace from root to leaf,
	   got bit is 0 to small(son[]), 1 to large (son[]+1) son node */

	while (c < t) {
		c += GetBit();
		c = son[c];
	}
	c -= t;
	update(c);
	if (crpt_flag) {
		crpt_message();
		return ENDOF;
	}
	crpt_flag = feof(stdin);
	return c;
}

/* Decodes the position info and returns it */

short DecodePosition ()
{
	register us_t i, j, c;

	/* decode upper 6 bits from the table */

	i = GetByte();
	crpt_flag = feof(stdin);

	c = (us_t)code[i] << 7;
	j = d_len[i] - 1;

	/* get lower 7 bits literally */

	return c | (((i << j) | GetNBits (j)) & 0x7f);
}


/* Initializes static Huffman arrays */

void init(table) uc_t * table; {
	short i, j, k, num;
	num = 0;

/* There are `table[i]' `i'-bits Huffman codes */

	for(i = 1, j = 0; i <= 8; i++) {
		num += table[i] << (8 - i);
		for(k = table[i]; k; j++, k--)
			p_len[j] = i;
	}
	if (num != 256) {
		fprintf(stderr, "Invalid position table\n");
		exit(1);
	}
	num = j;
	if (do_melt == 0)

/* Freezing: building the table for encoding */

		for(i = j = 0;;) {
			code[j] = i << (8 - p_len[j]);
			i++;
			j++;
			if (j == num) break;
			i <<= p_len[j] - p_len[j-1];
		}
	else {

/* Melting: building the table for decoding */

		for(k = j = 0; j < num; j ++)
			for(i = 1 << (8 - p_len[j]); i--;)
				code[k++] = j;

		for(k = j = 0; j < num; j ++)
			for(i = 1 << (8 - p_len[j]); i--;)
				d_len[k++] =  p_len[j];
	}
}

/* Writes a 3-byte header into the frozen form of file; Table[7] and
	Table[8] aren't necessary, see `read_header'.
*/

void write_header() {
	us_t i;

	i = Table2[5] & 0x1F; i <<= 4;
	i |= Table2[4] & 0xF; i <<= 3;
	i |= Table2[3] & 7;   i <<= 2;
	i |= Table2[2] & 3;   i <<= 1;
	i |= Table2[1] & 1;

	putchar((int)(i & 0xFF));
	putchar((int)((i >> 8)));
	putchar((int)(Table2[6] & 0x3F));
	if (ferror(stdout))
		writeerr();
}

/* Reconstructs `Table' from the header of the frozen file and checks
	its correctness. Returns 0 if OK, EOF otherwise.
*/

int read_header() {
	short i, j;
	i = getchar() & 0xFF;
	i |= (getchar() & 0xFF) << 8;
	Table2[1] = i & 1; i >>= 1;
	Table2[2] = i & 3; i >>= 2;
	Table2[3] = i & 7; i >>= 3;
	Table2[4] = i & 0xF; i >>= 4;
	Table2[5] = i & 0x1F; i >>= 5;

	if (i & 1 || (i = getchar()) & 0xC0) {
		fprintf(stderr, "Unknown header format.\n");
		crpt_message();
		return EOF;
	}

	Table2[6] = i & 0x3F;

	i = Table2[1] + Table2[2] + Table2[3] + Table2[4] +
	Table2[5] + Table2[6];

	i = 62 - i;     /* free variable length codes for 7 & 8 bits */

	j = 128 * Table2[1] + 64 * Table2[2] + 32 * Table2[3] +
	16 * Table2[4] + 8 * Table2[5] + 4 * Table2[6];

	j = 256 - j;    /* free byte images for these codes */

/*      Equation:
	    Table[7] + Table[8] = i
	2 * Table[7] + Table[8] = j
*/
	j -= i;
	if (j < 0 || i < j) {
		crpt_message();
		return EOF;
	}
	Table2[7] = j;
	Table2[8] = i - j;

#ifdef DEBUG
	fprintf(stderr, "Codes: %d %d %d %d %d %d %d %d\n",
		Table2[1], Table2[2], Table2[3], Table2[4],
		Table2[5], Table2[6], Table2[7], Table2[8]);
#endif
	return 0;
}

#ifdef COMPAT

uc_t Table1[9] = { 0, 0, 0, 1, 3, 8, 12, 24, 16 };

/* Old version of a routine above for handling files made by
	the 1st version of Freeze.
*/

short DecodePOld ()
{
	register us_t i, j, c;

	i = GetByte();
	crpt_flag = feof(stdin);

	c = (us_t)code[i] << 6;
	j = d_len[i] - 2;

	return c | (((i << j) | GetNBits (j)) & 0x3f);
}
#endif
