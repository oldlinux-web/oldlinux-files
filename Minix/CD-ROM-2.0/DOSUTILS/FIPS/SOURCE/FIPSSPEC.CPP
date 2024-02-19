/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module fipsspec.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/fipsspec.cpp 1.4 1995/01/19 00:00:53 schaefer Exp schaefer $

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

#include <dos.h>

#include "fipsspec.h"
#include "global.h"
#include "input.h"

#define FIRST_CHECK false
#define FINAL_CHECK true

#define DISK_INT 0x13

#define RESET_DISK 0
#define GET_DRIVE_PARAMS 8

void fips_bpb::print (void)
{
	printx ("Bytes per sector: %u\n",bytes_per_sector);
	printx ("Sectors per cluster: %u\n",sectors_per_cluster);
	printx ("Reserved sectors: %u\n",reserved_sectors);
	printx ("Number of FATs: %u\n",no_of_fats);
	printx ("Number of rootdirectory entries: %u\n",no_of_rootdir_entries);
	printx ("Number of sectors (short): %u\n",no_of_sectors);
	printx ("Media descriptor byte: %02Xh\n",media_descriptor);
	printx ("Sectors per FAT: %u\n",sectors_per_fat);
	printx ("Sectors per track: %u\n",sectors_per_track);
	printx ("Drive heads: %u\n",drive_heads);
	printx ("Hidden sectors: %lu\n",hidden_sectors);
	printx ("Number of sectors (long): %lu\n",no_of_sectors_long);
	printx ("Physical drive number: %02Xh\n",phys_drive_no);
	printx ("Signature: %02Xh\n\n",signature);
}

void fips_partition_table::print (void)
{
	printx ("     |        |     Start      |      |      End       | Start  |Number of|\n");
	printx ("Part.|bootable|Head Cyl. Sector|System|Head Cyl. Sector| Sector |Sectors  |  MB\n");
	printx ("-----+--------+----------------+------+----------------+--------+---------+----\n");
	for (int i=0;i<4;i++)
	{
		printx ("%u    |    %s |%4u %4u   %4u|   %02Xh|%4u %4u   %4u|%8lu| %8lu|%4lu\n",i+1,
		partition_info[i].bootable ? "yes" : " no",
		partition_info[i].start_head,partition_info[i].start_cylinder,partition_info[i].start_sector,
		partition_info[i].system,partition_info[i].end_head,partition_info[i].end_cylinder,partition_info[i].end_sector,
		partition_info[i].start_sector_abs,partition_info[i].no_of_sectors_abs,partition_info[i].no_of_sectors_abs / 2048);
	}
}

void fips_harddrive::get_geometry (void)
{
	union REGS regs;

	regs.h.ah = GET_DRIVE_PARAMS;
	regs.h.dl = number;
	int86 (DISK_INT,&regs,&regs);
	if (global.debug_mode)
	{
		fprintf (global.debugfile,"\nRegisters after call to int 13h 08h (drive %02Xh):\n\n",number);
		fprintf (global.debugfile,"   00       sc/cl    hd\n");
		fprintf (global.debugfile,"al ah bl bh cl ch dl dh   si    di    cflgs flags\n");
		hexwrite ((byte *) &regs,16,global.debugfile);
	}
	if ((errorcode = regs.h.ah) != 0) return;
	geometry.heads = (dword) regs.h.dh + 1;
	geometry.sectors = (dword) regs.h.cl & 0x3f;
	geometry.cylinders = ((dword) regs.h.ch | (((dword) regs.h.cl << 2) & 0x300)) + 1;

	if (global.debug_mode)
	{
		fprintf (global.debugfile, "\nGeometry reported by BIOS:\n");
		fprintf
		(
			global.debugfile,
			"%ld cylinders, %ld heads, %ld sectors\n",
			geometry.cylinders,
			geometry.heads,
			geometry.sectors
		);
	}
}

void fips_harddrive::reset (void)
{
	union REGS regs;

	regs.h.ah = RESET_DISK;
	regs.h.dl = number;
	int86 (DISK_INT,&regs,&regs);
	if (global.debug_mode)
	{
		fprintf (global.debugfile,"\nRegisters after call to int 13h 00h (drive %02Xh):\n\n",number);
		fprintf (global.debugfile,"al ah bl bh cl ch dl dh   si    di    cflgs flags\n");
		hexwrite ((byte *) &regs,16,global.debugfile);
	}
	errorcode = regs.h.ah;
}

void fips_logdrive_info::put_debug_info (void)
{
	fprintf (global.debugfile,"Calculated Partition Characteristica:\n\n");
	fprintf (global.debugfile,"Start of FAT 1: %lu\n",start_fat1);
	fprintf (global.debugfile,"Start of FAT 2: %lu\n",start_fat2);
	fprintf (global.debugfile,"Start of Rootdirectory: %lu\n",start_rootdir);
	fprintf (global.debugfile,"Start of Data: %lu\n",start_data);
	fprintf (global.debugfile,"Number of Clusters: %lu\n",no_of_clusters);
}

dword fips_partition::min_cylinder (fat16 fat, drive_geometry geometry)
{
	dword new_part_min_sector =
		info().start_data
		+ (dword) 4085
		* bpb().sectors_per_cluster;

	dword new_part_min_cylinder =
		(
			new_part_min_sector
			+ partition_info->start_sector_abs
			- 1
		)
		/ (geometry.heads * geometry.sectors)
		+ 1;

	if (new_part_min_cylinder > partition_info->end_cylinder)
		error ("Partition too small - can't split");

	dword min_free_cluster = fat.min_free_cluster ();
	dword min_free_sector =
		info().start_data
		+ (min_free_cluster - 2)
		* (dword) bpb().sectors_per_cluster;

	dword min_free_cylinder =
		(
			min_free_sector
			+ partition_info->start_sector_abs
			- 1
		)
		/ (geometry.heads * geometry.sectors)
		+ 1;

	if (min_free_cylinder > partition_info->end_cylinder)
		error ("Last cylinder is not free");

	if (new_part_min_cylinder < min_free_cylinder)
		new_part_min_cylinder = min_free_cylinder;

	return (new_part_min_cylinder);
}


boolean fips_partition::split (fips_harddrive hd)
{
	if (read_boot_sector ())
		error ("Error reading boot sector");

	if (global.debug_mode)
	{
		fprintf
		(
			global.debugfile,
			"\nBoot sector drive %02Xh, partition %u:\n\n",
			hd.number,
			number + 1
		);

		hexwrite
		(
			boot_sector->data,
			512,
			global.debugfile
		);
	}

	get_bpb ();

	printx ("\nBoot sector:\n\n");
	print_bpb ();

	get_info ();
	if (global.debug_mode)
		write_info_debugfile ();

	check ();

	fat16 fat1 (this,1);
	fat16 fat2 (this,2);

	fat1.check_against (&fat2);

	dword new_part_min_cylinder =
		min_cylinder (fat2, hd.geometry);

	if (ask_if_save()) save_root_and_boot(&hd,this);

	dword new_start_cylinder =
		ask_for_new_start_cylinder
		(
			partition_info->start_cylinder,
			new_part_min_cylinder,
			partition_info->end_cylinder,
			hd.geometry.heads * hd.geometry.sectors
		);

	fat2.check_empty
	(
		new_start_cylinder
			* hd.geometry.heads
			* hd.geometry.sectors
		- partition_info->start_sector_abs
	);

	hd.calculate_new_root (new_start_cylinder, this);

	hd.put_partition_table();
	hd.get_partition_table();

	printx ("\nNew partition table:\n\n");
	hd.print_partition_table ();

	hd.check (FINAL_CHECK);

	if (ask_if_continue () == false)
	{
		return (false);
	}

	calculate_new_boot ();

	put_bpb ();
	get_bpb ();

	printx ("\nNew boot sector:\n\n");
	print_bpb ();

	get_info ();
	if (global.debug_mode)
		write_info_debugfile ();

	check();

	if (!global.test_mode)
	{
		ask_for_write_permission ();

		if (hd.write_root_sector ())
			error ("Error writing root sector");

		if (write_boot_sector ())
			error ("Error writing boot sector");

		printx ("Repartitioning complete\n");
	}

	return (true);
}
