/*
 *   bin86/bccfp/modf.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

#include <math.h>

/* Slooow version. */

double modf(x, pint)
double x;
double *pint;
{
    if (x >= 0)
	*pint = floor(x);
    else
	*pint = ceil(x);
    return x - *pint;
}
