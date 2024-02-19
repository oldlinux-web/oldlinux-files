/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module logdr_st.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/logdr_str.cpp 1.4 1995/01/19 00:00:54 schaefer Exp schaefer $

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

#include <string.h>
#include "types.h"
#include "logdr_st.h"

/* ----------------------------------------------------------------------- */
/* Extract Bios Parameter Block from boot sector                           */
/* ----------------------------------------------------------------------- */

void bios_parameter_block::get (boot_sector *boot_sector)
{
	byte *bp = boot_sector->data;

	memcpy (jump_instruction,bp,3);
	memcpy (oem_name,bp+3,8);
	oem_name[8]=0;
	bytes_per_sector = *(bp+0xb) | (*(bp+0xc) << 8);
	sectors_per_cluster = *(bp+0xd);
	reserved_sectors = *(bp+0xe) | (*(bp+0xf) << 8);
	no_of_fats = *(bp+0x10);
	no_of_rootdir_entries = *(bp+0x11) | (*(bp+0x12) << 8);
	no_of_sectors = *(bp+0x13) | (*(bp+0x14) << 8);
	media_descriptor = *(bp+0x15);
	sectors_per_fat = *(bp+0x16) | (*(bp+0x17) << 8);
	sectors_per_track = *(bp+0x18) | (*(bp+0x19) << 8);
	drive_heads = *(bp+0x1a) | (*(bp+0x1b) << 8);
	hidden_sectors = (dword) *(bp+0x1c) | ((dword) *(bp+0x1d) << 8) | ((dword) *(bp+0x1e) << 16) | ((dword) *(bp+0x1f) << 24);
	no_of_sectors_long = (dword) *(bp+0x20) | ((dword) *(bp+0x21) << 8) | ((dword) *(bp+0x22) << 16) | ((dword) *(bp+0x23) << 24);
	phys_drive_no = *(bp+0x24);
	signature = *(bp+0x26);
	serial_number = (dword) *(bp+0x27) | ((dword) *(bp+0x28) << 8) | ((dword) *(bp+0x29) << 16) | ((dword) *(bp+0x2a) << 24);
	memcpy (volume_label,bp+0x2b,11);
	volume_label[11] = 0;
	memcpy (file_system_id,bp+0x36,8);
	file_system_id[8] = 0;
}

/* ----------------------------------------------------------------------- */
/* Write Bios Parameter Block back into boot sector                        */
/* ----------------------------------------------------------------------- */

void bios_parameter_block::put (boot_sector *boot_sector)
{
	byte *bp = boot_sector->data;

	memcpy (bp,jump_instruction,3);
	memcpy (bp+3,oem_name,8);
	*(bp+0xb) = bytes_per_sector & 0xff;
	*(bp+0xc) = (bytes_per_sector >> 8) & 0xff;
	*(bp+0xd) = sectors_per_cluster;
	*(bp+0xe) = reserved_sectors & 0xff;
	*(bp+0xf) = (reserved_sectors >> 8) & 0xff;
	*(bp+0x10) = no_of_fats;
	*(bp+0x11) = no_of_rootdir_entries & 0xff;
	*(bp+0x12) = (no_of_rootdir_entries >> 8) & 0xff;
	*(bp+0x13) = no_of_sectors & 0xff;
	*(bp+0x14) = (no_of_sectors >> 8) & 0xff;
	*(bp+0x15) = media_descriptor;
	*(bp+0x16) = sectors_per_fat & 0xff;
	*(bp+0x17) = (sectors_per_fat >> 8) & 0xff;
	*(bp+0x18) = sectors_per_track & 0xff;
	*(bp+0x19) = (sectors_per_track >> 8) & 0xff;
	*(bp+0x1a) = drive_heads & 0xff;
	*(bp+0x1b) = (drive_heads >> 8) & 0xff;
	*(bp+0x1c) = hidden_sectors & 0xff;
	*(bp+0x1d) = (hidden_sectors >> 8) & 0xff;
	*(bp+0x1e) = (hidden_sectors >> 16) & 0xff;
	*(bp+0x1f) = (hidden_sectors >> 24) & 0xff;
	*(bp+0x20) = no_of_sectors_long & 0xff;
	*(bp+0x21) = (no_of_sectors_long >> 8) & 0xff;
	*(bp+0x22) = (no_of_sectors_long >> 16) & 0xff;
	*(bp+0x23) = (no_of_sectors_long >> 24) & 0xff;
	*(bp+0x24) = phys_drive_no;
	*(bp+0x26) = signature;
	*(bp+0x27) = serial_number & 0xff;
	*(bp+0x28) = (serial_number >> 8) & 0xff;
	*(bp+0x29) = (serial_number >> 16) & 0xff;
	*(bp+0x2a) = (serial_number >> 24) & 0xff;
	memcpy (bp+0x2b,volume_label,11);
	memcpy (bp+0x36,file_system_id,8);
}

/* ----------------------------------------------------------------------- */
/* Extract some misc. drive parameters from BPB                            */
/* ----------------------------------------------------------------------- */

void logical_drive_info::get (const bios_parameter_block &bpb)
{
	start_fat1 = bpb.reserved_sectors;
	start_fat2 = start_fat1 + bpb.sectors_per_fat;
	start_rootdir = start_fat2 + bpb.sectors_per_fat;
	if (bpb.no_of_rootdir_entries == 0) start_data = start_rootdir;
	else start_data = start_rootdir + (bpb.no_of_rootdir_entries - 1) / 16 + 1;
	if (bpb.sectors_per_cluster == 0) no_of_clusters = 0;
	else no_of_clusters = ((bpb.no_of_sectors ? bpb.no_of_sectors : bpb.no_of_sectors_long) - start_data) / bpb.sectors_per_cluster;
};

