/* common.c  -  Common data structures and functions. */

/* Written 1992 by Werner Almesberger */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"
#include "lilo.h"


int verbose = 0,compact = 0;


void pdie(char *msg)
{
    perror(msg);
    exit(1);
}


void die(char *fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    vfprintf(stderr,fmt,ap);
    va_end(ap);
    fputc('\n',stderr);
    exit(1);
}


void *alloc(int size)
{
    void *this;

    if ((this = malloc(size)) == NULL) pdie("Out of memory");
    return this;
}


char *stralloc(char *str)
{
    char *this;

    if ((this = strdup(str)) == NULL) pdie("Out of memory");
    return this;
}


int to_number(char *num)
{
    int number;
    char *end;

    number = strtol(num,&end,0);
    if (end && *end) die("Not a number: \"%s\"",num);
    return number;
}


static char *name(int stage)
{
    switch (stage) {
	case STAGE_FIRST:
	    return "First boot sector";
	case STAGE_SECOND:
	    return "Second boot sector";
	case STAGE_CHAIN:
	    return "Chain loader";
	default:
	    die("Internal error: Unknown stage code %d",stage);
    }
    return NULL; /* makes GCC happy */
}


void check_version(BOOT_SECTOR *sect,int stage)
{
    if (strncmp(sect->par_1.signature,"LILO",4))
	die("%s doesn't have a valid LILO signature",name(stage));
    if (sect->par_1.stage != stage)
	die("%s has an invalid stage code (%d)",name(stage),sect->par_1.stage);
    if (sect->par_1.version != VERSION)
	die("%s is version %d. Expecting version %d.",name(stage),
	  sect->par_1.version,VERSION);
}
