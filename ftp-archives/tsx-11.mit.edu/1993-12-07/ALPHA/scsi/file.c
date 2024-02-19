/* frag.c - simple fragmentation checker */
/* A non-destructive program to print out the allocation blocks in use
   for a file */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


#define FIBMAP 1


int main(int argc,char **argv)
{
    int fd,last,frags,blocks,block;
    struct stat st;

    if (argc < 2) {
	fprintf(stderr,"usage: %s filename ...\n",argv[0]);
	exit(1);
    }
    while (--argc) {
	if ((fd = open(argv[1],O_RDONLY)) < 0) {
	    perror(argv[1]);
	    exit(1);
	}
	if (fstat(fd,&st) < 0) {
	    perror(argv[1]);
	    exit(1);
	}
	if (!S_ISREG(st.st_mode)) {
	    argv++;
	    (void) close(fd);
	    continue;
	}
	last = -1;
	frags = 0;
	for (blocks = 0; blocks < ((st.st_size+1023) >> 10); blocks++) {
	    block = blocks;
	    if (ioctl(fd,FIBMAP,&block) < 0) {
		perror(argv[1]);
		exit(1);
	    }
	    if (block) {
		if (last != block-1 && last != block+1) frags++;
		last = block;
	    }
	}
	printf(" %3d%%  %s  (%d fragment%s, %d block%s)\n",frags < 2 ? 0 :
	  frags*100/blocks,*++argv,frags,frags == 1 ? "" : "s",blocks,blocks ==
	  1 ? "" : "s");
	(void) close(fd);
    }
    exit(0);
}

