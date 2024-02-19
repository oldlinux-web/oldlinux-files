/*---------------------------------------------------------------------------+
 |  fpu_emu.h                                                                |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 +---------------------------------------------------------------------------*/


#ifndef _FPU_EMU_H_
#define _FPU_EMU_H_

#ifdef __ASSEMBLER__
#include "fpu_asm.h"
#define	Const(x)	$##x
#else
#define	Const(x)	x
#endif

#define EXP_BIAS	Const(0)
#define EXP_OVER	Const(0x4000)    /* smallest invalid large exponent */
/* #define EXP_MAX		Const(16384) */
#define	EXP_UNDER	Const(-0x3fff)   /* largest invalid small exponent */
/* #define	EXP_MIN		Const(-16384) */

#define SIGN_POS	Const(0)
#define SIGN_NEG	Const(1)

/* Keep the order TW_Valid, TW_Zero, TW_Denormal */
#define TW_Valid	Const(0)	/* valid */
#define TW_Zero		Const(1)	/* zero */
/* The following fold to 2 (Special) in the Tag Word */
#define TW_Denormal     Const(4)        /* De-normal */
#define TW_Infinity	Const(5)	/* + or - infinity */
#define	TW_NaN		Const(6)	/* Not a Number */

#define TW_Empty	Const(7)	/* empty */



#ifndef __ASSEMBLER__

typedef void (*FUNC)();

#define	REG	struct	reg

struct reg {
  char sign;
  char tag;
/*  short exp; *****/
  long exp;
  unsigned sigl;
  unsigned sigh;
};

#define	st(x)	( regs[((top+x) &7 )] )

#define	STACK_OVERFLOW	(st_new_ptr = &st(-1), st_new_ptr->tag != TW_Empty)
#define	NOT_EMPTY(i)	(st(i).tag != TW_Empty)
#define	NOT_EMPTY_0	(st0_tag ^ TW_Empty)

extern unsigned char FPU_modrm;
extern unsigned char FPU_rm;

extern	char	st0_tag;
extern	REG	*st0_ptr;

extern void  *FPU_data_address;
extern unsigned long FPU_entry_eip;

extern  REG  FPU_loaded_data;

#define pop()	{ st0_ptr->tag = TW_Empty; \
				  top++; st0_ptr = &(regs[top&7]); }

/* push() does not affect the tags */
#define push()	{ top--; st0_ptr = st_new_ptr; }


#define reg_move(x, y) { \
		 *(short *)&((y)->sign) = *(short *)&((x)->sign); \
		 *(long *)&((y)->exp) = *(long *)&((x)->exp); \
		 *(long long *)&((y)->sigl) = *(long long *)&((x)->sigl); }


/*----- Prototypes for functions written in assembler -----*/
/* extern void reg_move(REG *a, REG *b); */

extern void mul64(long long *a, long long *b, long long *result);
extern void poly_div2(long long *x);
extern void poly_div4(long long *x);
extern void poly_div16(long long *x);
extern void polynomial(unsigned accum[], unsigned x[],
		       unsigned short terms[][4], int n);
extern void normalize(REG *x);
extern void reg_div(REG *arg1, REG *arg2, REG *answ);
extern void reg_u_sub(REG *arg1, REG *arg2, REG *answ);
extern void reg_u_mul(REG *arg1, REG *arg2, REG *answ);
extern void reg_u_div(long long *arg1, long long *arg2, REG *answ);
extern void reg_u_add(REG *arg1, REG *arg2, REG *answ);
extern void wm_sqrt(REG *n);
extern unsigned	shrx(void *l, unsigned x);
extern unsigned	shrxs(void *v, unsigned x);
extern unsigned long div_small(unsigned long long *x, unsigned long y);

#ifndef MAKING_PROTO
#include "fpu_proto.h"
#endif

#endif __ASSEMBLER__

#endif _FPU_EMU_H_
