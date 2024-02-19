/*
   mini MKTEMP for ORIGAMI (ST-port)
   (C) 1990 M. Schwingen
   This does just enough to work. Improvements are welcome.
*/

#include <stdio.h>

void mktemp(char *str)
{
	static int tmpcnt = 0;
	char buf[10];
	char *p1,*p2;
	
	sprintf(buf,"%08d",tmpcnt++);
	p1 = buf;
	while (*p1)
		p1++;
	p2 = str;
	p1--;
	while (*p2)
	{
		if (*p2 == 'X')
			*p2 = *p1--;
		p2++;
	}
}

