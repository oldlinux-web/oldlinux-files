/*
	File to disk moving utility for the Atari ST
*/

#include <stdio.h>

char buffer[4610];
main(argc,argv)
int argc;
char *argv[];
{

	int i,handle,result=1,drive=0,sector=1,track=0,side=0,done,count=0;
	char dummy[255];
	long filler=0,buflen=4608;
	FILE *infile;
	
	printf("file2dsk.ttp  --  (c)1991 Ken Corey (aka kenc@vaxb.acs.unt.edu)\n");
	printf("Intended for use with the demonstration distribution of \n MINIX-ST (c)1990 Prentice Hall\n"); 
	if(argc<2)
	{
		printf("\nThis program will transfer a file onto a disk bit for bit.\n");
		printf("It's meant to be used with the Minix demo software, though it will\n");
		printf("use any file.  Use with other files will no doubt lead to\n");
		printf("interesting deadly, and useless (8^)) results.\n");
		printf("\nUsage: file2dsk demo_dsk.st [B:]\n");
		printf("\nwill copy the file demo_dsk.st onto a floppy, bit for bit, sector for sector.\n");
		printf("2 Caveats:\n\n");
		printf("  1)This software will completely erase any extant info on the floppy.\n");
		printf("\n  2)The disk must previously be formatted out to single sided,\n");
		printf("    using the standard st desktop.  No other configuration has been tested!\n");
		printf("\n\n BTW, have a great day!\n");

		printf("Hint: hit a key to exit to desktop....");
		gets(dummy);
		exit(1);
	}
 
	if(argc==3)
		drive=1;
	printf("\nOkay, I'm gonna use '%s' as the filename.\n",argv[1]);
	printf("Please insert a freshly formatted, SINGLE sided floppy in drive ");
	if(drive==0)
		printf("A:\n");
	else
		printf("B:\n");
	printf("and hit return...\n\n");
	printf("THIS FLOPPY WILL BE COMPLETELY ERASED!");

	gets(dummy);
	
	result=gemdos(0x3D,argv[1],0);
	if (result<0)
	{
		printf("I couldn't open the file '%s'!\n",argv[1]);
		exit(result);
	}

	handle=result;

	while((track<=80)&&(count<0x2D0)&&(done!=1))
	{
		for(i=0;i<4610;buffer[i++]=0xE5) ;
		
		result=gemdos(0x3F,handle,buflen,&buffer[0]);
		if (result<0)
		{
			printf("Error in reading '%s'!\n",argv[1]);
			exit(result);
		}

		if(result==0)
			done=1;

		printf("%d--%d t--s\n",track,sector);

		result=xbios(0x9,&buffer[0],filler,drive,sector,track,side,9);
		if(result<0)
		{
			printf("Error in writing to drive A:!\n");
			exit(result);
		}

		track += 1;
		count += 9;
	}

	printf("All done!\n");

}
	
