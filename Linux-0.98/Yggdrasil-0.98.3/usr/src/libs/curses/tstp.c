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

#ifndef lint
static char sccsid[] = "@(#)tstp.c	5.6 (Berkeley) 3/3/91";
#endif /* not lint */

# include	<signal.h>

# include	"curses.ext"

/*
 * handle stop and start signals
 *
 * @(#)tstp.c	5.6 (Berkeley) 3/3/91
 */
void
tstp() {

# ifdef SIGTSTP

	SGTTY	tty;
#ifdef POSIX
	sigset_t mask, omask;
#else
	int	omask;
#endif
# ifdef DEBUG
	if (outf)
		fflush(outf);
# endif
	tty = _tty;
	mvcur(0, COLS - 1, LINES - 1, 0);
	endwin();
	fflush(stdout);
	/* reset signal handler so kill below stops us */
	signal(SIGTSTP, SIG_DFL);
#ifdef POSIX
	sigemptyset(&mask);	
	sigaddset(&mask, SIGTSTP);
	sigprocmask(SIG_SETMASK, &mask, &omask);
	kill(0, SIGTSTP);
	sigprocmask(SIG_SETMASK, &omask, 0);
#else
#define	mask(s)	(1 << ((s)-1))
	omask = sigsetmask(sigblock(0) &~ mask(SIGTSTP));
	kill(0, SIGTSTP);
	sigblock(mask(SIGTSTP));
#endif
	signal(SIGTSTP, tstp);
	_tty = tty;
#ifdef POSIX
	tcsetattr(_tty_ch, TCSADRAIN, &_tty);
#else
	ioctl(_tty_ch, TIOCSETP, &_tty);
#endif
	wrefresh(curscr);
# endif	SIGTSTP
}
