#include "freeze.h"
#include "huf.h"
#include "bitio.h"

/*
 * Melt stdin to stdout.
 */

void melt2 ()
{
	register short    i, j, k, r, c;

/* Huffman-dependent part */
	if(read_header() == EOF)
		return;
	StartHuff(N_CHAR2);
	init(Table2);
/* end of Huffman-dependent part */

	InitIO();
	for (i = 0; i < N2 - F2; i++)
		text_buf[i] = ' ';
	r = N2 - F2;
	for (in_count = 0;; ) {
		c = DecodeChar();

		if (c == ENDOF)
			break;
		if (c < 256) {
#ifdef DEBUG
			if (debug)
				fprintf(stderr, "'%s'\n", pr_char((uc_t)c));
			else
#endif /* DEBUG */
				putchar (c);
			text_buf[r++] = c;
			r &= N2 - 1;
			in_count++;
		} else {
			i = (r - DecodePosition() - 1) & (N2 - 1);
			j = c - 256 + THRESHOLD;
#ifdef DEBUG
			if (debug)
				fputc('"', stderr);
#endif
			for (k = 0; k < j; k++) {
				c = text_buf[(i + k) & (N2 - 1)];
#ifdef DEBUG
				if (debug)
					fprintf(stderr, "%s", pr_char((uc_t)c));
				else
#endif
					putchar (c);
				text_buf[r++] = c;
				r &= (N2 - 1);
				in_count++;
			}
#ifdef DEBUG
			if (debug)
				fprintf(stderr, "\"\n");
#endif
		}
		INDICATOR
	}
}

#ifdef COMPAT
void melt1 ()
{
	register short    i, j, k, r, c;

	StartHuff(N_CHAR1);
	init(Table1);
	InitIO();
	for (i = 0; i < N1 - F1; i++)
		text_buf[i] = ' ';
	r = N1 - F1;
	for (in_count = 0;; ) {
		c =  DecodeChar();

		if (c == ENDOF)
			break;

		if (c < 256) {
#ifdef DEBUG
			if (debug)
				fprintf(stderr, "'%s'\n", pr_char((uc_t)c));
			else
#endif /* DEBUG */
				putchar (c);
			text_buf[r++] = c;
			r &= (N1 - 1);
			in_count++;
		} else {
			i = (r - DecodePOld() - 1) & (N1 - 1);
			j = c - 256 + THRESHOLD;
#ifdef DEBUG
			if (debug)
				fputc('"', stderr);
#endif
			for (k = 0; k < j; k++) {
				c = text_buf[(i + k) & (N1 - 1)];
#ifdef DEBUG
				if (debug)
					fprintf(stderr, "%s", pr_char((uc_t)c));
				else
#endif
					putchar (c);
				text_buf[r++] = c;
				r &= (N1 - 1);
				in_count++;
			}
#ifdef DEBUG
			if (debug)
				fprintf(stderr, "\"\n");
#endif
		}
		INDICATOR
	}
}
#endif
