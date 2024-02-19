#include <soft.h>

/*
	sinh(arg) returns the hyperbolic sine of its floating-
	point argument.

	The exponential function is called for arguments
	greater in magnitude than 0.5.

	A series is used for arguments smaller in magnitude than 0.5.
	The coefficients are #2029 from Hart & Cheney. (20.36D)

	cosh(arg) is computed from the exponential function for
	all arguments.
*/

static double sinh_p0  = -0.6307673640497716991184787251e+6;
static double sinh_p1  = -0.8991272022039509355398013511e+5;
static double sinh_p2  = -0.2894211355989563807284660366e+4;
static double sinh_p3  = -0.2630563213397497062819489e+2;
static double sinh_q0  = -0.6307673640497716991212077277e+6;
static double sinh_q1   = 0.1521517378790019070696485176e+5;
static double sinh_q2  = -0.173678953558233699533450911e+3;

double
sinh(arg)
double arg;
{
	double temp, argsq;
	register sign;

	sign = 1;
	if(arg < 0) {
		arg = - arg;
		sign = -1;
	}

	if(arg > 21.) {
		temp = exp(arg)/2;
		if (sign>0)
			return(temp);
		else
			return(-temp);
	}

	if(arg > 0.5) {
		return(sign*(exp(arg) - exp(-arg))/2);
	}

	argsq = arg*arg;
	temp = (((sinh_p3*argsq+sinh_p2)*argsq+sinh_p1)*argsq+sinh_p0)*arg;
	temp /= (((argsq+sinh_q2)*argsq+sinh_q1)*argsq+sinh_q0);
	return(sign*temp);
}

double cosh(double arg)
{
	if(arg < 0)
		arg = - arg;
	if(arg > 21.) {
		return(exp(arg)/2);
	}

	return((exp(arg) + exp(-arg))/2);
}
