/*
 * Suggested settings for Sun, NeXT and sgi (machines here at TAMU):
 */

#ifdef NeXT
#define NODIRENT
#define	PROTECT_ENV
#define PROTECT_JOB
#define NOCMDARG
#endif

#ifdef sgi
#define NOSIGCLD
#define	PROTECT_ENV
#endif

#ifdef sun
#define PROTECT_ENV
#endif

/*
 * Suggested settings for HP300 running 4.3BSD-utah (DWS):
 */

#if defined(hp300) && !defined(hpux)
#define NODIRENT
#define NOCMDARG
#define DEFAULTINTERP "/bin/sh"
#define PROTECT_ENV
#endif

/*
 * Suggested settings for Ultrix
 */

#ifdef ultrix
#define PROTECT_ENV
#endif
