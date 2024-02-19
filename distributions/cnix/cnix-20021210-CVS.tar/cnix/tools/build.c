#include <stdio.h>
#include <stdlib.h>	
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define MINIX_HEADER 32
#define GCC_HEADER 1024

void die(char * str)
{
	fprintf(stderr,"%s\n",str);
	exit(1);
}

void usage(void)
{
	die("Usage: build boot system [> image]");
}

int main(int argc, char ** argv)
{
	int i, c, id;
	char buf[512];

	if(argc != 3)
		usage();
	for(i = 0;i < sizeof buf; i++) 
		buf[i] = 0;
	if((id = open(argv[1], O_RDONLY, 0)) < 0)
		die("Unable to open 'boot'");
	
	i = read(id, buf, sizeof buf);
	fprintf(stderr,"Boot sector %d bytes.\n",i);
	
	i = write(1,buf,512);
	
	if(i!=512)
		die("Write call failed");
	close (id);
	
	if ((id = open(argv[2], O_RDONLY, 0)) < 0)
		die("Unable to open 'system'");
	
	for (i = 0; (c = read(id, buf, sizeof buf)) > 0 ; i += c )
		if (write(1, buf, c) != c)
			die("Write call failed");
	close(id);
	fprintf(stderr,"System %d bytes.\n",i);
	return(0);
}
