#include "stdioprivate.h"
#include "string.h"

int puts(const char *str)
{
    _G_size_t len = strlen(str);
    return ((((streambuf*)stdout)->sputn(str, len) != len) ||
  	(putchar('\n') == EOF)) ? EOF : 0;
}
