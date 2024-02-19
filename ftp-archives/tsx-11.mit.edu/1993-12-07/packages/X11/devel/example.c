/*
	VGAKIT Version 4.1

	Copyright 1988,89,90,91 John Bridges
	Free for use in commercial, shareware or freeware applications

	EXAMPLE.C
*/


extern int maxx,maxy;

main(argc,argv)
int argc;
char **argv;
{
	unsigned char palbuf[256][3];
	unsigned long x2;
	int svga;
	unsigned int color;
	unsigned int x,y;

	for(x=0;x<256;x++)
	{
		y=x&63;
		if(x&64) y=63-y;
		palbuf[x][0]=y;
		palbuf[x][1]=y*y/63;
		y=(x>>1)&63;
		if(x&128) y=63-y;
		palbuf[x][2]=y;
	}
	svga=whichvga();	
	if(svga)
	{
		svgamode();
	}
	else
	{
		mode13x();
	}
	setmany(palbuf,0,256);
	for(x=0;x<maxx/2;x++)
	{
		for(y=0;y<maxy/2;y++)
		{
			x2=(x+1)*(maxx-x);
			color=((x2*x2)/((y+1)*(long)(maxy-y))/113)&0xff;
			if(svga)
			{
				point(x,y,color);
				point((maxx-1)-x,y,color);
				point(x,(maxy-1)-y,color);
				point((maxx-1)-x,(maxy-1)-y,color);
			}
			else
			{
				point13x(x,y,color);
				point13x((maxx-1)-x,y,color);
				point13x(x,(maxy-1)-y,color);
				point13x((maxx-1)-x,(maxy-1)-y,color);
			}
		}
	}
	getch();
	txtmode();
}




