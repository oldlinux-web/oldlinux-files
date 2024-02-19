#include <unistd.h>
char tmp[512];

#define NEW_DEV 0x303
void main(void)
{
	int i;

	if (512 != read(0,tmp,512))
		exit(1);
	if (0xAA55 != *((unsigned short *)(tmp+510)))
		exit(2);
	*((unsigned short *)(tmp+508)) = NEW_DEV;
	if (512 != write(1,tmp,512))
		exit(3);
	while ((i=read(0,tmp,512)) > 0)
		if (i != write(1,tmp,i))
			exit(4);
	exit(0);
}
/*-------

		Devices:

Harddisks:
0x301 - /dev/hd1 - first partition on first drive
...
0x304 - /dev/hd2 - fourth partition on first drive

0x306 - /dev/hd1 - first partition on second drive
...
0x309 - /dev/hd2 - fourth partition on second drive

0x300 - /dev/hd0 - the whole first drive. BE CAREFUL
0x305 - /dev/hd5 - the whole second drive. BE CAREFUL


Floppies:
0x208 - 1.2M in A
0x209 - 1.2M in B
0x21C - 1.44M in A
0x21D - 1.44M in B
*/
