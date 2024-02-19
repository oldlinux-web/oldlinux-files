 #
 # UFC-crypt: ultra fast crypt(3) implementation
 # Copyright (C) 1991, 1992, Michael Glad, email: glad@daimi.aau.dk
 #
 # This library is free software, you can redistribute it and/or
 # modify it under the terms of the GNU Library General Public
 # License as published by the Free Software Foundation, either
 # version 2 of the License, or (at your option) any later version.
 #
 # This library is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY, without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # Library General Public License for more details.
 #
 # You should have received a copy of the GNU Library General Public
 # License along with this library, if not, write to the Free
 # Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 #
 # @(#)crypt.s300.S	2.5 12/30/91
 #
 # MC680x0/Hp9000 s300/s400 assembly code
 #

#define L1	%d0
#define L2	%d1
#define R1	%d2
#define R2	%d3
#define SCR1	%d4
#define SCR2	%d5

#define I	%d7
#define ITR	24(%a6)

#define SB0	%a0
#define SB1	%a1
#define SB2	%a2
#define SB3	%a3
#define KPTR	%a4
	
#define F(I,O1,O2,SBX,SBY)						\
	mov.l 	(KPTR)+,SCR1 ;		eor.l 	I,SCR1 ;		\
	mov.l	0(SBX,SCR1.w),SCR2 ; 	eor.l	SCR2,O1 ;		\
	mov.l	4(SBX,SCR1.w),SCR2 ;	eor.l	SCR2,O2 ;		\
	swap.w	SCR1 ;							\
	mov.l	0(SBY,SCR1.w),SCR2 ;	eor.l	SCR2,O1 ;		\
	mov.l	4(SBY,SCR1.w),SCR2 ;	eor.l	SCR2,O2 ;

#define G(I1,I2,O1,O2)			\
	F(I1,O1,O2,SB1,SB0) F(I2,O1,O2,SB3,SB2)

#define H G(R1,R2,L1,L2) ; G(L1,L2,R1,R2)

	text
	global _ufc_doit
_ufc_doit:
 #
 # Preamble
 #
	link    %a6,&-56
	movem.l &15612,(%sp)

	mov.l   20(%a6),R2 
	mov.l   16(%a6),R1 
	mov.l   12(%a6),L2 
	mov.l    8(%a6),L1
 #
 # Setup address registers with sb pointers
 #
	mov.l	&_ufc_sb0,SB0 ; mov.l &_ufc_sb1,SB1 
	mov.l   &_ufc_sb2,SB2 ; mov.l &_ufc_sb3,SB3
 #
 # And loop...
 #
Lagain:	
	mov.l	&_ufc_keytab,KPTR
	moveq	&8,I
Lagain1:
	H
	subq.l	&1,I
	bne	Lagain1
 # Permute
	mov.l L1,SCR1 ; mov.l R1,L1 ; mov.l SCR1,R1
	mov.l L2,SCR1 ; mov.l R2,L2 ; mov.l SCR1,R2
 #
	subq.l	&1,ITR
	tst.l	ITR
	bne	Lagain
 #
 # Output conversion
 #
	mov.l R2,-(%sp) ; mov.l R1,-(%sp) 
	mov.l L2,-(%sp) ; mov.l L1,-(%sp)
	jsr _ufc_dofinalperm ; add.l &16,%sp
 #
 # Postamble
 #
	movem.l	(-56)(%a6),&15612 ; unlk %a6 ; rts



