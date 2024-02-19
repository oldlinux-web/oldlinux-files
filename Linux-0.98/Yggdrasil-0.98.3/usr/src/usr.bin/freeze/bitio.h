/* Some definitions for faster bit-level I/O */

extern ul_t getbuf;    /* Bit I/O buffers */
extern us_t putbuf;
extern uc_t bitlen;            /* Number of bits actually in `???buf' */

extern uc_t crpt_flag;   /* 1 == EOF was read when melting */
extern uc_t __;          /* temporary variable for GetBit/Byte */

extern void EncodeEnd(), Putcode(), crpt_message();

#define bits(x) ((int)sizeof(x)*8)
#define BYSH  (bits(getbuf)-8)
#define BISH  (bits(getbuf)-1)

#define GetByte()       (bitlen >= 8 ? (__ = getbuf >> BYSH, bitlen -= 8,\
			getbuf <<= 8, __) : (getbuf |= (ul_t) \
			(getchar() & 0xFF) << (BYSH - bitlen), __ = getbuf\
			>> BYSH, getbuf <<= 8, __))

#define GetBit()        (bitlen ? (__ = getbuf >> BISH, bitlen--, \
			getbuf <<= 1, __) : (getbuf = __ = getchar(), \
			getbuf <<= BYSH + 1, bitlen = 7, __ >> 7))

#define InitIO()        { crpt_flag = bitlen = 0; putbuf = 0; getbuf = 0; }
