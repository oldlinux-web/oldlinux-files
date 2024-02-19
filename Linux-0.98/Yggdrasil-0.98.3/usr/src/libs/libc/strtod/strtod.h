/*
 * This file is part of the C library for Linux and is
 * covered by the GNU General Public license version 2, or
 * any later version.
 *
 * Copyright (C) 1992 Hoongjiu Lu
 *
 * This file is borrowed from gas 1.38.1.
 * Copyright (C) 1987 Free Software Foundation, Inc.
 */

/*
 *
 *	Arbitrary-precision floating point arithmetic.
 *
 *
 *	Notation: a floating point number is expressed as
 *	MANTISSA * (2 ** EXPONENT).
 *
 *	If this offends more traditional mathematicians, then
 *	please tell me your nomenclature for flonums!
 *
 */

#if !defined(__STDC__) && !defined(const)
#define const /* empty */
#endif

/*
 *
 *	Arbitrary-precision integer arithmetic.
 *	For speed, we work in groups of bits, even though this
 *	complicates algorithms.	
 *	Each group of bits is called a 'littlenum'.
 *	A bunch of littlenums representing a (possibly large)
 *	integer is called a 'bignum'.
 *	Bignums are >= 0.
 *
 */

#define	LITTLENUM_NUMBER_OF_BITS	(16)
#define	LITTLENUM_RADIX			(1 << LITTLENUM_NUMBER_OF_BITS)
#define	LITTLENUM_MASK			(0xFFFF)
#define LITTLENUM_SHIFT			(1)
#define CHARS_PER_LITTLENUM		(1 << LITTLENUM_SHIFT)
#ifndef BITS_PER_CHAR
#define BITS_PER_CHAR			(8)
#endif

typedef unsigned short int	LITTLENUM_TYPE;


/* JF truncated this to get around a problem with GCC */
#define	LOG_TO_BASE_2_OF_10		(3.3219280948873623478703194294893901758651 )
				/* WARNING: I haven't checked that the trailing digits are correct! */

/*
 *
 *	Variable precision floating point numbers.
 *
 *	Exponent is the place value of the low littlenum. E.g.:
 *	If  0:  low points to the units             littlenum.
 *	If  1:  low points to the LITTLENUM_RADIX   littlenum.
 *	If -1:  low points to the 1/LITTLENUM_RADIX littlenum.
 *
 */

/* JF:  A sign value of 0 means we have been asked to assemble NaN
   A sign value of 'P' means we've been asked to assemble +Inf
   A sign value of 'N' means we've been asked to assemble -Inf
 */
struct FLONUM_STRUCT
{
  LITTLENUM_TYPE *	low;	/* low order littlenum of a bignum */
  LITTLENUM_TYPE *	high;	/* high order littlenum of a bignum */
  LITTLENUM_TYPE *	leader;	/* -> 1st non-zero littlenum */
				/* If flonum is 0.0, leader==low-1 */
  long int		exponent; /* base LITTLENUM_RADIX */
  char			sign;	/* '+' or '-' */
};

typedef struct FLONUM_STRUCT FLONUM_TYPE;


/*
 *
 *	Since we can (& do) meet with exponents like 10^5000, it
 *	is silly to make a table of ~ 10,000 entries, one for each
 *	power of 10. We keep a table where item [n] is a struct
 *	FLONUM_FLOATING_POINT representing 10^(2^n). We then
 *	multiply appropriate entries from this table to get any
 *	particular power of 10. For the example of 10^5000, a table
 *	of just 25 entries suffices: 10^(2^-12)...10^(2^+12).
 *
 */


extern const FLONUM_TYPE flonum_positive_powers_of_ten[];
extern const FLONUM_TYPE flonum_negative_powers_of_ten[];
extern const int table_size_of_flonum_powers_of_ten;
				/* Flonum_XXX_powers_of_ten[] table has */
				/* legal indices from 0 to */
				/* + this number inclusive. */



/*
 *
 *	Declare worker functions.
 *
 */

#ifdef __STDC__
extern void flonum_copy(const FLONUM_TYPE *in, FLONUM_TYPE *out);
extern void flonum_multip(const FLONUM_TYPE *a, const FLONUM_TYPE *b,
		FLONUM_TYPE *product);
extern int atof_generic(char ** address_of_string_pointer,
		const char *string_of_decimal_marks,
		const char *string_of_decimal_exponent_marks,
		FLONUM_TYPE *address_of_generic_floating_point_number);
extern char *atof_ieee(const char *str, LITTLENUM_TYPE *words);
#else
extern void flonum_multip();
extern void flonum_copy();
extern int atof_generic();
extern char *atof_ieee();
#endif


/*
 *
 *	Declare error codes.
 *
 */

#define ERROR_CARRY_BIT		(1)
#define ERROR_EXPONENT_OVERFLOW (2)

/* Flonums returned here. Enough to hold most precise flonum. */
extern FLONUM_TYPE generic_floating_point_number;
extern LITTLENUM_TYPE generic_bignum []; /* Bignums returned here. */

#define SIZE_OF_LARGE_NUMBER (20)/* Number of littlenums in above. */

/* Precision in LittleNums. */
#define MAX_PRECISION	(6)
#define F_PRECISION	(2)
#define D_PRECISION	(4)
#define X_PRECISION	(6)
#define P_PRECISION	(6)

/* Length in LittleNums of guard bits. */
#define GUARD		(2)

#define NUMBER_OF_WORDS_IN_DOUBLE	4
#define DEFAULT_PRECISION		D_PRECISION
#define DEFAULT_EXPONENT_BITS		11

#if 0
#define NUMBER_OF_WORDS_IN_DOUBLE	2
#define DEFAULT_PRECISION		F_PRECISION
#define DEFAULT_EXPONENT_BITS		8

#define NUMBER_OF_WORDS_IN_DOUBLE	5
#define DEFAULT_PRECISION		X_PRECISION
#define DEFAULT_EXPONENT_BITS		15

#define DEFAULT_PRECISION		R_PRECISION
#define DEFAULT_EXPONENT_BITS		-1
#endif
