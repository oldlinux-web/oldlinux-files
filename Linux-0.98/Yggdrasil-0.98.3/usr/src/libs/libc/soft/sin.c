#include <soft.h>

/*
	C program for floating point sin/cos.
	Calls modf.
	There are no error exits.
	Coefficients are #3370 from Hart & Cheney (18.80D).
*/

static double twoopi	= 0.63661977236758134308;
static double cos_p0	=  .1357884097877375669092680e8;
static double cos_p1	= -.4942908100902844161158627e7;
static double cos_p2	=  .4401030535375266501944918e6;
static double cos_p3	= -.1384727249982452873054457e5;
static double cos_p4	=  .1459688406665768722226959e3;
static double cos_q0	=  .8644558652922534429915149e7;
static double cos_q1	=  .4081792252343299749395779e6;
static double cos_q2	=  .9463096101538208180571257e4;
static double cos_q3	=  .1326534908786136358911494e3;

static double sinus(double arg, int quad)
{
	double e, f;
	double ysq;
	double x,y;
	int k;
	double temcos_p1, temcos_p2;

	x = arg;
	if(x<0) {
		x = -x;
		quad = quad + 2;
	}
	x = x*twoopi;	/*underflow?*/
	if(x>32764){
		y = modf(x,&e);
		e = e + quad;
		modf(0.25*e,&f);
		quad = e - 4*f;
	}else{
		k = x;
		y = x - k;
		quad = (quad + k) & 03;
	}
	if (quad & 01)
		y = 1-y;
	if(quad > 1)
		y = -y;

	ysq = y*y;
	temcos_p1 = ((((cos_p4*ysq+cos_p3)*ysq+cos_p2)*ysq+cos_p1)*ysq+cos_p0)*y;
	temcos_p2 = ((((ysq+cos_q3)*ysq+cos_q2)*ysq+cos_q1)*ysq+cos_q0);
	return(temcos_p1/temcos_p2);
}

double cos(double arg)
{
	if(arg<0)
		arg = -arg;
	return(sinus(arg, 1));
}

double sin(double arg)
{
	return(sinus(arg, 0));
}
