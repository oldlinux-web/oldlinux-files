/* This tool is used to make a.img */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FD0_SIZE 1474560 /* 1.44M */
#define BUFSIZE 512
int main(int argc, char **argv)
{
	int fd_in, fd_out;
	int n;
	int size = 0;
	char buf[BUFSIZE];
	
	if (argc != 3) {
		printf("Usage:\t%s infile outfile\n",
		       argv[0]);
		exit(-1);
	}

	fd_in = open(argv[1], O_RDONLY);
	if (fd_in == -1) {
		printf("Can't open %s\n", argv[1]);
		exit(-1);
	}

	fd_out = open(argv[2], O_WRONLY | O_CREAT, 0600);
	if (fd_out == -1) {
		printf("Can't open %s\n", argv[2]);
		close(fd_in);
		exit(-1);
	}

//	printf("Begin to read %s...\n", argv[1]);
	while (1) {
		n = read(fd_in, buf, BUFSIZE);
		if (n == 0 || n == -1)
			break;
		size += n;

		if (write(fd_out, buf, n) != n) {
			printf("write error.\n");
			close(fd_in);
			close(fd_out);
			exit(-1);
		}
	}
//	printf("read %s done\n", argv[1]);
	
	if (size > FD0_SIZE) {
		printf("em..%s is bigger than 1.4M.\n", argv[2]);
		close(fd_in);
		close(fd_out);
		exit(-1);
	}

//	printf("Begin to lseek...\n");
	n = FD0_SIZE - size - 2;
	n = lseek(fd_out, n, SEEK_CUR);
	if (n == -1)
		printf("Lseek error\n");

//	printf("lseek done: %d\n", n);
	write(fd_out, "dd", 2);
	close(fd_in);
	close(fd_out);
	return 0;
}
		
