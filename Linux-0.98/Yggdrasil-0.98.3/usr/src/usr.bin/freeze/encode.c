#include "freeze.h"
#include "lz.h"
#include "huf.h"
#include "bitio.h"

/* for future versions ?... */

#define LENGTH_OFFSET   256
#define EncodeLiteral(l)        EncodeChar(l)
#define EncodeLength(l)         EncodeChar(l + LENGTH_OFFSET)

/*
 * Freezes stdin to stdout
 */

void freeze ()
{
	register us_t i, len, r, s;
	register short c;
	putchar(MAGIC1);
	putchar(MAGIC2_2);

/* Huffman-dependent part */
	write_header();
	StartHuff(N_CHAR2);
	init(Table2);
/* end of Huffman-dependent part */

	InitTree();     /* LZ dependent */
	InitIO();

	s = 0;
	r = N2 - F2;
	for (i = s; i < r; i++)
		text_buf[i] = ' ';
	for (len = 0; len < F2 && (c = getchar()) != EOF; len++)
		text_buf[r + len] = c;

	/* check for magic header */
	if(!topipe && !force && text_buf[r] == MAGIC1 &&
		text_buf[r + 1] >= MAGIC2_1) {
		if (quiet != 1)
			fprintf(stderr, " already frozen ");
		exit_stat = 2;
		return;
	}

	in_count = len;
	for (i = 0; i <= F2; i++)
		InsertNode(r + i - F2);

	while (len != 0) {
		match_length = THRESHOLD;
		Get_Next_Match(r,1);
		if (match_length > len)
			match_length = len;

		if (match_length <= THRESHOLD) {
			match_length = 1;
			EncodeLiteral(text_buf[r]);
#ifdef DEBUG
			symbols_out ++;
			if (verbose)
				fprintf(stderr, "'%s'\n",
					pr_char(text_buf[r]));
#endif /* DEBUG */
		} else if (greedy) {
/* GREEDY parsing (compression rate 1.5% worse, but 40% faster) */

			EncodeLength((us_t) (match_length - THRESHOLD));
			EncodePosition((us_t)match_position);

		} else {
			register us_t orig_length, orig_position, oldchar;

/* This fragment (delayed coding, non-greedy) is due to ideas of
	Jan Mark Wams' <jms@cs.vu.nl> COMIC:
*/
			oldchar = text_buf[r];
			orig_length = match_length;
			orig_position = match_position;

			DeleteNode(s);
			Next_Char(N2, F2);
			Get_Next_Match(r,2);

			if (match_length > len) match_length = len;

			if (orig_length >= match_length) {
				EncodeLength((us_t)
					(orig_length - THRESHOLD));
				EncodePosition((us_t)orig_position);
#ifdef DEBUG
				match_position = orig_position;
#endif  /* DEBUG */
				match_length = orig_length - 1;
			} else {
				EncodeLiteral(oldchar);
#ifdef DEBUG
				symbols_out ++;
				if (verbose)
					fprintf(stderr, "'%s'\n",
						pr_char(oldchar));
#endif  /* DEBUG */
				EncodeLength(match_length - THRESHOLD);
				EncodePosition(match_position);
			}
#ifdef DEBUG
			refers_out ++;
			if (verbose) {
				register short pos =
					(r - 1 - match_position) & (N2 - 1),
				leng = match_length;
				fputc('"', stderr);
				for(; leng; leng--, pos++)
					fprintf(stderr, "%s",
						pr_char(text_buf[pos]));
				fprintf(stderr, "\"\n");
			}
#endif /* DEBUG */
		}

/* Process the rest of the matched sequence (insertion in the list
	only, without any matching !!!)
*/

		for (i = 0; i < match_length &&
				(c = getchar()) != EOF; i++) {
			DeleteNode(s);
			text_buf[s] = c;
			if (s < F2 - 1)
				text_buf[s + N2] = c;
			s = (s + 1) & (N2 - 1);
			r = (r + 1) & (N2 - 1);
			InsertNode(r);
		}

		in_count += i;

		INDICATOR

		while (i++ < match_length) {
			DeleteNode(s);
			s = (s + 1) & (N2 - 1);
			r = (r + 1) & (N2 - 1);
			if (--len) InsertNode(r);
		}
	}

	/* to flush literals */
	EncodeLength((short)ENDOF - LENGTH_OFFSET);
#ifdef DEBUG
	symbols_out ++;
#endif
	EncodeEnd();
    /*
     * Print out stats on stderr
     */
    if(quiet != 1) {
#ifdef GATHER_STAT
	fprintf(stderr, "Average number of steps: ");
	prratio(stderr, node_steps, node_matches);
	fprintf(stderr, "\n");
#endif
#ifdef DEBUG
	fprintf( stderr,
		"%ld chars in, %ld codes (%ld bytes) out, freezing factor: ",
		in_count, symbols_out + refers_out, bytes_out);
	prratio( stderr, in_count, bytes_out );
	fprintf( stderr, "\n");
	fprintf( stderr, "\tFreezing as in compact: " );
	prratio( stderr, in_count-bytes_out, in_count );
	prbits( stderr, in_count, bytes_out);
	fprintf( stderr, "\n");
	fprintf( stderr, "\tSymbols: %ld; references: %ld.\n",
		symbols_out, refers_out);
#else /* !DEBUG */
	fprintf( stderr, "Freezing: " );
	prratio( stderr, in_count-bytes_out, in_count );
	prbits( stderr, in_count, bytes_out);
#endif /* DEBUG */
    }
    if(bytes_out >= in_count)    /* exit(2) if no savings */
	exit_stat = 2;
    return;
}

