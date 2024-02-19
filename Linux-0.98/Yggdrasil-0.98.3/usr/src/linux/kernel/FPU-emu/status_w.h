/*---------------------------------------------------------------------------+
 |  status_w.h                                                               |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 +---------------------------------------------------------------------------*/

#ifndef _STATUS_H_
#define _STATUS_H_


#ifdef __ASSEMBLER__
#define	Const__(x)	$##x
#else
#define	Const__(x)	x
#endif

#define SW_B		Const__(0x8000)	/* backward compatibility (=ES) */
#define SW_C3		Const__(0x4000)	/* condition bit 3 */
#define SW_TOP		Const__(0x3800)	/* top of stack */
#define SW_TOPS 	Const__(11)	/* shift for top of stack bits */
#define SW_C2		Const__(0x0400)	/* condition bit 2 */
#define SW_C1		Const__(0x0200)	/* condition bit 1 */
#define SW_C0		Const__(0x0100)	/* condition bit 0 */
#define SW_ES		Const__(0x0080)	/* exception summary */
#define SW_SF		Const__(0x0040)	/* stack fault */
#define SW_PE		Const__(0x0020)	/* loss of precision */
#define SW_UE		Const__(0x0010)	/* underflow */
#define SW_OE		Const__(0x0008)	/* overflow */
#define SW_ZE		Const__(0x0004)	/* divide by zero */
#define SW_DE		Const__(0x0002)	/* denormalized operand */
#define SW_IE		Const__(0x0001)	/* invalid operation */


#ifndef __ASSEMBLER__

#define COMP_A_GT_B	1
#define COMP_A_EQ_B	2
#define COMP_A_LT_B	3
#define COMP_NOCOMP	4
#define COMP_NAN	0x40
#define COMP_SNAN	0x80

#define setcc(cc) ({ \
  status_word &= ~(SW_C0|SW_C1|SW_C2|SW_C3); \
  status_word |= (cc) & (SW_C0|SW_C1|SW_C2|SW_C3); })

#endif __ASSEMBLER__

#endif _STATUS_H_
