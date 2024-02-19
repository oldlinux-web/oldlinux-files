	.text
__entry:
/*
 * The first thing we do is try to load the shared library. If that
 * fails, it won't return.
 */
	call ___load_shared_libraries
/*
 *	This is important, and was missing from the new version...
 */
	movl $45,%eax
	movl $0,%ebx
	int  $0x80
	movl %eax,____brk_addr
/*
 *	ok, set up the 387 flags
 */	
	fldcw init_cw
/*
 *	Setup profiling
 */
	pushl $__mcleanup
	call _atexit
	addl $4,%esp
	pushl $_etext
	pushl $__entry
	call _monstartup
	addl $8,%esp
/*
 *	Setup _environ and call _main
 */
	movl 8(%esp),%eax
	movl %eax,_environ
	call _main
	pushl %eax
	call _exit
/*
 *	Just in case _exit fails... We use int $0x80 for __exit().
 */
	popl %ebx
done:
	movl $1,%eax
	int $0x80
	jmp done
init_cw:
	.word 0x1272
/*
.comm ___SHARED_LIBRARIES__,8
*/
___shared_dummy__:
	.asciz ""
	.stabs "___SHARED_LIBRARIES__",24,0,0,___shared_dummy__


/* Here is the dirty part. Settup up your 387 through the control word
 * (cw) register. 
 *
 *     15-13    12  11-10  9-8     7-6     5    4    3    2    1    0
 * | reserved | IC | RC  | PC | reserved | PM | UM | OM | ZM | DM | IM
 *
 * IM: Invalid operation mask
 * DM: Denormalized operand mask
 * ZM: Zero-divide mask
 * OM: Overflow mask
 * UM: Underflow mask
 * PM: Precision (inexact result) mask
 * 
 * Mask bit is 1 means no interrupt.
 *
 * PC: Precision control
 * 11 - round to extended precision
 * 10 - round to double precision
 * 00 - round to single precision
 *
 * RC: Rounding control
 * 00 - rounding to nearest
 * 01 - rounding down (toward - infinity)
 * 10 - rounding up (toward + infinity)
 * 11 - rounding toward zero
 *
 * IC: Infinity control
 * That is for 8087 and 80287 only.
 *
 * The hardware default is 0x037f. I choose 0x1272.
 */
