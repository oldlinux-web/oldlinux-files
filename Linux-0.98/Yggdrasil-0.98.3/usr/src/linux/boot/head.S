/*
 *  linux/boot/head.s
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 *  head.s contains the 32-bit startup code.
 *
 * NOTE!!! Startup happens at absolute address 0x00000000, which is also where
 * the page directory will exist. The startup code will be overwritten by
 * the page directory.
 */
.text
.globl _idt,_gdt,
.globl _swapper_pg_dir,_pg0
.globl _empty_bad_page
.globl _empty_bad_page_table
.globl _tmp_floppy_area,_floppy_track_buffer

/*
 * swapper_pg_dir is the main page directory, address 0x00001000
 */
startup_32:
	cld
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	lss _stack_start,%esp
	call setup_idt
	xorl %eax,%eax
1:	incl %eax		# check that A20 really IS enabled
	movl %eax,0x000000	# loop forever if it isn't
	cmpl %eax,0x100000
	je 1b
/* check if it is 486 or 386. */
	movl %esp,%edi		# save stack pointer
	andl $0xfffffffc,%esp	# align stack to avoid AC fault
	pushfl			# push EFLAGS
	popl %eax		# get EFLAGS
	movl %eax,%ecx		# save original EFLAGS
	xorl $0x40000,%eax	# flip AC bit in EFLAGS
	pushl %eax		# copy to EFLAGS
	popfl			# set EFLAGS
	pushfl			# get new EFLAGS
	popl %eax		# put it in eax
	xorl %ecx,%eax		# check if AC bit is changed. zero is 486.
	jz 1f			# 486
	pushl %ecx		# restore original EFLAGS
	popfl
	movl %edi,%esp		# restore esp
	movl %cr0,%eax		# 386
	andl $0x80000011,%eax	# Save PG,PE,ET
	orl $2,%eax		# set MP
	jmp 2f	
/*
 * NOTE! 486 should set bit 16, to check for write-protect in supervisor
 * mode. Then it would be unnecessary with the "verify_area()"-calls.
 * 486 users probably want to set the NE (#5) bit also, so as to use
 * int 16 for math errors.
 */
1:	pushl %ecx		# restore original EFLAGS
	popfl
	movl %edi,%esp		# restore esp
	movl %cr0,%eax		# 486
	andl $0x80000011,%eax	# Save PG,PE,ET
	orl $0x10022,%eax	# set NE and MP
2:	movl %eax,%cr0
	call check_x87
	jmp after_page_tables

/*
 * We depend on ET to be correct. This checks for 287/387.
 */
check_x87:
	movl $0,_hard_math
	fninit
	fstsw %ax
	cmpb $0,%al
	je 1f
	movl %cr0,%eax		/* no coprocessor: have to set bits */
	xorl $6,%eax		/* reset MP, set EM */
	movl %eax,%cr0
	ret
.align 2
1:	movl $1,_hard_math
	.byte 0xDB,0xE4		/* fsetpm for 287, ignored by 387 */
	ret

/*
 *  setup_idt
 *
 *  sets up a idt with 256 entries pointing to
 *  ignore_int, interrupt gates. It doesn't actually load
 *  idt - that can be done only after paging has been enabled
 *  and the kernel moved to 0xC0000000. Interrupts
 *  are enabled elsewhere, when we can be relatively
 *  sure everything is ok. This routine will be over-
 *  written by the page tables.
 */
setup_idt:
	lea ignore_int,%edx
	movl $0x00080000,%eax
	movw %dx,%ax		/* selector = 0x0008 = cs */
	movw $0x8E00,%dx	/* interrupt gate - dpl=0, present */

	lea _idt,%edi
	mov $256,%ecx
rp_sidt:
	movl %eax,(%edi)
	movl %edx,4(%edi)
	addl $8,%edi
	dec %ecx
	jne rp_sidt
	ret

/*
 * page 0 is made non-existent, so that kernel NULL pointer references get
 * caught. Thus the swapper page directory has been moved to 0x1000
 */
.org 0x1000
_swapper_pg_dir:
/*
 * The page tables are initialized to only 4MB here - the final page
 * tables are set up later depending on memory size.
 */
.org 0x2000
_pg0:

.org 0x3000
_empty_bad_page:

.org 0x4000
_empty_bad_page_table:

.org 0x5000
/*
 * tmp_floppy_area is used by the floppy-driver when DMA cannot
 * reach to a buffer-block. It needs to be aligned, so that it isn't
 * on a 64kB border.
 */
_tmp_floppy_area:
	.fill 1024,1,0
/*
 * floppy_track_buffer is used to buffer one track of floppy data: it
 * has to be separate from the tmp_floppy area, as otherwise a single-
 * sector read/write can mess it up. It can contain one full track of
 * data (18*2*512 bytes).
 */
_floppy_track_buffer:
	.fill 512*2*18,1,0

after_page_tables:
	call setup_paging
	lgdt gdt_descr
	lidt idt_descr
	ljmp $0x08,$1f
1:	movl $0x10,%eax		# reload all the segment registers
	mov %ax,%ds		# after changing gdt.
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	lss _stack_start,%esp
	pushl $0		# These are the parameters to main :-)
	pushl $0
	pushl $0
	cld			# gcc2 wants the direction flag cleared at all times
	call _start_kernel
L6:
	jmp L6			# main should never return here, but
				# just in case, we know what happens.

/* This is the default interrupt "handler" :-) */
int_msg:
	.asciz "Unknown interrupt\n\r"
.align 2
ignore_int:
	cld
	pushl %eax
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	pushl $int_msg
	call _printk
	popl %eax
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret


/*
 * Setup_paging
 *
 * This routine sets up paging by setting the page bit
 * in cr0. The page tables are set up, identity-mapping
 * the first 4MB.  The rest are initialized later.
 *
 * NOTE! Although all physical memory should be identity
 * mapped by this routine, only the kernel page functions
 * use the >1Mb addresses directly. All "normal" functions
 * use just the lower 1Mb, or the local data space, which
 * will be mapped to some other place - mm keeps track of
 * that.
 *
 * For those with more memory than 16 Mb - tough luck. I've
 * not got it, why should you :-) The source is here. Change
 * it. (Seriously - it shouldn't be too difficult. Mostly
 * change some constants etc. I left it at 16Mb, as my machine
 * even cannot be extended past that (ok, but it was cheap :-)
 * I've tried to show which constants to change by having
 * some kind of marker at them (search for "16Mb"), but I
 * won't guarantee that's all :-( )
 *
 * (ref: added support for up to 32mb, 17Apr92)  -- Rik Faith
 * (ref: update, 25Sept92)  -- croutons@crunchy.uucp 
 * (ref: 92.10.11 - Linus Torvalds. Corrected 16M limit - no upper memory limit)
 */
.align 2
setup_paging:
	movl $1024*2,%ecx		/* 2 pages - swapper_pg_dir+1 page table */
	xorl %eax,%eax
	movl $_swapper_pg_dir,%edi	/* swapper_pg_dir is at 0x1000 */
	cld;rep;stosl
/* Identity-map the kernel in low 4MB memory for ease of transition */
	movl $_pg0+7,_swapper_pg_dir		/* set present bit/user r/w */
/* But the real place is at 0xC0000000 */
	movl $_pg0+7,_swapper_pg_dir+3072	/* set present bit/user r/w */
	movl $_pg0+4092,%edi
	movl $0x03ff007,%eax		/*  4Mb - 4096 + 7 (r/w user,p) */
	std
1:	stosl			/* fill the page backwards - more efficient :-) */
	subl $0x1000,%eax
	jge 1b
	cld
	movl $_swapper_pg_dir,%eax
	movl %eax,%cr3			/* cr3 - page directory start */
	movl %cr0,%eax
	orl $0x80000000,%eax
	movl %eax,%cr0		/* set paging (PG) bit */
	ret			/* this also flushes the prefetch-queue */

/*
 * The interrupt descriptor table has room for 256 idt's
 */
.align 4
.word 0
idt_descr:
	.word 256*8-1		# idt contains 256 entries
	.long 0xc0000000+_idt

.align 4
_idt:
	.fill 256,8,0		# idt is uninitialized

/*
 * The real GDT is also 256 entries long - no real reason
 */
.align 4
.word 0
gdt_descr:
	.word 256*8-1
	.long 0xc0000000+_gdt

/*
 * This gdt setup gives the kernel a 1GB address space at virtual
 * address 0xC0000000 - space enough for expansion, I hope.
 */
.align 4
_gdt:
	.quad 0x0000000000000000	/* NULL descriptor */
	.quad 0xc0c39a000000ffff	/* 1GB at 0xC0000000 */
	.quad 0xc0c392000000ffff	/* 1GB */
	.quad 0x0000000000000000	/* TEMPORARY - don't use */
	.fill 252,8,0			/* space for LDT's and TSS's etc */
