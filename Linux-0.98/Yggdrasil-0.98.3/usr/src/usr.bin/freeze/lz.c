#include "freeze.h"
#include "lz.h"

/*----------------------------------------------------------------------*/
/*                                                                      */
/*                          LZSS ENCODING                               */
/*                                                                      */
/*----------------------------------------------------------------------*/

uc_t    text_buf[N2 + F2 - 1];          /* cyclic buffer with an overlay */
us_t    match_position, match_length;   /* current characteristics of a
						matched pattern */
us_t    chain_length;                   /* max_chain_length ==
						CHAIN_THRESHOLD / greedy */


/*      next[N+1..] is used as hash table,
	the rest of next is a link down,
	prev is a link up.
*/

hash_t             prev[N2 + 1];

#ifndef __XENIX__
#ifdef __TURBOC__
us_t huge * next = (us_t huge *) NULL;
#else  /* __TURBOC__ */
us_t next[array_size];       /* a VERY large array :-) */
#endif /* __TURBOC__ */
#else  /* __XENIX__ */
#if parts == 2
us_t next0[32768L], next1[8193];
#else
us_t next0[32768L], next1[32768L], next2[8193];
#endif  /* parts */

/* A list of `next's parts */

us_t *next[parts] = {
next0, next1
#if parts != 2          /* was: parts > 2. Doesn't work on X286 cpp */
,next2
#endif
};
#endif  /* __XENIX__ */

#ifdef GATHER_STAT
long node_steps, node_matches;
#endif  /* GATHER_STAT */

/* Initialize the data structures and allocate memory, if needed.
	Although there is no more trees in the LZ algorithm
	implementation, routine name is kept intact :-)
*/

void InitTree ()
{
	hash_t i;
#ifdef GATHER_STAT
	node_steps = node_matches = 0;
#endif  /* GATHER_STAT */

#ifdef __TURBOC__
	if (!next && (next = (us_t huge *) farmalloc(
		(ul_t)array_size * sizeof(us_t))) == NULL) {

		fprintf(stderr, "Not enough memory (%luK wanted)\n",
			(ul_t)array_size * sizeof(us_t) / 1024);
		exit(3);
	}
#endif  /* __TURBOC__ */

	for (i = N2 + 1; i < array_size; i++ )
		nextof(i) = NIL;
	for (i = 0; i < sizeof(prev)/sizeof(*prev); i++ )
		prev[i] = NIL;
	chain_length = greedy ? CHAIN_THRESHOLD / greedy : 0;
}

/* Get the longest (longer than `match_length' when entering in subroutine)
	nearest match of the string beginning in text_buf[r]
	to the cyclic buffer. Result (length & position) is returned
	in correspondent global variables (`match_length' &
	`match_position'). Unchanged `match_length' denotes failure.
*/

#ifndef FAST
void get_next_match (r)
	us_t r;
{
	register us_t p = r;
	register int m;
	register uc_t  *key FIX_SI, *pattern FIX_DI;
	int     chain_count = chain_length;
#ifdef GATHER_STAT
	node_matches++;
#endif
	key = text_buf + r;
	do {
		do {
			/* From ZIP 1.0 by J.-L. Gailly et al. */

			if ((p = nextof(p)) == NIL ||
				(greedy && !chain_count--))
				return;

			pattern = text_buf + p;

			MATCHING;

		} while NOT_YET;

#ifdef GATHER_STAT
		node_steps++;
#endif

		for (m = match_length;
			++m < F2 && key[m] == pattern[m]; );

		match_length = m;
		match_position = ((r - p) & (N2 - 1)) - 1;
	} while (m < F2);

/* There are two equal F-char sequences, the oldest one must be
	deleted from the list.
*/


#ifdef DEBUG
	if (verbose)
		fprintf(stderr, "Replacing node: %d -> %d\n", p, r);
#endif
	nextof(prev[p]) = nextof(p);
	prev[nextof(p)] = prev[p];
	prev[p] = NIL;  /* remove p, it is further than r */
	return;
}
#endif
