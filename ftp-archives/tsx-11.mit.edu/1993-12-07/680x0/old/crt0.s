	.globl start,_environ
	.text
start:
	movel	sp@(8),d0
	movel	d0,_environ
	bsr	_main
	movel	d0,sp@-
1:	bsr	__exit
	bras	1b

	.data
_environ:
	.long 0
