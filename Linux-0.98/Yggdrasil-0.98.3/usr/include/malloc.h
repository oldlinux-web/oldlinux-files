/* Declarations for `malloc' and friends.
   Copyright 1990, 1991 Free Software Foundation
		  Written May 1989 by Mike Haertel.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation. */

#ifndef _MALLOC_H

#define _MALLOC_H	1

#include <traditional.h>

#ifdef	__STDC__
#include <stddef.h>
#else
#include <sys/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* malloc - Allocate SIZE bytes of memory.  */
/* calloc - Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
#if defined(_STDLIB_H) && !defined(NO_FIX_MALLOC)
extern __inline__ void * malloc _ARGS ((size_t __size));
extern __inline__ void * calloc _ARGS ((size_t __nmemb, size_t __size));
#else
extern void * malloc _ARGS ((size_t __size));
extern void * calloc _ARGS ((size_t __nmemb, size_t __size));
#endif
/* Re-allocate the previously allocated block
   in void *, making the new block SIZE bytes long.  */
extern void * realloc _ARGS ((void * __ptr, size_t __size));
/* Free a block allocated by `malloc', `realloc' or `calloc'.  */
extern void free _ARGS ((void * __ptr));	

/* Allocate SIZE bytes allocated to ALIGNMENT bytes.  */
extern void * memalign _ARGS ((size_t __alignment, size_t __size));

/* Allocate SIZE bytes on a page boundary.  */
extern void * valloc _ARGS ((size_t __size));

#ifdef __cplusplus
}
#endif

#ifdef _MALLOC_INTERNAL

#if	defined(__GNU_LIBRARY__) || defined(__STDC__)
#include <limits.h>
#else
#define	CHAR_BIT	8
#endif

/* The allocator divides the heap into blocks of fixed size; large
   requests receive one or more whole blocks, and small requests
   receive a fragment of a block.  Fragment sizes are powers of two,
   and all fragments of a block are the same size.  When all the
   fragments in a block have been freed, the block itself is freed.  */
#define INT_BIT		(CHAR_BIT * sizeof(int))
#define BLOCKLOG	(INT_BIT > 16 ? 12 : 9)
#define BLOCKSIZE	(1 << BLOCKLOG)
#define BLOCKIFY(SIZE)	(((SIZE) + BLOCKSIZE - 1) / BLOCKSIZE)

/* Determine the amount of memory spanned by the initial heap table
   (not an absolute limit).  */
#define HEAP		(INT_BIT > 16 ? 4194304 : 65536)

/* Number of contiguous free blocks allowed to build up at the end of
   memory before they will be returned to the system.  */
#define FINAL_FREE_BLOCKS	8

/* Where to start searching the free list when looking for new memory.
   The two possible values are 0 and _heapindex.  Starting at 0 seems
   to reduce total memory usage, while starting at _heapindex seems to
   run faster.  */
#define MALLOC_SEARCH_START	_heapindex

/* Data structure giving per-block information.  */
typedef union
  {
    /* Heap information for a busy block.  */
    struct
      {
	/* Zero for a large block, or positive giving the
	   logarithm to the base two of the fragment size.  */
	int type;
	union
	  {
	    struct
	      {
		size_t nfree;	/* Free fragments in a fragmented block.  */
		size_t first;	/* First free fragment of the block.  */
	      } frag;
	    /* Size (in blocks) of a large cluster.  */
	    size_t size;
	  } info;
      } busy;
    /* Heap information for a free block (that may be the first of
       a free cluster).  */
    struct
      {
	size_t size;		/* Size (in blocks) of a free cluster.  */
	size_t next;		/* Index of next free cluster.  */
	size_t prev;		/* Index of previous free cluster.  */
      } free;
  } malloc_info;

/* Pointer to first block of the heap.  */
extern char *_heapbase;

/* Table indexed by block number giving per-block information.  */
extern malloc_info *_heapinfo;

/* Address to block number and vice versa.  */
#define BLOCK(A) (((char *) (A) - _heapbase) / BLOCKSIZE + 1)
/* The following line MUST be split!  m4 will not process it otherwise.  */
#define ADDRESS(B) \
  ((void *) (((B) - 1) * BLOCKSIZE + _heapbase))

/* Current search index for the heap table.  */
extern size_t _heapindex;

/* Limit of valid info table indices.  */
extern size_t _heaplimit;

/* Doubly linked lists of free fragments.  */
struct list
  {
    struct list *next;
    struct list *prev;
  };

/* Free list headers for each fragment size.  */
extern struct list _fraghead[];

/* List of blocks allocated with `memalign' (or `valloc').  */
struct alignlist
  {
    struct alignlist *next;
    void * aligned;		/* The address that memaligned returned.  */
    void * exact;			/* The address that malloc returned.  */
  };
extern struct alignlist *_aligned_blocks;

/* Instrumentation.  */
extern size_t _chunks_used;
extern size_t _bytes_used;
extern size_t _chunks_free;
extern size_t _bytes_free;

/* Internal version of `free' used in `morecore'. */
extern void __free(void * __ptr);

#endif  /* _MALLOC_INTERNAL.  */

#ifdef __cplusplus
extern "C" {
#endif

/* Underlying allocation function; successive calls should
   return contiguous pieces of memory.  */
extern void * (*__morecore) _ARGS ((ptrdiff_t __size));

/* Default value of `__morecore'.  */
extern void * __default_morecore _ARGS ((ptrdiff_t __size));

/* Nonzero if `malloc' has been called and done its initialization.  */
extern int __malloc_initialized;

/* Hooks for debugging versions.  */
extern void (*__free_hook) _ARGS ((void * __ptr));
extern void * (*__malloc_hook) _ARGS ((size_t __size));
extern void * (*__realloc_hook) _ARGS ((void * __ptr, size_t __size));

/* Activate a standard collection of debugging hooks.  */
extern void mcheck _ARGS ((void (*__func)(void)));

/* Statistics available to the user.  */
struct mstats
  {
    size_t bytes_total;		/* Total size of the heap. */
    size_t chunks_used;		/* Chunks allocated by the user. */
    size_t bytes_used;		/* Byte total of user-allocated chunks. */
    size_t chunks_free;		/* Chunks in the free list. */
    size_t bytes_free;		/* Byte total of chunks in the free list. */
  };

/* Pick up the current statistics. */
extern struct mstats mstats _ARGS ((void));

#ifdef __cplusplus
}
#endif

#endif /* malloc.h  */
