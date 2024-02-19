/*
 *   bin86/ld/typeconv.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* typeconv.c - convert between char arrays and unsigneds */

/*
	c2u2(): 2 byte array to 2 byte unsigned
	c4u4(): 4 byte array to 4 byte unsigned
	cnu2(): n byte array to 2 byte unsigned
	cnu4(): n byte array to 4 byte unsigned
	u2c2(): 2 byte unsigned to 2 byte array
	u2cn(): 2 byte unsigned to n byte array
	u4c4(): 4 byte unsigned to 4 byte array
	u4cn(): 4 byte unsigned to n byte array
	typeconv_init: (re)initialise for given byte order.
		Default is no swapping, but the initialisation should be done
		anyway to provide some validity checks (returns FALSE if error).
	
	Not provided:
		c2u4(), c4u2(), u2c4(), u4c2().
	Each of these is best done by truncating or extending a return value
	or argument to the appropiate fixed-count function.
	c4u2() has too many cases to do in-line conveniently, and the others
	are hardly more efficient when done in-line.
	
	4 byte orderings for both char arrays and unsigneds are supported:
	0123 - little-endian
	3210 - big-endian
	2301 - little-endian with long words big-endian (pdp11)
	1032 - big-endian with long words little_endian (who knows?)

	The unsigned's byte order is that of the machine on which these
	routines are running.
	It is determined at run time initialisation since the compiler/
	preprocessor is too dumb to tell us at compile time.
*/

#include "const.h"
#include "type.h"
#include "globvar.h"

FORWARD u2_pt c2u2_00 P((char *buf));
FORWARD u4_pt c4u4_00 P((char *buf));
FORWARD u2_pt c2u2_ss P((char *buf));
FORWARD u4_pt c4u4_ss P((char *buf));
FORWARD u4_pt c4u4_s0 P((char *buf));
FORWARD u4_pt c4u4_0s P((char *buf));
FORWARD void u2c2_00 P((char *buf, u2_pt offset));
FORWARD void u4c4_00 P((char *buf, u4_t offset));
FORWARD void u2c2_ss P((char *buf, u2_pt offset));
FORWARD void u4c4_ss P((char *buf, u4_t offset));
FORWARD void u4c4_s0 P((char *buf, u4_t offset));
FORWARD void u4c4_0s P((char *buf, u4_t offset));

PRIVATE u2_pt (*pc2u2) P((char *buf)) = c2u2_00;
PRIVATE u4_pt (*pc4u4) P((char *buf)) = c4u4_00;
PRIVATE void (*pu2c2) P((char *buf, u2_pt offset)) = u2c2_00;
PRIVATE void (*pu4c4) P((char *buf, u4_t offset)) = u4c4_00;

/* === char arrays to unsigneds === */

/* no bytes swapped, longwinded to avoid alignment problems */

PRIVATE u2_pt c2u2_00(buf)
register char *buf;
{
    u2_t offset;

    ((char *) &offset)[0] = buf[0];
    ((char *) &offset)[1] = buf[1];
    return offset;
}

PRIVATE u4_pt c4u4_00(buf)
register char *buf;
{
    u4_t offset;

    ((char *) &offset)[0] = buf[0];
    ((char *) &offset)[1] = buf[1];
    ((char *) &offset)[2] = buf[2];
    ((char *) &offset)[3] = buf[3];
    return offset;
}

/* straight swapping for little-endian to big-endian and vice versa */

PRIVATE u2_pt c2u2_ss(buf)
register char *buf;
{
    u2_t offset;

    ((char *) &offset)[0] = buf[1];
    ((char *) &offset)[1] = buf[0];
    return offset;
}

PRIVATE u4_pt c4u4_ss(buf)
register char *buf;
{
    u4_t offset;

    ((char *) &offset)[0] = buf[3];
    ((char *) &offset)[1] = buf[2];
    ((char *) &offset)[2] = buf[1];
    ((char *) &offset)[3] = buf[0];
    return offset;
}

/* wierd swapping for different-endian u2's, same-endian u4's */

PRIVATE u4_pt c4u4_s0(buf)
register char *buf;
{
    u4_t offset;

    ((char *) &offset)[0] = buf[1];
    ((char *) &offset)[1] = buf[0];
    ((char *) &offset)[2] = buf[3];
    ((char *) &offset)[3] = buf[2];
    return offset;
}

/* very wierd swapping for same-endian u2's, different-endian u4's */

PRIVATE u4_pt c4u4_0s(buf)
register char *buf;
{
    u4_t offset;

    ((char *) &offset)[0] = buf[2];
    ((char *) &offset)[1] = buf[3];
    ((char *) &offset)[2] = buf[0];
    ((char *) &offset)[3] = buf[1];
    return offset;
}

/* === entry points === */

PUBLIC u2_pt c2u2(buf)
char *buf;
{
    return (*pc2u2) (buf);
}

PUBLIC u4_t c4u4(buf)
char *buf;
{
    return (*pc4u4) (buf);
}

PUBLIC u2_pt cnu2(buf, count)
char *buf;
unsigned count;
{
    switch (count)
    {
    case 1:
	return buf[0] & 0xFF;
    case 2:
	return (*pc2u2) (buf);
    case 4:
	return (u2_pt) (*pc4u4) (buf);
    default:
	return 0;
    }
}

PUBLIC u4_t cnu4(buf, count)
char *buf;
unsigned count;
{
    switch (count)
    {
    case 1:
	return buf[0] & 0xFF;
    case 2:
	return (*pc2u2) (buf);
    case 4:
	return (*pc4u4) (buf);
    default:
	return 0;
    }
}

/* === unsigneds to char arrays === */

/* no bytes swapped, longwinded to avoid alignment problems */

PRIVATE void u2c2_00(buf, offset)
register char *buf;
u2_pt offset;
{

    buf[0] = ((char *) &offset)[0];
    buf[1] = ((char *) &offset)[1];
}

PRIVATE void u4c4_00(buf, offset)
register char *buf;
u4_t offset;
{
    buf[0] = ((char *) &offset)[0];
    buf[1] = ((char *) &offset)[1];
    buf[2] = ((char *) &offset)[2];
    buf[3] = ((char *) &offset)[3];
}

/* straight swapping for little-endian to big-endian and vice versa */

PRIVATE void u2c2_ss(buf, offset)
register char *buf;
u2_pt offset;
{
    u2_t offset2;

    offset2 = offset;
    buf[0] = ((char *) &offset2)[1];
    buf[1] = ((char *) &offset2)[0];
}

PRIVATE void u4c4_ss(buf, offset)
register char *buf;
u4_t offset;
{
    buf[0] = ((char *) &offset)[3];
    buf[1] = ((char *) &offset)[2];
    buf[2] = ((char *) &offset)[1];
    buf[3] = ((char *) &offset)[0];
}

/* wierd swapping for different-endian u2's, same-endian u4's */

PRIVATE void u4c4_s0(buf, offset)
register char *buf;
u4_t offset;
{
    buf[0] = ((char *) &offset)[1];
    buf[1] = ((char *) &offset)[0];
    buf[2] = ((char *) &offset)[3];
    buf[3] = ((char *) &offset)[2];
}

/* very wierd swapping for same-endian u2's, different-endian u4's */

PRIVATE void u4c4_0s(buf, offset)
register char *buf;
u4_t offset;
{
    buf[0] = ((char *) &offset)[2];
    buf[1] = ((char *) &offset)[3];
    buf[2] = ((char *) &offset)[0];
    buf[3] = ((char *) &offset)[1];
}

/* === entry points === */

PUBLIC void u2c2(buf, offset)
register char *buf;
u2_pt offset;
{
    (*pu2c2) (buf, offset);
}

PUBLIC void u4c4(buf, offset)
register char *buf;
u4_t offset;
{
    (*pu4c4) (buf, offset);
}

PUBLIC void u2cn(buf, offset, count)
register char *buf;
u2_pt offset;
unsigned count;
{
    switch (count)
    {
    case 1:
	buf[0] = (char) offset;
	return;
    case 2:
	(*pu2c2) (buf, offset);
	return;
    case 4:
	(*pu4c4) (buf, (u4_t) offset);
	return;
    }
}

PUBLIC void u4cn(buf, offset, count)
register char *buf;
u4_t offset;
unsigned count;
{
    switch (count)
    {
    case 1:
	buf[0] = (char) offset;
	return;
    case 2:
	(*pu2c2) (buf, (u2_pt) (u2_t) offset);
	return;
    case 4:
	(*pu4c4) (buf, offset);
	return;
    }
}

/* initialise type conversion, return FALSE if it cannot be handled */

PUBLIC bool_pt typeconv_init(big_endian, long_big_endian)
bool_pt big_endian;
bool_pt long_big_endian;
{
    u2_pt conv2;
    u4_pt conv4;
    char *conv2ptr;
    char *conv4ptr;

    if (sizeof(u2_t) != 2 || sizeof(u4_t) != 4)
	/* dumb preprocessor's don't accept sizeof in #if expressions */
	return FALSE;

    if (big_endian)
    {
	conv2ptr = (conv4ptr = "\1\2\3\4") + 2;
	if (!long_big_endian)
	    conv4ptr = "\3\4\1\2";
    }
    else
    {
	conv2ptr = conv4ptr = "\4\3\2\1";
	if (long_big_endian)
	    conv4ptr = "\2\1\4\3";
    }
    conv2 = c2u2_00(conv2ptr);
    conv4 = c4u4_00(conv4ptr);
    if (conv2 == 0x0304)
    {
	pc2u2 = c2u2_00;
	pc4u4 = c4u4_00;
	pu2c2 = u2c2_00;
	pu4c4 = u4c4_00;
	if (conv4 == 0x03040102L)
	{
	    pc4u4 = c4u4_0s;
	    pu4c4 = u4c4_0s;
	}
	else if (conv4 != 0x01020304L)
	    return FALSE;
    }
    else if (conv2 == 0x0403)
    {
	pc2u2 = c2u2_ss;
	pc4u4 = c4u4_ss;
	pu2c2 = u2c2_ss;
	pu4c4 = u4c4_ss;
	if (conv4 == 0x02010403L)
	{
	    pc4u4 = c4u4_s0;
	    pu4c4 = u4c4_s0;
	}
	else if (conv4 != 0x04030201L)
	    return FALSE;
    }
    else
	return FALSE;
    return TRUE;
}

#ifdef DEBUG_TYPECONV

main()
{
    char *source;
    char target[4];
    u2_t u2;
    u2_t u2a;
    u4_t u4;
    u4_t u4a;

    printf("%u\n", typeconv_init(FALSE, FALSE));
    printf("%u\n", typeconv_init(FALSE, TRUE));
    printf("%u\n", typeconv_init(TRUE, FALSE));
    printf("%u\n", typeconv_init(TRUE, TRUE));

    typeconv_init(FALSE, FALSE);
    source = "\4\3\2\1";

    target[0] = 0;
    target[1] = 0;
    u2 = cnu2(source, 2);
    u2cn(target, u2, 2);
    if (strncmp(source, target, 2))
	printf("oops9\n");

    target[0] = 0;
    target[1] = 0;
    u4a = cnu4(source, 2);
    u4cn(target, u4a, 2);
    if (strncmp(source, target, 2))
	printf("oops10\n");

    target[0] = 0;
    target[1] = 0;
    target[2] = 0;
    target[3] = 0;
    u2a = cnu2(source, 4);
    u2cn(target, u2a, 4);
    if (strncmp(target, "\4\3\0\0", 4))
	printf("oops11\n");

    target[0] = 0;
    target[1] = 0;
    target[2] = 0;
    target[3] = 0;
    u4 = cnu4(source, 4);
    u4cn(target, u4, 4);
    if (strncmp(source, target, 4))
	printf("oops12\n");

    printf("%04x %04x %08lx %08lx\n", u2, u2a, u4, u4a);

    typeconv_init(FALSE, TRUE);
    source = "\2\1\4\3";

    target[0] = 0;
    target[1] = 0;
    u2 = cnu2(source + 2, 2);
    u2cn(target, u2, 2);
    if (strncmp(source + 2, target, 2))
	printf("oops13\n");

    target[0] = 0;
    target[1] = 0;
    u4a = cnu4(source + 2, 2);
    u4cn(target, u4a, 2);
    if (strncmp(source + 2, target, 2))
	printf("oops14\n");

    target[0] = 0;
    target[1] = 0;
    target[2] = 0;
    target[3] = 0;
    u2a = cnu2(source, 4);
    u2cn(target, u2a, 4);
    if (strncmp(target, "\0\0\4\3", 4))
	printf("oops15\n");

    target[0] = 0;
    target[1] = 0;
    target[2] = 0;
    target[3] = 0;
    u4 = cnu4(source, 4);
    u4cn(target, u4, 4);
    if (strncmp(source, target, 4))
	printf("oops16\n");

    printf("%04x %04x %08lx %08lx\n", u2, u2a, u4, u4a);

    typeconv_init(TRUE, FALSE);
    source = "\3\4\1\2";

    target[0] = 0;
    target[1] = 0;
    u2 = cnu2(source, 2);
    u2cn(target, u2, 2);
    if (strncmp(source, target, 2))
	printf("oops5\n");

    target[0] = 0;
    target[1] = 0;
    u4a = cnu4(source, 2);
    u4cn(target, u4a, 2);
    if (strncmp(source, target, 2))
	printf("oops6\n");

    target[0] = 0;
    target[1] = 0;
    target[2] = 0;
    target[3] = 0;
    u2a = cnu2(source, 4);
    u2cn(target, u2a, 4);
    if (strncmp(target, "\3\4\0\0", 4))
	printf("oops7\n");

    target[0] = 0;
    target[1] = 0;
    target[2] = 0;
    target[3] = 0;
    u4 = cnu4(source, 4);
    u4cn(target, u4, 4);
    if (strncmp(source, target, 4))
	printf("oops8\n");

    printf("%04x %04x %08lx %08lx\n", u2, u2a, u4, u4a);

    typeconv_init(TRUE, TRUE);
    source = "\1\2\3\4";

    target[0] = 0;
    target[1] = 0;
    u2 = cnu2(source + 2, 2);
    u2cn(target, u2, 2);
    if (strncmp(source + 2, target, 2))
	printf("oops1\n");

    target[0] = 0;
    target[1] = 0;
    u4a = cnu4(source + 2, 2);
    u4cn(target, u4a, 2);
    if (strncmp(source + 2, target, 2))
	printf("oops2\n");

    target[0] = 0;
    target[1] = 0;
    target[2] = 0;
    target[3] = 0;
    u2a = cnu2(source, 4);
    u2cn(target, u2a, 4);
    if (strncmp(target, "\0\0\3\4", 4))
	printf("oops3\n");

    target[0] = 0;
    target[1] = 0;
    target[2] = 0;
    target[3] = 0;
    u4 = cnu4(source, 4);
    u4cn(target, u4, 4);
    if (strncmp(source, target, 4))
	printf("oops4\n");

    printf("%04x %04x %08lx %08lx\n", u2, u2a, u4, u4a);
}

#endif				/* DEBUG_TYPECONV */
