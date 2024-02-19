#ifndef LINT
static char sccsid[]="@(#) lzd.c 2.6 88/01/30 20:39:18";
#endif /* LINT */

/*********************************************************************/
/* This file contains two versions of the lzd() decompression routine.
The default is to use a fast version coded by Ray Gardner.  If the
symbol SLOW_LZD is defined, the older slower one is used.  I have tested
Ray's code and it seems to be portable and reliable.  But if you
suspect any problems you can define SLOW_LZD for your system in
options.h and cause the older code to be used.  --R.D. */
/*********************************************************************/

#include "options.h"
#include "zoo.h"
#include "zooio.h"
#include "various.h"
#include "zoofns.h"           /* function definitions */
#include "zoomem.h"
#include "debug.h"
#include "assert.h"
#include "lzconst.h"

#ifndef SLOW_LZD

/* Extensive modifications for speed by Ray Gardner
** Public domain by Raymond D. Gardner  9/26/88
**
** I apologize for the comments being so dense in places as to impair
** readability, but some of the stuff isn't very obvious and needs
** some explaining.  I am also sorry for the messy control structure
** (quite a few labels and goto's) and very long lzd() function, but
** I don't know how to do this any other way without loss of speed.
**
** Ray Gardner
** 6374 S. Monaco Ct.
** Englewood, CO 80111
*/

#ifdef ANSI_HDRS
# include <string.h>		/* to get memcpy */
#else
  VOIDPTR memcpy();
#endif

#define  STACKSIZE   4000  /* allows for about 8Mb string in worst case? */
/* stack grows backwards in this version, using pointers, not counters */
static char *stack;
static char *stack_pointer;
static char *stack_lim;

void init_dtab PARMS((void));
unsigned rd_dcode PARMS((void));
/* void wr_dchar (char); */		/* now a macro */
void ad_dcode PARMS((void));

#ifdef FILTER
/* to send data back to zoofilt */
extern unsigned int filt_lzd_word;
#endif /* FILTER */

void xwr_dchar PARMS ((char));
static int firstchar PARMS ((int));
static void cbfill PARMS ((void));

/* wr_dchar() is a macro for speed */
#define wr_dchar(c) {                             \
                           if (outbufp<outbuflim) \
                              *outbufp++=(c);     \
                           else                   \
                              xwr_dchar(c);       \
                    }

extern char *out_buf_adr;        /* output buffer */
extern char *in_buf_adr;         /* input buffer */
                      /* use pointers (not counters) for buffer (for speed) */
static char *outbufp;            /* output buffer pointer */
static char *outbuflim;          /* output buffer limit */
static char *outbufguard;        /* output buffer "guard" */

char memflag = 0;                /* memory allocated? flag */
int *head;                       /* lzw prefix codes */
char *tail;                      /* lzw suffix codes */
static unsigned cur_code;
static unsigned old_code;
static unsigned in_code;

static unsigned free_code;
static int nbits;
static unsigned max_code;

/* We use a buffer of codes to avoid a function call to unpack each
** one as needed.  We allocate an extra slot past the end of the buffer
** and put a CLEAR code in it, to serve as a sentinel.  This way we can
** fold the test for code buffer runout into the test for a clear code
** and avoid having an extra test on each code processed.  Also, we don't
** always use the code buffer.  We can only use it when the input buffer
** is at a byte boundary, and when we know that the codesize won't change
** before we fill the code buffer, and when we know we won't run out of
** bytes in the input buffer before filling the code buffer.  So we start
** with the code buffer pointer pointing to the sentinel, and we always
** have it pointing at the sentinel when we can't (for one reason or
** another) be getting our codes from the code buffer.  We check for this
** condition whenever we get a CLEAR code, and if so, we get the code
** via the good old rd_dcode() routine.
**
** One other problem with the code buffer approach is that we might get
** a CLEAR code in the middle of the buffer.  This means that the next
** code is only 9 bits, but we have probably already unpacked a number of
** larger codes from the input into the buffer before we discover this.
** So we remember where (in the input buffer) the code buffer was filled
** from, and when a CLEAR code is encountered in the buffer (not the
** sentinel at the end) we back up the bit_offset pointer in the input
** buffer, and reset things to start unpacking the 9-bit codes from there.
*/

#define CODEBUF_SIZE 64      /* must be multiple of 8, experiment for best */
static unsigned codebuf[CODEBUF_SIZE+1];     /* code buffer */
static unsigned *codebufp;       /* code buffer pointer */
static unsigned *codebuflim;     /* code buffer limit */
      /* bit offset within the input buffer of where the code buffer began */
static unsigned codebufoffset;

static unsigned masks[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff };
static unsigned bit_offset;   /* note this only allows max 8K input buffer!!*/

#ifdef UNBUF_IO
#define		BLOCKFILE		int
#define		BLOCKREAD		read
#define		BLOCKWRITE		blockwrite
int read PARMS ((int, VOIDPTR, unsigned));
int write PARMS ((int, VOIDPTR, unsigned));
int blockwrite PARMS ((int, VOIDPTR, unsigned));
#else
#define		BLOCKFILE		ZOOFILE
#define		BLOCKREAD		zooread
#define		BLOCKWRITE		zoowrite
#endif /* UNBUF_IO */

static BLOCKFILE in_f, out_f;

/* rd_dcode() reads a code from the input (compressed) file and returns
its value. */
unsigned rd_dcode()
{
   register char *ptra, *ptrb;    /* miscellaneous pointers */
   unsigned word;                     /* first 16 bits in buffer */
   unsigned byte_offset;
   char nextch;                           /* next 8 bits in buffer */
   unsigned ofs_inbyte;               /* offset within byte */

   ofs_inbyte = bit_offset % 8;
   byte_offset = bit_offset / 8;
   bit_offset = bit_offset + nbits;

   assert(nbits >= 9 && nbits <= 13);

   if (byte_offset >= INBUFSIZ - 5) {
      int space_left;

      assert(byte_offset >= INBUFSIZ - 5);
      debug((printf ("lzd: byte_offset near end of buffer\n")))

      bit_offset = ofs_inbyte + nbits;
      space_left = INBUFSIZ - byte_offset;
      ptrb = byte_offset + in_buf_adr;          /* point to char */
      ptra = in_buf_adr;
      /* we now move the remaining characters down buffer beginning */
      debug((printf ("rd_dcode: space_left = %d\n", space_left)))
      while (space_left > 0) {
         *ptra++ = *ptrb++;
         space_left--;
      }
      assert(ptra - in_buf_adr == ptrb - (in_buf_adr + byte_offset));
      assert(space_left == 0);
      if (BLOCKREAD (in_f, ptra, byte_offset) == -1)
         prterror ('f', "I/O error in lzd:rd_dcode.\n");
      byte_offset = 0;
   }
   ptra = byte_offset + in_buf_adr;
 /* NOTE:  "word = *((int *) ptra)" would not be independent of byte order. */
   word = (unsigned char) *ptra; ptra++;
   word = word | ( ((unsigned char) *ptra) << 8 ); ptra++;

   nextch = *ptra;
   if (ofs_inbyte != 0) {
      /* shift nextch right by ofs_inbyte bits */
      /* and shift those bits right into word; */
      word = (word >> ofs_inbyte) | (((unsigned)nextch) << (16-ofs_inbyte));
   }
   return (word & masks[nbits]); 
} /* rd_dcode() */

void init_dtab()
{
   nbits = 9;
   max_code = 512;
   free_code = FIRST_FREE;
}

/* By making wr_dchar() a macro and calling this routine only on buffer
** full condition, we save a lot of function call overhead.
** We also use pointers instead of counters for efficiency (in the macro).
*/
void xwr_dchar (ch)
char ch;
{
   if (outbufp >= outbuflim) {      /* if buffer full */
      if (BLOCKWRITE (out_f, out_buf_adr, outbufp - out_buf_adr)
                                                != outbufp - out_buf_adr)
         prterror ('f', "Write error in lzd:wr_dchar.\n");
      addbfcrc(out_buf_adr, outbufp - out_buf_adr);     /* update CRC */
      outbufp = out_buf_adr;                  /* restore empty buffer */
   }
   assert(outbufp - out_buf_adr < OUTBUFSIZ);
   *outbufp++ = ch;
} /* wr_dchar() */


/* Code buffer fill routines
**
** We use a separate function for each code size.
** Each function unpacks 8 codes from a packed buffer (f)
** to an unpacked buffer (t)
** A lot of code space, but really speeds up bit picking.
*/
static unsigned char f[13];   /* must be unsigned for right shifts */
static unsigned t[8];

static void cb9fill ()
{
   t[0] = (f[0]     ) | ((f[1] &   1) << 8);
   t[1] = (f[1] >> 1) | ((f[2] &   3) << 7);
   t[2] = (f[2] >> 2) | ((f[3] &   7) << 6);
   t[3] = (f[3] >> 3) | ((f[4] &  15) << 5);
   t[4] = (f[4] >> 4) | ((f[5] &  31) << 4);
   t[5] = (f[5] >> 5) | ((f[6] &  63) << 3);
   t[6] = (f[6] >> 6) | ((f[7] & 127) << 2);
   t[7] = (f[7] >> 7) | ((f[8]      ) << 1);
}

static void cb10fill ()
{
   t[0] = (f[0]     ) | ((f[1] &   3) << 8);
   t[1] = (f[1] >> 2) | ((f[2] &  15) << 6);
   t[2] = (f[2] >> 4) | ((f[3] &  63) << 4);
   t[3] = (f[3] >> 6) | ((f[4]      ) << 2);
   t[4] = (f[5]     ) | ((f[6] &   3) << 8);
   t[5] = (f[6] >> 2) | ((f[7] &  15) << 6);
   t[6] = (f[7] >> 4) | ((f[8] &  63) << 4);
   t[7] = (f[8] >> 6) | ((f[9]      ) << 2);
}

static void cb11fill ()
{
   t[0] = (f[0]     ) | ((f[1] &   7) << 8);
   t[1] = (f[1] >> 3) | ((f[2] &  63) << 5);
   t[2] = (f[2] >> 6) | (f[3] << 2) | ((f[4] &  1) << 10);
   t[3] = (f[4] >> 1) | ((f[5] &  15) << 7);
   t[4] = (f[5] >> 4) | ((f[6] & 127) << 4);
   t[5] = (f[6] >> 7) | (f[7] << 1) | ((f[8] &  3) <<  9);
   t[6] = (f[8] >> 2) | ((f[9] &  31) << 6);
   t[7] = (f[9] >> 5) | ((f[10]     ) << 3);
}

static void cb12fill ()
{
   t[0] = (f[0]     )  | ((f[1] & 15) << 8);
   t[1] = (f[1] >> 4)  | ((f[2]     ) << 4);
   t[2] = (f[3]     )  | ((f[4] & 15) << 8);
   t[3] = (f[4] >> 4)  | ((f[5]     ) << 4);
   t[4] = (f[6]     )  | ((f[7] & 15) << 8);
   t[5] = (f[7] >> 4)  | ((f[8]     ) << 4);
   t[6] = (f[9]     )  | ((f[10] & 15) << 8);
   t[7] = (f[10] >> 4) | ((f[11]     ) << 4);
}

static void cb13fill ()
{
   t[0] = (f[0] ) | ((f[1] & 31) << 8);
   t[1] = (f[1] >> 5) | (f[2] << 3) | ((f[3] & 3) << 11);
   t[2] = (f[3] >> 2) | ((f[4] & 127) << 6);
   t[3] = (f[4] >> 7) | (f[5] << 1) | ((f[6] & 15) << 9);
   t[4] = (f[6] >> 4) | (f[7] << 4) | ((f[8] & 1) << 12);
   t[5] = (f[8] >> 1) | ((f[9] & 63) << 7);
   t[6] = (f[9] >> 6) | (f[10] << 2) | ((f[11] & 7) << 10);
   t[7] = (f[11] >> 3) | (f[12] << 5);
}

/* vector of code buffer fill routines
*/
void (*cbfillvec[])  PARMS ((void)) = { 0, 0, 0, 0, 0, 0, 0, 0, 0,
         cb9fill, cb10fill, cb11fill, cb12fill, cb13fill };

/* cbfill -- main code buffer fill routine
**
** moves data from inbuf[] to f[]
** then calls via vector to unpack to t[]
** then moves from t[] to codebuf[]
** A lot of moving around, but still faster than a lot of shifting and
** masking via variables (at least on a micro -- don't know about VAXen)
**  Uses memcpy() for block move
*/

static void cbfill ()
{
   char *inbp;
   inbp = in_buf_adr + bit_offset / 8;
   codebufp = codebuf;
   while ( codebufp < codebuflim ) {
     memcpy((VOIDPTR) f, inbp, nbits);
      (*cbfillvec[nbits])();
      memcpy((VOIDPTR) codebufp, (VOIDPTR) t, 8 * sizeof(unsigned int));
      inbp += nbits;
      codebufp += 8;
   }
   bit_offset += nbits * CODEBUF_SIZE;
}

/* The following is used in the KwKwK case because it's a pretty rare
** case, and doing it this way avoids the overhead of remembering the
** "finchar" (first input character) of every string
*/
static int firstchar(code)    /* find first character of a code */
int code;
{
   while ( code > 255 )
      code = head[code];
   return code;
}

int lzd(input_f, output_f)
BLOCKFILE input_f, output_f;          /* input & output files */
{
   in_f = input_f;                 /* make it avail to other fns */
   out_f = output_f;               /* ditto */
   nbits = 9;
   max_code = 512;
   free_code = FIRST_FREE;
   bit_offset = 0;
   outbuflim = out_buf_adr + OUTBUFSIZ;   /* setup out buffer limit */
   outbufguard = outbuflim - 12;     /* for checking avail. room in outbuf */
      /* note must allow for as many characters as we special-case (8) */
      /* used 12 for extra fudge factor (Rahul does it, so I can too) */
   outbufp = out_buf_adr;                 /* setup output buffer ptr */ 
   codebufp = codebuflim = &codebuf[CODEBUF_SIZE]; /* code buf ptr & limit */
   *codebuflim = CLEAR; /* phony CLEAR sentinel past end of code buffer */

   if (BLOCKREAD (in_f, in_buf_adr, INBUFSIZ) == -1) /* fill input buffer */
      return(IOERR);
   if (memflag == 0) {
     head = (int *) ealloc((MAXMAX+10) * sizeof(int));
     tail = (char *) ealloc((MAXMAX+10) * sizeof(char));
     stack = (char *) ealloc (sizeof (unsigned) * STACKSIZE + 20);
     memflag++;
   }

   stack_pointer = stack_lim = stack + STACKSIZE; /* setup stack ptr, limit*/
   init_dtab();             /* initialize table */

loop:
   cur_code = *codebufp++; /* get code from code buffer */

goteof: /* special case for CLEAR then Z_EOF, for 0-length files */
   if (cur_code == Z_EOF) {
      debug((printf ("lzd: Z_EOF\n")))

      if (outbufp != out_buf_adr) {
      	if (BLOCKWRITE (out_f, out_buf_adr, outbufp - out_buf_adr)
                                                  != outbufp - out_buf_adr)
         	prterror ('f', "Output error in lzd().\n");
			addbfcrc(out_buf_adr, outbufp - out_buf_adr);

      }
#ifdef FILTER
		/* get next two bytes and put them where zoofilt can find them */
		/* nbits known to be in range 9..13 */
		bit_offset = ((bit_offset + 7) / 8) * 8; /* round up to next byte */
		filt_lzd_word = rd_dcode();
		filt_lzd_word |= (rd_dcode() << nbits);
		filt_lzd_word &= 0xffff;
#endif
      return (0);
   }

   assert(nbits >= 9 && nbits <= 13);

   if (cur_code == CLEAR) {          /* was it sentinel or real CLEAR ? */
      if ( codebufp > codebuflim ) { /* it was the sentinel             */
         if ( bit_offset % 8 == 0 && /* if we're on byte boundary and   */
                   /* codesize won't change before codebuf is filled and */
                   /* codebuf can be filled without running out of inbuf */
                free_code + CODEBUF_SIZE < max_code &&
                bit_offset / 8 + (CODEBUF_SIZE * 13 / 8) < INBUFSIZ - 10 ) {
            codebufoffset = bit_offset; /* remember where we were when */
            cbfill();             /* we filled the code buffer */
            codebufp = codebuf;   /* setup code buffer pointer */
            goto loop;            /* now go get codes from code buffer */
         }                 /* otherwise, use rd_dcode to get code */
         codebufp = codebuflim;   /* reset codebuf ptr to sentinel */
         cur_code = rd_dcode();   /* get code via rd_dcode() */
         if ( cur_code != CLEAR ) /* if it's not CLEAR */
            goto got_code;        /* then go handle it */
      } else {          /* else it's really a CLEAR code, not sentinel */
 /* reset bit_offset to get next code in input buf after CLEAR code */
         bit_offset = codebufoffset + (codebufp - codebuf) * nbits;
      } 
      codebufp = codebuflim;      /* set code buf ptr to sentinel */
      debug((printf ("lzd: CLEAR\n")))
      init_dtab();                /* init decompression table, etc. */
      old_code = cur_code = rd_dcode(); /* get next code after CLEAR */
		if (cur_code == Z_EOF)		/* special case for 0-length files */
			goto goteof;
      wr_dchar(cur_code);         /* write it out */
      goto loop;                  /* and get next code */
   }

got_code: /* we got a code and it's not a CLEAR */

   if (cur_code == Z_EOF) {
      debug((printf ("lzd: Z_EOF\n")))
      if (outbufp != out_buf_adr) {
      	if (BLOCKWRITE (out_f, out_buf_adr, outbufp - out_buf_adr)
                                                  != outbufp - out_buf_adr)
         	prterror ('f', "Output error in lzd().\n");
         addbfcrc(out_buf_adr, outbufp - out_buf_adr);
      }
      return (0);
   }

   in_code = cur_code;              /* save original code */
   if (cur_code >= free_code) {        /* if code not in table (k<w>k<w>k) */
      cur_code = old_code;             /* previous code becomes current */
                                       /* push first character of old code */
      *--stack_pointer = firstchar(old_code);
      goto unwind;                     /* and go "unwind" the current code */
   }              /* (use general unwind because the stack isn't empty now) */

/* Unwind a code.  The basic idea is to use a sort of loop-unrolling
** approach to really speed up the processing by treating the codes
** which represent short strings (the vast majority of codes) as
** special cases.  Avoid a lot of stack overflow checking safely.
*/

   if (cur_code > 255) {                  /* if cur_code is not atomic */
      *--stack_pointer = tail[cur_code];  /* push its tail code */
      cur_code = head[cur_code];          /* and replace with its head code */
   } else {                        /* else 1-byte string */
      if ( outbufp > outbufguard ) /* if outbuf near end, */
         goto write_stack;         /* write via general routine */
      *outbufp++ = cur_code;       /* we got space, put char out */
      goto add_code;               /* add code to table */
   }

   if (cur_code > 255) {                  /* if cur_code is not atomic */
      *--stack_pointer = tail[cur_code];  /* push its tail code */
      cur_code = head[cur_code];          /* and replace with its head code */
   } else {                        /* else 2-byte string */
      if ( outbufp > outbufguard ) /* if outbuf near end, */
         goto write_stack;         /* write via general routine */
      *outbufp++ = cur_code;       /* we got space, put char out, and */
      goto move_1_char;            /* go move rest of stack to outbuf */
   }
   if (cur_code > 255) {                  /* if cur_code is not atomic */
      *--stack_pointer = tail[cur_code];  /* push its tail code */
      cur_code = head[cur_code];          /* and replace with its head code */
   } else {                        /* else 3-byte string */
      if ( outbufp > outbufguard ) /* if outbuf near end, */
         goto write_stack;         /* write via general routine */
      *outbufp++ = cur_code;       /* we got space, put char out, and */
      goto move_2_char;            /* go move rest of stack to outbuf */
   }

/* we handle codes representing strings of 4 thru 8 bytes similarly */

   if (cur_code > 255) {
      *--stack_pointer = tail[cur_code];
      cur_code = head[cur_code];
   } else {                        /* 4-byte string */
      if ( outbufp > outbufguard )
         goto write_stack;
      *outbufp++ = cur_code;
      goto move_3_char;
   }
   if (cur_code > 255) {
      *--stack_pointer = tail[cur_code];
      cur_code = head[cur_code];
   } else {                        /* 5-byte string */
      if ( outbufp > outbufguard )
         goto write_stack;
      *outbufp++ = cur_code;
      goto move_4_char;
   }
   if (cur_code > 255) {
      *--stack_pointer = tail[cur_code];
      cur_code = head[cur_code];
   } else {                        /* 6-byte string */
      if ( outbufp > outbufguard )
         goto write_stack;
      *outbufp++ = cur_code;
      goto move_5_char;
   }
   if (cur_code > 255) {
      *--stack_pointer = tail[cur_code];
      cur_code = head[cur_code];
   } else {                        /* 7-byte string */
      if ( outbufp > outbufguard )
         goto write_stack;
      *outbufp++ = cur_code;
      goto move_6_char;
   }
   if (cur_code > 255) {
      *--stack_pointer = tail[cur_code];
      cur_code = head[cur_code];
   } else {                        /* 8-byte string */
      if ( outbufp > outbufguard )
         goto write_stack;
      *outbufp++ = cur_code;
      goto move_7_char;
   }

/* Here for KwKwK case and strings longer than 8 bytes */
/* Note we have to check stack here, but not elsewhere */

unwind:
   while (cur_code > 255) {               /* if code, not character */
      *--stack_pointer = tail[cur_code];         /* push suffix char */
      if (stack_pointer < stack+12)
         prterror ('f', "Stack overflow in lzd().\n");
      cur_code = head[cur_code];          /* head of code is new code */
   }

/* General routine to write stack with check for output buffer full */

write_stack:
   assert(nbits >= 9 && nbits <= 13);
   wr_dchar(cur_code);    /* write this code, don't need to stack it first */
   while ( stack_pointer < stack_lim ) {
      wr_dchar(*stack_pointer++);
   }
   goto add_code;                           /* now go add code to table */

/* Here to move strings from stack to output buffer */
/* only if we know we have enough room in output buffer */
/* because (outbufp <= outbufguard) */

move_7_char:
   *outbufp++ = *stack_pointer++;
move_6_char:
   *outbufp++ = *stack_pointer++;
move_5_char:
   *outbufp++ = *stack_pointer++;
move_4_char:
   *outbufp++ = *stack_pointer++;
move_3_char:
   *outbufp++ = *stack_pointer++;
move_2_char:
   *outbufp++ = *stack_pointer++;
move_1_char:
   *outbufp++ = *stack_pointer++;

assert(stack_pointer == stack_lim); /* I haven't tested this! rdg */

/* add_code is now inline to avoid overhead of function call on */
/*   each code processed */

add_code:
   assert(nbits >= 9 && nbits <= 13);
   assert(free_code <= MAXMAX+1);
   tail[free_code] = cur_code;                /* save suffix char */
   head[free_code] = old_code;                /* save prefix code */
   free_code++;
   assert(nbits >= 9 && nbits <= 13);
   if (free_code >= max_code) {
      if (nbits < MAXBITS) {
         debug((printf("lzd: nbits was %d\n", nbits)))
         nbits++;
         assert(nbits >= 9 && nbits <= 13);
         debug((printf("lzd: nbits now %d\n", nbits)))
         max_code = max_code << 1;        /* double max_code */
         debug((printf("lzd: max_code now %d\n", max_code)))
      }
   }
   old_code = in_code;

   assert(nbits >= 9 && nbits <= 13);

   goto loop;
} /* lzd() */

#else /* SLOW_LZD defined, so use following instead */

/*********************************************************************/
/* Original slower lzd().                                            */
/*********************************************************************/

/*
Lempel-Ziv decompression.  Mostly based on Tom Pfau's assembly language
code.  The contents of this file are hereby released to the public domain.
                                 -- Rahul Dhesi 1986/11/14
*/

#define  STACKSIZE   4000

struct tabentry {
   unsigned next;
   char z_ch;
};

void init_dtab PARMS((void));
unsigned rd_dcode PARMS((void));
void wr_dchar PARMS((int));
void ad_dcode PARMS((void));

#ifdef FILTER
/* to send data back to zoofilt */
extern unsigned int filt_lzd_word;
#endif /* FILTER */


static unsigned stack_pointer = 0;
static unsigned *stack;

#define  push(x)  {  \
                     stack[stack_pointer++] = (x);                   \
                     if (stack_pointer >= STACKSIZE)                 \
                        prterror ('f', "Stack overflow in lzd().\n");\
                  }
#define  pop()    (stack[--stack_pointer])

extern char *out_buf_adr;        /* output buffer */
extern char *in_buf_adr;         /* input buffer */

char memflag = 0;                /* memory allocated? flag */
extern struct tabentry *table;   /* hash table from lzc.c */
static unsigned cur_code;
static unsigned old_code;
static unsigned in_code;

static unsigned free_code;
static int nbits;
static unsigned max_code;

static char fin_char;
static char k;
static unsigned masks[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff };
static unsigned bit_offset;
static unsigned output_offset;

#ifdef UNBUF_IO
#define		BLOCKFILE		int
#define		BLOCKREAD		read
#define		BLOCKWRITE		blockwrite
int read PARMS ((int, VOIDPTR, unsigned));
int write PARMS ((int, VOIDPTR, unsigned));
#else
#define		BLOCKFILE		ZOOFILE
#define		BLOCKREAD		zooread
#define		BLOCKWRITE		zoowrite
#endif /* UNBUF_IO */

static BLOCKFILE in_f, out_f; 

int lzd(input_f, output_f)
BLOCKFILE input_f, output_f;          /* input & output file handles */
{
   in_f = input_f;                 /* make it avail to other fns */
   out_f = output_f;               /* ditto */
   nbits = 9;
   max_code = 512;
   free_code = FIRST_FREE;
   stack_pointer = 0;
   bit_offset = 0;
   output_offset = 0;

   if (BLOCKREAD (in_f, in_buf_adr, INBUFSIZ) == -1)
      return(IOERR);
   if (memflag == 0) {
     table = (struct tabentry *) ealloc((MAXMAX+10) * sizeof(struct tabentry));
     stack = (unsigned *) ealloc (sizeof (unsigned) * STACKSIZE + 20);
     memflag++;
   }

   init_dtab();             /* initialize table */

loop:
   cur_code = rd_dcode();
goteof: /* special case for CLEAR then Z_EOF, for 0-length files */
   if (cur_code == Z_EOF) {
      debug((printf ("lzd: Z_EOF\n")))
      if (output_offset != 0) {
         if (BLOCKWRITE (out_f, out_buf_adr, output_offset) != output_offset)
            prterror ('f', "Output error in lzd().\n");
         addbfcrc(out_buf_adr, output_offset);
      }
#ifdef FILTER
		/* get next two bytes and put them where zoofilt can find them */
		/* nbits known to be in range 9..13 */
		bit_offset = ((bit_offset + 7) / 8) * 8; /* round up to next byte */
		filt_lzd_word = rd_dcode();
		filt_lzd_word |= (rd_dcode() << nbits);
		filt_lzd_word &= 0xffff;
#endif
      return (0);
   }

   assert(nbits >= 9 && nbits <= 13);

   if (cur_code == CLEAR) {
      debug((printf ("lzd: CLEAR\n")))
      init_dtab();
      fin_char = k = old_code = cur_code = rd_dcode();
		if (cur_code == Z_EOF)		/* special case for 0-length files */
			goto goteof;
      wr_dchar(k);
      goto loop;
   }

   in_code = cur_code;
   if (cur_code >= free_code) {        /* if code not in table (k<w>k<w>k) */
      cur_code = old_code;             /* previous code becomes current */
      push(fin_char);
   }

   while (cur_code > 255) {               /* if code, not character */
      push(table[cur_code].z_ch);         /* push suffix char */
      cur_code = table[cur_code].next;    /* <w> := <w>.code */
   }

   assert(nbits >= 9 && nbits <= 13);

   k = fin_char = cur_code;
   push(k);
   while (stack_pointer != 0) {
      wr_dchar(pop());
   }
   assert(nbits >= 9 && nbits <= 13);
   ad_dcode();
   old_code = in_code;

   assert(nbits >= 9 && nbits <= 13);

   goto loop;
} /* lzd() */

/* rd_dcode() reads a code from the input (compressed) file and returns
its value. */
unsigned rd_dcode()
{
   register char *ptra, *ptrb;    /* miscellaneous pointers */
   unsigned word;                     /* first 16 bits in buffer */
   unsigned byte_offset;
   char nextch;                           /* next 8 bits in buffer */
   unsigned ofs_inbyte;               /* offset within byte */

   ofs_inbyte = bit_offset % 8;
   byte_offset = bit_offset / 8;
   bit_offset = bit_offset + nbits;

   assert(nbits >= 9 && nbits <= 13);

   if (byte_offset >= INBUFSIZ - 5) {
      int space_left;

#ifdef CHECK_BREAK
	check_break();
#endif

      assert(byte_offset >= INBUFSIZ - 5);
      debug((printf ("lzd: byte_offset near end of buffer\n")))

      bit_offset = ofs_inbyte + nbits;
      space_left = INBUFSIZ - byte_offset;
      ptrb = byte_offset + in_buf_adr;          /* point to char */
      ptra = in_buf_adr;
      /* we now move the remaining characters down buffer beginning */
      debug((printf ("rd_dcode: space_left = %d\n", space_left)))
      while (space_left > 0) {
         *ptra++ = *ptrb++;
         space_left--;
      }
      assert(ptra - in_buf_adr == ptrb - (in_buf_adr + byte_offset));
      assert(space_left == 0);
      if (BLOCKREAD (in_f, ptra, byte_offset) == -1)
         prterror ('f', "I/O error in lzd:rd_dcode.\n");
      byte_offset = 0;
   }
   ptra = byte_offset + in_buf_adr;
   /* NOTE:  "word = *((int *) ptra)" would not be independent of byte order. */
   word = (unsigned char) *ptra; ptra++;
   word = word | ( ((unsigned char) *ptra) << 8 ); ptra++;

   nextch = *ptra;
   if (ofs_inbyte != 0) {
      /* shift nextch right by ofs_inbyte bits */
      /* and shift those bits right into word; */
      word = (word >> ofs_inbyte) | (((unsigned)nextch) << (16-ofs_inbyte));
   }
   return (word & masks[nbits]); 
} /* rd_dcode() */

void init_dtab()
{
   nbits = 9;
   max_code = 512;
   free_code = FIRST_FREE;
}

void wr_dchar (ch)
int ch;
{
   if (output_offset >= OUTBUFSIZ) {      /* if buffer full */
#ifdef CHECK_BREAK
	check_break();
#endif
      if (BLOCKWRITE (out_f, out_buf_adr, output_offset) != output_offset)
         prterror ('f', "Write error in lzd:wr_dchar.\n");
      addbfcrc(out_buf_adr, output_offset);     /* update CRC */
      output_offset = 0;                  /* restore empty buffer */
   }
   assert(output_offset < OUTBUFSIZ);
   out_buf_adr[output_offset++] = ch;        /* store character */
} /* wr_dchar() */

/* adds a code to table */
void ad_dcode()
{
   assert(nbits >= 9 && nbits <= 13);
   assert(free_code <= MAXMAX+1);
   table[free_code].z_ch = k;                /* save suffix char */
   table[free_code].next = old_code;         /* save prefix code */
   free_code++;
   assert(nbits >= 9 && nbits <= 13);
   if (free_code >= max_code) {
      if (nbits < MAXBITS) {
         debug((printf("lzd: nbits was %d\n", nbits)))
         nbits++;
         assert(nbits >= 9 && nbits <= 13);
         debug((printf("lzd: nbits now %d\n", nbits)))
         max_code = max_code << 1;        /* double max_code */
         debug((printf("lzd: max_code now %d\n", max_code)))
      }
   }
}
#endif /* ! SLOW_LZD */
