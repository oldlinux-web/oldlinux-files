/*
	VGAKIT Version 4.1

	Copyright 1988,89,90,91 John Bridges
	Free for use in commercial, shareware or freeware applications

	STICKS.C
*/


extern int maxx,maxy;

extern void bline(int x1,int y1,int x2,int y2,int color,void (*func)());
extern void xpoint(),xpoint13x();

main(argc,argv)
int argc;
char **argv;
{
	unsigned char palbuf[256][3];
	int svga;
	unsigned int color;
	unsigned int x,y,mx;
	void (*func)();

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
		func=xpoint;
	}
	else
	{
		mode13x();
		func=xpoint13x;
	}
	setmany(palbuf,0,256);
	if(maxx>maxy)
		mx=maxx;
	else
		mx=maxy;

	for(x=0;x<mx;x++)
	{
		for(y=0;y<mx;y++)
		{
			bline(x,y,y,x,y&0xff,func);
		}
	}
	getch();
	txtmode();
}


