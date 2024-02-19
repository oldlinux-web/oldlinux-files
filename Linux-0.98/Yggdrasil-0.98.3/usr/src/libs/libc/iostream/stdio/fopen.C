#include "stdioprivate.h"

FILE * fopen (const char *filename, const char *mode)
{
    filebuf* fbuf = new filebuf();
    if (fbuf->open(filename, mode) != NULL)
	return (FILE*)fbuf;
    delete fbuf;
    return NULL;
}
