/*
 * Function hyphenate() takes as arguments a word to be split up
 * by hyphenation, the room available to fit the first part into, and
 * the hyphenation level.  If successful in finding a hyphenation
 * point that will permit a small enough first part, the function
 * returns the length of the first part, including one space for the
 * hyphen that will presumably be added.  Otherwise, the original
 * length of the word is returned.
 * Hyphenation level >= 6 blocks hyphenation after hyphen, and
 * level >= 7 blocks hyphenation when there are non-alphabetic
 * characters surrounded by alphabetic ones (such as in email
 * addresses).
 *
 * The tables were taken from a dump of TeX (initex) after
 * it finished digesting the file hyphen.tex, and consequently this
 * data is derivative from the work of Donald E. Knuth and Frank
 * M. Liang.  Almost all the code for the hyphenation function was
 * taken from TeX, from sections 923-924 of TeX: The Program, by
 * Donald E. Knuth, 1986, Addison Wesley.  See section 919-920 of
 * that book for other references.  The original algorithm is
 * intended to apply only to spans of alphabetic characters, but
 * here it is applied more generally.  If it malfunctions, then,
 * that is likely my fault.
 *
 * This program code is in the public domain.
 * Greg Lee, 4/10/92.
 */

#include <ctype.h>
#include "trie_link.h"
#include "trie_char.h"
#include "trie_op.h"

unsigned char hyf_distance[] = { 0,
 0, 3, 1, 1, 2, 1, 2, 2, 0, 1, 3, 0, 0, 3, 1, 0, 1, 1, 4, 4,
 4, 2, 2, 3, 3, 3, 4, 3, 4, 2, 5, 5, 4, 5, 3, 2, 5, 2, 6, 2,
 2, 3, 4, 1, 3, 2, 2, 3, 3, 3, 2, 3, 1, 5, 5, 1, 4, 3, 1, 2,
 2, 3, 4, 3, 4, 4, 3, 2, 3, 1, 2, 7, 5, 1, 2, 2, 3, 5, 2, 2,
 3, 1, 3, 1, 3, 4, 4, 3, 3, 3, 2, 2, 2, 2, 3, 3, 1, 1, 4, 3,
 3, 2, 3, 6, 2, 3, 4, 4, 3, 4, 1, 4, 4, 3, 2, 2, 2, 3, 4, 4,
 3, 4, 4, 2, 3, 2, 2, 4, 4, 2, 2, 4, 3, 4, 3, 3, 4, 4, 3, 3,
 3, 4, 3, 3, 4, 8, 1, 3, 1, 1, 3, 2, 3, 6, 2, 4, 4, 3, 2, 2,
 2, 3, 4, 1, 2, 5, 2, 5, 5, 1, 4, 1, 2, 4, 3, 3, 3, 3, 2, 3,
 2 };

unsigned char hyf_num[] = { 0,
 4, 4, 1, 3, 5, 5, 3, 4, 5, 4, 5, 2, 3, 5, 3, 1, 2, 5, 5, 5,
 5, 1, 2, 4, 3, 2, 4, 4, 3, 4, 4, 5, 4, 3, 4, 3, 5, 2, 5, 5,
 5, 5, 5, 4, 1, 2, 4, 4, 4, 4, 4, 2, 5, 3, 3, 2, 4, 5, 5, 2,
 4, 2, 2, 5, 4, 4, 3, 2, 2, 4, 1, 5, 4, 3, 4, 5, 3, 4, 1, 3,
 2, 1, 3, 1, 1, 5, 1, 4, 1, 4, 4, 2, 1, 1, 4, 2, 4, 2, 4, 5,
 3, 1, 3, 4, 4, 3, 4, 2, 1, 4, 2, 4, 2, 1, 2, 2, 4, 1, 3, 5,
 3, 4, 5, 5, 2, 5, 4, 3, 4, 2, 4, 2, 5, 5, 2, 4, 4, 4, 2, 1,
 4, 1, 4, 2, 3, 5, 4, 5, 1, 2, 2, 3, 4, 5, 3, 3, 1, 3, 2, 1,
 3, 1, 5, 4, 1, 5, 3, 5, 5, 3, 2, 2, 2, 5, 2, 5, 4, 2, 4, 3,
 5 };

unsigned char hyf_next[] = { 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 1, 0, 0, 9, 0, 5,
 11, 0, 0, 23, 0, 0, 0, 10, 0, 10, 0, 19, 11, 0, 22, 4, 0, 17, 0, 1,
 6, 10, 42, 12, 0, 3, 3, 17, 6, 1, 4, 7, 1, 25, 5, 12, 45, 1, 12, 59,
 6, 5, 10, 8, 7, 22, 12, 6, 22, 1, 70, 0, 29, 12, 74, 10, 8, 19, 17, 10,
 80, 1, 10, 12, 51, 7, 0, 4, 3, 5, 82, 84, 3, 10, 7, 23, 16, 1, 25, 9,
 13, 12, 7, 0, 1, 4, 106, 0, 1, 23, 9, 8, 25, 4, 9, 4, 84, 10, 2, 2,
 30, 103, 6, 4, 124, 9, 12, 10, 10, 1, 53, 8, 23, 133, 3, 3, 6, 26, 41, 23,
 30, 2, 8, 1, 25, 0, 9, 5, 13, 16, 79, 17, 152, 148, 13, 7, 45, 3, 74, 56,
 9, 8, 10, 13, 1, 11, 1, 25, 10, 13, 11, 13, 12, 4, 17, 4, 79, 94, 17, 1,
 17 };

#define DEL 127

int hyphenate(word, room, hlevel)
char *word;
int room, hlevel;
{
	unsigned char *hc, *hyf;
	char embflag = 0;
	int i, j, z, l, v, ret;
	int hn = strlen(word);

	if (room >= hn) return(hn);
	if (hn < 5) return(hn);
	if ((hc = (unsigned char *)calloc(hn + 3, 1)) == 0) return(hn);
	if ((hyf = (unsigned char *)calloc(hn + 3, 1)) == 0) {
		free(hc);
		return(hn);
	}

	hc[0] = hc[hn+1] = DEL; hc[hn+2] = 256;
	for (j = 0; word[j]; j++)
		hc[j+1] = (isalpha(word[j]))? tolower(word[j]) - 1 : DEL;

	for (j = 0; j <= hn - 2; j++) {
		z = hc[j]; l = j;
		while (hc[l] == trie_char[z]) {
			if (v = trie_op[z]) {
				do {
					i = l - hyf_distance[v];
					if (hyf_num[v] > hyf[i]) hyf[i] = hyf_num[v];
					v = hyf_next[v];
				} while (v);
			}
			l++;  z = trie_link[z] + hc[l];
		}
	}

	hyf[1] = 0; hyf[hn-1] = 0; hyf[hn] = 0; hyf[hn-2] = 0;
	for (j = 0; word[j]; j++) {
		if (!isalpha(word[j])) {
			hyf[j+1] = 0;
			hyf[j+2] = 0;
			hyf[j] = 0;
			if (j > 0) hyf[j-1] = 0;
			if (j > 1) hyf[j-2] = 0;
			if (embflag == 1) embflag++;
		}
		/* hlevel 7 or greater prevents split when non-alpha
		 * is in the midst of alpha characters */
		else if (hlevel >= 7) {
			if (!embflag) embflag++;
			else if (embflag == 2) return(hn);
		}
		/* hlevel 6 or greater prevents split after '-' */
		if (hlevel <= 5 && word[j] == '-') hyf[j+1] = 1;
	}

	ret = hn;
	for (j = hn-3 ; j > 1; j--) {
		z = (hc[j]+1 == '-')? j : j+1;
		if ((hyf[j] % 2) && (z <= room)) {
			ret = z;
			break;
		}
	}
	free(hc);
	free(hyf);
	return(ret);
}
