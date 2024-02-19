/*
 * Config.h	- configuration for init and halt.
 */

/* Minix can't kill an entire process group, so init does not attempt
 * to do it either. If it did, it would kill ALL processes...
 * Including the calling process...
 *
 * linux does know how to kill pgrps - poe
 */
#define KILLBUG	    0			/* Can't kill(-pgrp, sig) */

/* If INIT opens /dev/console, it becomes it's controlling TTY.
 * And there is no way to reset that. All children will have the
 * console as controlling tty too. So INIT forks before doing
 * any terminal IO.
 */
#define PGRPBUG	    0			/* INIT gets a controlling tty */
