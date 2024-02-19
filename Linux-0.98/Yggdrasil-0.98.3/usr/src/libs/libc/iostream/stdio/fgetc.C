#include "stdioprivate.h"

#undef fgetc

int fgetc(FILE *fp)
{
    return getc(fp);
}
