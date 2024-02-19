|
| UFC-crypt: ultra fast crypt(3) implementation
| Copyright (C) 1991, 1992, Michael Glad, email: glad@daimi.aau.dk
|
| This library is free software, you can redistribute it and/or
| modify it under the terms of the GNU Library General Public
| License as published by the Free Software Foundation, either
| version 2 of the License, or (at your option) any later version.
|
| This library is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY, without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
| Library General Public License for more details.
|
| You should have received a copy of the GNU Library General Public
| License along with this library, if not, write to the Free
| Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
|
| @(#)crypt.sun3.S	2.8 01/11/92
|
| Assembly code for SUN3 68000 based workstations.
|

#define L1	d0
#define L2	d1
#define R1	d2
#define R2	d3
#define SCR1	d4
#define SCR2	d5

#define I       d7
#define ITR	a6@(24)

#define SB0	a0
#define SB1	a1
#define SB2	a2
#define SB3	a3
#define KPTR	a4
	
#define F(I,O1,O2,SBX,SBY)						\
	movl 	KPTR@+,SCR1 ;		eorl 	I,SCR1 ;		\
	movl	SBX@(0,SCR1:w),SCR2 ; 	eorl	SCR2,O1 ;		\
	movl	SBX@(4,SCR1:w),SCR2 ;	eorl	SCR2,O2 ;		\
	swap	SCR1 ;							\
	movl	SBY@(0,SCR1:w),SCR2 ;	eorl	SCR2,O1 ;		\
	movl	SBY@(4,SCR1:w),SCR2 ;	eorl	SCR2,O2 ;

#define G(I1,I2,O1,O2)			\
	F(I1,O1,O2,SB1,SB0) F(I2,O1,O2,SB3,SB2)

#define H G(R1,R2,L1,L2) ; G(L1,L2,R1,R2)

	.text
	.proc
	.globl _ufc_doit
_ufc_doit:
|
| Preamble
|
	link a6,#-56
	moveml #15612,sp@

	movl	a6@(20),R2
	movl	a6@(16),R1
	movl	a6@(12),L2
	movl	a6@(8),L1
|
| Setup address registers with sb pointers
|
	movl	#_ufc_sb0,SB0 ; movl #_ufc_sb1,SB1 
	movl    #_ufc_sb2,SB2 ; movl #_ufc_sb3,SB3
| 
| And loop...
|
Lagain:	
	movl  #_ufc_keytab,KPTR
	moveq #8,I
Lagain1:
	H
	subql #1,I
	tstl  I
	jne   Lagain1	
| Permute
	movl L1,SCR1 ; movl R1,L1 ; movl SCR1,R1
	movl L2,SCR1 ; movl R2,L2 ; movl SCR1,R2
|
	subql	#1,ITR
	jne	Lagain
|
| Output conversion
|
	movl R2,sp@- ; movl R1,sp@-
	movl L2,sp@- ; movl L1,sp@-
	jbsr _ufc_dofinalperm ; addl #16,sp
|
| Postamble
|
	moveml 	a6@(-56),#15612 ; unlk a6 ; rts
