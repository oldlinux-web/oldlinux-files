/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module fipsspec.h

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/fipsspec.h 1.4 1995/01/19 00:01:26 schaefer Exp schaefer $

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

#ifndef FIPSSPEC_H
#define FIPSSPEC_H

#include "logdr_st.h"
#include "hdstruct.h"
#include "primpart.h"
#include "fat.h"
#include "disk_io.h"

class fips_bpb:public bios_parameter_block
{
public:
	void print (void);
	void calculate_new_boot (const partition_info &partition_info);
};

class fips_partition_table:public partition_table
{
public:
	void print (void);
	void calculate_new_root (dword new_start_cylinder,partition *partition,const drive_geometry &geometry);
	void correct_physical (const drive_geometry &geometry);
	int select (void);
};

class fips_harddrive:public harddrive
{
	fips_partition_table pr_partition_table;
protected:
	void get_geometry (void);
public:
	void reset (void);
	class partition_table &partition_table() { return pr_partition_table; }
	void print_partition_table (void) { pr_partition_table.print(); }
	void calculate_new_root (dword new_start_cylinder, partition *partition)
	{
		pr_partition_table.calculate_new_root (new_start_cylinder,partition,geometry);
	}
	void check (boolean final_check);

	fips_harddrive (int number)
		:harddrive (number)
	{
		get_geometry ();
		// to write register info to debugfile
	}

	fips_harddrive (fips_harddrive &hd):harddrive (hd)
	{
		harddrive::operator= (hd);
		// in constructor of base class virtual functions are not yet
		// accessible => assign again so that partition_table() is
		// copied correctly
	}

	void operator= (fips_harddrive &hd)
	{
		harddrive::operator= (hd);
	}
};

class fips_logdrive_info:public logical_drive_info
{
public:
	void put_debug_info (void);
};

class fips_partition:public partition
{
	fips_bpb pr_bpb;
	fips_logdrive_info pr_info;
public:
	bios_parameter_block &bpb() { return pr_bpb; }
	logical_drive_info &info() { return pr_info; }

	void print_bpb (void) { pr_bpb.print(); }
	void write_info_debugfile (void) { pr_info.put_debug_info(); }
	void calculate_new_boot (void)
	{
		pr_bpb.calculate_new_boot (*partition_info);
	}
	void check (void);

	fips_partition (class fips_harddrive *drive,int number):partition(drive,number) {}

	dword min_cylinder (fat16 fat, drive_geometry geometry);
	boolean split (fips_harddrive hd);
};

#endif
