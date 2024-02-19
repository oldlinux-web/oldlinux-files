/*
 * Copyright (c) 1981 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if 0
#ifndef lint
static char sccsid[] = "@(#)curses.c	5.7 (Berkeley) 6/1/90";
#endif /* not lint */
#endif

/*
 * Define global variables
 *
 */
# include	"curses.h"

bool	_echoit		= TRUE,	/* set if stty indicates ECHO		*/
	_rawmode	= FALSE,/* set if stty indicates RAW mode	*/
	My_term		= FALSE,/* set if user specifies terminal type	*/
	_endwin		= FALSE;/* set if endwin has been called	*/

char	ttytype[50] = "",	/* long name of tty		*/

	*Def_term = "unknown";	/* default terminal type	*/

int	_tty_ch	= 0,	/* file channel which is a tty		*/
	LINES = 24,	/* number of lines allowed on screen	*/
	COLS = 80;	/* number of columns allowed on screen	*/

tcflag_t _res_iflg = 0;
tcflag_t _res_lflg = 0;

WINDOW	*stdscr		= NULL,
	*curscr		= NULL;

# ifdef DEBUG
FILE	*outf = 0;	/* debug output file		*/
# endif

SGTTY	_tty = {0, 0, 0, 0, 0, ""};	/* tty modes	*/

bool	AM = 0,
	BS = 0,
	CA = 0,
	DA = 0,
	DB = 0,
	EO = 0,
	HC = 0,
	HZ = 0,
	IN = 0,
	MI = 0,
	MS = 0,
	NC = 0,
	NS = 0,
	OS = 0,
	UL = 0,
	XB = 0,
	XN = 0,
	XT = 0,
	XS = 0,
	XX = 0;
char	*AL = 0,
	*BC = 0,
	*BT = 0,
	*CD = 0,
	*CE = 0,
	*CL = 0,
	*CM = 0,
	*CR = 0,
	*CS = 0,
	*DC = 0,
	*DL = 0,
	*DM = 0,
	*DO = 0,
	*ED = 0,
	*EI = 0,
	*K0 = 0,
	*K1 = 0,
	*K2 = 0,
	*K3 = 0,
	*K4 = 0,
	*K5 = 0,
	*K6 = 0,
	*K7 = 0,
	*K8 = 0,
	*K9 = 0,
	*HO = 0,
	*IC = 0,
	*IM = 0,
	*IP = 0,
	*KD = 0,
	*KE = 0,
	*KH = 0,
	*KL = 0,
	*KR = 0,
	*KS = 0,
	*KU = 0,
	*LL = 0,
	*MA = 0,
	*ND = 0,
	*NL = 0,
	*RC = 0,
	*SC = 0,
	*SE = 0,
	*SF = 0,
	*SO = 0,
	*SR = 0,
	*TA = 0,
	*TE = 0,
	*TI = 0,
	*UC = 0,
	*UE = 0,
	*UP = 0,
	*US = 0,
	*VB = 0,
	*VS = 0,
	*VE = 0,
	*AL_PARM = 0,
	*DL_PARM = 0,
	*UP_PARM = 0,
	*DOWN_PARM = 0,
	*LEFT_PARM = 0,
	*RIGHT_PARM = 0;
char	PC = 0;

/*
 * From the tty modes...
 */

bool	GT = 0, NONL = 0, UPPERCASE = 0, normtty = 0, _pfast = 0;
