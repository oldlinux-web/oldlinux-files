CC = gcc
CFLAGS = -g

all: build makeimg

build: build.c
	$(CC) $(CFLAGS) -o build build.c

makeimg: makeimg.c
	$(CC) $(CFLAGS) -o makeimg makeimg.c

clean:
	rm -f *.o build makeimg 
	rm -f *~
	rm -f .*.swp
