/* utmp2.c for Linux, by poe@daimi.aau.dk */
/* This code is freely distributable and is covered by the GNU copyleft */

/* This implementation of the utmp and wtmp routines, is supposed to 
   conform to the XPG2 and SVID2 standards */
   
#include "utmp.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

static char Utname[40] = {'/','e','t','c','/','u','t','m','p',0};
static int Fd = -1;
static struct utmp Ut;

/* kludge for the benefit of getutent() whose semantics differs from the
   other get..() functions. When set, getutent() will not read the 
   next entry in utmp, but immediatly returns &Ut, and resets the
   flag. */
static int _getutent_flag = 0;

static int _setutent(void)
{
	if(Fd != -1) close(Fd);
	
	/* first try to get write permission, if that fails then
	   make do with read access */
	if((Fd = open(Utname, O_RDWR)) < 0) {
		if((Fd = open(Utname, O_RDONLY)) < 0) {
			perror("_setutent: Can't open utmp file");
			Fd = -1;
			return 0;
		}
	}
	if(read(Fd, (char *)&Ut, sizeof(Ut)) != sizeof(Ut))
		return 0;

	return 1;
}

void setutent(void)
{
	(void)_setutent();
	_getutent_flag = 1;
}

void utmpname(const char *name)
{
	(void)strncpy(Utname, name, sizeof(Utname)-1);
	if(Fd != -1) close(Fd);
	Fd = -1;
}

/* always get the logical *next* entry from the utmp file */
struct utmp *getutent(void)
{
	if(Fd == -1) {
		if(!_setutent())
			return (struct utmp *)0;
		else {
			_getutent_flag = 0;
			return &Ut;
		}
	}

	if(_getutent_flag) {
		/* well, setutent() really got it for us */
		_getutent_flag = 0;
		return &Ut;
	}

	if(read(Fd, (char *)&Ut, sizeof(Ut)) != sizeof(Ut))
		return (struct utmp *)0;
	else
		return &Ut;
}

struct utmp *getutid(struct utmp *ut)
{
	if(Fd == -1)
		if(!_setutent()) return (struct utmp *)0;
	
	do {
		if(RUN_LVL <= ut->ut_type && ut->ut_type <= OLD_TIME &&
		   Ut.ut_type == ut->ut_type) return &Ut;

		if(INIT_PROCESS <= ut->ut_type && ut->ut_type <= DEAD_PROCESS &&
		   !strncmp(Ut.ut_id, ut->ut_id, sizeof(Ut.ut_id)))
			return &Ut;
		
	} while(read(Fd, (char *)&Ut, sizeof(Ut)) == sizeof(Ut));

	return (struct utmp *)0;
}

struct utmp *getutline(struct utmp *ut)
{
	if(Fd == -1)
		if(!_setutent()) return (struct utmp *)0;
	
	do {
		if((Ut.ut_type == USER_PROCESS || Ut.ut_type == LOGIN_PROCESS)
		   && !strncmp(Ut.ut_line, ut->ut_line, sizeof(Ut.ut_line)))
		return &Ut;
	} while(read(Fd, (char *)&Ut, sizeof(Ut)) == sizeof(Ut));

	return (struct utmp *)0;
}

struct utmp *_pututline(struct utmp *ut)
{
	struct utmp my_ut;
	
	/* don't stomp on the new value even if it's in Ut */
	if(ut == &Ut) {
		(void)memcpy(&my_ut, &Ut, sizeof(Ut));
		ut = &my_ut;
	}

	/* we don't check Fd here, getutid() does it for us; this way
	   we can create an utmp file from an empty file, if it should
	   get lost. */
	if(getutid(ut)) {
		(void)lseek(Fd, (off_t)-sizeof(Ut), SEEK_CUR);
		if(write(Fd, (char *)ut, sizeof(Ut)) != sizeof(Ut))
			return (struct utmp *)0;
	} else {
		(void)lseek(Fd, (off_t)0, SEEK_END);
		if(write(Fd, (char *)ut, sizeof(Ut)) != sizeof(Ut))
			return (struct utmp *)0;
	}
	
	/* if we get this far, we are successful */
	if(ut == &my_ut)
		(void)memcpy(&Ut, &my_ut, sizeof(Ut));

	return &Ut;
}

void pututline(struct utmp *ut)
{
	(void)_pututline(ut);
}

void endutent(void)
{
	if(Fd != -1) close(Fd);
	Fd = -1;
}
