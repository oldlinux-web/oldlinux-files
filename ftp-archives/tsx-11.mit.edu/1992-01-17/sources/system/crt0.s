/* 
 *	crt0.S for linux 0.1x 2003 
 */

	.text
__entry:
	movl 8(%esp),%eax
	movl %eax,___environ
	call _main
	pushl %eax
1:
	call _exit
	jmp 1b	
