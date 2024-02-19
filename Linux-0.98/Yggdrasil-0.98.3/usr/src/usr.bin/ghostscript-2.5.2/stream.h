/* Copyright (C) 1989, 1992 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */

/* stream.h */
/* Definitions for Ghostscript stream package */
/* Requires stdio.h */

/*
 * Note that the stream package works with bytes, not chars.
 * This is to ensure unsigned representation on all systems.
 * A stream currently can only be read or written, not both.
 * Note also that the read procedure returns an int,
 * not a char or a byte, so we can use negative values for EOFC and ERRC.
 * We distinguish "data" from "signal" results (EOFC, ERRC) with a macro:
 */

#define char_is_data(c) ((c) >= 0)
#define char_is_signal(c) ((c) < 0)
typedef struct stream_s stream;

/*
 * We cast EOFC and ERRC to int explicitly, because some compilers
 * don't do this if the other arm of a conditional is a byte.
 * Clients should use char_is_data and char_is_signal (see above)
 * to test for exceptional results.
 */
#define EOFC ((int)(-1))
#define ERRC ((int)(-2))
/****** ERRC IS NOT RECOGNIZED EVERYWHERE YET ******/

/* ------ Define the "virtual" stream procedures ------ */

typedef struct {

		/* Store # available for reading. */
		/* Return 0 if OK, ERRC if error or not implemented. */
	int (*available)(P2(stream *, long *));

		/* Set position. */
		/* Return 0 if OK, ERRC if error or not implemented. */
	int (*seek)(P2(stream *, long));

		/* Flush buffered data. */
		/* Return 0 if OK, ERRC if error. */
	int (*flush)(P1(stream *));

		/* Flush data (if writing) & close stream. */
		/* Return 0 if OK, ERRC if error. */
	int (*close)(P1(stream *));

		/* Refill buffer (setting endptr) and reset cptr. */
		/* Return ERRC if not implemented; */
		/* otherwise, set end_status appropriately and return 0. */
	int (*read_buf)(P1(stream *));

		/* Write buffer and reset cptr. */
		/* Return 0 if OK, ERRC if error or not implemented. */
	int (*write_buf)(P1(stream *));

} stream_procs;

/* ------ Structs for specialized streams -- see below. ------ */

typedef struct CCITTFax_state_s {
	/* The following are set before initialization. */
	int Uncompressed;	/* boolean */
	int K;
	int EndOfLine;		/* boolean */
	int EncodedByteAlign;	/* boolean */
	int Columns;
	int Rows;
	int EndOfBlock;		/* boolean */
	int BlackIs1;		/* boolean */
	int DamagedRowsBeforeError;
	uint raster;
	/* The following are updated dynamically. */
	int k_left;		/* number of next rows to encode in 2-D */
				/* (only if K > 0) */
	int cbit;		/* bits left to fill in current decoded */
				/* byte at *cptr (0..7, input only) */
	uint bits;		/* most recent bits of input or */
				/* current bits of output */
	int bits_left;		/* # of valid low bits (input) or */
				/* unused low bits (output) in above */
	uint prev_pos;		/* position of previous line in buffer */
	/* The following are input-only and not used yet. */
	int uncomp_run;		/* non-0 iff we are in an uncompressed */
				/* run straddling a scan line (-1 if white, */
				/* 1 if black) */
	int uncomp_left;	/* # of bits left in the run */
	int uncomp_exit;	/* non-0 iff this is an exit run */
				/* (-1 if next run white, 1 if black) */
} CCITTFax_state;

struct lzw_decode_table_s;
struct lzw_encode_table_s;
typedef struct LZW_state_s {
	/* The following are set at initialization. */
	int enhanced;			/* if true, use Aladdin's */
					/* enhanced compression algorithm */
	/* The following are updated dynamically. */
	struct lzw_decode_table_s *decode_table;	/* decoding table */
	struct lzw_encode_table_s *encode_table;	/* encoding table */
	uint next_code;			/* next code to be assigned */
	int code_size;			/* current # of bits per code */
	int prev_code;			/* previous code recognized */
					/* or assigned */
	byte bits;		/* most recent byte of input or */
				/* current byte of output */
	int bits_left;		/* # of unused low bits in above, [0..7] */
} LZW_state;

typedef struct SubFile_state_s {
	ulong count;		/* # of EODs to scan over */
	const byte *eod_string;
	uint string_size;
	uint match;		/* # of matched chars preceding end of buffer */
} SubFile_state;

/* ------ The actual stream structure ------ */

struct stream_s {
	byte *cptr;			/* pointer to last byte */
					/* read or written */
	byte *endptr;			/* pointer to last byte */
					/* containing data for reading, */
					/* or to be filled for writing */
	byte *cbuf;			/* base of buffer */
	uint bsize;			/* size of buffer, 0 if closed */
	uint cbsize;			/* size of buffer */
	uint modes;			/* (not byte, to keep alignment) */
#define s_mode_read 1
#define s_mode_write 2
#define s_mode_seek 4
#define s_is_valid(s) ((s)->modes != 0)
#define s_is_reading(s) (((s)->modes & s_mode_read) != 0)
#define s_is_writing(s) (((s)->modes & s_mode_write) != 0)
#define s_can_seek(s) (((s)->modes & s_mode_seek) != 0)
	int end_status;			/* EOFC if at EOF when buffer */
					/* becomes empty, ERRC if error */
	long position;			/* file position of beginning of */
					/* buffer */
	stream_procs procs;
	int num_format;			/* format for Level 2 */
					/* encoded number reader */
					/* (only used locally) */
	stream *strm;			/* the underlying stream, non-zero */
					/* iff this is a filter stream */
	int strm_is_temp;		/* if true, strm is a temporary */
					/* stream and should be freed */
					/* when this stream is closed */
	ushort read_id;			/* "unique" serial # for detecting */
					/* references to closed streams */
					/* and for validating read access */
	ushort write_id;		/* ditto to validate write access */
	int char_temp;			/* temporary for inline access */
					/* (see spgetc_inline below) */
	/*
	 * If were were able to program in a real object-oriented style, 
	 * the remaining data would be per-subclass.  It's just too much
	 * of a nuisance to do this in C, so we allocate space for the
	 * private data of ALL subclasses.
	 */
	/* The following are for file streams. */
	FILE *file;			/* file handle for C library */
	int can_close;			/* 0 for stdin/out/err, */
					/* -1 for line/statementedit, */
					/* 1 for other files */
	stream *prev, *next;		/* keep track of all files */
	/*
	 * The remaining members are only for filters.
	 * Again, we simply allocate space for all of them,
	 * rather than use a union.
	 */
	/* The following is used by several decoding filters. */
	int odd;			/* odd digit */
	/* The following is for RunLengthEncode. */
	ulong record_size;
	/* The following is for RunLengthEncode and PFBDecode. */
	ulong record_left;		/* bytes left in current record */
	/* The following are for PFBDecode. */
	int record_type;
	int binary_to_hex;
	/* The following are for eexecDecode. */
	ushort cstate;			/* encryption state */
	ushort _skip;			/* (keep int alignment) */
	int binary;			/* true=binary, false=hex */
	/* The following are for various filters. */
	CCITTFax_state cfs;
	LZW_state lzws;
	SubFile_state sfs;
};

/* ------ Stream functions ------ */

/* Some of these are macros -- beware. */
/* Note that unlike the C stream library, */
/* ALL stream procedures take the stream as the first argument. */
#define sendbufp(s) ((s)->cptr >= (s)->endptr)	/* not for clients */

/* Following are valid for all streams. */
/* flush is NOT a no-op for read streams -- it discards data until EOF. */
/* close is NOT a no-op for non-file streams -- */
/* it actively disables them. */
/* The close routine must do a flush if needed. */
#define sseekable(s) s_can_seek(s)
#define serrorp(s) ((s)->cptr >= (s)->endptr && (s)->end_status == ERRC)
#define savailable(s,pl) (*(s)->procs.available)(s,pl)
#define sflush(s) (*(s)->procs.flush)(s)
#define sclose(s) (*(s)->procs.close)(s)

/* Following are only valid for read streams. */
extern int spgetc(P1(stream *));
/* The first alternative should read */
/*	(int)(*++((s)->cptr))	*/
/* but the Borland compiler generates truly atrocious code for this. */
#define sgetc(s) (!sendbufp(s) ? (++((s)->cptr), (int)*(s)->cptr) : spgetc(s))
extern uint sgets(P3(stream *, byte *, uint));
extern int sreadhex(P6(stream *, byte *, uint, uint *, int *, int));
extern int sungetc(P2(stream *, byte));	/* ERRC on error, 0 if OK */
#define sputback(s) ((s)->cptr--)	/* can only do this once! */
#define seofp(s) (sendbufp(s) && (s)->end_status == EOFC)
extern int spskip(P2(stream *, long));
#define sskip(s,n) spskip(s,(long)(n))

/* Following are only valid for write streams. */
extern int spputc(P2(stream *, byte));
/* The first alternative should read */
/*	((int)(*++((s)->cptr)=(c)))	*/
/* but the Borland compiler generates truly atrocious code for this. */
#define sputc(s,c)\
  (!sendbufp(s) ? (++((s)->cptr), (int)(*(s)->cptr=(c))) : spputc((s),(c)))
extern uint sputs(P3(stream *, const byte *, uint));

/* Following are only valid for positionable streams. */
#define stell(s) ((s)->cptr + 1 - (s)->cbuf + (s)->position)
#define sseek(s,pos) (*(s)->procs.seek)(s,(long)(pos))

/* Following are for high-performance clients. */
/* bufptr points to the next item, bufend points beyond the last item. */
#define sbufptr(s) ((s)->cptr + 1)
#define sbufend(s) ((s)->endptr + 1)
#define ssetbufptr(s,ptr) ((s)->cptr = (ptr) - 1)
#define sbufskip(s,n) ((s)->cptr += (n))
#define sbufavailable(s) ((s)->endptr - (s)->cptr)

/* The following are for very high-performance clients of read streams, */
/* who unpack the stream state into local variables. */
/* Note that any non-inline operations must do a s_end_inline before, */
/* and a s_begin_inline after. */
#define s_declare_inline(s, cp, ep)\
  register byte *cp;\
  byte *ep
#define s_begin_inline(s, cp, ep)\
  cp = (s)->cptr, ep = (s)->endptr
#define s_end_inline(s, cp, ep)\
  (s)->cptr = cp
#define sendbufp_inline(s, cp, ep)\
  (cp >= ep)
#define sgetc_inline(s, cp, ep)\
  (sendbufp_inline(s, cp, ep) ? spgetc_inline(s, cp, ep) : *++cp)
#define spgetc_inline(s, cp, ep)\
  (s_end_inline(s, cp, ep), (s)->char_temp = spgetc(s),\
   s_begin_inline(s, cp, ep), (s)->char_temp)
#define sputback_inline(s, cp, ep)\
  --cp

/* Stream creation procedures */
extern	void	sread_string(P3(stream *, const byte *, uint)),
		swrite_string(P3(stream *, byte *, uint));
extern	void	sread_file(P4(stream *, FILE *, byte *, uint)),
		swrite_file(P4(stream *, FILE *, byte *, uint));

/* Standard stream initialization */
extern	void	s_std_init(P5(stream *, byte *, uint, const stream_procs *, int /*mode*/));

/* Standard stream finalization */
extern	void	s_disable(P1(stream *));

/* Generic stream procedures exported for filters */
extern	int	s_std_null(P1(stream *)),
		s_std_read_flush(P1(stream *)),
		s_std_write_flush(P1(stream *)),
		s_std_noavailable(P2(stream *, long *)),
		s_std_noseek(P2(stream *, long)),
		s_std_close(P1(stream *));
