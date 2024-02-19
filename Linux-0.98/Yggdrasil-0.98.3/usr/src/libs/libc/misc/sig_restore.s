.globl ____sig_restore
.globl ____masksig_restore

____sig_restore:
	addl $4,%esp	# signr
	popl %eax
	popl %ecx
	popl %edx
	popfl
	ret

____masksig_restore:
	addl $4,%esp		# signr
	call ____ssetmask	# old blocking
	addl $4,%esp
	popl %eax
	popl %ecx
	popl %edx
	popfl
	ret
