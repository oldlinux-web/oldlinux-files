// host_os.h	host operating system classes
//		dave mccaldon (d.mccalden@staffordshire.ac.uk)

#ifndef HOST_OS_H
#define HOST_OS_H

#include "types.h"

#define	NOT_OK		0		// NOT_OK for FIPS to run
#define	OK		1		// OK for FIPS to run
#define UNKNOWN		2


class host_os
{
public:
	host_os();                      // constructor
	virtual int     ok () { return status; };
	virtual char    *information( char * );

protected:
	char* (*format)();

	void msdos_version ();
	boolean mswindows_detect ();
	boolean dosemu_detect ();
	boolean desqview_detect ();

	int             status;         // status value
	int             ver_major;      // major version number
	int             ver_minor;      // minor version number
};

#endif
