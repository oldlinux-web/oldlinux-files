/*
    FIPS - the First nondestructive Interactive Partition Splitting program
    
    Module types.h

    RCS - Header:
    $Header: c:/daten/c/fips/source/RCS/types.h%v 2.0 1993/04/28 03:32:20 schaefer Exp schaefer $

    Copyright (C) 1993 Arno Schaefer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

typedef unsigned char byte;
typedef unsigned int word;
typedef unsigned long dword;

typedef enum {false,true} boolean;

struct drive_geometry
{
	dword heads;
	dword cylinders;
	dword sectors;
};

struct physical_sector
{
	dword head;
	dword cylinder;
	dword sector;
};

struct partition_info
{
	byte bootable;                  /* 80h or 0 */
	byte start_head;                /* location of first sector (bootsector) */
	word start_cylinder;
	byte start_sector;
	byte system;                    /* 1 = 12-bit FAT, 4 = 16-bit FAT & 16-bit sector number */
					/* 6 = 16-bit FAT & 32-bit sector number (BIGDOS) */
	byte end_head;                  /* location of last sector */
	word end_cylinder;
	byte end_sector;
	dword start_sector_abs;         /* = start_cylinder * heads * sectors + start_head * sectors */
					/* + start_sector - 1 */
	dword no_of_sectors_abs;        /* = end_cylinder * heads * sectors + end_head * sectors */
					/* + end_sector - start_sector_abs */
};

struct bootsector_info
{
	word bytes_per_sector;          /* usually 512 */
	byte sectors_per_cluster;       /* may differ */
	word reserved_sectors;          /* usually 1 (bootsector) */
	byte no_of_fats;                /* usually 2 */
	word no_of_rootdir_entries;     /* usually 512 (?) */
	word no_of_sectors;             /* 0 on BIGDOS partitions */
	byte media_descriptor;          /* usually F8h */
	word sectors_per_fat;           /* depends on partition size */
	word sectors_per_track;         /* = sectors */
	word drive_heads;               /* = heads */
	dword hidden_sectors;           /* first sector of partition */
	dword no_of_sectors_long;       /* number of sectors on BIGDOS partitions */
	byte phys_drive_no;             /* 80h or 81h */
	byte signature;                 /* usually 29h */
};
