#include <soft.h>

/*
	log returns the natural logarithm of its floating
	point argument.

	The coefficients are #2705 from Hart & Cheney. (19.38D)

	It calls frexp.
*/

static double	log2	= 0.693147180559945309e0;
static double	ln10	= 2.302585092994045684;
static double	sqrto2	= 0.707106781186547524e0;
static double	log_p0	= -.240139179559210510e2;
static double	log_p1	= 0.309572928215376501e2;
static double	log_p2	= -.963769093368686593e1;
static double	log_p3	= 0.421087371217979714e0;
static double	log_q0	= -.120069589779605255e2;
static double	log_q1	= 0.194809660700889731e2;
static double	log_q2	= -.891110902798312337e1;

double log(double arg)
{
	double x,z, zsq, temp;
	int exp;

	if(arg <= 0.) {
		errno = EDOM;
		perror ("log");
		return(-HUGE_VAL);
	}
	x = frexp(arg,&exp);
	while(x<0.5) {
		x = x*2;
		exp = exp-1;
	}
	if(x<sqrto2) {
		x = 2*x;
		exp = exp-1;
	}

	z = (x-1)/(x+1);
	zsq = z*z;

	temp = ((log_p3*zsq + log_p2)*zsq + log_p1)*zsq + log_p0;
	temp = temp/(((1.0*zsq + log_q2)*zsq + log_q1)*zsq + log_q0);
	temp = temp*z + exp*log2;
	return(temp);
}

double log10(double arg)
{
	return(log(arg)/ln10);
}
