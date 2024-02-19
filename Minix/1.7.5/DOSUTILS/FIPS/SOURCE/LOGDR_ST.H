/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module logdr_st.h

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/logdr_st.h 1.4 1995/01/19 00:01:27 schaefer Exp $

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

#ifndef LOGDR_ST_H
#define LOGDR_ST_H

#include "types.h"
#include "disk_io.h"


/* ----------------------------------------------------------------------- */
/* Class boot_sector - derived from structure sector                       */
/* Must be initialized with pointer to logical drive object                */
/* Read() and write() read/write sector 0 of logical drive                 */
/* ----------------------------------------------------------------------- */

class boot_sector:public sector
{
	class logical_drive *logical_drive;
public:
// constructor

	boot_sector (class logical_drive *logical_drive)
	{ boot_sector::logical_drive = logical_drive; }


// functions

	int read (void);
	int write (void);
};


/* ------------------------------------------------------------------------ */
/* Bios Parameter Block structure                                           */
/* This is not exactly the BPB as understood by DOS, because it contains    */
/* the additional fields that are in the boot_sector like jump_instruction, */
/* oem_name etc. Get() extracts info from the boot_sector, put() writes the */
/* info back into the boot_sector buffer.                                   */
/* ------------------------------------------------------------------------ */

struct bios_parameter_block
{
	byte jump_instruction[3];	// EB xx 90 or E9 xx xx
	char oem_name[9];
	word bytes_per_sector;          // usually 512
	byte sectors_per_cluster;       // may differ
	word reserved_sectors;          // usually 1 (boot_sector)
	byte no_of_fats;                // usually 2
	word no_of_rootdir_entries;     // usually 512 for HDs (?), 224 for
					// HD-Floppies, 112 for DD-Floppies
	word no_of_sectors;             // 0 on BIGDOS partitions
	byte media_descriptor;          // usually F8h
	word sectors_per_fat;           // depends on partition size
	word sectors_per_track;         // depends on drive
	word drive_heads;               // dto.
	dword hidden_sectors;           // first sector of partition or 0 for FDs
	dword no_of_sectors_long;       // number of sectors on BIGDOS partitions
	byte phys_drive_no;             // usually 80h
	byte signature;                 // usually 29h
	dword serial_number;            // random
	char volume_label[12];
	char file_system_id[9];

	void get (boot_sector *boot_sector);
	void put (boot_sector *boot_sector);
};


/* ----------------------------------------------------------------------- */
/* Some miscellaneous figures about the drive                              */
/* Get() extracts this info from the BPB                                   */
/* ----------------------------------------------------------------------- */

struct logical_drive_info
{
	dword start_fat1;
	dword start_fat2;
	dword start_rootdir;
	dword start_data;
	dword no_of_clusters;

	virtual void get (const bios_parameter_block &bpb);
};


/* ----------------------------------------------------------------------- */
/* Abstract Class logical_drive. This can be any DOS drive that allows     */
/* direct reading and writing of sectors, like Harddisk Partitions, Floppy */
/* disks or Ramdisks                                                       */
/* ----------------------------------------------------------------------- */

class logical_drive
{
// private data

	struct bios_parameter_block pr_bpb;
	struct logical_drive_info pr_info;

public:

// public data

	class boot_sector *boot_sector;

// member access functions

	virtual bios_parameter_block &bpb() { return pr_bpb; }
	virtual logical_drive_info &info() { return pr_info; }

// functions

	virtual int read_sector (dword number,sector *sector) = 0;
	virtual int write_sector (dword number,sector *sector) = 0;
	// pure virtual functions

	int read_boot_sector (void) { return (boot_sector->read ()); }
	int write_boot_sector (void) { return (boot_sector->write ()); }

	void get_bpb (void) { bpb().get (boot_sector); }
	void put_bpb (void) { bpb().put (boot_sector); }

	void get_info (void) { info().get (bpb ()); }
};


/* ----------------------------------------------------------------------- */
/* Function to read boot_sector from logical drive                         */
/* It must be in the header file because it is inline                      */
/* ----------------------------------------------------------------------- */

inline int boot_sector::read (void)
{
	return logical_drive->read_sector (0,this);
}


/* ----------------------------------------------------------------------- */
/* Function to write boot_sector to logical drive                          */
/* ----------------------------------------------------------------------- */

inline int boot_sector::write (void)
{
	return logical_drive->write_sector (0,this);
}

#endif
