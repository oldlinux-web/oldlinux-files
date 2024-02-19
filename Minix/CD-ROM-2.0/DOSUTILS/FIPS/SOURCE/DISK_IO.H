/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module disk_io.h

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/disk_io.h 1.4 1995/01/19 00:01:25 schaefer Exp schaefer $

	Copyright (C) 1993 Arno Schaefer

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


	Report problems and direct all questions to:

	schaefer@rbg.informatik.th-darmstadt.de
*/

#ifndef DISK_IO_H
#define DISK_IO_H

#include "types.h"


/* ----------------------------------------------------------------------- */
/* Structure to hold information about the drive geometry                  */
/* ----------------------------------------------------------------------- */

struct drive_geometry
{
	dword heads;
	dword cylinders;
	dword sectors;
};


/* ----------------------------------------------------------------------- */
/* Low level structure physical_drive, contains drive number and geometry, */
/* as well as low level sector read and write routines                     */
/* Geometry is determined on initialization, errorcode contains error      */
/* number after call to get_geometry() and reset().                        */
/* Initialization requires number (e.g. physical_drive drive1 (0x80);).    */
/*                                                                         */
/* Read and write are called max. 3 times in case of failure, return code  */
/* contains 0 if successful. Sector CRC is verified after write.           */
/* sector_number is absolute logical sector number (0 = master boot record)*/
/* ----------------------------------------------------------------------- */

class physical_drive
{
protected:
	virtual void get_geometry (void);

public:
// constructors & operators

	physical_drive (int number);
	physical_drive (physical_drive &pd);
	void operator= (physical_drive &pd);

// public data

	int number;
	int errorcode;
	drive_geometry geometry;

// functions

	virtual void reset (void);

	int read_sector (struct sector* sector, dword sector_number);
	int write_sector (struct sector* sector, dword sector_number);
};


/* ----------------------------------------------------------------------- */
/* Physical_sector_no holds and calculates physical sector number (Head,   */
/* Cylinder, Sector). Number is calculated on initialization. Log_sector   */
/* is absolute logical sector number (0 = master boot record). Usage:      */
/* physical_sector_no mbr (0,geometry);                                    */
/* physical_sector_no mbr (0,0,1);                                         */
/* ----------------------------------------------------------------------- */

struct physical_sector_no
{
// public data

	dword head;
	dword cylinder;
	dword sector;

// constructors

	physical_sector_no (dword logical_sector, const drive_geometry &geometry);
	physical_sector_no (dword head, dword cylinder, dword sector)
	{
		physical_sector_no::head = head;
		physical_sector_no::cylinder = cylinder;
		physical_sector_no::sector = sector;
	}
};


/* ----------------------------------------------------------------------- */
/* Structure sector - contains only sector data                            */
/* ----------------------------------------------------------------------- */

struct sector
{
	byte data[512];
};


/* ----------------------------------------------------------------------- */
/* Prototype for bios call get_disk_type - returns 0 if drive not present. */
/* Valid drive numbers: 0 - 255, result: 1 - floppy without disk change    */
/* detection, 2 - floppy with disk change detection, 3 - harddisk          */
/* ----------------------------------------------------------------------- */

int get_disk_type (int drive_number);

/* Bios call get_no_of_drives */

int get_no_of_drives (void);

#endif
