#ifdef	_MINIX
.define _XYZ
.text
.extern __XYZ_
_XYZ:
jmp __XYZ_
#endif
#ifdef	_SUN
.text
.globl	_XYZ
_XYZ:
jmp 	__XYZ_
#endif
#ifdef	_PYR
.text
.globl	_XYZ
_XYZ:
jump	__XYZ_
#endif
