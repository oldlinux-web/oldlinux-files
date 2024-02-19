/* match.c: pattern matching routines */

#include "rc.h"
#include "utils.h"
#include "match.h"

static int rangematch(char *, char);

/*
   match() matches a single pattern against a single string.
*/

boolean match(char *p, char *m, char *s) {
	int i = 0;
	int j;

	if (m == NULL)
		return streq(p, s);

	while(1) {
		if (p[i] == '\0')
			return *s == '\0';
		else if (m[i]) {
			switch (p[i++]) {
			case '?':
				if (*s++ == '\0')
					return FALSE;
				break;
			case '*':
				/* collapse multiple stars */
				while (p[i] == '*' && m[i] == 1)
					i++;

				/* star at end of pattern? */
				if (p[i] == '\0')
					return TRUE;

				while (*s != '\0')
					if (match(p + i, m + i, s++))
						return TRUE;
				return FALSE;
			case '[':
				if (*s == '\0' || ((j = rangematch(p + i, *s++)) < 0))
					return FALSE;
				i += j;
				break;
			default:
				fprint(2,"%c is not a metacharacter\n", p[i-1]);
				return FALSE;
			}
		} else if (p[i++] != *s++) {
				return FALSE;
		}
	}
}

/*
   From the ed(1) man pages (on ranges):

	The `-' is treated as an ordinary character if it occurs first
	(or first after an initial ^) or last in the string.

	The right square bracket does not terminate the enclosed string
	if it is the first character (after an initial `^', if any), in
	the bracketed string.

   rangematch() matches a single character against a class, and returns
   an integer offset to the end of the range on success, or -1 on
   failure.
*/

static int rangematch(char *p, char c) {
	char *orig = p;
	boolean neg = (*p == '~');
	boolean matched = FALSE;

	if (neg)
		p++;

	if (*p == ']') {
		p++;
		matched = (c == ']');
	}

	for (; *p != ']'; p++) {
		if (*p == '\0')
			return -1;	/* bad syntax */
		if (p[1] == '-' && p[2] != ']') { /* check for [..-..] but ignore [..-] */
			if (c >= *p)
				matched |= (c <= p[2]);
			p += 2;
		} else {
			matched |= (*p == c);
		}
	}

	if (matched ^ neg)
		return p - orig + 1; /* skip the right-bracket */
	else
		return -1;
}

