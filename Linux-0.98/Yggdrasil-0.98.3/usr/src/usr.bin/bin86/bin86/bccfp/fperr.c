/*
 *   bin86/bccfp/fperr.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

#include <stdio.h>
#include <signal.h>

#include "fperr.h"

void fperr(errno)
int errno;
{

#if defined(DEBUG) || 0
	switch(errno) {

	case EFDENORMAL:
		fputs("\nDenormal - ", stderr);
		break;

	case EFINFINITY:
		fputs("\nInfinity - ", stderr);
		break;

	case EFNAN:
		fputs("\nNaN - ", stderr);
		break;

	case EFOVERFLOW:
		fputs("\nOverflow - ", stderr);
		break;

	case EFUNDERFLOW:
		fputs("\nUnderflow - ", stderr);
		break;

	case EFDIVZERO:
		fputs("\nZero divide - ", stderr);
		break;

	default:
		fprintf(stderr, "\nUnknown error 0x%x - ", errno);
	}
	fflush(stderr);
#endif

	kill(getpid(), SIGFPE);
}
