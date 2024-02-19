extern void InitTree();

#ifndef BITS
#define BITS    14      /* for 16-bits machines */
#endif

#if BITS < 13
#undef BITS
#define BITS    13      /* 1:1 hash */
#endif

#if BITS > 16
#undef BITS
#define BITS    16      /* 128K hash table, if sizeof(us_t) == 2 */
#endif

/* The following hash-function isn't optimal but it is very fast:

	HASH =      ((first + (second << LEN0) +
		    (third << LEN1)) & ((1L << BITS) - 1);

  The difference of LENs is no more than one bit.
*/

#define LEN0    ((BITS-8)/2)
#define LEN1    (BITS-8)

#define NIL     N2

#if defined(M_XENIX) && !defined(M_I386) && (BITS > 14)
#define __XENIX__
#endif

/* `array_size' is the size of array `next', which contains
	the heads of linked lists and the references to
	next members of these lists.
*/

#define array_size      (N2 + 1 + (1L << BITS))

extern hash_t prev[];
extern us_t      match_position, match_length, chain_length;

#ifndef __XENIX__
#define nextof(i)       next[i]

#ifdef __TURBOC__
extern us_t huge * next;
#else   /* __TURBOC__ */
extern us_t next[];
#endif  /* __TURBOC__ */

#else   /* __XENIX__ */

/* We divide the array `next' in `parts' which fit into 286's segment */
/* There may be 2 or 3 parts, because BITS <= 16 now */

#define parts (array_size/32768L + 1)
#define nextof(i)       next[(i) >> 15][(i) & 0x7fff]
#if parts == 2
extern us_t next0[], next1[];
#else
extern us_t next0[], next1[], next2[];
#endif  /* parts */

extern us_t *next[];
#endif  /* __XENIX__ */

/* Some defines to eliminate function-call overhead */

/* Deleting of a node `n' from a linked list */

#define DeleteNode(n) \
{\
       nextof(prev[n]) = NIL;\
       prev[n] = NIL;\
}

/* Inserting of a node `r' into hashed linked list: `r' becomes
	the head of list.
*/

#define InsertNode(r)\
{\
	register hash_t p; register us_t first_son;\
	register uc_t  *key;\
	key = &text_buf[r];\
	p = N2 + 1 + (((hash_t)key[0] + ((hash_t)key[1] << LEN0) +\
		    ((hash_t)key[2] << LEN1)) & ((1L << BITS) - 1));\
	first_son = nextof(p);\
	nextof(r) = first_son;\
	nextof(p) = r;\
	prev[r] = p;\
	prev[first_son] = r;\
}

/* This routine inputs the char from stdin and does some other
	actions depending of this char's presence.
*/

#define Next_Char(N,F)\
if ((c = getchar()) != EOF) {\
	text_buf[s] = c;\
	if (s < F - 1)\
		text_buf[s + N] = c;\
	s = (s + 1) & (N - 1);\
	r = (r + 1) & (N - 1);\
	InsertNode(r);\
	in_count++;\
} else {\
	s = (s + 1) & (N - 1);\
	r = (r + 1) & (N - 1);\
	if (--len) InsertNode(r);\
}

#if defined(__GNUC__)
#if defined(__i386__)
/* Optimizer cannot allocate these registers correctly :( (v1.39) */
#define FIX_SI  asm("si")
#define FIX_DI  asm("di")
#else

/* GNU-style register allocations for other processors are welcome! */

#define FIX_SI
#define FIX_DI
#endif
#else

/* Dummy defines for non-GNU compilers */

#define FIX_SI
#define FIX_DI
#endif

#ifndef DO_INLINE

/* This statement is due to ideas of Boyer and Moore: */
/* Is somewhere an optimizing compiler which can vectorize this? ;-) */

#define MATCHING for (m = match_length; m >= 0 && key[m] == pattern[m]; m--)
#define NOT_YET (m >= 0)

#else

/* Hope this function will be intrinsic (Microsoft C).
   Ideally this memcmp should be right-to-left, but this works
   fast enough.
*/
#define MATCHING m = memcmp(key, pattern, match_length + 1)
#define NOT_YET (m != 0)

#endif

#define CHAIN_THRESHOLD (F2 * 3)

#ifdef  FAST
/* Simple inline replacement for get_next_match; they match
literally except return --> goto quote(leave)l. No obfuscations !! */

#ifdef __STDC__
#define LEAVE(num) leave##num
#else
#define quote(x)x
#define LEAVE(num) quote(leave)num
#endif

#define Get_Next_Match(r,l) {register us_t p=r;register int m;\
register uc_t *key FIX_SI, *pattern FIX_DI;int chain_count=chain_length;\
key=text_buf+r;do{ do{ if((p=nextof(p))==NIL||(greedy &&\
!chain_count--))goto LEAVE(l);pattern=text_buf+p;MATCHING;}while NOT_YET;\
for(m=match_length;++m<F2&&key[m]==pattern[m];);\
match_length=m;match_position=((r-p)&(N2-1))-1;}while(m<F2);\
nextof(prev[p])=nextof(p);prev[nextof(p)]=prev[p];prev[p]=NIL;LEAVE(l):;}

#else

#define Get_Next_Match(r,l)     get_next_match(r)

#endif
