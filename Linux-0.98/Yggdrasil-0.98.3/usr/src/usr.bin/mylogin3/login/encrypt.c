/* encrypt.c - poe */
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

char *crypt();

main(argc,argv)
	int argc;
	char *argv[];
{
	char	salt[2];
	time_t	tm;
	
	time(&tm);
	salt[0] = (tm & 0x0f) +	'A';
	salt[1] = ((tm & 0xf0) >> 4) + 'a';
	
	printf("passwd is %s\n", argv[1]);
	printf("salt is %c%c\n", salt[0], salt[1]);
	printf("encryption is %s\n", crypt(argv[1], salt));	
}