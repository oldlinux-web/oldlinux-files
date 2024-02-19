/***********************************************************
	slide.c -- sliding dictionary with percolating update
***********************************************************/
#include "lharc.h"
#include "slidehuf.h"
#include "intrface.h"

#define PERCOLATE  1
#define NIL        0

node *next = NULL;
int unpackable;
unsigned long origsize, compsize;
unsigned short dicbit;
unsigned short maxmatch;
unsigned long count;
unsigned short loc;
unsigned char *text;
int prev_char;

static unsigned long encoded_origsize;

static struct encode_option encode_define[2] = {
#if defined(__STDC__) || defined(AIX)
/* lh1 */
	{(void(*)())output_dyn,
	 (void(*)())encode_start_fix,
	 (void(*)())encode_end_dyn},
/* lh4, 5 */
	{(void(*)())output_st1,
	(void(*)())encode_start_st1,
	(void(*)())encode_end_st1}
#else
/* lh1 */
	{(int(*)())output_dyn,
	 (int(*)())encode_start_fix,
	 (int(*)())encode_end_dyn},
/* lh4, 5 */
	{(int(*)())output_st1,
	(int(*)())encode_start_st1,
	(int(*)())encode_end_st1}
#endif
};

static struct decode_option decode_define[7] = {
/* lh1 */
	{decode_c_dyn, decode_p_st0, decode_start_fix},
/* lh2 */
	{decode_c_dyn, decode_p_dyn, decode_start_dyn},
/* lh3 */
	{decode_c_st0, decode_p_st0, decode_start_st0},
/* lh4 */
	{decode_c_st1, decode_p_st1, decode_start_st1},
/* lh5 */
	{decode_c_st1, decode_p_st1, decode_start_st1},
/* lzs */
	{decode_c_lzs, decode_p_lzs, decode_start_lzs},
/* lz5 */
	{decode_c_lz5, decode_p_lz5, decode_start_lz5}
};

static struct encode_option encode_set;
static struct decode_option decode_set;

static node pos, matchpos, avail,
		*position, *parent, *prev;
static int remainder, matchlen;
static unsigned char *level, *childcount;
static unsigned short dicsiz;
static unsigned short max_hash_val;
static unsigned short hash1, hash2;

int encode_alloc(method)
int method;
{
  if (method == 1) {
    encode_set = encode_define[0];
    maxmatch = 60;
    dicbit = 12;
  } else {
    encode_set = encode_define[1];
    maxmatch = MAXMATCH;
    dicbit = 13;
  }
  while (1) {
    dicsiz = 1 << dicbit;
    max_hash_val = 3 * dicsiz + (dicsiz / 512 + 1) * UCHAR_MAX;
    text = (unsigned char *)malloc(dicsiz * 2 + maxmatch);
    level = (unsigned char *)malloc((dicsiz + UCHAR_MAX + 1) * sizeof(*level));
    childcount = (unsigned char *)malloc((dicsiz + UCHAR_MAX + 1) * sizeof(*childcount));
#if PERCOLATE
    position = (node *)malloc((dicsiz + UCHAR_MAX + 1) * sizeof(*position));
#else
    position = (node *)malloc(dicsiz * sizeof(*position));
#endif
    parent     = (node *)malloc(dicsiz * 2 * sizeof(*parent));
    prev       = (node *)malloc(dicsiz * 2 * sizeof(*prev));
    next       = (node *)malloc((max_hash_val + 1) * sizeof(*next));
    if (next == NULL || 
	method > 1 && alloc_buf() == NULL) {
      if (next) free(next);
      if (prev) free(prev);
      if (parent) free(parent);
      if (position) free(position);
      if (childcount) free(childcount);
      if (level) free(level);
      if (text) free(text);
    } else {
      break;
    }
    if (--dicbit < 12 )
      /* error(MEMOVRERR, NULL); */
      exit( 207 );
  }
  if (method == 5)
    method = dicbit - 8;
  return method;
}

static void init_slide(void)
{
	node i;

	for (i = dicsiz; i <= dicsiz + UCHAR_MAX; i++) {
		level[i] = 1;
#if PERCOLATE
			position[i] = NIL;  /* sentinel */
#endif
	}
	for (i = dicsiz; i < dicsiz * 2; i++) parent[i] = NIL;
	avail = 1;
	for (i = 1; i < dicsiz - 1; i++) next[i] = i + 1;
	next[dicsiz - 1] = NIL;
	for (i = dicsiz * 2; i <= max_hash_val; i++) next[i] = NIL;
	hash1 = dicbit - 9;
	hash2 = dicsiz * 2;
}

#define HASH(p, c) ((p) + ((c) << hash1) + hash2)

static /* inline */ node child(q, c)
	/* q's child for character c (NIL if not found) */
node q;
unsigned char c;
{
	node r;

	r = next[HASH(q, c)];
	parent[NIL] = q;  /* sentinel */
	while (parent[r] != q) r = next[r];
	return r;
}

static /* inline */ void makechild(q, c, r)
	/* Let r be q's child for character c. */
node q;
unsigned char c;
node r;
{
	node h, t;

	h = HASH(q, c);
	t = next[h];  next[h] = r;  next[r] = t;
	prev[t] = r;  prev[r] = h;
	parent[r] = q;  childcount[q]++;
}

static /*inline*/ void split(old)
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

static void insert_node(void)
{
	node q, r, j, t;
	unsigned char c, *t1, *t2;

	if (matchlen >= 4) {
		matchlen--;
		r = (matchpos + 1) | dicsiz;
		while ((q = parent[r]) == NIL) r = next[r];
		while (level[q] >= matchlen) {
			r = q;  q = parent[q];
		}
#if PERCOLATE
			t = q;
			while (position[t] < 0) {
				position[t] = pos;  t = parent[t];
			}
			if (t < dicsiz) position[t] = pos | SHRT_MIN;
#else
			t = q;
			while (t < dicsiz) {
				position[t] = pos;  t = parent[t];
			}
#endif
	} else {
		q = text[pos] + dicsiz;  c = text[pos + 1];
		if ((r = child(q, c)) == NIL) {
			makechild(q, c, pos);  matchlen = 1;
			return;
		}
		matchlen = 2;
	}
	for ( ; ; ) {
		if (r >= dicsiz) {
			j = maxmatch;  matchpos = r;
		} else {
			j = level[r];
			matchpos = position[r] & SHRT_MAX;
		}
		if (matchpos >= pos) matchpos -= dicsiz;
		t1 = &text[pos + matchlen];  t2 = &text[matchpos + matchlen];
		while (matchlen < j) {
			if (*t1 != *t2) {  split(r);  return;  }
			matchlen++;  t1++;  t2++;
		}
		if (matchlen == maxmatch) break;
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

static void delete_node(void)
{
#if PERCOLATE
		node q, r, s, t, u;
#else
		node r, s, t, u;
#endif

	if (parent[pos] == NIL) return;
	r = prev[pos];  s = next[pos];
	next[r] = s;  prev[s] = r;
	r = parent[pos];  parent[pos] = NIL;
	if (r >= dicsiz || --childcount[r] > 1) return;
#if PERCOLATE
		t = position[r] & SHRT_MAX;
#else
		t = position[r];
#endif
	if (t >= pos) t -= dicsiz;
#if PERCOLATE
		s = t;  q = parent[r];
		while ((u = position[q]) < 0) {
			u &= SHRT_MAX;  if (u >= pos) u -= dicsiz;
			if (u > s) s = u;
			position[q] = (s | dicsiz);  q = parent[q];
		}
		if (q < dicsiz) {
			if (u >= pos) u -= dicsiz;
			if (u > s) s = u;
			position[q] = (s | dicsiz) | SHRT_MIN;
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

/* static */void get_next_match(void)
{
	int n;

	remainder--;
	if (++pos == dicsiz * 2) {
		bcopy(&text[dicsiz], &text[0], dicsiz + maxmatch);
		n = fread_crc(&text[dicsiz + maxmatch], dicsiz, infile);
		encoded_origsize += n;
		remainder += n;  pos = dicsiz;
	}
	delete_node();  insert_node();
}

void encode(interface)
struct interfacing *interface;
{
	int lastmatchlen, dicsiz1;
	node lastmatchpos;

	infile = interface -> infile;
	outfile = interface -> outfile;
	origsize = interface -> original;
	compsize = count = 0L;
	crc = unpackable = 0;
	init_slide();  encode_set.encode_start();
	dicsiz1 = dicsiz - 1;
	pos = dicsiz + maxmatch;
	memset(&text[pos], ' ', dicsiz);
	remainder = fread_crc(&text[pos], dicsiz, infile);
	encoded_origsize = remainder;
	matchlen = 0;
	insert_node();
	while (remainder > 0 && ! unpackable) {
		lastmatchlen = matchlen;  lastmatchpos = matchpos;
		get_next_match();
		if (matchlen > remainder) matchlen = remainder;
		if (matchlen > lastmatchlen || lastmatchlen < THRESHOLD) {
			encode_set.output(text[pos - 1], 0);
			count++;
		} else {
			encode_set.output(lastmatchlen + (UCHAR_MAX + 1 - THRESHOLD),
				   (pos - lastmatchpos - 2) & dicsiz1);
			while (--lastmatchlen > 0) {
				get_next_match();
				count++;
			}
			if (matchlen > remainder) matchlen = remainder;
		}
	}
	encode_set.encode_end();
	interface -> packed = compsize;
	interface -> original = encoded_origsize;
}

void decode(interface)
struct interfacing *interface;
{
	int i, j, k, c, dicsiz1, offset;

	infile = interface -> infile;
	outfile = interface -> outfile;
	dicbit = interface -> dicbit;
	origsize = interface -> original;
	compsize = interface -> packed;
	decode_set = decode_define[interface -> method - 1];
	crc = 0;
	prev_char = -1;
	dicsiz = 1 << dicbit;
	text = (unsigned char *)malloc(dicsiz);
	if (text == NULL)
		/* error(MEMOVRERR, NULL); */
		exit( errno );
	memset(text, ' ', dicsiz);
	decode_set.decode_start();
	dicsiz1 = dicsiz - 1;
	offset = (interface -> method == 6) ? 0x100 - 2 : 0x100 - 3;
	count = 0;  loc = 0;
	while (count < origsize) {
		c = decode_set.decode_c();
		if (c <= UCHAR_MAX) {
			text[loc++] = c;
			if (loc == dicsiz) {
				fwrite_crc(text, dicsiz, outfile);
				loc = 0;
			}
			count++;
		} else {
			j = c - offset;
			i = (loc - decode_set.decode_p() - 1) & dicsiz1;
			count += j;
			for (k = 0; k < j; k++) {
				c = text[(i + k) & dicsiz1];
				text[loc++] = c;
				if (loc == dicsiz) {
					fwrite_crc(text, dicsiz, outfile);
					loc = 0;
				}
			}
		}
	}
	if (loc != 0) {
		fwrite_crc(text, loc, outfile);
	}
	free(text);
}

