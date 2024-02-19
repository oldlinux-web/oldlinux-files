#include <stdioprivate.h>

int ungetc(int c, FILE* fp)
{
    if (!__validfp(fp) || c == EOF)
	return EOF;
    return ((streambuf*)fp)->sputbackc(c);
}
