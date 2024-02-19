/*
 * set of macros to do inplace byteorder changes
 * The dec_* routines decode a short (_s) or long (_l) from little endian(_le)
 * or bigendian(_be) to native format.
 * The enc_* are similar for native to net format
 */


/* littleendian version for ibm pc */

#define dec_s_le(s)	/* nothing */
#define dec_s_be(s)	(*(s))=((((*(s))>>8)&0xFF)|(((*(s))&0xFF)<<8))

#define dec_l_le(l)	/* nothing */
#define dec_l_be(l)	(*(l))=((((*(l))>>24)&0xFF)|(((*(l))>>8)&0xFF00)|(((*(l))<<8)&0xFF0000)|(((*(l))<<24)&0xFF000000))

#define enc_s_le(s)	/* nothing */
#define enc_s_be(s)	dec_s_be(s)

#define enc_l_le(l)	/* nothing */
#define enc_l_be(l)	dec_l_be(l)
