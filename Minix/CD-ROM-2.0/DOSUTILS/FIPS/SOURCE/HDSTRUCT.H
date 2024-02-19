/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module hdstruct.h

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/hdstruct.h 1.4 1995/01/19 00:01:26 schaefer Exp schaefer $

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

#ifndef HDSTRUCT_H
#define HDSTRUCT_H

#include "types.h"
#include "disk_io.h"


/* ----------------------------------------------------------------------- */
/* Class root_sector - derived from structure sector                        */
/* Must be initialized with a pointer to a physical_drive object           */
/* Read() and Write() read/write sector 0 of physical drive                */
/* ----------------------------------------------------------------------- */

class root_sector:public sector
{
	physical_drive *drive;
public:
// constructors and operators

	root_sector (physical_drive *drive) { root_sector::drive = drive; }
	root_sector (root_sector &rs);
	void operator= (root_sector &rs);


// functions

	int read (void) { return drive->read_sector (this, 0); }
	int write (void) { return drive->write_sector (this, 0); }
};


/* ----------------------------------------------------------------------- */
/* Partition Info Structure                                                */
/* Each entry in the partition table contains this information             */
/* ----------------------------------------------------------------------- */

struct partition_info
{
	byte bootable;                  // 80h or 0
	byte start_head;                // location of first sector (boot_sector)
	word start_cylinder;
	byte start_sector;
	byte system;			// 1 = 12-bit FAT
					// 4 = 16-bit FAT & 16-bit sector number
					// 6 = 16-bit FAT & 32-bit sector number (BIGDOS)
	byte end_head;                  // location of last sector
	word end_cylinder;
	byte end_sector;
	dword start_sector_abs;         // = start_cylinder * heads * sectors
					// + start_head * sectors + start_sector - 1
	dword no_of_sectors_abs;        // = end_cylinder * heads * sectors + end_head * sectors
					// + end_sector - start_sector_abs
};


/* ----------------------------------------------------------------------- */
/* Partition Table Structure                                               */
/* The partition table consists of 4 entries for the 4 possible partitions */
/* Get() reads the partition table from the root_sector, put() writes the   */
/* data back into the root_sector buffer                                    */
/* ----------------------------------------------------------------------- */

struct partition_table
{
	partition_info partition_info[4];
	void get (root_sector *root_sector);
	void put (root_sector *root_sector);
};


/* ----------------------------------------------------------------------- */
/* Harddrive Class, derived from physical_drive                            */
/* Represents one physical harddrive. Must be initialized with the drive   */
/* number (0x80 for 1st HDD). Contains the root_sector and partition table. */
/* ----------------------------------------------------------------------- */

class harddrive:public physical_drive
{
	partition_table pr_partition_table;

public:
// constructors, destructors, operators

	harddrive (int number):physical_drive (number)
	{
		root_sector = new class root_sector (this);
	}
	harddrive (harddrive &hd):physical_drive (hd)
	{
		root_sector = new class root_sector (*(hd.root_sector));
		partition_table () = hd.partition_table ();
	}
	void operator= (harddrive &hd);
	~harddrive (void) { delete root_sector; }

// public data

	root_sector *root_sector;

// member access functions

	virtual partition_table &partition_table() { return pr_partition_table; }

// functions

	int read_root_sector (void) { return (root_sector->read ()); }
	int write_root_sector (void) { return (root_sector->write ()); }

	void get_partition_table (void);	// extract pt data from root sector
	void put_partition_table (void)		// put pt data into root sector
	{partition_table().put (root_sector);}
};

#endif
