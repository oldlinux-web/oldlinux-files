#include "freeze.h"
#include "bitio.h"

ul_t     getbuf = 0;     /* assume sizeof (ul_t) >= 4 */
us_t     putbuf;
uc_t     bitlen = 0, __, crpt_flag = 0;

/* get N bits (N <= 16), returning in Bit(N-1)...Bit 0 */

short GetNBits (n)
	register us_t n;
{
	register ul_t dx = getbuf;
	register uc_t c;

	static us_t mask[17] = {
		0x0000,
		0x0001, 0x0003, 0x0007, 0x000f,
		0x001f, 0x003f, 0x007f, 0x00ff,
		0x01ff, 0x03ff, 0x07ff, 0x0fff,
		0x1fff, 0x3fff, 0x7fff, 0xffff };

	while (bitlen < n)
		{
			c = getchar ();
			dx |= (ul_t) c << (BYSH - bitlen);
			bitlen += 8;
		}
	crpt_flag = feof(stdin);
	getbuf = dx << n;
	bitlen -= n;
	return (dx >> (bits(getbuf) - n)) & mask[n];
}

/* output `l' high bits of `c' */

void Putcode (l, c)
	register us_t l;
	us_t c;
{
	register us_t len = bitlen;
	register us_t b = (us_t)putbuf;
	b |= c >> len;
	if ((len += l) >= 8) {
		putchar ((int)(b >> 8));
		if ((len -= 8) >= 8) {
			putchar ((int)b);
			bytes_out += 2;
			len -= 8;
			b = c << (l - len);
		} else {
			b <<= 8;
			bytes_out++;
		}
	}
	if (ferror(stdout))
		writeerr();
	putbuf = b;
	bitlen = len;
}


/* Flushes the bit I/O buffers and check the state of stdout */

void EncodeEnd ()
{
	if (bitlen) {
		putchar((int)(putbuf >> 8));
		bytes_out++;
		if (ferror(stdout))
			writeerr();
	}
}

/* File too short or invalid header, print a message */

void crpt_message ( )
{
	fprintf ( stderr, "melt: corrupt input\n" );
}

