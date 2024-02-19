#ifndef LINT
static char sccsid[]="@(#) lzc.c 2.6 88/01/30 18:39:15";
#endif /* LINT */

/*
Lempel-Ziv compression.  Mostly based on Tom Pfau's assembly language
code.

The contents of this file are hereby released to the public domain.

                                    -- Rahul Dhesi  1986/12/31
*/

#include "options.h"
#include "zoo.h"
#include "zooio.h"
#include "various.h"
#include "zoofns.h"           /* function definitions */
/* zoomem.h defines IN_BUF_SIZE & OUT_BUF_SIZE */
#include "zoomem.h"
#include "debug.h"
#include "assert.h"
/* lzconst.h contains constants for lzd() and lzc() */
#include "lzconst.h"

void init_ctab PARMS((void));
void wr_ccode PARMS((int));
int rd_cch PARMS((void));
int lukup_ccode PARMS((int, int, int *));
void ad_ccode PARMS((int, int, int));
void check_ratio PARMS((void));
void flush_c PARMS((int));

/* interval at which to check ratio */
#define CHECKGAP 4000
#define NEXT_USE  1
#define FIRST_USE 2
#define FOUND 0

struct   tabentry {
   int first;
   int next;
   char z_ch;
};

extern char *out_buf_adr;
extern char *in_buf_adr;
extern char memflag;                    /* memory allocated? */
struct tabentry *table;                 /* this table also used by lzd.c */
static unsigned int free_code;
static int nbits;
static unsigned int max_code;
static unsigned int bitsout;
static int bit_interval;
static unsigned int bytesin, ratio, ratflag;
static unsigned int in_offset, in_size;
static unsigned int bit_offset;

#ifdef UNBUF_IO
#define		BLOCKFILE		int
#define		BLOCKREAD		read
#define		BLOCKWRITE		write
int read PARMS ((int, VOIDPTR, unsigned));
int write PARMS ((int, VOIDPTR, unsigned));
#else
#define		BLOCKFILE		ZOOFILE
#define		BLOCKREAD		zooread
#define		BLOCKWRITE		zoowrite
#endif /* UNBUF_IO */

static BLOCKFILE in_f, out_f;

int lzc (input_f, output_f)
BLOCKFILE input_f, output_f;
{
   int nextch, prefix_code, k;
   int status;
   int where;

   in_f = input_f;
   out_f = output_f;

   bit_offset = in_offset = in_size = 0;

   if (memflag == 0) {
     table = (struct tabentry *) ealloc((MAXMAX+10) * sizeof(struct tabentry));
     memflag++;
   }

   init_ctab();
   wr_ccode(CLEAR);
   nextch = rd_cch();
   if (nextch == EOF) {                  /* note real EOF, not Z_EOF */
      wr_ccode (Z_EOF);
		flush_c ((int) ((bit_offset + 7) / 8));
      return (0);                         /* normal return from compress */
   }

   /* compression loop begins here with nextch holding the next input char */
loop1:
   if (ratflag != 0)
      check_ratio();
   nextch &= 0xff;                       /* turn character to code */
   assert(nextch < 256);
loop2:
   prefix_code = nextch;
   nextch = rd_cch();
   if (nextch == EOF) {                  /* note real EOF, not Z_EOF */
      wr_ccode (prefix_code);
      wr_ccode (Z_EOF);
		flush_c ((int) ((bit_offset + 7) / 8));
      return (0);                         /* normal return from compress */
   }
   nextch &= 0xff;                        /* force to 8 bits */
   assert(nextch < 256);

   k = nextch;
   status = lukup_ccode (prefix_code, nextch, &where);
   if (status == FOUND) {
      nextch = where;                     /* where found */
      goto loop2;
   }
   assert(status == FIRST_USE || status == NEXT_USE);

   /* reach here with status = FIRST_USE or NEXT_USE */
   ad_ccode (status, nextch, where);

   wr_ccode (prefix_code);
   nextch = k;

   if (free_code <= max_code)
      goto loop1;
   assert(nbits >= 9 && nbits <= MAXBITS);
   if (nbits >= MAXBITS) {
   /* To continue using table after it is full, remove next two lines */
      wr_ccode (CLEAR);
      init_ctab();

      goto loop1;
   }

   nbits++;
   assert(nbits >= 9 && nbits <= MAXBITS);
   max_code = max_code << 1;
   goto loop1;
} /* end lzc() */

void wr_ccode (code)
int code;
{
   unsigned int ofs_inbyte, hibits;
   int byte_offset;

#ifdef DEBUG
if (code == CLEAR)
   printf(" CLEAR\n");
#endif

   assert(nbits >= 9 && nbits <= MAXBITS);
   bitsout += nbits;                /* total number of bits written */
   bit_interval -= nbits;
   if (bit_interval < 0)
      ratflag = 1;                  /* time to check ratio */

   byte_offset = bit_offset / 8;
   ofs_inbyte = bit_offset % 8;     /* offset within byte */
   bit_offset += nbits;             /* allowing for new code */

   if (byte_offset >= OUTBUFSIZ - 4) {
      flush_c (byte_offset);
      bit_offset = ofs_inbyte + nbits;
      out_buf_adr[0] = out_buf_adr [byte_offset];
      byte_offset = 0;
   }

   code = code & 0xffff;            /* force to 16 bits */

   if (ofs_inbyte == 0)
      out_buf_adr[byte_offset]  = code & 0xff;
   else
      out_buf_adr[byte_offset] |= (code << ofs_inbyte) & 0xff;

   hibits = ((unsigned int) code) >> (8 - ofs_inbyte);
   out_buf_adr[byte_offset+1] = hibits & 0xff;
   out_buf_adr[byte_offset+2] = (((unsigned int) hibits) >> 8) & 0xff;

   assert(nbits >= 9 && nbits <= MAXBITS);
} /* end wr_ccode() */

void init_ctab()
{
   int i;
   bytesin = bitsout = ratio = ratflag = 0;
   bit_interval = CHECKGAP;
   nbits = 9;
   max_code = 512;
#ifdef COMMENT
   for (i = 0; i < 256; i++) {
#endif
   for (i = 0; i < MAXMAX+1; i++) {
      table[i].z_ch = table[i].first = table[i].next = -1;
   }
#ifdef COMMENT
   /*DEBUG*/ table[MAXMAX].first   = table[MAXMAX].next = -1;
   /*DEBUG*/ table[MAXMAX-1].first = table[MAXMAX-1].next = -1;
#endif
   free_code = FIRST_FREE;
} /* end init_ctab() */

int rd_cch()
{
   int count;
   bytesin++;
   if (in_offset == in_size) {
      count = BLOCKREAD (in_f, in_buf_adr, INBUFSIZ);
      if (count == -1)
         prterror ('f', "Error reading input file during compression.\n");
      addbfcrc (in_buf_adr, count);
      if (count == 0) {
         debug((printf("\nEOF on input\n")))
         return (EOF);              /* real EOF, not Z_EOF */
      }
      in_size = count;
      debug((printf("\ninput %d chars\n", count)))
      in_offset = 0;
   }
   in_offset++;
   return (in_buf_adr[in_offset-1] & 0xff);
} /* end rd_cch () */

void check_ratio()
{
#ifdef COMMENT
   int rat;
   if (bitsout > 16383) {     /* avoid overflow */
      bitsout /= 4;
      bytesin /= 4;
   }
   rat = (2 * bitsout) / bytesin;
   if (1.1 * rat < ratio) {
      printf("#");
      wr_ccode (CLEAR);
      init_ctab();
      bit_interval = CHECKGAP;
      bitsout = 0;
      bytesin = 0;
      ratio = 0;
   } else
      ratio = ((ratio << 2) + ((2 * bitsout) / bytesin)) / 5;
#else
   bit_interval = CHECKGAP;
   bitsout = 0;
   bytesin = 0;
#endif
} /* end check_ratio() */

void ad_ccode (status, ch, index)
int status, index, ch;
{
   assert(status == FIRST_USE || status == NEXT_USE);
#ifdef COMMENT
   if (free_code >= MAXMAX)      /* to fix apparent bug in original */
      return;
#endif
#ifdef COMMENT
   if (status == NEXT_USE)
      table[index].next = free_code;
   else                 /* else must be FIRST_USE */
      table[index].first = free_code;
#endif
   if (status == NEXT_USE)
      table[index].next = (free_code >= MAXMAX ? -1 : free_code);
   else                 /* else must be FIRST_USE */
      table[index].first = (free_code >= MAXMAX ? -1 : free_code);

#ifdef COMMENT
   if (free_code < MAXMAX) {
#endif
   if (free_code <= MAXMAX) {
      table[free_code].first = table[free_code].next = -1;
      table[free_code].z_ch = ch & 0xff;
      free_code++;
   }
} /* end ad_ccode() */

int lukup_ccode (index, ch, where)
int index;                        /* where to start looking */
int ch;                             /* char to look for */
int *where;                       /* last entry looked at */
{
   *where = index;
   index = table[index].first;
   if (index == -1) {
      return (FIRST_USE);           /* not found, first use */
   } else {
      while (1) {
         if ((table[index].z_ch & 0xff) == (ch & 0xff)) {
            *where = index;
            return (FOUND);
         }
         *where = index;
         index = table[index].next;
         if (index == -1) {
            return (NEXT_USE);
         }
      } /* end while */
   } /* end else */
} /* end lukup_ccode() */

void flush_c (count)
int count;
{
   int status;
#ifdef DEBUG
printf(" <flushed %d bytes> ", count);
#endif

#ifdef CHECK_BREAK
	check_break();
#endif

	if (count != 0) {
		status = BLOCKWRITE (out_f, out_buf_adr, count);
		if (status == -1)
			prterror ('f', "Error writing during compression.\n");
	}
}
