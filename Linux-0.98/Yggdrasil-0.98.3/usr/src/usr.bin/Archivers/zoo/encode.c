/*$Source: /usr/home/dhesi/zoo/RCS/encode.c,v $*/
/*$Id: encode.c,v 1.41 91/07/09 01:39:47 dhesi Exp $*/

/*
Adapted from "ar" archiver written by Haruhiko Okumura.
*/

#include "options.h"
#include "zoo.h"
#include "ar.h"
#include "lzh.h"

extern void prterror();
extern char *out_buf_adr;

#include <assert.h>

#ifdef ANSI_HDRS
# include <stdlib.h>
# include <string.h>
#endif

#include "errors.i"

FILE *lzh_infile;
FILE *lzh_outfile;

/*
sliding dictionary with percolating update
*/

#define PERCOLATE  1
#define NIL        0
#define MAX_HASH_VAL (3 * DICSIZ + (DICSIZ / 512 + 1) * UCHAR_MAX)

typedef short node;

static uchar *text, *childcount;
static node pos, matchpos, avail,
	*position, *parent, *prev, *next = NULL;
static int remainder, matchlen;

#if MAXMATCH <= (UCHAR_MAX + 1)
	static uchar *level;
# define T_LEVEL  uchar *
#else
	static ushort *level;
# define T_LEVEL  ushort *
#endif

static void allocate_memory()
{
	if (next != NULL) return;
	/* text = (uchar *) malloc(DICSIZ * 2 + MAXMATCH); */
	text = (uchar *) out_buf_adr; /* reuse I/O buffer used elsewhere */
	level      = (T_LEVEL) malloc((DICSIZ + UCHAR_MAX + 1) * sizeof(*level));
	childcount = (uchar *)malloc((DICSIZ + UCHAR_MAX + 1) * sizeof(*childcount));
#ifdef PERCOLATE
	  position = (node *) malloc((DICSIZ + UCHAR_MAX + 1) * sizeof(*position));
#else
	  position = (node *) malloc(DICSIZ * sizeof(*position));
#endif
	parent     = (node *) malloc(DICSIZ * 2 * sizeof(*parent));
	prev       = (node *) malloc(DICSIZ * 2 * sizeof(*prev));
	next       = (node *) malloc((MAX_HASH_VAL + 1) * sizeof(*next));
	if (next == NULL) prterror('f', no_memory);
}

static void init_slide()
{
	node i;

	for (i = DICSIZ; i <= DICSIZ + UCHAR_MAX; i++) {
		level[i] = 1;
#ifdef PERCOLATE
			position[i] = NIL;  /* sentinel */
#endif
	}
	for (i = DICSIZ; i < DICSIZ * 2; i++) parent[i] = NIL;
	avail = 1;
	for (i = 1; i < DICSIZ - 1; i++) next[i] = i + 1;
	next[DICSIZ - 1] = NIL;
	for (i = DICSIZ * 2; i <= MAX_HASH_VAL; i++) next[i] = NIL;
}

#define HASH(p, c) ((p) + ((c) << (DICBIT - 9)) + DICSIZ * 2)

static node child(q, c)
node q;
uchar c;
	/* q's child for character c (NIL if not found) */
{
	node r;

	r = next[HASH(q, c)];
	parent[NIL] = q;  /* sentinel */
	while (parent[r] != q) r = next[r];
	return r;
}

static void makechild(q, c, r)
node q;
uchar c;
node r;
	/* Let r be q's child for character c. */
{
	node h, t;

	h = HASH(q, c);
	t = next[h];  next[h] = r;  next[r] = t;
	prev[t] = r;  prev[r] = h;
	parent[r] = q;  childcount[q]++;
}

void split(old)
node old;
{
	node new, t;

	new = avail;  avail = next[new];  childcount[new] = 0;
	t = prev[old];  prev[new] = t;  next[t] = new;
	t = next[old];  next[new] = t;  prev[t] = new;
	parent[new] = parent[old];
	level[new] = matchlen;
	position[new] = pos;
	makechild(new, text[matchpos + matchlen], old);
	makechild(new, text[pos + matchlen], pos);
}

static void insert_node()
{
	node q, r, j, t;
	uchar c, *t1, *t2;

	if (matchlen >= 4) {
		matchlen--;
		r = (matchpos + 1) | DICSIZ;
		while ((q = parent[r]) == NIL) r = next[r];
		while (level[q] >= matchlen) {
			r = q;  q = parent[q];
		}
#ifdef PERCOLATE
			t = q;
			while (position[t] < 0) {
				position[t] = pos;  t = parent[t];
			}
			if (t < DICSIZ) position[t] = pos | PERC_FLAG;
#else
			t = q;
			while (t < DICSIZ) {
				position[t] = pos;  t = parent[t];
			}
#endif
	} else {
		q = text[pos] + DICSIZ;  c = text[pos + 1];
		if ((r = child(q, c)) == NIL) {
			makechild(q, c, pos);  matchlen = 1;
			return;
		}
		matchlen = 2;
	}
	for ( ; ; ) {
		if (r >= DICSIZ) {
			j = MAXMATCH;  matchpos = r;
		} else {
			j = level[r];
			matchpos = position[r] & ~PERC_FLAG;
		}
		if (matchpos >= pos) matchpos -= DICSIZ;
		t1 = &text[pos + matchlen];  t2 = &text[matchpos + matchlen];
		while (matchlen < j) {
			if (*t1 != *t2) {  split(r);  return;  }
			matchlen++;  t1++;  t2++;
		}
		if (matchlen >= MAXMATCH) break;
		position[r] = pos;
		q = r;
		if ((r = child(q, *t1)) == NIL) {
			makechild(q, *t1, pos);  return;
		}
		matchlen++;
	}
	t = prev[r];  prev[pos] = t;  next[t] = pos;
	t = next[r];  next[pos] = t;  prev[t] = pos;
	parent[pos] = q;  parent[r] = NIL;
	next[r] = pos;  /* special use of next[] */
}

static void delete_node()
{
#ifdef PERCOLATE
		node q, r, s, t, u;
#else
		node r, s, t, u;
#endif

	if (parent[pos] == NIL) return;
	r = prev[pos];  s = next[pos];
	next[r] = s;  prev[s] = r;
	r = parent[pos];  parent[pos] = NIL;
	if (r >= DICSIZ || --childcount[r] > 1) return;
#ifdef PERCOLATE
		t = position[r] & ~PERC_FLAG;
#else
		t = position[r];
#endif
	if (t >= pos) t -= DICSIZ;
#ifdef PERCOLATE
		s = t;  q = parent[r];
		while ((u = position[q]) & PERC_FLAG) {
			u &= ~PERC_FLAG;  if (u >= pos) u -= DICSIZ;
			if (u > s) s = u;
			position[q] = (s | DICSIZ);  q = parent[q];
		}
		if (q < DICSIZ) {
			if (u >= pos) u -= DICSIZ;
			if (u > s) s = u;
			position[q] = s | DICSIZ | PERC_FLAG;
		}
#endif
	s = child(r, text[t + level[r]]);
	t = prev[s];  u = next[s];
	next[t] = u;  prev[u] = t;
	t = prev[r];  next[t] = s;  prev[s] = t;
	t = next[r];  prev[t] = s;  next[s] = t;
	parent[s] = parent[r];  parent[r] = NIL;
	next[r] = avail;  avail = r;
}

static void get_next_match()
{
	int n;

	remainder--;
	if (++pos == DICSIZ * 2) {
#ifdef CHECK_BREAK
		check_break();
#endif
		(void) MOVE_LEFT((char *) &text[0], (char *) &text[DICSIZ], DICSIZ + MAXMATCH);
		n = fread_crc(&text[DICSIZ + MAXMATCH], DICSIZ, lzh_infile);
		remainder += n;  pos = DICSIZ;  
#ifdef SHOW_DOTS
		(void) putc('.', stderr);
		(void) fflush(stderr);
#endif
	}
	delete_node();  insert_node();
}

/* read from infile, compress, write to outfile */
void encode(infile, outfile)
FILE *infile;
FILE *outfile;
{
	int lastmatchlen;
	node lastmatchpos;

	/* make input/output files visible to other functions */
	lzh_infile = infile;
	lzh_outfile = outfile;

	allocate_memory();  init_slide();  huf_encode_start();
	remainder = fread_crc(&text[DICSIZ], DICSIZ + MAXMATCH, lzh_infile);
#ifdef SHOW_DOTS
	(void) putc('.', stderr);
	(void) fflush(stderr);
#endif
	matchlen = 0;
	pos = DICSIZ;  insert_node();
	if (matchlen > remainder) matchlen = remainder;
	while (remainder > 0 && ! unpackable) {
		lastmatchlen = matchlen;  lastmatchpos = matchpos;
		get_next_match();
		if (matchlen > remainder) matchlen = remainder;
		if (matchlen > lastmatchlen || lastmatchlen < THRESHOLD) {
#if 0
			d1log("%c", text[pos-1]);
#endif
			output(text[pos - 1], 0);
		} else {
#if 0
		(void) putc('.', stderr); (void) fflush(stderr);
#endif

#if 0
			{
				int i; 
				d1log("\nlastmatchlen=%d, pos=%d, lastmatchpos=%d",
							lastmatchlen, pos, lastmatchpos);
				d1log("\n[%d: ", (int) lastmatchlen);
				for (i = 0;  i < lastmatchlen;  i++)
					d1log("%c", text[lastmatchpos + i]);
				d1log("]\n");
			}
#endif

			output((uint) (lastmatchlen + (UCHAR_MAX + 1 - THRESHOLD)),
				   (uint) ((pos - lastmatchpos - 2) & (DICSIZ - 1)));
			while (--lastmatchlen > 0) get_next_match();
			if (matchlen > remainder) matchlen = remainder;
		}
	}
	huf_encode_end();
}

#ifdef NEED_MEMMOVE
/* like memmove, but for moving stuff LEFT (downwards in memory) only!! */
void move_left(dest, src, len)
char *dest;
char *src;
int len;
{
	while (len-- > 0)
		*dest++ = *src++;
}
#endif /* NEED_MEMMOVE */
