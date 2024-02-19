struct WORDREGS {
	unsigned int	ax, bx, cx, dx, si, di, cflag, flags;
};

struct BYTEREGS {
	unsigned char	al, ah, bl, bh, cl, ch, dl, dh;
};

union	REGS	{
	struct	WORDREGS x;
	struct	BYTEREGS h;
};

struct	SREGS	{
	unsigned int	es;
	unsigned int	cs;
	unsigned int	ss;
	unsigned int	ds;
};

struct regs {
	union REGS regs;
	struct SREGS sregs;
};

#ifndef AZTEC
#define sysint(intno, iregs, oregs) (int86x(intno, iregs.regs, iregs.regs, iregs.sregs), *iregs.regs.x.flags)
#endif

/* Macros to simulate Turbo-C style register pseudovariables */
#define	_AX	regs.regs.x.ax
#define	_AL	regs.regs.h.al
#define	_AH	regs.regs.h.ah
#define	_BX	regs.regs.x.bx
#define	_BL	regs.regs.h.bl
#define	_BH	regs.regs.h.bh
#define	_CX	regs.regs.x.cx
#define	_CL	regs.regs.h.cl
#define	_CH	regs.regs.h.ch
#define	_DX	regs.regs.x.dx
#define	_DL	regs.regs.h.dl
#define	_DH	regs.regs.h.dh
#define	_DI	regs.regs.x.di
#define	_SI	regs.regs.x.si
#define	_DS	regs.sregs.ds
#define	_ES	regs.sregs.es

/* Segment number of pointer */
#define	FP_SEG(x)	(sizeof(x) == 4 ? ((long)(x) >> 16) : getds())
/* Offset of pointer */
#define FP_OFF(x)	((unsigned short)((long)(x) & 0xffff))

#define	FLAG_CARRY	(1 << 0)	/* Carry flag */
