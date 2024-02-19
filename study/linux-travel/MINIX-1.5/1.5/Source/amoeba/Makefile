all:	
	@echo /usr/lib/cpp needs 50000 bytes of stack space.  Current size is:
	@size /usr/lib/cpp
	@echo If it has less than 50K, het DEL and use chmem to change it.
	cd kernel; 	make
	cd fs;		make
	cd mm;		make	
	cd ../tools;	make net

