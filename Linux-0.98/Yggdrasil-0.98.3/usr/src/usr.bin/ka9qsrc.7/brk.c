/* This is a reverse-compilation of Borland's brk() and sbrk() functions.
** Borland would prefer that I distribute these things on an object-only
** basis, but since this is a bug fix that they knew about on version 1.0
** and DIDN'T fix on version 1.5, to heck with them.
**
** This is only needed in small data models, hence the leading #ifdef.
** The fix lets us define the so-called red zone, which is the buffer
** space between the bottom of the heap and the top of the stack.  Also
** fixes a bug wherein brk() returned the new brklvl on success.  Wrong!
**
** If you '#define original', you'll get Borland's original code.
*/

#if defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)

#pragma inline

extern int errno;
extern int __brklvl;

#ifdef original
#define _STKRED 0x100
#else
extern int _STKRED;
#endif

int __brk(unsigned bar)
{
	_AX = bar;
	asm	mov	dx,sp
	_DX -= _STKRED;
	if (_AX < _DX) {
		__brklvl = _AX;
#ifndef original
		return 0;
#endif
	}
	else {
		errno = 8;
		_AX = -1;
	}
}

/*
Original from Turbo C's cm library:
BRK_TEXT:___brk:
3999:0001 55             PUSH	BP 
3999:0002 8BEC           MOV	BP,SP 
3999:0004 8B4606         MOV	AX,[BP+06] 
3999:0007 8BD4           MOV	DX,SP 
3999:0009 81EA0001       SUB	DX,0100 
3999:000D 3BC2           CMP	AX,DX 
3999:000F 7305           JNB	___brk+15 (0016) 
3999:0011 A38E00         MOV	[008E],AX 
3999:0014 EB09           JMP	___brk+1E (001F) 
3999:0016 C70688000800   MOV	Word Ptr [0088],0008 
3999:001C B8FFFF         MOV	AX,FFFF 
3999:001F 5D             POP	BP 
3999:0020 CB             RETF
*/

__sbrk(unsigned long bar)
{
	asm	mov	ax,[bp+06] 
	asm	mov	dx,[bp+08] 
	asm	add	ax,__brklvl
	asm	adc	dx,0
	asm	mov	cx,ax
	_CX += _STKRED;
	asm	adc	dx,0
	if (_DX == 0 && _CX < _SP) asm xchg ax,__brklvl;
	else {
		errno = 8;
		_AX = -1;
	}
}

/*
Original from Turbo C's cm library:
BRK_TEXT:___sbrk:
3999:0021 55             PUSH	BP 
3999:0022 8BEC           MOV	BP,SP 
3999:0024 8B4606         MOV	AX,[BP+06] 
3999:0027 8B5608         MOV	DX,[BP+08] 
3999:002A 03068E00       ADD	AX,[008E] 
3999:002E 83D200         ADC	DX,+00 
3999:0031 8BC8           MOV	CX,AX 
3999:0033 81C10001       ADD	CX,0100 
3999:0037 83D200         ADC	DX,+00 
3999:003A 0BD2           OR	DX,DX 
3999:003C 750A           JNZ	___sbrk+27 (0048) 
3999:003E 3BCC           CMP	CX,SP 
3999:0040 7306           JNB	___sbrk+27 (0048) 
3999:0042 87068E00       XCHG	AX,[008E] 
3999:0046 EB09           JMP	___sbrk+30 (0051) 
3999:0048 C70688000800   MOV	Word Ptr [0088],0008 
3999:004E B8FFFF         MOV	AX,FFFF 
3999:0051 5D             POP	BP 
3999:0052 CB             RETF	 
*/

int brk(unsigned bar) {return __brk(bar);}

/*
Original from Turbo C's cm library:
BRK_TEXT:_brk:
3999:0053 55             PUSH	BP 
3999:0054 8BEC           MOV	BP,SP 
3999:0056 FF7606         PUSH	[BP+06] 
3999:0059 0E             PUSH	CS 
3999:005A E8A4FF         CALL	___brk 
3999:005D 59             POP	CX 
3999:005E 5D             POP	BP 
3999:005F CB             RETF	 
*/

sbrk(int bar) {return __sbrk(bar);}

/*
Original from Turbo C's cm library:
BRK_TEXT:_sbrk:
3999:0060 55             PUSH	BP 
3999:0061 8BEC           MOV	BP,SP 
3999:0063 8B4606         MOV	AX,[BP+06] 
3999:0066 99             CWD	 
3999:0067 52             PUSH	DX 
3999:0068 50             PUSH	AX 
3999:0069 0E             PUSH	CS 
3999:006A E8B4FF         CALL	___sbrk 
3999:006D 8BE5           MOV	SP,BP 
3999:006F 5D             POP	BP 
3999:0070 CB             RETF	 
*/
#endif
