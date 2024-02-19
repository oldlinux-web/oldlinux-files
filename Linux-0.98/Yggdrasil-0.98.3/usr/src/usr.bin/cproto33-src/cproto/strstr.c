/* $Id: strstr.c 3.2 92/03/06 00:51:10 cthuang Exp $
 *
 * Simple implementation of the ANSI strstr() function
 */
#include <stdio.h>
#include "config.h"

/* Search for a substring within the given string.
 * Return a pointer to the first occurence within the string,
 * or NULL if not found.
 */
char *
strstr (src, key)
char *src, *key;
{
    char *s;
    int keylen;

    keylen = strlen(key);
    s = strchr(src, *key);
    while (s != NULL) {
	if (strncmp(s, key, keylen) == 0)
	    return s;
	s = strchr(s+1, *key);
    }
    return NULL;
}
