!
! UFC-crypt: ultra fast crypt(3) implementation
! Copyright (C) 1991, 1992, Michael Glad, email: glad@daimi.aau.dk
!
! This library is free software, you can redistribute it and/or
! modify it under the terms of the GNU Library General Public
! License as published by the Free Software Foundation, either
! version 2 of the License, or (at your option) any later version.
!
! This library is distributed in the hope that it will be useful,
! but WITHOUT ANY WARRANTY, without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
! Library General Public License for more details.
!
! You should have received a copy of the GNU Library General Public
! License along with this library, if not, write to the Free
! Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
!
! @(#)crypt.sparc.S	2.5 12/30/91
!
! Assembly code for SPARC architecture machines
!

#define I	%i3
#define ITR	%i4

#define L1	%o0
#define L2	%o1
#define R1	%o2
#define R2	%o3

#define KPTR	%g1
#define MASK	%g2
#define SCR1a	%g3
#define SCR1b	%g4
#define SCR1	%g5
#define SCR2    %g6
#define SCR3	%g7

#define SB0a	%l0
#define SB0b	%l1
#define SB1a	%l2
#define SB1b	%l3
#define SB2a	%l4
#define SB2b	%l5
#define SB3a	%l6
#define SB3b	%l7

#define ASSIGN(reg,value) sethi %hi(value),reg ; or %lo(value),reg,reg ;


#define F(I,O1,O2,SBX1,SBX2,SBY1,SBY2)				\
	ld 	[KPTR],SCR1 ;					\
	xor	SCR1,I,SCR1 ; 					\
 	and	SCR1,MASK,SCR1a ;				\
								\
	ld	[SBX1+SCR1a],SCR2 ;	inc	4,KPTR ;	\
 	srl	SCR1,16,SCR1b ;					\
	ld	[SBX2+SCR1a],SCR3 ;	xor	SCR2,O1,O1 ;	\
								\
	ld	[SBY1+SCR1b],SCR2 ;	xor 	SCR3,O2,O2 ;	\
	ld	[SBY2+SCR1b],SCR3 ;	xor 	SCR2,O1,O1 ;	\
								\
	xor 	SCR3,O2,O2 ;

#define G(I1,I2,O1,O2)			\
	F(I1,O1,O2,SB1a,SB1b,SB0a,SB0b) F(I2,O1,O2,SB3a,SB3b,SB2a,SB2b)

#define H G(R1,R2,L1,L2) ; G(L1,L2,R1,R2)

	.seg	"text"
	.global	_ufc_doit

_ufc_doit:
!
! Preamble
!
	save    %sp,-104,%sp
	mov %i0,L1 ;	mov %i1,L2
	mov %i2,R1 ;	mov %i3,R2
!
! Set up sb pointers
!
	ASSIGN(SB0a,_ufc_sb0) ; add SB0a,4,SB0b
	ASSIGN(SB1a,_ufc_sb1) ; add SB1a,4,SB1b 
	ASSIGN(SB2a,_ufc_sb2) ; add SB2a,4,SB2b
	ASSIGN(SB3a,_ufc_sb3) ; add SB3a,4,SB3b
!
	ASSIGN(MASK,0xffff)
!
! Loop
!
Lagain:
	ASSIGN(KPTR,_ufc_keytab)
	ASSIGN(I, 8)
Lagain1:
	H
	deccc	I
	bnz 	Lagain1
	nop
! Permute
	mov L1,SCR1 ; mov R1,L1 ; mov SCR1,R1
	mov L2,SCR1 ; mov R2,L2 ; mov SCR1,R2
!
	deccc 	ITR
	bnz 	Lagain
	nop
!
! Output conversion
!
	call	_ufc_dofinalperm,4
	nop
!
! Postamble
!
	ret ; restore %g0,%o0,%o0
