#include <soft.h>

double fabs(double arg)
{
	if(arg < 0.)
		arg = -arg;
	return(arg);
}
