
protoize:
	gcc -s protoize.c -o protoize 


install:
	cp protoize /usr/bin
	ln /usr/bin/protoize /usr/bin/unprotoize
	cp SYSCALLS.c.X /usr/local/lib
	cp protoize.1 unprotoize.1 /usr/man/man1

