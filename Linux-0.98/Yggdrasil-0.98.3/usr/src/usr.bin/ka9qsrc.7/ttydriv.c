/* TTY input driver */
#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "global.h"
#include "session.h"

extern unsigned char escape;	/* default escape character is ^] */

#define	TTY_LIT	0		/* Send next char literally */
#define	TTY_RAW	1
#define TTY_COOKED	2
/* TTY_ESC is escape char seen */
#define TTY_ESC 3

int ttymode = TTY_COOKED;
int ttyecho=1;
#define	TTY_NOECHO	0
#define	TTY_ECHO	1

#ifdef	FLOW
int ttyflow=1;
#endif

#define	LINESIZE	256

#define CTLR	18
#define	CTLU	21
#define	CTLV	22
#define	CTLW	23
#define	CTLZ	26
#define	RUBOUT	127

/*
 * First, some general explanation.
 * raw and cooked are used with the usual meaning, but they don't
 * involve any change in TTY modes.  Rather, the tty is set into
 * a raw mode, and cooked mode is implemented by doing our own
 * editing here.  The only TTY mode we normally toggle is flow control.
 *
 * This code also handles telnet special characters.  It could be
 * done in telnet, but it turns out to be easier to code it here.
 *
 * Note that raw mode is either TTY_RAW or TTY_ESC, and cooked
 * is either TTY_COOKED or TTY_LIT.  TTY_ESC and TTY_LIT are special
 * substates used when a special character has been seen.
 */

raw()
{
	/* 
	 * TTY_ESC is part of cooked mode, so if it's set, leave it.
	 */
	if (ttymode != TTY_ESC && ttymode != TTY_RAW)
		set_stdout(TTY_RAW);	/* CR/LF vs LF madness...  -- hyc */
	if (ttymode != TTY_ESC)
		ttymode = TTY_RAW;
}

cooked()
{
	/* 
	 * TTY_LIT is part of cooked mode, so if it's set, leave it.
	 */
	if (ttymode != TTY_LIT && ttymode != TTY_COOKED)
		set_stdout(TTY_COOKED);	/* CR/LF vs LF madness...  -- hyc */
	if (ttymode != TTY_LIT)
		ttymode = TTY_COOKED;
	set_stdout(TTY_COOKED);
}

void
echo()
{
	ttyecho = TTY_ECHO;
}

void
noecho()
{
	ttyecho = TTY_NOECHO;
}
 
/*
 * Accept characters from the incoming tty buffer and process them
 * (if in cooked mode) or just pass them directly (if in raw mode).
 * Returns the number of characters available for use; if non-zero,
 * also stashes a pointer to the character(s) in the "buf" argument.
 *
 * This routine is called whenever a character is input.
 * When in cooked mode, returns zero until we've got a whole line
 * or there's some other reason to want to activate.
 *
 * This routine performs the mode switch to conversational mode.
 * If this becomes a problem, perhaps we could return a negative
 * number or some other special code.
 */
 /*Control-R added by df for retype of lines - useful in Telnet */
 /*Then df got impatient and added Control-W for erasing words  */
 /* Control-V for the literal-next function, slightly improved
  * flow control, local echo stuff -- hyc */
int
ttydriv(c,buf)
unsigned char c;
unsigned char **buf;
{
	static unsigned char linebuf[LINESIZE];
	static unsigned char *cp = linebuf;
	unsigned char *rp ;
	int cnt;
	int seenprint;

	if(buf == (char **)NULL)
		return 0;	/* paranoia check */

	cnt = 0;
	switch(ttymode){
	
	/* TTY_LIT means we've seen a ^V in cooked mode. */
	case TTY_LIT:
		ttymode = TTY_COOKED;	/* Reset to cooked mode */
		*cp++ = c;
		if(cp >= &linebuf[LINESIZE]){
		      cnt = cp - linebuf;
		      cp = linebuf;
		}
		break;

	/* TTY_ESC means we've seen the escape character in raw mode. */
	/* Raw mode is only used by telnet, so we can safely generate IAC's */
	case TTY_ESC:
		ttymode = TTY_RAW;
		if (c == escape)
		    *cp++ = c;
		else switch(c & 0x1f) {
/*BRK*/		    case '\002':  *cp++ = 255; *cp++ = 243; break; 
/*IP*/		    case '\003':  *cp++ = 255; *cp++ = 244; break;
/*AO*/		    case '\017':  *cp++ = 255; *cp++ = 245; break;
/*AYT*/		    case '\024':  *cp++ = 255; *cp++ = 246; break;
/*EC*/		    case '\010':  *cp++ = 255; *cp++ = 247; break;
/*EL*/		    case '\025':  *cp++ = 255; *cp++ = 248; break;
		    case '\030':	printf("\r\n");	cmdmode(); break;
		    case '\037':
	       printf("\r\n");
	       printf("Type the escape character followed by\r\n");
	       printf("  escape character - send a real escape character\r\n");
	       printf("  x or ^x - return to command mode\r\n");
	       printf("  b or ^b - send break\r\n");
	       printf("  c or ^c - send interrupt process\r\n");
	       printf("  o or ^o - abort output\r\n");
	       printf("  t or ^t - are you there?\r\n");
	       printf("  h or ^h - send telnet erase character\r\n");
	       printf("  u or ^u - send telnet erase line\r\n");
	       printf("  ?       - print this help message\r\n");
		             break;
		}
		cnt = cp - linebuf;
		cp = linebuf;
		break;
	case TTY_RAW:
		if (c == escape) {
		    ttymode = TTY_ESC;
		    break;
		}
		/* 
		 * More telnet-specific stuff.  There's some debate
		 * what to do with CR.  In theory a telnet end of
		 * line is CR LF, so Cisco turns CR into CR LF.  But
		 * experience shows that with full duplex systems
		 * CR 0 (which means a real CR character) is safer.
		 * Some Unix systems turn CR LF into LF.  Presumably
		 * on systems where CR doesn't make sense we'll be
		 * in half-duplex.  Our half-duplex code does use CR LF
		 */
		switch(c) {
		case '\r':  /* CR must be sent as CR 0 or CR LF */
		    *cp++ = c;
		    c = '\0';
		    break;
		case 0xff: /* IAC must be doubled */
		    *cp++ = c;
		    break;
		}
		*cp++ = c;
		cnt = cp - linebuf;
		cp = linebuf;
		break;
	case TTY_COOKED:
		/* Perform cooked-mode line editing */
		if (mode == CONV_MODE) { /* should really check for telnet */
		  if (c == escape) {
		    printf("\r\n"); cmdmode(); cp = linebuf; goto endline;
		  } else switch(c & 0x7f) {
		    case CTLV: ttymode = TTY_LIT; goto nochar;
/*BRK*/		    case '\002':  *cp++ = 255; *cp++ = 243; goto endline;
/*IP*/		    case '\003':  *cp++ = 255; *cp++ = 244; goto endline;
/*AO*/		    case '\017':  *cp++ = 255; *cp++ = 245; goto endline;
/*AYT*/		    case '\024':  *cp++ = 255; *cp++ = 246; goto endline;
		  }
		}
#ifdef PC9801
		switch(c){
#else
		switch(c & 0x7f){
#endif
		case '\r':	/* CR and LF are equivalent */
		case '\n':
			*cp++ = '\r';
			*cp++ = '\n';
			printf("\n");
endline:
			cnt = cp - linebuf;
			cp = linebuf;
nochar:
			break;
		case RUBOUT:
		case '\b':		/* Backspace */
			if(cp != linebuf){
				cp--;
				if (ttyecho)
					printf("\b \b");
			}
			break;
		case CTLR:	/* print line buffer */
			if(ttyecho)
				printf("^R");
			printf("\n");
			if(ttyecho) {
				rp = linebuf ;
				while (rp < cp)
					putchar(*rp++) ;
			}
			break ;
		case CTLU:	/* Line kill */
			if(ttyecho) {
				while(cp != linebuf){
					cp--;
					printf("\b \b");
				}
			} else
				cp = linebuf;
			break;
		case CTLV:
			ttymode = TTY_LIT;
			break;
		case CTLW:	/* erase word */
			seenprint = 0 ;	/* we haven't seen a printable char yet */
			while (cp != linebuf) {
				cp--;
				if(ttyecho)
					printf("\b \b") ;
				if (isspace(*cp)) {
					if (seenprint)
						break ;
				}
				else
					seenprint = 1 ;
			}
			break ;
		default:	/* Ordinary character */
			*cp++ = c;
#ifndef	AMIGA
			/* ^Z apparently hangs the terminal emulators under
			 * DoubleDos and Desqview. I REALLY HATE having to patch
			 * around other people's bugslike this!!!
			 */
			if (ttyecho && (c != CTLZ))
				putchar(c);
#endif
			if(cp >= &linebuf[LINESIZE]){
				cnt = cp - linebuf;
				cp = linebuf;
			}
			break;
		}
	}
	if(cnt > 0)
		*buf = linebuf;
	else
		*buf = NULLCHAR;
	/*
	 * This isn't flow control in XON/XOFF sense.  Rather, it's
	 * a feature that holds output while the user is typing a
	 * line.  So if we're in the middle of a line, disable output
	 */
#ifdef	FLOW
	if(cp > linebuf)
		ttyflow = 0;
	else
		ttyflow = 1;
#endif
	fflush(stdout);
	return cnt;
}
