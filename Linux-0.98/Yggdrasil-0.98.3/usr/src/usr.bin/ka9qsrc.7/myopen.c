/*
 * Quick kludge to set file buffer size for stdio...
 *	2-20-89, 17:05  -- hyc
 */

#include <stdio.h>
FILE *myopen(name,mode)
char *name, *mode;
{
	FILE *fp;
	char *buf, *malloc();
	int	size=30720;

	fp = fopen(name,mode);
	if (fp != NULL) {
		if ((buf=malloc(size)) != NULL)
			setbuffer(fp, buf, size);
	}
	return(fp);
}
