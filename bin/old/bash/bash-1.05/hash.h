/* hash.h -- the data structures used in hashing in Gsh. */

typedef struct bucket_contents {
  struct bucket_contents *next;	/* The next item whose key hashes to this bucket. */
  char *key;			/* What we look up. */
  char *data;			/* What we really want. */
  int times_found;		/* Number of times this item has been looked up. */
} BUCKET_CONTENTS;

typedef struct hash_table {
  BUCKET_CONTENTS **bucket_array;	/* Where the data is kept. */
  int nbuckets;			/* How many buckets does this table have. */
  int nentries;			/* How many entries does this table have. */
} HASH_TABLE;

extern BUCKET_CONTENTS
  *find_hash_item (), *remove_hash_item (), *add_hash_item (),
  *get_hash_bucket ();

extern int hash_string ();
extern HASH_TABLE *make_hash_table ();


#ifndef NULL
#define NULL 0x0
#endif
