/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module fat.h

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/fat.h 1.4 1995/01/19 00:01:25 schaefer Exp schaefer $

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

#ifndef FAT_H
#define FAT_H

#include "types.h"
#include "disk_io.h"
#include "logdr_st.h"

class fat
{
protected:
	logical_drive *logical_drive;
	int number;
	dword start_sector;
	sector *buffer;
	dword sector_in_buffer;
	void read_sector (dword sector);
public:
	virtual dword next_cluster (dword cluster_number) = 0;

	fat (class logical_drive *logical_drive,int number);
	~fat (void) { delete buffer; }
};

class fat16:public fat
{
public:
	dword next_cluster (dword cluster_number);
	void check_empty (dword new_start_cluster);
	void check_against (class fat16 *fat2);
	dword min_free_cluster (void);

	fat16 (class logical_drive *logical_drive,int number):fat (logical_drive,number) {}
};

#endif
