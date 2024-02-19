/*			b i t s e t
 *
 * (C) Copyright C E Chew
 *
 * Feel free to copy, use and distribute this software provided:
 *
 *	1. you do not pretend that you wrote it
 *	2. you leave this copyright notice intact.
 *
 * This implements a set of operators to manipulate bitsets
 * is a machine independent way. It may be necessary for other
 * machines to set up the constants at the front (such as the
 * width of int, etc).
 *
 * To avoid double evaluation of the argument in some of the
 * macros, define the data structure as follows:
 *
 *	1. the bitset is int []
 *	2. the first int will be used as scratch int [0]
 *	3. all the following elements will be used for the bitset proper
 *
 * Patchlevel 2.0
 *
 * Edit History:
 */

/* Declare the relative sizing of the components */

#define _BITELEMENT	unsigned int
#define _BITSINCHAR	CHAR_BIT

/* Determine how many bits there are in an elemental bitset */

#define _BITS		(sizeof(_BITELEMENT)*_BITSINCHAR)

/* Declare the relative offsets of the pieces in the structure */

#define _BIT_SCRATCH_	0
#define	_BIT_SET_	1

/* Define how to find a bit within a bitset */

#define _BITSIZE_(n)	(((n)+_BITS-1)/_BITS)
#define _BITGROUP_(n)	((n)/_BITS)
#define _BITMASK_(n)	(1<<((n)%_BITS))

/* Define how to find a bit within the data structure */

#define _BITSIZE(n)	(_BITSIZE_(n)+_BIT_SET_)
#define _BITGROUP(n)	(_BITGROUP_(n)+_BIT_SET_)
#define _BITMASK(n)	(_BITMASK_(n))

/*
 * Declare a bitset of length n bits
 */

#define bitstring(name, n) _BITELEMENT name[_BITSIZE(n)]

/*
 * Clear all elements of the bitset
 */

#define bitempty(name, n) MEMSET((char *) &name[_BIT_SET_], 0, \
				 (size_t) (_BITSIZE_(n)*sizeof(_BITELEMENT)))

/*
 * Set all elements of the bitset
 */

#define bitfill(name, n) (_BITFILL(&name[_BIT_SET_], ~0, \
				   _BITSIZE_(n)*sizeof(_BITELEMENT)))

/*
 * Set one bit in the bitset
 */

#define bitset(name, n) (name[_BIT_SCRATCH_]=(n), \
			 name[_BITGROUP(name[_BIT_SCRATCH_])] |= \
			   _BITMASK(name[_BIT_SCRATCH_]))

/*
 * Clear one bit in the bitset
 */

#define bitclear(name, n) (name[_BIT_SCRATCH_]=(n), \
			   name[_BITGROUP(name[_BIT_SCRATCH_])] &= \
			     ~_BITMASK(name[_BIT_SCRATCH_]))

/*
 * Test one bit in the bitset
 */

#define bittest(name, n) (name[_BIT_SCRATCH_]=(n), \
			  name[_BITGROUP(name[_BIT_SCRATCH_])] & \
			    _BITMASK(name[_BIT_SCRATCH_]))
