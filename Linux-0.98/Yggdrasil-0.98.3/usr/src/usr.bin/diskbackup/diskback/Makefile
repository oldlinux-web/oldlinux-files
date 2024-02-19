all: diskbackup diskrestore
diskbackup: diskbackup.c
	cc diskbackup.c -o db
diskrestore: diskrestore.c
	cc diskrestore.c -o dr

install:
	mv db /usr/bin/diskbackup
	mv dr /usr/bin/diskrestore
