# To make  'cp', type:   make f=cp
# To make  'ls', type:   make f=ls
# To make  'cat', type:   make f=cat
# Get the idea?

# To make everything, type:	make all

l=/usr/lib
CFLAGS=  -F -D_MINIX -D_POSIX_SOURCE

File:	$l/libc.a $f.s
	@echo ------------------------------ $f ------------------------------
	@cc -i -o bin/$f $f.s
	@chmem =8192 bin/$f >/dev/null

clean:	
	@rm -f *.s *.bak
	@rm -f */*.s
	@rm -f */*.bak

all:	
	@ls *.c >tmp.1
	@sed <tmp.1 >tmp.2 '/\.c/s///;/^/s//make f=/'
	@rm -rf tmp.1 bin
	@mkdir bin
	@sh tmp.2
	@rm tmp.2
